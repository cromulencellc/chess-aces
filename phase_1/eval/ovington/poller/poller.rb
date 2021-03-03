#!/usr/bin/env ruby

require "forwardable"
require "logger"
require "socket"
require "timeout"
require "digest/crc32"

require_relative "oqtopus"

# range of lengths to pick in the absence of a `LENGTH` environment variable
LENGTH_RANGE=(100..200)

# seconds to wait for a given activity
DEFAULT_TIME_LIMIT = 5

# number of rows to load in a dataset at a time
DATA_LOAD_RANGE=(10..20)

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

# represents a dataset; column definitions and rows
class Table < Struct.new(:defs, :data)
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
  tautology
  inspect
  quick_maths
  numeric_relations
  car_cdr quote
  if_test
  cond_test
  data_load
  variable_binding
}

  # activities that do require data to have been loaded
  NEED_TABLE_ACTIVITIES = %i{
  all_rows
  count_rows
  detect
  select
  sort
  stats
  map
  my_mean
  get_defs
}

  ALL_ACTIVITIES = ACTIVITIES + NEED_TABLE_ACTIVITIES

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3015).to_i

    @report = Hash[ALL_ACTIVITIES.map do |a|
                     [a, Timing.new]
                   end]

    @assertions = 0

    @lists = []
    @tables = {}

    seed_rng!
    pick_length!
    pick_timeout!
  end

  def run!
    @sock = TCPSocket.new @host, @port

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
      if @tables.length > 0
        universe += NEED_TABLE_ACTIVITIES
      end
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
  # validate that challenge returns very simple expressions
  def tautology
    stmt = rand(100)
    assert_exec stmt, stmt
  end

  # test converting values to strings
  def inspect
    val = rand(100)
    assert_exec ["inspect".k, val], "#{val} "
  end

  # validate challenge can evaluate nested arithmetic expressions
  def quick_maths
    stmt = nil
    expectation = nil
    prev_ex = nil
    rand(2..6).times do
      op = %w{+ - * /}.sample.k
      operands = rand(2..6).times.map{ rand(1..10) }
      operands << stmt unless stmt.nil?
      operands.shuffle!

      begin
        expectation = operands.inject do |memo, oa|
          memo = prev_ex if memo.is_a? Array
          oa = prev_ex if oa.is_a? Array
          memo.to_f.send(op, oa.to_f).to_i
        end
        prev_ex = expectation
      rescue FloatDomainError => e
        LLL.error "got #{e} trying to #{ op } #{ operands }"
        LLL.error "going with #{stmt} -> #{prev_ex} for now"
        expectation = prev_ex
        break stmt
      end
      stmt = [op] + operands
      # LLL.info "#{stmt} -> #{expectation}"
    end

    assert_exec stmt, expectation
  end

  # validate numeric relationships (i.e. equality, lt/gt/etc.)
  def numeric_relations
    left = rand(-100..100)
    left += rand() if (rand() > 0.5)

    right = rand(-100..100)
    right += rand() if (rand() > 0.5)

    op = %w{< <= == >= > !=}.sample

    expectation = left.send(op, right) ? 1 : []
    assert_exec [op.k, left, right], expectation

    expectation = right.send(op, left) ? 1 : []
    assert_exec [op.k, right, left], expectation
  end

  # head & tail of lists
  def car_cdr
    assert_exec ["car".k, 1, 2], 1
    assert_exec ["cdr".k, 1, 2], [2]
  end

  # quoting arguments without evaluating them
  def quote
    assert_exec ["quote".k, [1, 2]], [1, 2]
  end

  # if statements
  def if_test
    assert_exec ["if".k, 1, 2], 2
    assert_exec ["if".k, 1, 2, 3], 2
    assert_exec ["if".k, "false".k, 2, 3], 3
  end

  # cond, a more complex if statement
  def cond_test
    assert_exec ["cond".k,
                 "false".k, 1,
                 "true".k, 2,
                 "true".k, 3
                ], 2
    assert_exec ["cond".k,
                 "false".k, 1],
                []
  end

  # demonstrate binding a given value to a variable name
  def variable_binding
    name = WORDS.sample + rand(100).to_s
    value = rand(10_000)
    assert_exec ["set".k, name.k, value], name
    assert_exec ["+".k, name.k, 0], value
  end

  # load data, using the data-load protocol
  def data_load
    column_types = { Integer => 's', Float => 'f', String => 'z'}
    columns = ([['id', Integer], ['desc', String]] +
              rand(1..4).times.map do
      name = WORDS.sample + rand(100).to_s
      type = [Integer, Float, String].sample
      [name, type]
                   end).to_h

    table = rand(DATA_LOAD_RANGE).times.map do
      columns.map do |k, v|
        v.randomize
      end
    end
    column_defs = columns.map do |name, type|
      column_types[type] + name
    end.join("\x1f") + "\x1d"

    @sock.write "data-load".k.to_oqtopus
    table_name = @sock.read 36
    LLL.debug table_name
    LLL.debug columns
    LLL.debug "#{table.length} rows"
    @sock.write column_defs

    first_row = true
    outer_ck = Digest::CRC32.new
    table.each_with_index do |r, i|
      @sock.write "\x1e" unless first_row
      first_row = false
      crc_buf = Digest::CRC32.new
      crc_buf << [i].pack('q<')
      enc_buf = ''

      r.map do |v|
        enc = nil
        case v
        when String
          crc_buf << v
          enc = [v.length].pack('C') + v
        when Integer
          enc = [v].pack('q<')
          crc_buf << enc
        when Float
          enc = [v].pack('E')
          crc_buf << enc
        else
          fail "couldn't serialize #{v.inspect}"
        end
        enc_buf << enc
        @sock.write enc
      end
      hexed_buf = enc_buf.unpack('H2' * enc_buf.length)
      #LLL.debug "row #{i.to_s(16)} crc #{crc_buf.hexdigest}"
      outer_ck << [crc_buf.checksum].pack('L')

    end
    @sock.write "\x1c"
    got_ck = @sock.read(4).unpack('L').first
    LLL.debug "outer ck want #{outer_ck.hexdigest} got #{got_ck.to_s(16)}"
    assert_equal outer_ck.checksum, got_ck
    @tables[table_name] = Table.new columns, table
  end

  def count_rows
    table_name = @tables.keys.sample
    table = @tables[table_name]

    assert_exec(["count-rows".k, ["table".k, table_name]], table.data.length)
  end

  def all_rows
    table_name = @tables.keys.sample
    table = @tables[table_name]

    assert_exec(["rows".k,
                 ["table".k, table_name]],
                table.data)
  end

  # find the first row that matches an expression
  def detect
    table_name = @tables.keys.sample
    table = @tables[table_name]

    target_row = table.data.sample

    assert_exec(["detect".k,
                 ["table".k, table_name],
                 ["row-lambda".k, ["==".k, target_row.first, "id".k]]],
                target_row)
  end

  # find all rows that match an expression
  def select
    table_name = @tables.keys.sample
    table = @tables[table_name]

    some_id = Integer.randomize
    some_direction = %w{< <= > >=}.sample

    # LLL.info "#{some_id} #{some_direction}"

    found_rows = table.data.select do |r|
      cmp = some_id.send some_direction.to_sym, r.first

      # LLL.info "#{r.first} => #{cmp}"

      next cmp
    end

    assert_exec(["rows".k, ["select".k,
                 ["table".k, table_name],
                 ["row-lambda".k, [some_direction.k, some_id, "id".k]]]],
                found_rows)
  end

  # sort a table based on a row-lambda
  def sort
    table_name = @tables.keys.sample
    table = @tables[table_name]

    column = table.defs.to_a.sample
    while String == column[1]
      column = table.defs.to_a.sample
    end

    column_idx = table.defs.keys.index column[0]

    sorted = table.data.sort_by{ |r| r[column_idx] }

    assert_exec(["rows".k, ["sort-by".k,
                            ["table".k, table_name],
                            ["row-lambda".k, column[0].k]]],
                sorted)
  end

  # calculate statistical values for a column on a table
  def stats
    table_name = @tables.keys.sample
    table = @tables[table_name]

    column = table.defs.to_a.sample
    while String == column[1]
      column = table.defs.to_a.sample
    end

    column_idx = table.defs.keys.index column[0]

    values = table.data.map{ |r| r[column_idx].to_f }.sort

    # LLL.info values

    count = values.length
    mean = values.inject{  |memo, obj| memo + obj } / count

    midpoint = count / 2
    median = values[midpoint]

    squared_differences = values.map do |obj|
      d = mean - obj
      d * d
    end

    # LLL.info squared_differences

    stddev = Math.sqrt(squared_differences.sum / (count - 1))

    pseudo_expectation = [["stddev".k, stddev],
                          ["median".k, median],
                          ["count".k, count],
                          ["mean".k, mean]]

    got = evaluate(["stats".k, ["table".k, table_name],
           ["row-lambda".k, column[0].k]]).to_h

    pseudo_expectation.each do |x|
      assert_in_epsilon(x[1], got[x[0]])
    end
  end

  # return results of a lambda expression over each row
  def map
    table_name = @tables.keys.sample
    table = @tables[table_name]


    column = table.defs.to_a.sample
    while String == column[1]
      column = table.defs.to_a.sample
    end

    column_idx = table.defs.keys.index column[0]

    some_float = Float.randomize
    some_direction = %w{+ -}.sample

    got = table.data.map do |r|
      [some_float.send(some_direction, r[column_idx])]
    end

    assert_exec(["rows".k,
                 ["map".k,
                  ["table".k, table_name],
                  ["row-lambda".k,
                   [some_direction.k, some_float, column[0].k]]]],
                got)
  end

  # get the defs for a table
  def get_defs
    table_name = @tables.keys.sample
    table = @tables[table_name]

    got = evaluate ["defs".k, ["table".k, table_name]]

    got.each do |pair|
      klass = case pair[0]
              when "stringz".k
                String
              when "float64".k
                Float
              when "sint64".k
                Integer
              end

      assert_equal klass, table.defs[pair[1].to_s]
    end
  end

  # compare calculating the mean in ruby vs. in oqtopus
  def my_mean
    table_name = @tables.keys.sample
    table = @tables[table_name]

    column = table.defs.to_a.sample
    while (String == column[1])
      column = table.defs.to_a.sample
    end

    column_idx = table.defs.keys.index column[0]

    values = table.data.map{ |r| r[column_idx].to_f }
    mean = values.inject{  |memo, obj| memo + obj } / values.size

    got = evaluate ["/".k,
                    ["reduce".k,
                     ["map".k,
                      ["table".k, table_name],
                      ["row-lambda".k, column[0].k]],
                     0.0,
                     ["row-lambda".k, ["+".k, "_memo".k, "_".k]]
                    ],
                    ["count-rows".k, ["table".k, table_name]]
                   ]

    assert_in_epsilon(mean, got)
  end

  # evaluate a statement on the challenge
  def evaluate(statement)
    oqtopus_statement = statement.to_oqtopus
    # LLL.info oqtopus_statement.inspect
    @sock.write oqtopus_statement
    Oqtopus.read @sock
  end

  # assert the result of a statement is equal to our expectation
  def assert_exec(statement, expectation)
    got = evaluate(statement)

    if expectation != got
      LLL.debug statement
      LLL.debug "expected #{expectation}"
      LLL.debug "got #{got}"
    end

    assert_equal expectation, got
  end

  # assert a value is within epsilon of our expectation
  # epsilon is basically delta but multiplicative instead of additive
  def assert_in_epsilon(expected, actual, epsilon = 0.1)
    delta = [expected.abs, actual.abs].min * epsilon
    assert_in_delta(expected, actual, delta)
  end

  # assert a value is within delta of our expectation
  def assert_in_delta(exp, actual, delta = 0.001)
    diff = (exp - actual).abs
    assert(delta > diff,
           "expected #{exp}, got #{actual} which was #{diff} (> #{delta}) away")
  end

  # assert two values are equal
  def assert_equal(expected, actual)
    assert(expected == actual, "expected #{expected}, got #{actual}")
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
