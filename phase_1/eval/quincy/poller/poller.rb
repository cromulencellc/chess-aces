#!/usr/bin/env ruby

require "forwardable"
require "logger"
require 'nokogiri'
require "socket"
require "timeout"

# the web.rb file contains the Sinatra application that serves documents
# to the challenge (i.e. the web client)
require_relative 'web.rb'

# range of lengths to pick in the absence of a `LENGTH` environment variable
LENGTH_RANGE=(100..200)

# seconds to wait for a given activity
DEFAULT_TIME_LIMIT = 5

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end,
                 level: Logger::DEBUG)

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



# gathers statistics on how much time a given activity takes
class Timing
  attr_reader :count, :total, :min, :max

  def initialize
    @count = 0
    @total = 0.0
    @min = DEFAULT_TIME_LIMIT + 1
    @max = -1
  end

  def add(time)
    @count += 1
    @total += time
    @min = time if @min > time
    @max = time if @max < time
  end

  def mean
    @total / @count.to_f
  end
end

# allows fanning socket writes to a dump file for seeding a fuzzer
class Interceptor
  attr_accessor :sock, :dumper
  extend Forwardable

  def_delegators :@sock, :read, :getc

  def write(*args)
    dumper.write *args
    sock.write *args
  end
end

# the poller
class Poller
  attr_reader :seed, :length
  attr_reader :host, :port

  attr_reader :report, :assertions

  # activities that don't require data to have been loaded
  ACTIVITIES = %i{
  parse_url
  helloworld
  parse_doc
  }

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3017).to_i
    @http_listen_port = (ENV['HTTP_LISTEN_PORT'] || 8080).to_i

    @report = Hash[ACTIVITIES.map do |a|
                     [a, Timing.new]
                   end]

    @assertions = 0

    seed_rng!
    pick_length!
    pick_timeout!
    start_web_server!
  end

  # actually runs the poller
  def run!
    @sock = TCPSocket.new @host, @port

    # the challenge tells us what address we connect from to make
    # it easy for us to figure out how to compose HTTP URLs for it
    # to fetch
    @host_sees = @sock.gets
    @host_sees_addr = @host_sees.split(':').first
    LLL.info "Host sees us at #{@host_sees_addr}"

    LLL.info "also " + @sock.gets

    if example_filename = ENV['DUMP_EXAMPLE']
      interceptor = Interceptor.new
      interceptor.sock = @sock
      interceptor.dumper = File.open(example_filename, 'w')
      @sock = interceptor
      at_exit do
        interceptor.dumper.close
      end
    end

    length.times do
      prompt = @sock.read(2)
      LLL.info "prompt #{prompt}"

      universe = ACTIVITIES
      activity = universe.sample
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

  # requests the web server to stop, and waits a second for it to do so
  def cleanup!
    @web_thread.kill
    sleep 1
  end

  private
  # validates that a fetched document is returned successfully
  def helloworld
    @sock.puts "fetch http://#{@host_sees_addr}:#{@http_listen_port}/"
    got = @sock.gets.strip
  end

  # checks that URL parsing works as expected
  def parse_url
    scheme = WORDS.sample
    host = WORDS.sample
    port = ':' + rand(2**16).to_s
    target = '/' + WORDS.sample

    @sock.puts "parse-url #{scheme}://#{host}#{port}#{target}"
    got = @sock.gets.strip

    assert_equal("Url( scheme=`#{scheme}` host=`#{host}` port=`#{port}` target=`#{target}` )", got)
  end

  # 12345678-1234-5678-1234-567812345678
  PARSE_DOC_EXPECTATION = /\A(
  [[:xdigit:]]{8}-
  [[:xdigit:]]{4}-
  [[:xdigit:]]{4}-
  [[:xdigit:]]{4}-
  [[:xdigit:]]{12}
)\Z/x

  # does a surface-level check that a pretty-printed document is
  # similar in structure and content to the input document
  def parse_doc
    scramble_view_locals

    @sock.puts "parse-doc http://#{@host_sees_addr}:#{@http_listen_port}/"

    # grab a UUID that marks the end of the pretty-printed document
    expectation = @sock.gets.strip
    matched_expectation = PARSE_DOC_EXPECTATION.match expectation
    assert(matched_expectation,
           "didn't get bytes to inspect in `#{expectation}`")

    uuid = matched_expectation[1]
    got = []

    loop do
      candidate = @sock.gets.chomp
      LLL.debug candidate
      break if uuid == candidate
      got << candidate
    end

    LLL.info got

    doc = Nokogiri::HTML.parse(got.join)

    body = doc.css('body').first

    assert_equal(ViewLocals[:body_class],
                 body.attributes['class'].value)

    bd = body.attributes['data-'+ViewLocals[:body_data_key]]
    assert bd
    assert_equal bd.value, ViewLocals[:body_data_value]

    assert_equal(ViewLocals[:h1], doc.css('h1').first.text.strip)
    assert_equal(ViewLocals[:first_para], doc.css('p')[0].text.strip)
    assert_equal(ViewLocals[:second_para], doc.css('p')[1].text.strip)

    assert_equal(ViewLocals[:list_item_count], doc.css('li').length)
  end

  # assert two values are equal
  def assert_equal(expected, actual)
    assert(expected == actual, "expected `#{expected}`, got `#{actual}`")
  end

  # assert a predicate is true
  def assert(predicate, message='failed assertion')
    @assertions += 1
    return if predicate
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

  # start the Sinatra server defined in the `Web` in `web.rb` in
  # a separate thread in the poller
  def start_web_server!
    @web_thread = Thread.new do
      LLL.info "starting web server..."
      Web.run! port: @http_listen_port, bind: '0.0.0.0'
    end

    sleep 1
  end

  # pick new random view locals
  def scramble_view_locals
    ViewLocals[:body_class] = WORDS.sample

    ViewLocals[:body_data_key] = WORDS.sample
    ViewLocals[:body_data_value] = name

    ViewLocals[:h1] = name
    ViewLocals[:first_para] = name
    ViewLocals[:second_para] = name

    ViewLocals[:list_item_count] = rand(1..20)
  end

  # make a random string with two random words and a random number
  def name
    "#{WORDS.sample} #{WORDS.sample} #{rand(1_000_000)}"
  end
end

poller = Poller.new
at_exit do
  LLL.info((["SEED=#{poller.seed}",
            "LENGTH=#{poller.length}",
           "ruby", $0] + ARGV).join(' '))
end
poller.run!

poller.cleanup!

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
