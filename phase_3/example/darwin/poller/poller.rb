#!/usr/bin/env ruby
require 'tempfile'  ## dont think we need this
require 'timeout'   ## dont think we need this
require 'net/ftp'
require 'digest/md5'

require_relative './utilities'

##GLOBAL CONSTANTS
DEBUGGING = false           # debug statements
LENGTH_RANGE = (100..200)   # range of lengths if no 'LENGTH' env var
DEFAULT_TIME_LIMIT = 5      # seconds to wait for a given activity
PROCESS_ATTEMPTS = 5        # times to try some requests
WAIT_BETWEEN_ATTEMPTS = PROCESS_ATTEMPTS / (2.0 + DEFAULT_TIME_LIMIT) # time between attempts
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

##GLOBAL VARIABLES
$base_dir_ftp = '/'             # base dir on server
$base_dir_home = '/data/'  # base dir in poller
$current_level = 0
$activities
$login_count = 0

## THE POLLER
class Poller
  attr_reader :seed, :length
  attr_reader :host , :port
  attr_reader :report, :assertions

  ACTIVITIES = %i{
    bad_login
    good_login
    logout
    get_pwd
    get_list
    get_nlst
    change_dir
    go_home_dir
    make_directory
    remove_directory
    get_file
    put_file
    do_nothing
  } # delete_file
  ZERO_LVL_ACTIVITY = %i{
    bad_login
    good_login
  }
  ONE_LVL_ACTIVITY = %i{
    get_pwd
    get_list
    get_nlst
    make_directory
    remove_directory
    change_dir
    get_file
    put_file
    do_nothing
    logout
  }
  TWO_LVL_ACTIVITY =%i{
    go_home_dir
    logout
  }
  ALL_ACTIVITIES = [ZERO_LVL_ACTIVITY, ONE_LVL_ACTIVITY, TWO_LVL_ACTIVITY,]

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3040).to_i
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
      $activities = ALL_ACTIVITIES[$current_level]
      activity = $activities.sample
      
      if $login_count == 2 # prevent being locked out
        activity = ALL_ACTIVITIES[0][1] # login instead
      end
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
  # cuz c
  def printf msg 
    $stdout.puts msg
    $stdout.flush
  end

  ## LOGIN Functions
  def good_login
    $login_count = 0;
    directory = WORDS.
    select{|n| File.directory?(File.join($base_dir_home, n))}.
    sample
    _dir, username, password = File.read('/data/cheatsheet').
      lines.
      detect{|l| l.include? directory}.
      strip.
      split(':')
    begin
      resp = client.login(username, password)
    rescue Net::FTPPermError
      fail "Failed to login"
    end
    validate_resp_a 200, client.last_response_code.to_i
    $current_level = 1
    $base_dir_home += username
  end
  def bad_login
    directory = WORDS.
    select{|n| File.directory?(File.join($base_dir_home, n))}.
    sample
    _dir, username, password = File.read('/data/cheatsheet').
      lines.
      detect{|l| l.include? directory}.
      strip.
      split(':')
    begin
      client.login(username, 'wr0ngp@s$wORd')
    rescue Net::FTPPermError
      validate_resp_a 530, client.last_response_code.to_i
      $login_count +=1
      return
    end
    fail "Logged in with incorrect password"
  end
  def logout 
    resp = client.quit()
    validate_resp_a 221, client.last_response_code.to_i
    client.connect(@host,@port)
    $base_dir_home = '/data/'
    $current_level = 0
  end

  ### Possible Functions to perform ###
  # do_nothing, get_pwd, get_list, get_nlst, get_file, put_file, delete_file
  ##change_dir, go_home_dir, make_directory, remove_directory
  ## MISC
  def do_nothing
    client.noop
    validate_resp_a 200, client.last_response_code.to_i
  end
  ## LISTS
  def get_pwd
    resp = client.pwd
    if DEBUGGING 
      printf resp
    end
    validate_resp_a 257, client.last_response_code.to_i
  end
  def get_list(value = '*')
    resp = client.list(value)
    if DEBUGGING
      printf resp
    end
    validate_resp_a 200, client.last_response_code.to_i
    return resp
  end
  def get_nlst(value = '')
    resp = client.nlst(value)
    if DEBUGGING 
      printf resp
    end
    validate_resp_a 200, client.last_response_code.to_i
    return resp
  end
  ## FILES
  def get_file
    listing = get_list().select { |i| i[/^-/] }# get list of files
    file = ''
    if listing.count > 0
      file = listing.sample.split[8]
    else
      #printf "No files, can't get one"
      return
    end
    resp = client.getbinaryfile(file, file + 'd', 1024)
    validate_resp_a 226, client.last_response_code.to_i
  end
  def put_file
    file = Dir.entries($base_dir_home).
    reject{|n| %w{. ..}.include? n}.
    select{|n| File.file?(File.join($base_dir_home, n))}.
    sample
    client.putbinaryfile(File.join($base_dir_home, file))
    validate_resp_a 226, client.last_response_code.to_i
    get_nlst(file)
    validate_resp_a 200, client.last_response_code.to_i
  end
  def delete_file
    listing = get_list().select { |i| i[/^-/] }# get list of files
    file = ''
    if listing.count > 0
      file = listing.sample.split[8]
    else
      #printf "No files, can't delete"
      return
    end
    resp = client.delete(file)
    validate_resp_a 250, client.last_response_code.to_i
  end

  ## DIRECTORIES
  def change_dir
    listing = get_list().select { |i| i[/^d/] }
    folder = ''
    if listing.count > 0
      folder = listing.sample.split[8]
    else
      #printf "No directories, can't change dir"
      return
    end
    resp = client.chdir folder
    validate_resp_a 250, client.last_response_code.to_i
    $base_dir_home += "#{folder}/"
    $current_level = 2
  end
  def go_home_dir
    if $base_dir_home == '/data/'
      #printf "Already home"
      return
    end
    resp = client.chdir '/'
    validate_resp_a 250, client.last_response_code.to_i
    $base_dir_home = '/data/'
    $current_level = 1
  end
  def make_directory
    directory_name = make_name()
    resp = client.mkdir(directory_name)
    validate_resp_a 257, client.last_response_code.to_i
    listing = get_nlst()
    assert_status true, listing.include?(directory_name)
  end
  def remove_directory
    listing = get_list().select { |i| i[/^d/] }# get list of directories
    folder = ''
    if listing.count > 0
      folder = listing.sample.split[8]
    else
      #printf "No directories, can't delete"
      return
    end
    resp = client.rmdir(folder)
    validate_resp_a 250, client.last_response_code.to_i
  end




  ## client
  def client
    @client ||= Net::FTP.new(@host, port: @port) 
  end

  def make_name
    return "#{WORDS.sample}_"+"#{WORDS.sample}_"+"#{rand(1_000_000)}_"+"#{Time.now.to_i}"
  end

  def validate_resp_a(exp, resp)
    assert_status exp, resp
  end
  def validate_resp_r(exp, resp)
    refute_status exp, resp
  end
  def assert_digest_equal(expected, actual)
    hash = Digest::MD5.hexdigest(expected.strip)
    dl_hash = Digest::MD5.hexdigest(actual.strip)
    assert(hash == dl_hash, "expected `#{expected}`, got `#{actual}`")
  end
  def assert_status(expected, response)
    assert expected == response, "wanted #{expected}, got #{response}"
  end
  def refute_status(expected, response)
    refute expected == response, "wanted #{expected}, got #{response}"
  end
  # assert a predicate is true
  def assert(predicate, message='failed assertion')
    @assertions += 1
    return if predicate
    fail message
  end
  # refute a predicate is true
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
end

poller = Poller.new
sleep 1
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
