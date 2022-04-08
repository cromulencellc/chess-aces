#!/usr/bin/env ruby

require 'tempfile'
require 'timeout'

require 'mechanize'
require 'nokogiri'
require 'equivalent-xml'

require_relative './utilities'

# range of lengths to pick in the absence of a `LENGTH` environment variable
LENGTH_RANGE = (100..200)

# seconds to wait for a given activity
DEFAULT_TIME_LIMIT = 5

# times to try some requests
PROCESS_ATTEMPTS = 5

# time between attempts
WAIT_BETWEEN_ATTEMPTS = PROCESS_ATTEMPTS / (2.0 + DEFAULT_TIME_LIMIT)

# for a less-noisy poller experience, set the `LOGGER_MIN_LEVEL` environment
# variable to `WARN`, `ERROR`, or `FATAL`
if ENV['LOGGER_MIN_LEVEL']
  LLL.level = Logger.const_get(ENV['LOGGER_MIN_LEVEL'])
end

# words used to build random strings
WORDS = %w{
           alpha bravo charlie delta echo foxtrot golf hotel india
           juliet kilo lima mike november oscar papa quebec romeo
           sierra tango uniform victor whiskey xray yankee zulu
           }

UUID_REGEXP = /([0-9a-f]{8}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{12})/

# the poller
class Poller
  attr_reader :seed, :length
  attr_reader :host, :port

  attr_reader :report, :assertions

  ACTIVITIES = %i{
    process_classic
    process_udmf
    process_malformed

    get_html
    get_svg
  }

  CLASSICS = Dir.glob(File.join(__dir__, 'classic', '*.wad'))
  UDMFS = Dir.glob(File.join(__dir__, 'classic', '*.wad'))
  MALFORMEDS = Dir.glob(File.join(__dir__, 'malformed', '*.wad'))

  ERROR_SVG = File.read(File.join(__dir__, 'lamartine-failed.svg'))

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3038).to_i

    @base_uri = URI "http://#{@host}:#{@port}/"

    @report = Hash[ACTIVITIES.map do |a|
                     [a, Timing.new]
                   end]

    @assertions = 0

    @maps = {}

    seed_rng!
    pick_length!
    pick_timeout!

    pick_focus!
  end

  # actually runs the poller
  def run!
    process_classic

    length.times do
      activity = ACTIVITIES.sample

      unless respond_to?(activity, true)
        LLL.error "activity #{activity} not implemented"
        next
      end

      LLL.info "trying #{activity}"
      before_time = Time.now.to_f
      Timeout.timeout(@timeout) do
        send activity
      end
      after_time = Time.now.to_f

      @report[activity].add(after_time - before_time)
    end
  end

  private
  def process_classic
    process(CLASSICS.sample)
  end

  def process_udmf
    process UDMFS.sample
  end

  def process_malformed
    process(
      MALFORMEDS.sample, 
      svg_expectation: ERROR_SVG, 
      save_mapping: false)
  end

  def process(wad_name, svg_expectation: nil, save_mapping: true)
    LLL.debug(wad_name)

    svg_expectation ||= File.read(wad_name.sub('.wad', '.svg'))

    with_agent do |a|
      index = a.get @base_uri
      form = index.form_with id: 'upload_map_form'

      form.file_upload('map').file_name = wad_name
      post_submit = form.submit

      assert_equal '303', post_submit.code

      map_page_uri = @base_uri.dup
      map_page_uri.path = post_submit.response['location']
      id = map_page_uri.path.match(UUID_REGEXP)[1]

      map_page = nil

      PROCESS_ATTEMPTS.times do 
        maybe_map = a.get map_page_uri
        if '200' == maybe_map.code 
          map_page = maybe_map
          break
        end
        sleep WAIT_BETWEEN_ATTEMPTS 
      end

      assert map_page, "Couldn't load map page"
      
      svg_chunk = map_page.body.match(%r{(\<svg.+<\/svg>)})[1]

      assert_equal svg_expectation, svg_chunk
      
      map_svg_uri = map_page_uri.dup
      map_svg_uri.path.gsub!('html', 'svg')
      svg_page = a.get map_svg_uri

      assert_equal Nokogiri::XML(svg_expectation), Nokogiri::XML(svg_page.body)

      @maps[id] = wad_name if save_mapping
    end
  end

  def get_html
    id = @maps.keys.sample
    wad_name = @maps[id]

    svg_expectation = File.read(wad_name.sub('.wad', '.svg'))

    with_agent do |a|
      index = a.get @base_uri

      map_page = index.link_with(href: "/maps/#{id}.html").click
      
      svg_chunk = map_page.body.match(%r{(\<svg.+<\/svg>)})[1]

      assert_equal svg_expectation, svg_chunk
    end
  end
  
  def get_svg
    id = @maps.keys.sample
    wad_name = @maps[id]

    svg_expectation = File.read(wad_name.sub('.wad', '.svg'))

    with_agent do |a|
      index = a.get @base_uri

      svg_page = index.link_with(href: "/maps/#{id}.svg").click
      
      assert_equal Nokogiri::XML(svg_expectation), Nokogiri::XML(svg_page.body)
    end
  end

  def with_agent
    Mechanize.start do |mech|
      begin
        mech.follow_redirect = false
        mech.agent.allowed_error_codes = [404, 425]
        yield mech
      rescue

        dump_filename = File.join(__dir__, 'tmp',
                                  "#{Time.now.to_i}-s#{@seed}")

        File.open(dump_filename, 'w'){|d| d.write mech.current_page.body}

        LLL.fatal dump_filename
        raise
      end
    end
  end

  XML_DUMP_OPTIONS = (
    Nokogiri::XML::Node::SaveOptions::DEFAULT_XML
  )

  # assert two values are equal
  def assert_equal(expected, actual)
    if expected.is_a?(Nokogiri::XML::Node) && actual.is_a?(Nokogiri::XML::Node)
      assert(EquivalentXml.equivalent?(expected, actual,
        element_order: false, normalize_whitespace: true),
        "XML nodes were not equal")
    elsif expected.is_a?(String) && actual.is_a?(String)
      ex_bin = expected.dup.force_encoding('BINARY')
      act_bin = actual.dup.force_encoding('BINARY')

      assert(ex_bin == act_bin,
              "strings were not equal")
    else
      assert(expected == actual, "expected `#{expected}`, got `#{actual}`")
    end
  end

  # assert a predicate is true
  def assert(predicate, message='failed assertion')
    @assertions += 1
    return if predicate
    fail message
  end

  def refute(predicate, message='failed refutation')
    @assertions += 1
    return unless predicate
    fail message
  end

  # get the seed from either the `SEED` environment variable or from urandom
  def seed_rng!
    @seed = (ENV['SEED'] || Random.new_seed).to_i
    $stdout.puts "SEED=#{@seed}"
    $stdout.flush
    srand(@seed)
  end

  # get the length (number of activities) from either `LENGTH` environment
  # variable or by sampling a length from the `LENGTH_RANGE` constant
  def pick_length!
    @length = (ENV['LENGTH'] || rand(LENGTH_RANGE)).to_i
    $stdout.puts "LENGTH=#{@length}"
    $stdout.flush
  end

  # pick a timeout (seconds to wait per activity) from either `TIMEOUT`
  # environment variable or the `DEFAULT_TIME_LIMIT` constant
  def pick_timeout!
    @timeout = (ENV['TIMEOUT'] || DEFAULT_TIME_LIMIT).to_i
    $stdout.puts "TIMEOUT=#{@timeout}"
    $stdout.flush
  end

  def pick_focus!
    return unless ENV['OOPS_ALL']

    focus = ENV['OOPS_ALL'].to_sym

    ACTIVITIES.keep_if{|e| e == focus}

    fail "Couldn't foucs on #{focus}" if ACTIVITIES.empty?
  end

  # make a random string with two random words and a random number
  def name
    "#{WORDS.sample} #{WORDS.sample} #{rand(1_000_000)} #{Time.now.to_i}"
  end
end

poller = Poller.new
at_exit do
  LLL.info((["SEED=#{poller.seed}",
            "LENGTH=#{poller.length}",
           "ruby", $0] + ARGV).join(' '))
end
poller.run!

$stderr.puts("%20s\t%s\t%s\t%s\t%s" % %w{name count min mean max})
total_count = 0
total_min = DEFAULT_TIME_LIMIT + 1
total_max = -1
total_mean = 0
poller.report.each do |r|
  timing = r[1]
  total_count += timing.count
  total_min = timing.min if timing.min < total_min
  total_max = timing.max if timing.max > total_max
  total_mean += (timing.count * timing.mean)
  $stderr.puts("%20s\t%d\t%.4f\t%.4f\t%.4f" % [
                 r[0],
                 timing.count, timing.min, timing.mean, timing.max
               ])
end
$stderr.puts("%20s\t%d\t%.4f\t%.4f\t%.4f" % [
               'TOTAL',
               total_count, total_min, total_mean/total_count, total_max
             ])

LLL.info "#{poller.assertions} total assertions"

LLL.info "success :)"
