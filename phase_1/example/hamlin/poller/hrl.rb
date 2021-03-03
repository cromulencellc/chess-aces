require 'rmagick'

class Hrl
  MAGIC = 0x48524c65 # 'HRLe'

  attr_reader :pixels, :width, :height

  alias_method :columns, :width
  alias_method :rows, :height

  def initialize(content)
    if content.is_a? Magick::Image
      return initialize_from_image content
    end

    return initialize_from_string content
  end

  def to_blob
    @header_blob + @pixel_blob
  end

  def to_image
    geom = "#{width}x#{height}"
    i = Magick::Image.from_blob(pixels.flatten.map(&:chr).join) do |o|
      o.size = geom
      o.format = 'RGB'
      o.depth = 8
    end
    return i.first
  end

  def ==(other)
    return false unless other.is_a? self.class
    return false unless other.width == width
    return false unless other.height == height
    return false unless other.pixels == pixels

    return true
  end

  private

  def initialize_from_image(image)
    @width = image.columns
    @height = image.rows
    image.color_profile = nil
    image.format = 'RGB'
    blob = image.to_blob

    @pixels = blob.each_byte.each_slice(3).to_a

    red_counts = @pixels.inject(Array.new(256, 0)) do |counts, cur|
      counts[cur[0]] += 1
      counts
    end

    least_popular_count = image.rows * image.columns
    least_popular_value = 0

    red_counts.each_with_index do |count, value|
      if count < least_popular_count
        least_popular_count = count
        least_popular_value = value
      end
    end

    @sigil = least_popular_value

    LLL.info "sigil #{@sigil} with count #{least_popular_count}"

    @header_blob = [MAGIC, image.columns, image.rows, @sigil].pack('NNNC')

    @pixel_blob = ''

    total_pixels = @width * @height
    current_pixel = 0

    while (total_pixels > current_pixel)
      cur_color = @pixels[current_pixel]

      if (current_pixel + 1) < total_pixels
        next_color = @pixels[current_pixel + 1]
        if cur_color == next_color
          run_count = 1
          while cur_color == next_color
            run_count += 1
            break if 255 == run_count
            next_color = @pixels[current_pixel + run_count]
          end

          @pixel_blob << ([@sigil, run_count] + cur_color).pack('CCCCC')
          current_pixel += run_count
          next
        end
      end

      if (@sigil == cur_color.first)
        @pixel_blob << ([@sigil, 1] + cur_color).pack('CCCCC')
        current_pixel += 1
        next
      end

      @pixel_blob << cur_color.pack('CCC')
      current_pixel += 1
    end
  end

  def initialize_from_string(string)
    @header_blob = string[0..12]
    @pixel_blob = string[13..-1]

    magic, @width, @height, @sigil = @header_blob.unpack('NNNC')
    if magic != MAGIC
      message = "got unexpected magic number #{magic} for HRLe"
      LLL.fatal message
      fail message
    end

    pixel_count = @width * @height
    remaining_pixels = pixel_count

    @pixels = Array.new

    pixel_bytes = @pixel_blob.bytes

    cursor = 0

    while remaining_pixels > 0
      sentry = pixel_bytes[cursor]
      # LLL.debug("sentry #{sentry}")
      if @sigil == sentry
        _sigil, run_count, r, g, b = pixel_bytes[cursor..cursor+4]
        # LLL.debug("run #{pixel_bytes[cursor..(cursor+4)]}")
        run_count.times do
          @pixels.push r, g, b
        end

        remaining_pixels -= run_count
        cursor += 5
      else
        # LLL.debug("normal #{pixel_bytes[cursor..(cursor+2)]}")
        @pixels.push *pixel_bytes[cursor..(cursor+2)]
        remaining_pixels -= 1
        cursor += 3
      end
    end
    LLL.debug("loaded (#{columns}x#{rows}) from #{@pixel_blob.size} blob for #{@pixels.size / 3}px")
  end
end
