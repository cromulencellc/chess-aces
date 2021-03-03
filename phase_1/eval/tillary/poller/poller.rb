#!/usr/bin/env ruby

require "forwardable"
require "logger"
require 'nokogiri'
require "http"
require "timeout"

# range of lengths to pick in the absence of a `LENGTH` environment variable
LENGTH_RANGE=(100..200)

# seconds to wait for a given activity
DEFAULT_TIME_LIMIT = 5

# number of HTTP connections to make
CLIENT_COUNT=1

# maximum number of tweets to check on the index
TWEETS_TO_CHECK=10

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
  fetch_static
  fetch_favicon
  post_tweet
  index_tweets
  get_tweet
  trace
  fail_trace
  fail_static
  }

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3020).to_i
    @http_listen_port = (ENV['HTTP_LISTEN_PORT'] || 8080).to_i

    @report = Hash[ACTIVITIES.map do |a|
                     [a, Timing.new]
                   end]

    @assertions = 0

    @tweets = {}

    seed_rng!
    pick_length!
    pick_timeout!
  end

  # actually runs the poller
  def run!
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

  private
  # creates and holds a pool of http connections
  def client
    return @clients.sample if defined? @clients

    @clients = CLIENT_COUNT.times.map do
      HTTP.persistent "http://#{@host}:#{@port}"
    end

    return @clients.sample
  end

  def temp_client
    return HTTP.persistent "http://#{@host}:#{port}"
  end

  def fetch_favicon
    want = File.read(__dir__ + "/static/favicon.ico")
    resp = client.get('/favicon.ico')
    assert_equal want, resp.body.to_s
  end

  def fetch_static
    filename = Dir.glob('static/*').sample
    want = File.read(__dir__ + '/' + filename)
    resp = client.get('/' + filename)
    assert_equal want, resp.body.to_s
  end

  def post_tweet
    content = name
    resp = client.post('/tweets/', body: content)
    assert_status 303, resp
    location = resp.headers["Location"]
    assert location

    @tweets[location] = content
  end

  def index_tweets
    resp = client.get('/tweets/')
    assert_status 200, resp

    return if @tweets.empty?

    sample = @tweets.keys.sample(TWEETS_TO_CHECK)
    doc = Nokogiri::HTML(resp.body.to_s)

    sample.each do |tweet|
      found = doc.css("a[href='#{tweet}']")
      refute found.empty?
      got = found.first
      assert got
      assert_equal @tweets[tweet], got.text
    end
  end

  def get_tweet
    return if @tweets.empty?

    loc = @tweets.keys.sample
    # p loc
    content = @tweets[loc]

    resp = client.get loc

    assert_status 200, resp

    body = resp.body.to_s
    # p body

    doc = Nokogiri::HTML(body)

    got = doc.css('p').first

    assert_equal content, got.text
  end

  def trace
    resp = client.trace '/tweets/'
    assert_status 200, resp
    assert_equal 'message/http', resp.headers['Content-Type']
    assert resp.headers['Using-Resource']
    resp.flush
  end

  def fail_trace
    c = temp_client
    resource = '/static/asdf'
    resp = c.trace resource
    assert_status 405, resp
    c.close
  end

  def fail_static
    c = temp_client
    resp = c.get('/static/aslkdjf.invalid')
    assert_status 404, resp
    c.close
  end

  # assert response has a given status, output first line if it doesn't
  def assert_status(expected, response)
    assert expected == response.code,
           "wanted code #{expected}, got #{response.code}: #{response.body.to_s.lines.first}"
  end

  # assert two values are equal
  def assert_equal(expected, actual)
    if expected.is_a?(String) && actual.is_a?(String)
      ex_bin = expected.force_encoding('BINARY')
      act_bin = actual.force_encoding('BINARY')

      assert(ex_bin == act_bin,
             "expected string `#{ex_bin}`, got `#{act_bin}`")
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
