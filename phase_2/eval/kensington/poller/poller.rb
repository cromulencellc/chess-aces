#!/usr/bin/env ruby

require 'tempfile'
require 'timeout'
require 'net/ftp'

require_relative './utilities'
require_relative './models'

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

DATABASE_PATH = ENV['DATABASE_PATH'] || '/data/kensington.sqlite3'
unless File.exist? DATABASE_PATH
  Logger.FATAL "sqlite3 file #{DATABASE_PATH} doesn't exist, cannot work"
  exit 2
end
ActiveRecord::Base.establish_connection(
  adapter: "sqlite3",
  database: DATABASE_PATH
)

# the poller
class Poller
  attr_reader :seed, :length
  attr_reader :host, :port

  attr_reader :report, :assertions

  ACTIVITIES = %i{
    put_anon
    cant_get_anon

    move_anon_to_named
    get_named
    put_named

    bad_username
    bad_password
  }

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3038).to_i


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
    @scratch_dir_name = File.join('/tmp',
      "kensington-poller-#{Time.now.to_i}-#{rand(1_000)}")
    LLL.info "scratch directory #{@scratch_dir_name}"
    FileUtils.mkdir_p @scratch_dir_name
    Dir.chdir @scratch_dir_name

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

    unless ENV['NO_CLEANUP_ON_SUCCESS']
      LLL.info "NO_CLEANUP_ON_SUCCESS unset, deleting #{@scratch_dir_name}"
      Dir.chdir '/'
      FileUtils.rm_rf @scratch_dir_name
    end
  end

  private
  def put_anon
    dest_name = "/data/ftproot/_anonymous/#{mk_name()}"
    File.open(dest_name, 'w'){|f| f.write mk_name()}
      
    as_anon do |ftp|
      ftp.put dest_name
    end
  end

  def cant_get_anon
    fn = Dir.glob('/data/ftproot/_anonymous/*').sample
    bn = File.basename(fn)
    scratch_dest = File.join(@scratch_dir_name, bn)
    begin
      as_anon do |ftp|
        ftp.get bn, scratch_dest
      end
    rescue Net::FTPPermError => e
      assert true, 'desired error'
      return
    end

    assert false, 'Expected Net::FTPPermError'
  end

  def move_anon_to_named
    src_file = Dir.glob('/data/ftproot/_anonymous/*').sample
    if src_file.nil?
      LLL.error "can't move_anon_to_named without anon files"
      return
    end
    bn = File.basename(src_file)

    dest_dir = Dir.glob('/data/ftproot/*').sample

    while '/data/ftproot/_anonymous' == dest_dir
      dest_dir = Dir.glob('/data/ftproot/*').sample
    end

    dn = File.basename(dest_dir)

    FileUtils.mv(src_file, File.join(dest_dir, bn))

    as_user do |ftp, _user|
      ftp.rename "_anonymous/#{bn}", "#{dn}/#{bn}"
    end 
  end

  def get_named
    subdir = Dir.glob('/data/ftproot/*').sample
    fn = Dir.glob("#{subdir}/*").sample
    subdir = File.basename(File.dirname(fn))
    bn = File.basename(fn)
    content_expectation = File.read(fn)
    scratch_dest = File.join(@scratch_dir_name, bn)

    FileUtils.rm_f scratch_dest

    as_user do |ftp, _user|
      ftp.get File.join(subdir, bn), scratch_dest
    end

    got_content = File.read(scratch_dest)
    assert_equal content_expectation, got_content
  end

  def put_named
    dest_dir = Dir.glob('/data/ftproot/*').sample
    bn = mk_name()
    subdir = File.basename(dest_dir)
    fn = File.join(dest_dir, bn)
    File.open(fn, 'w'){|f| f.write mk_name()}

    as_user do |ftp, _user|
      ftp.put(fn, "#{subdir}/#{bn}")
    end
  end

  def bad_username
    non_username = mk_name()
    non_user = UserCheatsheet.find_by(name: non_username)

    until non_user.nil?
      non_username = mk_name()
      non_user = UserCheatsheet.find_by(name: non_username)
    end

    begin
      Net::FTP.open(host, port: port,
        username: non_username, password: mk_name()) do |ftp|
          ftp.noop
      end
    rescue Net::FTPPermError => e
      assert true, 'desired error'
      return
    end

    assert false, 'Expected Net::FTPPermError'
  end

  def bad_password
    user = UserCheatsheet.sample
    non_password = mk_name()

    while user.password == non_password
      non_password = mk_name()
    end

    begin
      Net::FTP.open(host, port: port,
        username: user.name, password: non_password) do |ftp|
          ftp.noop
      end
    rescue Net::FTPPermError => e
      assert true, 'desired error'
      return
    end

    assert false, 'Expected Net::FTPPermError'
  end

  def as_anon
    Net::FTP.open(host, port: port,
      username: 'anonymous', password: mk_name) do |ftp|
        ftp.binary = true
        ftp.passive = false
        yield ftp
    end
  end

  def as_user
    user = UserCheatsheet.sample
    Net::FTP.open(host, port: port,
      username: user.name, password: user.password) do |ftp|
        ftp.binary = true
        ftp.passive = false
        yield ftp, user
    end
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
  def mk_name
    "#{Time.now.to_i}-#{WORDS.sample}-#{WORDS.sample}-#{rand(1_000_000)}"
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
