#!/bin/env ruby

require 'digest'
require "logger"
require "securerandom"

require 'pry'
require 'byebug'

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

IMAGE_FORMATS = {
  ppm: 0x50504d, # '\0PPM',
  hrl: 0x48524c, # '\0HRL',
  png: 0x504e47, # '\0PNG',
}

OUTPUT_FORMATS = {
  ppm: 0x5054d,
  hrl: 0x48524c,
}

FUN_BLOB = File.read(
  File.join(__dir__, 'images', 'fun.png')
)

class Pov
  def initialize
    connect!
  end

  def run!
    png_request = [
      IMAGE_FORMATS[:png],
      IMAGE_FORMATS[:ppm],
      FUN_BLOB.length
    ].pack('NNQ>')

    @out.write png_request
    @out.flush
    @out.write FUN_BLOB
    @out.flush

    response_len = @in.read(8).unpack('Q>').first
    got_blob = @in.read(response_len)

    got_ptr = got_blob[0x14..0x1b].unpack('Q<').first

    if 0 == got_ptr
      LLL.fatal "Expected non-zero pointer"
      assert(0 != got_ptr)
    end

    $stdout.puts("ADDRESS=%016x" % got_ptr)
  end

  private

  def assert_equal(expected, got, note="")
    return if expected == got

    die "expected #{expected.inspect[0..100]} but got #{got.inspect[0..100]} #{note}"
  end

  def assert(condition)
    return if condition
    die "failed assertion"
  end

  def refute(condition)
    return unless condition
    die "failed refutation"
  end

  def die(message)
    LLL.fatal message

    byebug if ENV['BYEBUG_ON_DIE']

    fail message
  end

  def connect!
    @host = ENV['HOST']
    @port = ENV['PORT']

    refute @host.nil?
    refute @port.nil?

    sock = nil
    10.times do
      begin
        sock = TCPSocket.new @host, @port
        break
      rescue SocketError
        sleep 1
        next
      rescue Errno::ECONNREFUSED
        sleep 1
        next
      end
    end
    sock.setsockopt(Socket::IPPROTO_TCP, Socket::TCP_NODELAY, true)
    @in = sock
    @out = sock
  end
end

pov = Pov.new
at_exit do
  LLL.info (["ruby", $0] +
            ARGV).join(' ')
end
pov.run!

LLL.info "success :)"
