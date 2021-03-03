#!/bin/env ruby

require "logger"
require "timeout"

require "http"
require "nokogiri"

LENGTH_RANGE=(50..100)
LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

CLIENT_COUNT=5
MAX_LIST_ASSERTIONS=10
DEFAULT_TIME_LIMIT=2

WORDS = %w{
           alpha bravo charlie delta echo foxtrot golf hotel india
           juliet kilo lima mike november oscar papa quebec romeo
           sierra tango uniform victor whiskey xray yankee zulu
           }

class List
  attr_accessor :id, :name, :items
end

class Item
  attr_accessor :id, :content, :complete
end

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

class Poller
  attr_reader :seed, :length
  attr_reader :host, :port

  attr_reader :report, :assertions

  ACTIVITIES = %i{
  index_lists
  create_list
  show_list

  create_task
  toggle_task
  validate_task

  show_admin
  fail_admin
  succeed_admin
}

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = ENV['PORT'] || '3013'

    @admin_password = ENV['ADMIN_PASSWORD'] || 'asdfasdf'
    @expected_token = ENV['TOKEN'] # nil is okay

    @report = Hash[ACTIVITIES.map do |a|
                     [a, Timing.new]
                   end]

    @assertions = 0

    @lists = []

    seed_rng!
    pick_length!
    pick_timeout!
    ping!
  end

  def run!
    create_list
    create_task

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
  def client
    return @clients.sample if defined? @clients

    @clients = CLIENT_COUNT.times.map do
      HTTP.persistent "http://#{ENV['HOST']}:#{ENV['PORT']}"
    end

    return @clients.sample
  end

  def index_lists
    got = get("/")
    list_rows = got.css("table.lists tbody tr")
    @lists.sample(MAX_LIST_ASSERTIONS).each do |candidate_list|
      candidate_id = "list_#{candidate_list.id}"
      assert(list_rows.find{ |lr| lr['id'] == candidate_id },
             "couldn't find list with id #{candidate_list.id}")
    end
  end

  def create_list
    expected_name = name
    got = post("/lists", name: expected_name)
    assert_equal expected_name, got.css('h1').text

    new_list = List.new
    new_list.id = got.css('.list_id').text
    new_list.name = expected_name
    new_list.items = []

    @lists << new_list
  end

  def show_list
    candidate_list = @lists.sample
    got = get "/lists/#{candidate_list.id}"

    assert_equal candidate_list.name, got.css('.list_name').text
    candidate_list.items.sample(MAX_LIST_ASSERTIONS).each do |i|
      r = got.css("#item_#{i.id}")
      assert_equal i.complete.to_s, r.css('.complete').text
      assert_equal i.content, r.css('.content').text
    end
  end

  def create_task
    candidate_list = @lists.sample
    task_content = name

    got = post("/lists/#{candidate_list.id}/items", content: task_content)
    task_rows = got.css("table.items tbody tr")
    found_task = task_rows.find do |tr|
      tr.css("td.content").text == task_content
    end

    assert(found_task,
           "couldn't find task with content %s in list %s" % [
             task_content, candidate_list.id
           ])

    new_task = Item.new
    new_task.id = found_task['data-id']
    new_task.complete = false
    new_task.content = task_content

    candidate_list.items << new_task
  end

  def toggle_task
    candidate_list = nil
    candidate_task = nil
    until candidate_task
      candidate_list = @lists.sample
      candidate_task = candidate_list.items.sample
    end

    got = post("/lists/#{candidate_list.id}/items/#{candidate_task.id}/toggle")
    task_row = got.css("tr#item_#{candidate_task.id}")
    candidate_task.complete = !candidate_task.complete
    assert_equal candidate_task.complete.to_s, task_row.css('.complete').text
  end

  def validate_task
    candidate_list = nil
    candidate_task = nil
    until candidate_task
      candidate_list = @lists.sample
      candidate_task = candidate_list.items.sample
    end

    got = get("/lists/#{candidate_list.id}")
    task_row = got.css("tr#item_#{candidate_task.id}")
    assert_equal candidate_task.complete.to_s, task_row.css('.complete').text
    assert_equal candidate_task.content, task_row.css('.content').text
  end

  def show_admin
    got = get('/admin')
    assert got.css('#token').empty?
    assert got.css('form[action="/admin"]')
  end

  def fail_admin
    candidate_password = @admin_password
    until candidate_password != @admin_password
      candidate_password = name
    end

    got = post("/admin", password: candidate_password)
    assert got.css('#token').empty?
    assert got.css('form[action="/admin"]')
  end

  def succeed_admin
    got = post("/admin", password: @admin_password)

    found_token = got.css('#token')
    assert(!(found_token.empty?),
           "expected to find token")
    LLL.info "found token #{found_token.text}"

    if @expected_token
      assert_equal @expected_token, found_token.text
    end
  end

  def get(path)
    resp = client.get(path)
    doc = Nokogiri::HTML(resp.body.to_s)
    resp.flush
    return doc
  end

  def post(path, params={})
    resp = client.post(path, form: params)
    doc = Nokogiri::HTML(resp.body.to_s)
    resp.flush
    return doc
  end

  def ping!
    txid = get("/_info").css("span#txid").text
    assert(txid =~ /^[a-fA-F0-9\\-]+$/, 'txid should be a uuid')
  end

  def assert_equal(expected, actual)
    assert(expected == actual, "expected #{expected} to == #{actual}")
  end

  def assert(predicate, message='failed assertion')
    @assertions += 1
    return if predicate
    fail message
  end

  def seed_rng!
    @seed = (ENV['SEED'] || Random.new_seed).to_i
    $stdout.puts "SEED=#{@seed}"
    $stdout.flush
    srand(@seed)
  end

  def pick_length!
    @length = (ENV['LENGTH'] || LENGTH_RANGE.to_a.sample).to_i
    $stdout.puts "LENGTH=#{@length}"
    $stdout.flush
  end

  def pick_timeout!
    @timeout = (ENV['TIMEOUT'] || DEFAULT_TIME_LIMIT).to_i
    $stdout.puts "TIMEOUT=#{@timeout}"
    $stdout.flush
  end

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

$stderr.puts "activity\tcount\tmin s\tavg s\tmax s"
poller.report.each do |r|
  timing = r[1]
  $stderr.puts("%s\t%d\t%.4f\t%.4f\t%.4f" % [
                 r[0],
                 timing.count, timing.min, timing.mean, timing.max
               ])
end

LLL.info "#{poller.assertions} total assertions"

LLL.info "success :)"
