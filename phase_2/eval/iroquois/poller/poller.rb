#!/usr/bin/env ruby

require 'timeout'

require 'mechanize'
require 'mqtt'

require_relative './models'
require_relative './utilities'

# range of lengths to pick in the absence of a `LENGTH` environment variable
LENGTH_RANGE = (100..200)

# seconds to wait for a given activity
DEFAULT_TIME_LIMIT = 5

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

DATABASE_PATH = ENV['DATABASE_PATH'] || '/data/iroquois.sqlite3'

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
    create_account
    fail_create_account

    login_unknown_user
    login_wrong_password

    see_dashboard

    view_subscription
    add_existing_subscription
    add_new_subscription
    remove_subscription

    post_message
  }

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3035).to_i

    @base_uri = URI "http://#{@host}:#{@port}/"

    @report = Hash[ACTIVITIES.map do |a|
                     [a, Timing.new]
                   end]

    @assertions = 0

    @sessions = []

    @mqtt_url = ENV['MQTT_URL'] || 'mqtt://mosquitto:1883'

    @mqtt = MQTT::Client.new(@mqtt_url)
    @mqtt.connect

    seed_rng!
    pick_length!
    pick_timeout!
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
  def create_account
    new_user_name = name
    new_user_password = name

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_user_form'
      form['user[name]'] = new_user_name
      form['user[password]'] = new_user_password
      form['user[password_confirmation]'] = new_user_password

      after_created = form.submit
      message = after_created.search('dd.info')
      assert message
      assert_equal 'created user, please log in', message.text
    end

    UserCheatsheet.create name: new_user_name, password: new_user_password
    User.create name: new_user_name, password_digest: 'x'
  end

  def fail_create_account
    new_user_name = name
    new_user_password = name

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_user_form'
      form['user[name]'] = new_user_name
      form['user[password]'] = new_user_password

      after_created = form.submit
      message = after_created.search('dd.error')
      assert message
      assert_equal 'password and password confirmation didn\'t match',
                   message.text
    end
  end

  def login_unknown_user
    unknown_user_name = name
    while User.find_by(name: unknown_user_name)
      unknown_user_name = name
    end

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_session_form'
      form['username'] = unknown_user_name
      form['password'] = 'x'

      after_login = form.submit
      message = after_login.search('dd.error')
      assert message
      assert_equal 'Failed to log in', message.text
    end
  end

  def login_wrong_password
    user_cheatsheet = UserCheatsheet.order('random() asc').take

    wrong_password = name
    while user_cheatsheet.password == wrong_password
      wrong_password = name
    end

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_session_form'
      form['username'] = user_cheatsheet.name
      form['password'] = wrong_password

      after_login = form.submit
      message = after_login.search('dd.error')
      assert message
      assert_equal 'Failed to log in', message.text
    end
  end

  def see_dashboard
    user_cheatsheet = UserCheatsheet.order('random() asc').take
    user = User.find_by(name: user_cheatsheet.name)

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_session_form'
      form['username'] = user_cheatsheet.name
      form['password'] = user_cheatsheet.password

      after_login = form.submit

      header = after_login.search('h1')
      assert header
      assert_equal 'dashboard', header.text

      whoami = after_login.search('p code')
      assert whoami
      assert_equal user_cheatsheet.name, whoami.text

      subs = after_login.search('table#subscriptions tbody tr')
      assert subs

      expected_subs = user.subscriptions.
                        joins(:topic).
                        order('topics.name asc')

      assert_equal expected_subs.count, subs.length
      subs.zip(expected_subs.all) do |z|
        got, expected = *z
        assert_equal expected.id.to_s, got.search('td.subscription_id').text
        assert_equal expected.topic.name,
                     got.search('td.subscription_topic_name').text
      end
    end
  end

  def view_subscription
    subscription = Subscription.order('random() asc').take
    user = subscription.user
    user_cheatsheet = UserCheatsheet.find_by(name: user.name)

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_session_form'
      form['username'] = user_cheatsheet.name
      form['password'] = user_cheatsheet.password

      after_login = form.submit
      subscription_row =
        after_login.search("tr#subscription_#{subscription.id}")
      assert subscription_row

      subscription_href = subscription_row.search('a').first['href']

      view_sub = after_login.link_with(href: subscription_href).click
      sub_messages = view_sub.search('table#messages tbody tr')

      expected_messages = subscription.messages

      assert_equal expected_messages.count, sub_messages.length

      sub_messages.zip(expected_messages.all) do |z|
        got, expected = *z

        assert_equal expected.id.to_s, got.search('td.message_id').text
        assert_equal expected.content, got.search('td.message_content').text
      end
    end
  end

  def add_existing_subscription
    topic = Topic.order('random() asc').take
    existing_user_ids = topic.subscriptions.
                          select(:user_id).
                          all.map(&:user_id)
    user = User.
             where.not(id: existing_user_ids).
             order('random() asc').take
    user_cheatsheet = UserCheatsheet.find_by(name: user.name)

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_session_form'
      form['username'] = user_cheatsheet.name
      form['password'] = user_cheatsheet.password

      after_login = form.submit
      subscription_form = after_login.form_with action: '/subscriptions'
      subscription_form['subscription[topic][name]'] = topic.name


      did_subscribe = subscription_form.submit
      Subscription.create(user: user, topic: topic)
      subscription_name_cell = did_subscribe.search(
        "td.subscription_topic_name:contains(\"#{topic.name}\")").first
      subscription_row = subscription_name_cell.parent
      subscription_href = subscription_row.search('td.actions a').first['href']

      content = name()
      @mqtt.publish topic.name, content
      Message.create(topic: topic, content: content)

      view_sub = did_subscribe.link_with(href: subscription_href).click
      sub_messages = view_sub.search('table#messages tr')
      assert sub_messages.detect do |row|
        row.search('td.message_content').text == content
      end
    end
  end

  def add_new_subscription
    topic_name = name()
    user = User.
             order('random() asc').take
    user_cheatsheet = UserCheatsheet.find_by(name: user.name)

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_session_form'
      form['username'] = user_cheatsheet.name
      form['password'] = user_cheatsheet.password

      after_login = form.submit
      subscription_form = after_login.form_with action: '/subscriptions'
      subscription_form['subscription[topic][name]'] = topic_name


      did_subscribe = subscription_form.submit
      topic = Topic.create(name: topic_name)
      Subscription.create(user: user, topic: topic)
      subscription_name_cell = did_subscribe.search(
        "td.subscription_topic_name:contains(\"#{topic.name}\")").first
      subscription_row = subscription_name_cell.parent
      subscription_href = subscription_row.search('td.actions a').first['href']

      content = name()
      @mqtt.publish topic.name, content
      Message.create(topic: topic, content: content)

      view_sub = did_subscribe.link_with(href: subscription_href).click
      sub_messages = view_sub.search('table#messages tr')
      assert sub_messages.detect do |row|
        row.search('td.message_content').text == content
      end
    end
  end

  def remove_subscription
    subscription = Subscription.order('random() asc').take
    user = subscription.user
    user_cheatsheet = UserCheatsheet.find_by(name: user.name)

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_session_form'
      form['username'] = user_cheatsheet.name
      form['password'] = user_cheatsheet.password

      after_login = form.submit

      unsubscribe_form = after_login.form_with(
        css: "tr#subscription_#{subscription.id} form.unsubscribe")
      after_unsub = unsubscribe_form.submit

      topic = subscription.topic
      subscription.destroy

      assert_equal(0, after_unsub.
                        search("tr#subscription_#{subscription.id}").
                        length)
    end
  end

  def post_message
    subscription = Subscription.order('random() asc').take
    user = subscription.user
    user_cheatsheet = UserCheatsheet.find_by(name: user.name)

    with_agent do |m|
      index = m.get @base_uri

      form = index.form_with id: 'new_session_form'
      form['username'] = user_cheatsheet.name
      form['password'] = user_cheatsheet.password

      after_login = form.submit
      subscription_row =
        after_login.search("tr#subscription_#{subscription.id}")
      assert subscription_row

      subscription_href = subscription_row.search('a').first['href']

      view_sub = after_login.link_with(href: subscription_href).click

      @mqtt.subscribe subscription.topic.name

      content = name()

      post_form = view_sub.form_with id: 'post_message_form'
      post_form['message[content]'] = content
      after_post = post_form.submit

      subscription.topic.messages.create content: content

      got_topic, got_message = @mqtt.get

      @mqtt.unsubscribe subscription.topic.name

      assert_equal subscription.topic.name, got_topic
      assert_equal content, got_message

      sub_messages = after_post.search('table#messages tbody tr')

      assert sub_messages.detect do |row|
        row.search('td.message_content').text == content
      end
    end
  end

  def with_agent
    Mechanize.start do |agent|
      begin
        agent.follow_redirect = true
        yield agent
      rescue

        LLL.fatal agent.current_page.parser
        raise
      end
    end
  end

  # assert two values are equal
  def assert_equal(expected, actual)
    if expected.is_a?(String) && actual.is_a?(String)
      ex_bin = expected.dup.force_encoding('BINARY')
      act_bin = actual.dup.force_encoding('BINARY')

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
