#!/bin/env ruby

require "logger"
require "securerandom"

require "redis"

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

class Pov
  def initialize
    @redis = Redis.new host: ENV['HOST'], port: ENV['PORT'].to_i
  end

  def run!
    wait_to_connect

    token = @redis.get "../token"

    $stdout.puts "TOKEN=#{token}"
  end

  private
  def wait_to_connect
    LLL.info "Trying to connect..."
    loop do
      begin
        @redis.ping
        break
      rescue Redis::CannotConnectError
        $stderr.print "."
        sleep 1
      end
    end
    $stderr.print "\r"
    LLL.info "connected"
  end
end

p = Pov.new

p.run!
