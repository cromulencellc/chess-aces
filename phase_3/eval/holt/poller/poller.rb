#!/usr/bin/env ruby

require 'tempfile'
require 'timeout'

require 'http'
require 'net/http/digest_auth'
require 'nokogiri'

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
    get_top_index

    get_authed_index
    get_authed_file
    
    get_home_index
    get_home_file

    fail_get_password_file
    fail_get_noexist
    fail_get_wrong_password
    fail_get_non_user
  }

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

  def get_top_index
    resp = client.get '/'

    validate_index '/data/webroot', resp
  end

  def get_home_index
    directory = Dir.entries('/data/homes').
      reject{|n| %w{. ..}.include? n}.
      select{|n| File.directory?(File.join('/data/homes', n))}.
      sample

    resp = client.get "/~#{directory}/"

    validate_index "/data/homes/#{directory}/webroot", resp
  end

  def get_home_file
    directory = Dir.entries('/data/homes').
      reject{|n| %w{. ..}.include? n}.
      select{|n| File.directory?(File.join('/data/homes', n))}.
      sample

      
    file = Dir.entries(File.join('/data/homes', directory, 'webroot')).
      reject{|n| %w{. ..}.include? n}.
      select{|n| File.file?(File.join('/data/homes', directory, 'webroot', n))}.
      sample

    file_data = File.read(File.join('/data/homes', directory, 'webroot', file))

    resp = client.get "/~#{directory}/#{file}"

    assert_status 200, resp
    assert_equal file_data, resp.body.to_s
  end

  def get_authed_index
    directory = Dir.entries('/data/webroot').
      reject{|n| %w{. ..}.include? n}.
      select{|n| File.directory?(File.join('/data/webroot', n))}.
      sample

    pre_resp = client.get "/#{directory}/"
    assert_status 401, pre_resp

    _dir, username, password = File.read('/data/cheatsheet').
      lines.
      detect{|l| l.include? directory}.
      strip.
      split(':')

    is_digest = File.exist?("/data/webroot/#{directory}/.htdigest")
    auth_client = nil

    if is_digest
      uri = URI.parse "http://#{host}:#{@port}/#{directory}/"
      uri.user = username
      uri.password = password

      gizmo = Net::HTTP::DigestAuth.new
      header = gizmo.auth_header uri, pre_resp['www-authenticate'], 'GET'
      auth_client = client.auth(header)
    else
      auth_client = client.
        basic_auth(user: username, pass: password)
    end

    pre_resp.flush

    real_resp = auth_client.get("/#{directory}/")

    validate_index File.join('/data/webroot', directory), real_resp
  end

  
  def get_authed_file
    directory = Dir.entries('/data/webroot').
      reject{|n| %w{. ..}.include? n}.
      select{|n| File.directory?(File.join('/data/webroot', n))}.
      sample

    file = Dir.entries(File.join('/data/webroot', directory)).
      reject{|n| %w{. .. .htpasswd .htdigest}.include? n}.
      select{|n| File.file?(File.join('/data/webroot', directory, n))}.
      sample

    file_data = File.read(File.join('/data/webroot', directory, file))

    pre_resp = client.get "/#{directory}/#{file}"
    assert_status 401, pre_resp

    _dir, username, password = File.read('/data/cheatsheet').
      lines.
      detect{|l| l.include? directory}.
      strip.
      split(':')

    is_digest = File.exist?("/data/webroot/#{directory}/.htdigest")
    auth_client = nil

    if is_digest
      uri = URI.parse "http://#{host}:#{@port}/#{directory}/#{file}"
      uri.user = username
      uri.password = password

      gizmo = Net::HTTP::DigestAuth.new
      header = gizmo.auth_header uri, pre_resp['www-authenticate'], 'GET'

      auth_client = client.auth(header)
    else
      auth_client = client.
        basic_auth(user: username, pass: password)
    end

    pre_resp.flush

    real_resp = auth_client.get("/#{directory}/#{file}")

    assert_status 200, real_resp
    assert_equal file_data, real_resp.body.to_s
  end

  def fail_get_password_file
    directory = Dir.entries('/data/webroot').
      reject{|n| %w{. ..}.include? n}.
      select{|n| File.directory?(File.join('/data/webroot', n))}.
      sample

    file_path = Dir.glob(File.join('/data/webroot', directory, '.ht*')).
      sample

    file = File.basename(file_path)

    pre_resp = client.get "/#{directory}/#{file}"
    assert_status 401, pre_resp

    _dir, username, password = File.read('/data/cheatsheet').
      lines.
      detect{|l| l.include? directory}.
      strip.
      split(':')

    is_digest = File.exist?("/data/webroot/#{directory}/.htdigest")
    auth_client = nil

    if is_digest
      uri = URI.parse "http://#{host}:#{@port}/#{directory}/#{file}"
      uri.user = username
      uri.password = password

      gizmo = Net::HTTP::DigestAuth.new
      header = gizmo.auth_header uri, pre_resp['www-authenticate'], 'GET'
      auth_client = client.auth(header)
    else
      auth_client = client.
        basic_auth(user: username, pass: password)
    end

    pre_resp.flush

    real_resp = auth_client.get("/#{directory}/#{file}")

    assert_status 403, real_resp
  end

  def fail_get_noexist
    directory = Dir.entries('/data/homes').
      reject{|n| %w{. ..}.include? n}.
      select{|n| File.directory?(File.join('/data/homes', n))}.
      sample

    
    file = rand(26**10).to_s(26)
    while File.exist?(File.join('/data/webroot', directory, file))
      file = rand(26**10).to_s(26)
    end

    resp = client.get "/~#{directory}/#{file}"

    assert_status 404, resp
  end

  def fail_get_wrong_password
    directory = Dir.entries('/data/webroot').
      reject{|n| %w{. ..}.include? n}.
      select{|n| File.directory?(File.join('/data/webroot', n))}.
      sample

    file = Dir.entries(File.join('/data/webroot', directory)).
      reject{|n| %w{. .. .htaccess .htdigest}.include? n}.
      select{|n| File.file?(File.join('/data/webroot', directory, n))}.
      sample

    pre_resp = client.get "/#{directory}/#{file}"
    assert_status 401, pre_resp

    _dir, username, password = File.read('/data/cheatsheet').
      lines.
      detect{|l| l.include? directory}.
      strip.
      split(':')

    wrong_password = rand(26**10).to_s(26)
    while password == wrong_password
      wrong_password = rand(26**10).to_s(26)
    end

    is_digest = File.exist?("/data/webroot/#{directory}/.htdigest")
    auth_client = nil

    if is_digest
      uri = URI.parse "http://#{host}:#{@port}/#{directory}/#{file}"
      uri.user = username
      uri.password = wrong_password

      gizmo = Net::HTTP::DigestAuth.new
      header = gizmo.auth_header uri, pre_resp['www-authenticate'], 'GET'

      auth_client = client.auth(header)
    else
      auth_client = client.
        basic_auth(user: username, pass: wrong_password)
    end

    pre_resp.flush

    real_resp = auth_client.get("/#{directory}/#{file}")

    assert_status 401, real_resp
  end

  def fail_get_non_user
    directory = Dir.entries('/data/webroot').
      reject{|n| %w{. ..}.include? n}.
      select{|n| File.directory?(File.join('/data/webroot', n))}.
      sample

    file = Dir.entries(File.join('/data/webroot', directory)).
      reject{|n| %w{. .. .htaccess .htdigest}.include? n}.
      select{|n| File.file?(File.join('/data/webroot', directory, n))}.
      sample

    pre_resp = client.get "/#{directory}/#{file}"
    assert_status 401, pre_resp

    known_users = Set.new

    File.read('/data/cheatsheet').lines.each do |line|
      _dir, username, _password = line.strip.split(':')

      known_users << username
    end

    unknown_username = rand(26**10).to_s(26)
    while known_users.include? unknown_username
      unknown_username = rand(26**10).to_s(26)
    end
    
    wrong_password = rand(26**10).to_s(26)
    
    is_digest = File.exist?("/data/webroot/#{directory}/.htdigest")
    auth_client = nil

    if is_digest
      uri = URI.parse "http://#{host}:#{@port}/#{directory}/#{file}"
      uri.user = unknown_username
      uri.password = wrong_password

      gizmo = Net::HTTP::DigestAuth.new
      header = gizmo.auth_header uri, pre_resp['www-authenticate'], 'GET'

      auth_client = client.auth(header)
    else
      auth_client = client.
        basic_auth(user: unknown_username, pass: wrong_password)
    end

    pre_resp.flush

    real_resp = auth_client.get("/#{directory}/#{file}")

    assert_status 401, real_resp
  end

  def validate_index(local_path, resp)
    assert_status 200, resp
    doc = Nokogiri::HTML resp.body.to_s
    entries = doc.css '.list table tbody tr'
    entries.each do |etr|
      name = etr.css('.n a').first.text
      kind = etr.css('.t').first.text

      next if '../' == name

      assert File.exist?(File.join(local_path, name))
      if 'Directory' == kind
        assert File.directory?(File.join(local_path, name))
      elsif 'application/octet-stream'
        assert File.file?(File.join(local_path, name))
      end
    end
  end

  def client
    @client ||= HTTP.persistent("http://#{@host}:#{@port}/").
      headers(host: 'localhost')
  end

  # assert two values are equal
  def assert_equal(expected, actual)
    if expected.is_a?(String) && actual.is_a?(String)
      ex_bin = expected.dup.force_encoding('BINARY')
      act_bin = actual.dup.force_encoding('BINARY')

      assert(ex_bin == act_bin,
              "strings were not equal")
    else
      assert(expected == actual, "expected `#{expected}`, got `#{actual}`")
    end
  end

  def assert_status(expected, response)
    assert expected == response.code,
      "wanted #{expected}, got #{response.code}: #{response.body.to_s.lines.first}"
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
