#!/bin/env ruby

require 'digest'
require "logger"
require "securerandom"
require 'timeout'

require 'pry'
require 'byebug'

require "rmagick"

require_relative './hrl'

LENGTH_RANGE=(50..100)

ALLOWABLE_DIFFERENCE = 0.01

DEFAULT_TIME_LIMIT=1

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

IMAGES = Dir.glob(File.join(__dir__, 'images', '*'))
HAMLIN_LOGO = Magick::Image.read(
  Dir.glob(
    File.join(__dir__, 'images', 'hamlin.png')
  ).first
).first

LLL.info IMAGES

WORDS = %w{
           alpha bravo charlie delta echo foxtrot golf hotel india
           juliet kilo lima mike november oscar papa quebec romeo
           sierra tango uniform victor whiskey xray yankee zulu
           }

GRAVITIES = %w{NorthWest North NorthEast East SouthEast South SouthWest
          West Center}.map{ |w| Magick.const_get("#{w}Gravity") }

class Timing
  attr_reader :count, :total, :min, :max

  def initialize
    @count = 0
    @total = 0.0
    @min = DEFAULT_TIME_LIMIT + 1
    @max = -1
  end

  def add(time)
    @count += 1
    @total += time
    @min = time if @min > time
    @max = time if @max < time
  end

  def mean
    @total / @count.to_f
  end
end

class Poller
  attr_reader :seed, :length, :report

  def initialize
    seed_rng!
    pick_length!
    connect!

    @report = Hash[IMAGE_FORMATS.keys.product(OUTPUT_FORMATS.keys).map do |a|
                     [a, Timing.new]
                   end]
  end

  def run!
    length.times do
      input_format = IMAGE_FORMATS.keys.sample
      output_format = (OUTPUT_FORMATS).keys.sample

      LLL.debug [input_format, IMAGE_FORMATS[input_format].to_s(16),
                 output_format, IMAGE_FORMATS[output_format].to_s(16),
                 ]

      image_filename = IMAGES.sample

      image = Magick::Image.read(image_filename).first

      new_width = rand(32..128)
      new_height = rand(32..128)

      filter = [
        Magick::LanczosFilter,
        Magick::PointFilter,
        Magick::BoxFilter,
        Magick::TriangleFilter
      ].sample

      image.resize!(new_width, new_height, filter)

      rotation = [0, 90, 180, 270].sample
      image.rotate!(rotation)

      text = Magick::Draw.new
      text.font_family = 'DejaVu Sans'

      text.pointsize = rand() * image.rows
      text.gravity = GRAVITIES.sample

      text.annotate(image, 0, 0, 0, 0, WORDS.sample) do
        fill_color = rand(0xFFFFFF)
        self.fill = "#%06s" % fill_color.to_s(16)
      end

      image.composite!(HAMLIN_LOGO, GRAVITIES.sample,
                       Magick::OverCompositeOp
                      )

      LLL.debug("orig(#{image_filename}) now(#{image.columns}x#{image.rows})")

      out_blob = get_blob(image, input_format)

      LLL.debug "len #{out_blob.length}"

      request = [
        IMAGE_FORMATS[input_format],
        IMAGE_FORMATS[output_format],
        out_blob.length
      ].pack('NNQ>')

      LLL.debug Digest.hexencode(request)
      LLL.debug Digest.hexencode(out_blob[0..32])

      response_len = nil
      got_blob = nil

      before_time = Time.now.to_f
      Timeout::timeout(@timeout) do
        @out.write request
        @out.flush
        @out.write out_blob
        @out.flush

        response_len = @in.read(8).unpack('Q>').first
        got_blob = @in.read(response_len)
      end

      after_time = Time.now.to_f

      @report[[input_format, output_format]].add(after_time - before_time)

      got_image = from_blob(got_blob, output_format)
      reread_image = from_blob(out_blob, input_format)
      reread_image.format = got_image.format
      next if reread_image == got_image

      fuzzy_compare(reread_image, got_image)
    end
  end

  private
  def get_blob(image, format)
    if :hrl == format
      hrl = Hrl.new image
      return hrl.to_blob
    end

    image.class_type = Magick::DirectClass
    image.color_profile = nil
    image.format = format.to_s

    return image.to_blob do
      self.image_type = Magick::TrueColorType
      LLL.info self.image_type
    end
  end

  def from_blob(blob, format)
    if :hrl == format
      hrl = Hrl.new blob
      return hrl.to_image
    end

    i = Magick::Image.from_blob(blob).first
    assert_equal format.to_s, i.format.downcase
    return i
  end

  def assert_equal(expected, got, note="")
    return if expected == got

    die "expected #{expected.inspect[0..100]} but got #{got.inspect[0..100]} #{note}"
  end

  def fuzzy_compare(expected_image, got_image)
    assert_equal expected_image.rows, got_image.rows
    assert_equal expected_image.columns, got_image.columns

    mean_error, normalized_mean_error, normalized_max_error =
                                       expected_image.difference(got_image)

    return if normalized_mean_error < ALLOWABLE_DIFFERENCE

    LLL.fatal "Too much error"
    LLL.fatal "\tMean error: #{mean_error}"
    LLL.fatal "\tPercent: #{normalized_mean_error} (> #{ALLOWABLE_DIFFERENCE})"
    LLL.fatal "\tNormalized max error: #{normalized_max_error}"

    assert(false)
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

  def seed_rng!
    @seed = (ENV['SEED'] || Random.new_seed).to_i
    $stdout.puts "SEED=#{@seed}"
    $stdout.flush
    srand(@seed)
  end

  def pick_length!
    @length = (ENV['LENGTH'] || LENGTH_RANGE.to_a.sample).to_i
    $stdout.puts "LENGTH=#{@length}"
    $stdout.flush
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

    if example_filename = ENV['DUMP_EXAMPLE']
      $example_file = File.open(example_filename, 'w')
      class << @out
        def write_with_dump(data)
          $example_file.write data
          write_without_dump data
        end

        alias_method :write_without_dump, :write
        alias_method :write, :write_with_dump
      end
      at_exit do
        $example_file.close
      end
    end
  end
end

poller = Poller.new
at_exit do
  LLL.info (["SEED=#{poller.seed}",
             "LENGTH=#{poller.length}",
             "ruby", $0] +
            ARGV).join(' ')
end
poller.run!

$stderr.puts "activity\tcount\tmin s\tavg s\tmax s"
poller.report.each do |r|
  timing = r[1]
  $stderr.puts("%s\t%d\t%.4f\t%.4f\t%.4f" % [
                 r[0].map(&:to_s).join('->'),
                 timing.count, timing.min, timing.mean, timing.max
               ])
end

LLL.info "success :)"
