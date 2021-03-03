#!/bin/env ruby

require "logger"
require "securerandom"

require "redis"

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

ACTIVITIES = %i{
           get set
           append append_but_really_set
           mget mset
           strlen
           sadd sadd_new_set sadd_existing_member sadd_mix
           srem
           sismember_yes sismember_no sismember_super_no
           smembers scard
           sdiff sinter sunion
           sdiffstore sinterstore sunionstore

           zadd_new_set
           zcard
           zrank zrank_no zrank_super_no
           zscore zscore_no zscore_super_no
           zrem
           zrange zrange_with_scores
           zrevrange

           ping }
KEY_LENGTH = 16 # arbitrary, w/e
VALUE_LENGTH = 16

MAX_MULTI = 10

ZSET_SCORE_MULTIPLIER = 100

COMMON_SET_ENTRIES = %w{
                   alpha bravo charlie delta echo foxtrot golf hotel india
                   juliet kilo lima mike november oscar papa quebec romeo
                   sierra tango uniform victor whiskey xray yankee zulu
                   }

if example_filename = ENV['DUMP_EXAMPLE']
  EXAMPLE_FILE = File.open(example_filename, "w")
  module Redis::Connection::SocketMixin
    def write_with_dump(data)
      EXAMPLE_FILE.write data
      write_without_dump(data)
    end

    alias_method :write_without_dump, :write
    alias_method :write, :write_with_dump
  end
  at_exit do
    EXAMPLE_FILE.close
  end
end

class Poller

  attr_reader :seed, :length

  def initialize
    @redis = Redis.new host: ENV['HOST'], port: ENV['PORT'].to_i
    seed_rng!
    pick_length!

    @activity_counts = Hash.new

    @getset = Hash.new

    @sets = Hash.new

    @zsets = Hash.new
  end

  def run!
    wait_to_connect
    ping

    set # need an initial key/value pair
    sadd_new_set # initial set too
    zadd_new_set

    @length.times do
      my_activity = ACTIVITIES.sample
      @activity_counts[my_activity] = 0 unless @activity_counts[my_activity]
      @activity_counts[my_activity] += 1
      LLL.debug "Trying #{my_activity}"
      send my_activity
    end
  end

  def report!
    max_activity_name_len = @activity_counts.keys.map(&:length).max
    max_activity_count = @activity_counts.values.max
    max_activity_count_len = Math.log10(max_activity_count).to_i + 1

    activity_formatter = "%#{max_activity_name_len}s\t%#{max_activity_count_len}d"

    @activity_counts.sort_by{ |k, v| [-v, k] }.each do |k, v|
      LLL.info(activity_formatter % [k, v])
    end

    missing_activities = ACTIVITIES - @activity_counts.keys

    if missing_activities.empty?
      LLL.info "No missing activities :)"
    else
      LLL.info "Missing activities: #{missing_activities}"
    end
  end

  private
  def wait_to_connect
    LLL.info "Trying to connect..."
    loop do
      begin
        @redis.ping
        break
      rescue Redis::CannotConnectError
        $stderr.print "."
        sleep 1
      end
    end
    $stderr.print "\r"
    LLL.info "connected"
  end

  def ping
    assert @redis.ping
  end

  def set
    k = mkkey
    v = mkval

    LLL.debug("#{k} => #{v.inspect}")

    @redis.set k, v

    @getset[k] = v
  end

  def get
    k = @getset.keys.sample
    assert_equal @redis.get(k), @getset[k], "get[#{k}]"
  end

  def append
    k = @getset.keys.sample
    ve = @getset[k]
    va = mkval
    len = @redis.append(k, va)

    LLL.debug("#{k} => e#{ ve.inspect} + n#{va.inspect}")

    assert_equal (ve.length + va.length), len

    @getset[k] += va
  end

  def append_but_really_set
    k = mkkey
    v = mkval

    len = @redis.append(k, v)

    assert_equal v.length, len

    @getset[k] = v
  end

  def mget
    count = rand(MAX_MULTI)

    keys = @getset.keys.sample(count)
    LLL.debug("mgetting #{keys.inspect}")
    got = @redis.mget keys

    keys.zip(got) do |k, v|
      assert_equal @getset[k], v
    end
  end

  def mset
    count = rand(MAX_MULTI)

    pairs = count.times.map do
      [mkkey,
       mkval]
    end

    LLL.debug("msetting #{pairs.inspect}")

    assert_equal "OK", @redis.mset(pairs.flatten)

    pairs.each do |p|
      k,v = p
      @getset[k] = v
    end
  end

  def strlen
    k = @getset.keys.sample
    assert_equal @getset[k].length, @redis.strlen(k)
  end

  def sadd_new_set
    k = mkkey
    @sets[k] = Set.new

    count = rand(MAX_MULTI)

    vs = count.times.map do
      v = mkval

      @sets[k].add v
      v
    end

    assert_equal count, @redis.sadd(k, vs)
  end

  def sadd
    k = @sets.keys.sample

    count = rand(MAX_MULTI)

    vs = count.times.map do
      v = mkval
      @sets[k].add v
      v
    end

    assert_equal count, @redis.sadd(k, vs)
  end

  def sadd_existing_member
    k = @sets.keys.sample
    s = @sets[k]

    count = rand([s.size, MAX_MULTI].min)

    vs = s.to_a.sample(count)

    assert_equal 0, @redis.sadd(k, vs)
  end

  def sadd_mix
    k = @sets.keys.sample
    s = @sets[k]

    exist_count = rand([s.size, MAX_MULTI].min)
    new_count = rand(MAX_MULTI)

    exist = s.to_a.sample(exist_count)
    neu = new_count.times.map do
      v = mkval
      s.add v
      v
    end

    assert_equal new_count, @redis.sadd(k, exist + neu)
  end

  def srem
    k = @sets.keys.sample
    s = @sets[k]

    rm_count = rand([s.size - 1, MAX_MULTI].min).to_i
    if (rm_count < 0)
      LLL.info("can't srem #{rm_count} entries, skipping")
      rm_count = 0
    end

    if (rm_count > s.size)
      LLL.info("can't srem #{rm_count} from set of #{s.size} entries, skipping")
      rm_count = 0
    end

    bogus_rm = rand(MAX_MULTI).times.map{ mkval }

    to_rm = s.to_a.sample(rm_count)

    s.subtract to_rm

    assert_equal rm_count, @redis.srem(k, bogus_rm + to_rm)
  end

  def sismember_yes
    k = @sets.keys.sample
    v = @sets[k].to_a.sample

    if v.nil?
      LLL.info("asserting sismeber_no for #{k} because it was empty")
      assert_equal false, @redis.sismember(k, mkval)
      return
    end

    assert_equal true, @redis.sismember(k, v)
  end

  def sismember_no
    k = @sets.keys.sample
    v = mkval

    assert_equal false, @redis.sismember(k, v)
  end

  def sismember_super_no
    k = mkkey
    v = mkval

    assert_equal false, @redis.sismember(k, v)
  end

  def smembers
    k = @sets.keys.sample
    s = @sets[k]

    got = @redis.smembers(k)

    assert_equal(s, Set.new(got))
  end

  def scard
    k = @sets.keys.sample

    assert_equal @sets[k].size, @redis.scard(k)
  end


  def sdiff
    k1 = mkkey
    k2 = mkkey

    common = Set.new COMMON_SET_ENTRIES.sample(rand(MAX_MULTI))
    uniq = Set.new rand(MAX_MULTI).times.map{ mkval }
    other = Set.new rand(MAX_MULTI).times.map{ mkval }

    s1 = common + uniq
    s2 = common + other

    @redis.sadd k1, s1.to_a
    @redis.sadd k2, s2.to_a

    assert_equal uniq, Set.new(@redis.sdiff(k1, k2))
  end

  def sdiffstore
    k1 = mkkey
    k2 = mkkey
    kd = mkkey

    common = Set.new COMMON_SET_ENTRIES.sample(rand(MAX_MULTI))
    uniq = Set.new rand(MAX_MULTI).times.map{ mkval }
    other = Set.new rand(MAX_MULTI).times.map{ mkval }

    s1 = common + uniq
    s2 = common + other

    @redis.sadd k1, s1.to_a
    @redis.sadd k2, s2.to_a

    assert_equal uniq.count, @redis.sdiffstore(kd, k1, k2)
    assert_equal uniq, Set.new(@redis.smembers(kd))
  end

  def sinter
    k1 = mkkey
    k2 = mkkey

    common = Set.new COMMON_SET_ENTRIES.sample(rand(MAX_MULTI))
    uniq = Set.new rand(MAX_MULTI).times.map{ mkval }
    other = Set.new rand(MAX_MULTI).times.map{ mkval }

    s1 = common + uniq
    s2 = common + other

    @redis.sadd k1, s1.to_a
    @redis.sadd k2, s2.to_a

    assert_equal common, Set.new(@redis.sinter(k1, k2))
  end

  def sinterstore
    k1 = mkkey
    k2 = mkkey
    kd = mkkey

    common = Set.new COMMON_SET_ENTRIES.sample(rand(MAX_MULTI))
    uniq = Set.new rand(MAX_MULTI).times.map{ mkval }
    other = Set.new rand(MAX_MULTI).times.map{ mkval }

    s1 = common + uniq
    s2 = common + other

    @redis.sadd k1, s1.to_a
    @redis.sadd k2, s2.to_a

    assert_equal common.count, @redis.sinterstore(kd, k1, k2)
    assert_equal common, Set.new(@redis.smembers(kd))
  end

  def sunion
    k1 = mkkey
    k2 = mkkey

    common = Set.new COMMON_SET_ENTRIES.sample(rand(MAX_MULTI))
    uniq = Set.new rand(MAX_MULTI).times.map{ mkval }
    other = Set.new rand(MAX_MULTI).times.map{ mkval }

    s1 = common + uniq
    s2 = common + other

    @redis.sadd k1, s1.to_a
    @redis.sadd k2, s2.to_a

    assert_equal (common + uniq + other), Set.new(@redis.sunion(k1, k2))
  end

  def sunionstore
    k1 = mkkey
    k2 = mkkey
    kd = mkkey

    common = Set.new COMMON_SET_ENTRIES.sample(rand(MAX_MULTI))
    uniq = Set.new rand(MAX_MULTI).times.map{ mkval }
    other = Set.new rand(MAX_MULTI).times.map{ mkval }

    s1 = common + uniq
    s2 = common + other

    @redis.sadd k1, s1.to_a
    @redis.sadd k2, s2.to_a

    assert_equal (common + uniq + other).count, @redis.sunionstore(kd, k1, k2)
    assert_equal (common + uniq + other), Set.new(@redis.smembers(kd))
  end

  # ZSETS

  def zadd_new_set
    k = mkkey
    body = Array.new

    count = rand(MAX_MULTI)

    vs = count.times.map do
      v = mkval
      s = mkzscore

      pair = [s, v]
      body << pair
      pair
    end

    @zsets[k] = body

    LLL.info "k #{k} body(#{count}) #{body}"

    assert_equal count, @redis.zadd(k, vs.flatten)
  end

  def zcard
    k = @zsets.keys.sample
    v = @zsets[k]

    assert_equal v.count, @redis.zcard(k)
  end

  def zrank
    k = @zsets.keys.sample
    v = @zsets[k]

    if 0 == v.size
      assert_equal nil, @redis.zrank(k, mkval)
      return
    end

    desired = v.sample
    sorted_v = v.sort

    expected_rank = sorted_v.index desired

    LLL.info "expecting #{expected_rank} from #{k}(#{v.inspect}) at #{desired}"

    assert_equal expected_rank, @redis.zrank(k, desired[1])
  end

  def zrank_no
    k = @zsets.keys.sample
    de = mkval

    assert_equal nil, @redis.zrank(k, de)
  end

  def zrank_super_no
    k = mkkey
    de = mkval

    assert_equal nil, @redis.zrank(k, de)
  end

  def zscore
    k = @zsets.keys.sample
    v = @zsets[k]

    if 0 == v.size
      assert_equal nil, @redis.zscore(k, mkval)
      return
    end

    desired = v.sample
    expected_score = desired[0]
    candidate = desired[1]

    assert_in_delta expected_score, @redis.zscore(k, candidate)
  end

  def zscore_no
    k = @zsets.keys.sample
    de = mkval

    assert_equal nil, @redis.zscore(k, de)
  end

  def zscore_super_no
    k = mkkey
    de = mkval

    assert_equal nil, @redis.zscore(k, de)
  end

  def zrem
    k = mkkey
    e = mkval
    o = mkval
    es = rand()
    os = rand()

    LLL.debug "zrem on #{k} adding #{e}(#{es}) #{o}(#{os})"

    assert_equal 2, @redis.zadd(k, [es, e, os, o])
    assert_equal 1, @redis.zrem(k, [e, mkval])
    assert_in_delta os, @redis.zscore(k, o)
    assert_equal nil, @redis.zscore(k, e)
    assert_equal true, @redis.zrem(k, o)
  end

  def zrange
    k = @zsets.keys.sample
    v = @zsets[k].sort

    if 0 == v.size
      assert_equal [], @redis.zrange(k, 0, -1)
    end

    start = rand(v.size - 1)
    stop = rand(v.size + 1) - 1

    if (start > stop) && (stop >= 0)
      swap = start
      start = stop
      stop = swap
    end

    if start == stop
      start = 0
      stop = -1
    end

    expectation = v[start..stop].map{ |v| v[1] }
    assert_equal expectation, @redis.zrange(k, start.to_i, stop.to_i)
  end

  def zrange_with_scores
    k = @zsets.keys.sample
    v = @zsets[k].sort

    if 0 == v.size
      assert_equal [], @redis.zrange(k, 0, -1)
    end

    start = rand(v.size - 1)
    stop = rand(v.size + 1) - 1

    if (start > stop) && (stop >= 0)
      swap = start
      start = stop
      stop = swap
    end

    expectation = v[start..stop].map{|e| [e[1], e[0]] }
    got = @redis.zrange(k, start.to_i, stop.to_i, withscores: true)

    expectation.zip(got).each do |e, g|
      assert_equal e[0], g[0]
      assert_in_delta e[1], g[1]
    end
  end

  def zrevrange
    k = @zsets.keys.sample
    v = @zsets[k].sort

    if 0 == v.size
      assert_equal [], @redis.zrange(k, 0, -1)
    end

    start = rand(v.size + 1) - 1
    stop = rand(v.size + 1) - 1

    if (start > stop) && (stop >= 0)
      swap = start
      start = stop
      stop = swap
    end

    if start == stop
      start = 0
      stop = -1
    end

    expectation = v[start..stop].map{ |v| v[1] }.reverse
    assert_equal expectation, @redis.zrevrange(k, start, stop)
  end

  def zrevrange_with_scores
    k = @zsets.keys.sample
    v = @zsets[k].sort

    if 0 == v.size
      assert_equal [], @redis.zrange(k, 0, -1)
    end

    start = rand(v.size - 1)
    stop = rand(v.size + 1) - 1

    if (start > stop) && (stop >= 0)
      swap = start
      start = stop
      stop = swap
    end

    expectation = v[start..stop].map{|e| [e[1], e[0]] }.reverse
    got = @redis.zrange(k, start.to_i, stop.to_i, withscores: true)

    expectation.zip(got).each do |e, g|
      assert_equal e[0], g[0]
      assert_in_delta e[1], g[1]
    end
  end

  def assert_equal(expected, got, note="")
    return if expected == got

    die "expected #{expected.inspect} but got #{got.inspect} #{note}"
  end

  def assert_in_delta(expected, got, delta=0.0001, note="")
    return unless ((expected - got).abs > delta)

    die "expected #{expected.inspect} to be within #{delta} of #{got.inspect} #{note}"
  end

  def assert(condition)
    return if condition
    die "failed assertion"
  end

  def die(message)
    LLL.fatal message
    fail message
  end

  def seed_rng!
    @seed = (ENV['SEED'] || Random.new_seed).to_i
    $stdout.puts "SEED=#{@seed}"
    srand(@seed)
  end

  def pick_length!
    @length = (ENV['LENGTH'] || (200..300).to_a.sample).to_i
    $stdout.puts "LENGTH=#{@length}"
  end

  def mkkey
    rand(36**KEY_LENGTH).to_s(36)
  end

  def mkval
    rand(36**VALUE_LENGTH).to_s(36)
  end

  def mkzscore
    rand() * ZSET_SCORE_MULTIPLIER
  end

  def mkcval
    COMMON_SET_ENTRIES.sample
  end
end

p = Poller.new

at_exit do
  LLL.info "SEED=#{p.seed} LENGTH=#{p.length}"
end

p.run!
p.report!
