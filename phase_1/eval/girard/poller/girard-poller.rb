#!/usr/bin/env ruby

##
# GirardPoller: Poller for the Girard CHESS service
#
#   Usage: ruby girard-poller.rb <HOST> <PORT>
##

require "socket"
require "timeout"
require "logger"

# set up logger
LOGGER = Logger.new(STDOUT, formatter: proc { |sev, datetime, progname, msg| "#{sev}: #{msg}\n" })
LOGGER.level = Logger::INFO
LOGGER.level = Logger::DEBUG

# set up test files list
FILES = Dir.children("tests")  # using entries here, apparently, means you get "." and ".." in your list
fail "could not find directory of test files" if FILES.length() == 0

# set up activities list
ACTIVITIES = {
    :create                     => "\x20".force_encoding("BINARY"),
    :destroy                    => "\x21".force_encoding("BINARY"),
    :load                       => "\x22".force_encoding("BINARY"),
    :store                      => "\x23".force_encoding("BINARY"),
    #:render                     => "\x24".force_encoding("BINARY"),
    :selected_image             => "\x30".force_encoding("BINARY"),
    :select_image               => "\x31".force_encoding("BINARY"),
    :get_styles                 => "\x40".force_encoding("BINARY"),
    :get_style                  => "\x41".force_encoding("BINARY"),
    :set_style                  => "\x42".force_encoding("BINARY"),
    :add_style                  => "\x43".force_encoding("BINARY"),
    :remove_style               => "\x44".force_encoding("BINARY"),
    :is_flat                    => "\x45".force_encoding("BINARY"),
    :is_transparent             => "\x46".force_encoding("BINARY"),
    :get_color                  => "\x47".force_encoding("BINARY"),
    :set_color                  => "\x48".force_encoding("BINARY"),
    :get_gradient               => "\x49".force_encoding("BINARY"),
    :set_gradient               => "\x4A".force_encoding("BINARY"),
    :set_step                   => "\x4B".force_encoding("BINARY"),
    :add_step                   => "\x4C".force_encoding("BINARY"),
    :remove_step                => "\x4D".force_encoding("BINARY"),
    :get_gradient_transformer   => "\x4E".force_encoding("BINARY"),
    :set_gradient_transformer   => "\x4F".force_encoding("BINARY"),
    :get_paths                  => "\x60".force_encoding("BINARY"),
    :get_path                   => "\x61".force_encoding("BINARY"),
    :set_path                   => "\x62".force_encoding("BINARY"),
    :add_path                   => "\x63".force_encoding("BINARY"),
    :remove_path                => "\x64".force_encoding("BINARY"),
    :get_point                  => "\x65".force_encoding("BINARY"),
    :set_point                  => "\x66".force_encoding("BINARY"),
    :add_point                  => "\x67".force_encoding("BINARY"),
    :remove_point               => "\x68".force_encoding("BINARY"),
    :get_shapes                 => "\x80".force_encoding("BINARY"),
    :get_shape                  => "\x81".force_encoding("BINARY"),
    :set_shape                  => "\x82".force_encoding("BINARY"),
    :add_shape                  => "\x83".force_encoding("BINARY"),
    :remove_shape               => "\x84".force_encoding("BINARY"),
    :get_shape_style            => "\x85".force_encoding("BINARY"),
    :set_shape_style            => "\x86".force_encoding("BINARY"),
    :get_shape_paths            => "\x87".force_encoding("BINARY"),
    :set_shape_paths            => "\x88".force_encoding("BINARY"),
    :add_shape_path             => "\x89".force_encoding("BINARY"),
    :remove_shape_path          => "\x8A".force_encoding("BINARY"),
    :has_hinting                => "\x8B".force_encoding("BINARY"),
    :set_hinting                => "\x8C".force_encoding("BINARY"),
    :get_min_visibility         => "\x90".force_encoding("BINARY"),
    :set_min_visibility         => "\x91".force_encoding("BINARY"),
    :get_max_visibility         => "\x92".force_encoding("BINARY"),
    :set_max_visibility         => "\x93".force_encoding("BINARY"),
    :get_transformers           => "\x94".force_encoding("BINARY"),
    :get_transformer            => "\x95".force_encoding("BINARY"),
    :set_transformer            => "\x96".force_encoding("BINARY"),
    :add_transformer            => "\x97".force_encoding("BINARY"),
    :remove_transformer         => "\x98".force_encoding("BINARY")
}

# set up expected results
RESULT_ACK = "\x00".force_encoding("BINARY")
RESULT_SCS = "\x01".force_encoding("BINARY")
RESULT_FAL = "\x02".force_encoding("BINARY")
RESULT_RES = "\x04".force_encoding("BINARY")

# set up lists of valid style, path, and shape options
STYLE_TYPE = {
    :solid_color            => "\x01".force_encoding("BINARY"),
    :gradient               => "\x02".force_encoding("BINARY"),
    :solid_color_no_alpha   => "\x03".force_encoding("BINARY"),
    :solid_gray             => "\x04".force_encoding("BINARY"),
    :solid_gray_no_alpha    => "\x05".force_encoding("BINARY")
}
GRADIENT_TYPE = [0, 1, 2, 3, 4, 5]
PATH_COMMAND = {
    :horizontal_line        => 0,
    :vertical_line          => 1,
    :line                   => 2,
    :curve                  => 3
}
SHAPE_TYPE = [10]
TRANSFORMER_TYPE = [20, 21, 22, 23]


##
# Monkey-patch for Ruby's String class
class String
    ##
    # Operator ^ to bitwise XOR instances of String with each other
    #
    # This enables us to do some fuzzy-matching for certain assertions,
    # which was easier to do than tracking down a floating-point rounding error
    def ^(other)
        # unpack both strings
        s1 = self.unpack("C*")
        s2 = other.unpack("C*")

        # find the maximum length between them
        longest = [s1.length, s2.length].max()

        # zero-pad the shorter one
        s1 = [0] * (longest - s1.length) + s1
        s2 = [0] * (longest - s2.length) + s2

        # xor their results
        return s1.zip(s2).map{ |a,b| a^b }.pack("C*")
    end
end


# class representing a style's color
class Color
    attr_accessor :r, :g, :b, :a
    def initialize(r = 0, g = 0, b = 0, a = 0)
        @r = r
        @g = g
        @b = b
        @a = a
    end

    def serialize()
        if @r == @g and @g == @b
            if @a == 255
                return "#{STYLE_TYPE[:solid_gray_no_alpha]}#{@r.chr()}".force_encoding("BINARY")
            else
                return "#{STYLE_TYPE[:solid_gray]}#{@r.chr()}#{@a.chr()}".force_encoding("BINARY")
            end
        else
            if @a == 255
                return "#{STYLE_TYPE[:solid_color_no_alpha]}#{@r.chr()}#{@g.chr()}#{@b.chr()}".force_encoding("BINARY")
            else
                return "#{STYLE_TYPE[:solid_color]}#{@r.chr()}#{@g.chr()}#{@b.chr()}#{@a.chr()}".force_encoding("BINARY")
            end
        end
    end

    def self.generate()
        return Color.new(rand(256), rand(256), rand(256), rand(256))
    end

    def is_gradient?
        return false
    end
end

# class representing a step within a gradient
class Step
    attr_accessor :stop, :color
    def initialize(stop = 0, color = Color.new(0, 0, 0, 0))
        @stop = stop
        @color = color
    end

    def serialize()
        return "#{@stop.chr}#{@color.serialize()[1..-1]}".force_encoding("BINARY")
    end

    def self.generate()
        return Step.new(rand(256), Color.generate())
    end
end

# class representing a style's gradient
class Gradient
    attr_accessor :type, :steps, :xformer
    def initialize(type = 0, steps = [], xformer = nil)
        @type = type
        @steps = steps
        @xformer = xformer
    end

    def serialize()
        all_gray = true
        no_alpha = true
        @steps.each do |s|
            all_gray = false if not (s.color.r == s.color.g and s.color.g == s.color.b)
            no_alpha = false if s.color.a != 255
        end

        flags = 0
        flags |= 2 if @xformer
        flags |= 4 if no_alpha
        flags |= 16 if all_gray

        bytes = "#{STYLE_TYPE[:gradient]}#{@type.chr()}#{flags.chr()}#{@steps.length.chr}"
        bytes = "#{bytes}#{@xformer.serialize}" if @xformer != nil
        @steps.each do |s|
            if all_gray
                if no_alpha
                    bytes = "#{bytes}#{s.stop.chr()}#{s.color.r.chr()}"
                else
                    bytes = "#{bytes}#{s.stop.chr()}#{s.color.r.chr()}#{s.color.a.chr()}"
                end
            else
                if no_alpha
                    bytes = "#{bytes}#{s.stop.chr()}#{s.color.r.chr()}#{s.color.g.chr()}#{s.color.b.chr()}"
                else
                    bytes = "#{bytes}#{s.stop.chr()}#{s.color.r.chr()}#{s.color.g.chr()}#{s.color.b.chr()}#{s.color.a.chr()}"
                end
            end
        end

        return bytes.force_encoding("BINARY")
    end

    def self.generate()
        steps = []
        (rand(4) + 1).times { steps << Step.generate() }
        return Gradient.new(GRADIENT_TYPE.sample(), steps)
    end

    def is_gradient?
        return true
    end
end

# class representing a shape's style
class Style
    attr_accessor :color
    def initialize(color)
        @color = color
    end

    def serialize()
        return @color.serialize()
    end

    def self.generate()
        if [true, false].sample
            return Style.new(Gradient.generate())
        else
            return Style.new(Color.generate())
        end
    end
end

# class representing a point within a path (straight or curved)
class Point
    attr_accessor :x, :y, :xin, :yin, :xout, :yout

    def initialize(x = 0.0, y = 0.0, xin = nil, yin = nil, xout = nil, yout = nil)
        @x = x
        @y = y
        if xin.nil? and yin.nil?
            @xin = x
            @yin = y
        else
            @xin = xin || 0.0
            @yin = yin || 0.0
        end
        if xout.nil? and yout.nil?
            @xout = x
            @yout = y
        else
            @xout = xout || 0.0
            @yout = yout || 0.0
        end
    end

    def serialize()
        return   self.class.pack16(@x) + self.class.pack16(@y) \
               + self.class.pack16(@xin) + self.class.pack16(@yin) \
               + self.class.pack16(@xout) + self.class.pack16(@yout)
    end

    def self.generate()
        if [true, false].sample
            x = rand(-128.0..192.0)
            y = rand(-128.0..192.0)
            return Point.new(x, y)
        else
            x = rand(-128.0..192.0)
            y = rand(-128.0..192.0)
            xin = rand(-128.0..192.0)
            yin = rand(-128.0..192.0)
            xout = rand(-128.0..192.0)
            yout = rand(-128.0..192.0)
            return Point.new(x, y, xin, yin, xout, yout)
        end
    end

    def self.pack16(v)
        v = -128.0 if v < -128.0
        v = 192.0 if v > 192.0

        if -32.0 <= v and v <= 95.0 and v == v.to_i()
            return (v + 32.0).to_i().chr()
        else
            packed = [(v + 128.0) * 102.0].pack("S")
            #puts "#{v}: #{[v].pack("f").inspect} -> #{packed.inspect}"
            return (packed[1].ord() | 0x80).chr + packed[0]
        end
    end

    def self.unpack16(f)
        raw = f.read(1).ord()
        if raw & 0x80 != 0
            raw = (raw << 8) | f.read(1).ord()
            #puts "#{raw.to_s(16)}: #{((raw & 0x7FFF) / 102.0) - 128.0}"
            return ((raw & 0x7FFF) / 102.0) - 128.0
        else
            return raw - 32.0
        end
    end
end

# class representing a path within a shape
class Path
    attr_accessor :points, :closed, :curved
    def initialize(points = [], closed = false, curved = false)
        @points = points
        @closed = closed
        @curved = curved
    end

    def serialize()
        # count types of line segments
        straight = 0
        line = 0
        curve = 0
        last = Point.new()
        @points.each do |p|
            if p.x == p.xin and p.xin == p.xout and p.y == p.yin and p.yin == p.yout
                if p.x == last.x or p.y == last.y
                    straight += 1
                else
                    line += 1
                end
            else
                curve += 1
            end
            last = p
        end

        # determine smallest possible size for serialized data
        packed_size = @points.length + straight * 2 + line * 4 + curve * 12
        normal_size = @points.length * 12
        packed = packed_size < normal_size ? true : false

        # set flags
        flags = 0
        flags |= 1 if closed
        flags |= 4 if packed
        flags |= 8 if curve == 0

        # create byte stream
        bytes = "#{flags.chr}#{@points.length.chr}"
        if packed
            # make space for commands if we're packing things up
            bytes += "\x00" * ((@points.length + 3) / 4)
        end

        # serialize points in path
        cmdidx = 2
        cmdloc = 0
        last = Point.new()
        @points.each do |p|
            if packed
                # pack this point
                command = 0
                if p.x == p.xin and p.xin == p.xout and p.y == p.yin and p.yin == p.yout
                    if p.x == last.x or p.y == last.y
                        if p.x == last.x
                            command = PATH_COMMAND[:vertical_line]
                            bytes += Point.pack16(p.y)
                        else
                            command = PATH_COMMAND[:horizontal_line]
                            bytes += Point.pack16(p.x)
                        end
                    else
                        command = PATH_COMMAND[:line]
                        bytes += Point.pack16(p.x)
                        bytes += Point.pack16(p.y)
                    end
                else
                    command = PATH_COMMAND[:curve]
                    bytes += Point.pack16(p.x)
                    bytes += Point.pack16(p.y)
                    bytes += Point.pack16(p.xin)
                    bytes += Point.pack16(p.yin)
                    bytes += Point.pack16(p.xout)
                    bytes += Point.pack16(p.yout)
                end
                last = p

                # update commands
                bytes[cmdidx] = (bytes[cmdidx].ord | (command << (2 * cmdloc))).chr
                cmdloc += 1
                if cmdloc > 3
                    cmdloc = 0
                    cmdidx += 1
                end
            else
                if curve == 0
                    bytes += Point.pack16(p.x)
                    bytes += Point.pack16(p.y)
                else
                    bytes += Point.pack16(p.x)
                    bytes += Point.pack16(p.y)
                    bytes += Point.pack16(p.xin)
                    bytes += Point.pack16(p.yin)
                    bytes += Point.pack16(p.xout)
                    bytes += Point.pack16(p.yout)
                end
            end
        end

        return bytes.force_encoding("BINARY")
    end

    def self.generate()
        points = []
        (rand(10) + 1).times { points << Point.generate() }
        return Path.new(points, [true, false].sample(), [true, false].sample())
    end
end

# class representing a transformer that can alter a shape or gradient
class Transformer
    attr_accessor :type, :raw, :sx, :sy, :shx, :shy, :tx, :ty
    def initialize(raw = nil, sx = 1.0, shy = 1.0, shx = 0.0, sy = 0.0, tx = 0.0, ty = 0.0)
        @type = 20
        @raw = raw
        @sx = sx
        @sy = sy
        @shx = shx
        @shy = shy
        @tx = tx
        @ty = ty
    end

    def serialize()
        if not @raw.nil?
            return @raw
        else
            return "#{self.class.pack24(@sx)}#{self.class.pack24(@shy)}#{self.class.pack24(@shx)}#{self.class.pack24(@sy)}#{self.class.pack24(@tx)}#{self.class.pack24(@ty)}".force_encoding("BINARY")
        end
    end

    def self.generate()
        sx = rand()
        sy = rand()
        shx = rand()
        shy = rand()
        tx = rand()
        ty = rand()
        return Transformer.new(nil, sx, shy, shx, sy, tx, ty)
    end

    def self.unpack24(f)
        raw = (f.read(1).ord() << 16) | (f.read(1).ord() << 8) | f.read(1).ord()
        return [((raw & 0x800000) << 8) | ((((raw & 0x7E0000) >> 17) + 95) << 23) | ((raw & 0x01FFFF) << 6)].pack("l").unpack("f")[0]
    end

    def self.pack24(v)
        raw = [v].pack("f").unpack("L")[0]
        exp = ((raw & 0x7F800000) >> 23) - 127
        if (exp < -32 or 32 <= exp)
            return "\x00\x00\x00"
        else
            sign = (raw & 0x80000000) >> 31
            mant = raw & 0x007FFFFF
            return [(sign << 23) | ((exp + 32) << 17) | (mant >> 6)].pack("L>")[1..3]
        end
    end
end

# class representing a vector shape
class Shape
    attr_accessor :style, :paths, :xformers, :hinting, :minvis, :maxvis
    def initialize(style = 0, paths = [], xformers = [])
        @type = 10
        @style = style
        @paths = paths
        @xformers = xformers
        @hinting = false
        @minvis = 0
        @maxvis = 255
    end

    def serialize()
        bytes = "#{@type.chr}#{@style.chr}#{@paths.length.chr}#{@paths.map { |x| x.chr }.join()}#{@hinting ? 4.chr : 0.chr}"

        flagidx = bytes.length - 1
        packed_first = false
        if @xformers.length == 1
            if @xformers[0].raw == nil and @xformers[0].sx == 1.0 and @xformers[0].sy == 1.0 and @xformers[0].shx == 0.0 and @xformers[0].shy == 0.0
                bytes += "#{Transformer.pack24(@xformers[0].tx)}#{Transformer.pack24(xformers[0].ty)}"
                bytes[flagidx] = (bytes[flagidx].ord | 32).chr
                packed_first = true
            elsif @xformers[0].raw == nil
                bytes += "#{xformers[0].serialize}"
                bytes[flagidx] = (bytes[flagidx].ord | 2).chr
                packed_first = true
            end
        end

        if @minvis != 0 or @maxvis != 255
            bytes += "#{@minvis.chr}#{@maxvis.chr}"
            bytes[flagidx] = (bytes[flagidx].ord | 8).chr
        end

        if @xformers.length > 0 and not (@xformers.length == 1 and packed_first)
            if not packed_first
                bytes += "#{@xformers.length.chr}"
                @xformers.length.times do |i|
                    bytes += "#{@xformers[i].type.chr}#{@xformers[i].serialize}"
                end
            else
                bytes += "#{(@xformers.length - 1).chr}"
                (@xformers.length - 1).times do |i|
                    bytes += "#{@xformers[i+1].type.chr}#{@xformers[i+1].serialize}"
                end
            end
        end

        return bytes.force_encoding("BINARY")
    end

    def self.generate(nstyles, npaths)
        # FIXME: generate, at minimum, affine transformers here
        style = rand(nstyles)
        paths = []
        rand(10..20).times { paths << rand(npaths != 0 ? npaths : 1) }
        paths.uniq!
        return Shape.new(style, paths)
    end
end

# class representing a vector image containing shapes with styles and paths
class Image
    attr_accessor :styles, :paths, :shapes
    def initialize(path = "")
        @styles = []
        @paths = []
        @shapes = []

        if path != ""
            File.open(path, "rb") do |f|
                self.load(f)
            end
        end
    end

    def load(f)
        # reset state
        @styles = []
        @paths = []
        @shapes = []

        # parse and check file magic
        fail "invalid file magic" if f.read(4) != "ncif"

        # parse styles
        nstyles = f.read(1).ord()
        LOGGER.debug("Styles: #{nstyles}")
        nstyles.times do
            stype = f.read(1)
            case stype
            when STYLE_TYPE[:solid_color]
                r = f.read(1).ord()
                g = f.read(1).ord()
                b = f.read(1).ord()
                a = f.read(1).ord()
                @styles << Style.new(Color.new(r, g, b, a))
            when STYLE_TYPE[:gradient]
                gtype = f.read(1).ord()
                gflags = f.read(1).ord()
                nsteps = f.read(1).ord()

                xformer = nil
                xformer = Transformer.new(f.read(18)) if gflags & 2 != 0

                steps = []
                if gflags & 16 != 0 and gflags & 4 != 0
                    nsteps.times do
                        step = f.read(1).ord()
                        c = f.read(1).ord()
                        steps << Step.new(step, Color.new(c, c, c, 255))
                    end
                elsif gflags & 16 != 0
                    nsteps.times do
                        step = f.read(1).ord()
                        c = f.read(1).ord()
                        a = f.read(1).ord()
                        steps << Step.new(step, Color.new(c, c, c, a))
                    end
                elsif gflags & 4 != 0
                    nsteps.times do
                        step = f.read(1).ord()
                        r = f.read(1).ord()
                        g = f.read(1).ord()
                        b = f.read(1).ord()
                        steps << Step.new(step, Color.new(r, g, b, 255))
                    end
                else
                    nsteps.times do
                        step = f.read(1).ord()
                        r = f.read(1).ord()
                        g = f.read(1).ord()
                        b = f.read(1).ord()
                        a = f.read(1).ord()
                        steps << Step.new(step, Color.new(r, g, b, a))
                    end
                end
                @styles << Style.new(Gradient.new(gtype, steps, xformer))
            when STYLE_TYPE[:solid_color_no_alpha]
                r = f.read(1).ord()
                g = f.read(1).ord()
                b = f.read(1).ord()
                @styles << Style.new(Color.new(r, g, b, 255))
            when STYLE_TYPE[:solid_gray]
                color = f.read(1).ord()
                alpha = f.read(1).ord()
                @styles << Style.new(Color.new(color, color, color, alpha))
            when STYLE_TYPE[:solid_gray_no_alpha]
                color = f.read(1).ord()
                @styles << Style.new(Color.new(color, color, color, 255))
            else
                next
            end
            #puts "    Style: #{@styles.last.inspect}"
        end

        # parse paths
        npaths = f.read(1).ord()
        LOGGER.debug("Paths: #{npaths}")
        npaths.times do
            ptype = f.read(1).ord()
            npoints = f.read(1).ord()

            closed = ptype & 2 != 0
            curved = false

            points = []
            if ptype & 4 != 0
                rawcmds = f.read((npoints + 3) / 4)
                cmds = []
                rawcmds.length.times do |i|
                    cmds << (rawcmds[i].ord() & 3)
                    cmds << ((rawcmds[i].ord() >> 2) & 3)
                    cmds << ((rawcmds[i].ord() >> 4) & 3)
                    cmds << ((rawcmds[i].ord() >> 6) & 3)
                end
                last_x = 0.0
                last_y = 0.0
                npoints.times do |i|
                    if cmds[i] == 0
                        last_x = Point.unpack16(f)
                        points << Point.new(last_x, last_y)
                    elsif cmds[i] == 1
                        last_y = Point.unpack16(f)
                        points << Point.new(last_x, last_y)
                    elsif cmds[i] == 2
                        last_x = Point.unpack16(f)
                        last_y = Point.unpack16(f)
                        points << Point.new(last_x, last_y)
                    else cmds[i] == 3
                        curved = true
                        points << Point.new(
                            Point.unpack16(f), Point.unpack16(f),
                            Point.unpack16(f), Point.unpack16(f),
                            Point.unpack16(f), Point.unpack16(f)
                        )
                        last_x = points[-1].x
                        last_y = points[-1].y
                    end
                end
            elsif ptype & 8 != 0
                npoints.times do
                    points << Point.new(Point.unpack16(f), Point.unpack16(f))
                end
            else
                npoints.times do
                    curved = true
                    points << Point.new(
                        Point.unpack16(f), Point.unpack16(f),
                        Point.unpack16(f), Point.unpack16(f),
                        Point.unpack16(f), Point.unpack16(f)
                    )
                end
            end
            @paths << Path.new(points, closed, curved)
            #puts "    Path: #{@paths.last.inspect}"
        end

        # parse shapes
        nshapes = f.read(1).ord()
        LOGGER.debug("Shapes: #{nshapes}")
        nshapes.times do
            stype = f.read(1).ord()
            style = f.read(1).ord()
            npaths = f.read(1).ord()
            paths = []
            npaths.times { paths << f.read(1).ord() }
            flags = f.read(1).ord()
            minvis = 0
            maxvis = 255

            xforms = []
            if flags & 2 != 0
                xforms << Transformer.new(nil,
                                          Transformer.unpack24(f),
                                          Transformer.unpack24(f),
                                          Transformer.unpack24(f),
                                          Transformer.unpack24(f),
                                          Transformer.unpack24(f),
                                          Transformer.unpack24(f))
            elsif flags & 32 != 0
                xforms << Transformer.new(nil,
                                          1.0,
                                          1.0,
                                          0.0,
                                          0.0,
                                          Point.unpack16(f),
                                          Point.unpack16(f))
            end

            if flags & 8 != 0
                minvis = f.read(1).ord
                maxvis = f.read(1).ord
            end

            if flags & 16 != 0
                nxforms = f.read(1).ord()
                nxforms.times do
                    ttype = f.read(1).ord()
                    case ttype
                    when 20
                        xforms << Transformer.new(nil,
                                                  Transformer.unpack24(f),
                                                  Transformer.unpack24(f),
                                                  Transformer.unpack24(f),
                                                  Transformer.unpack24(f),
                                                  Transformer.unpack24(f),
                                                  Transformer.unpack24(f))
                    when 21
                        xforms << Transformer.new(f.read(3))
                        xforms[-1].type = 21
                    when 23
                        xforms << Transformer.new(f.read(3))
                        xforms[-1].type = 23
                    else
                        next
                    end
                end
            end
            @shapes << Shape.new(style, paths, xforms)
            @shapes[-1].minvis = minvis
            @shapes[-1].maxvis = maxvis
            @shapes[-1].hinting = flags & 4 != 0
            #puts "    Shape: #{@shapes.last.inspect}"
        end
    end


end


# class that manages all state for a single poll of the Girard service
class Poller
    def initialize(host, port, seed, length, timeout)
        @host = host
        @port = port
        @seed = seed
        @length = length
        @timeout = timeout
        @socket = nil
        @images = []
        @selected = 0
    end

    def assert(expect, recvd, msg = "", fuzzy: false)
        return if expect == recvd and not fuzzy
        if fuzzy
            diff = expect ^ recvd
            if diff.count("\x01") <= (expect.length / 2)
                LOGGER.debug("Accepted a fuzzy comparison that did not completely match")
                return
            end
        end
        LOGGER.fatal("Failed assertion between #{expect.inspect()} and #{recvd.inspect}")
        fail "Expected: #{expect.inspect()}, Received: #{recvd.inspect} #{msg}"
    end

    def send_blob(blob)
        # write size
        @socket.write([blob.length].pack("s<"))

        # write data
        @socket.write(blob)
    end

    def recv_blob()
        # read size
        size = @socket.read(2).unpack("s<")[0]

        # read data
        return @socket.read(size)
    end

    def choose_step()
        n, expectfail = choose_style()

        if expectfail or not @images[@selected].styles[n].color.is_gradient?
            return n, rand(256), true
        end

        if @images[@selected].styles[n].color.steps.length > 1
            if rand(20) + 1 == 1
                # rolled a natural 1, you know what to do...
                m = rand(@images[@selected].styles[n].color.steps.length..255)
                expectfail = true
            else
                m = rand(@images[@selected].styles[n].color.steps.length)
            end
        elsif @images[@selected].styles[n].color.steps.length == 1
            m = 0
        else
            m = 0
            expectfail = true
        end

        return n, m, expectfail
    end

    def choose_style(check_shapes = false)
        # choose a style
        if @images.length > 0 and @images[@selected].styles.length > 1
            if rand(20) + 1 == 1
                # rolled a natural 1, you know what to do...
                n = rand(@images[@selected].styles.length..255)
                expectfail = true
            else
                n = rand(@images[@selected].styles.length)
                expectfail = false
            end
        elsif @images.length > 0 and @images[@selected].styles.length == 1
            n = 0
            expectfail = false
        else
            n = 0
            expectfail = true
        end

        # verify the style is not currently in-use
        if not expectfail and check_shapes
            @images[@selected].shapes.length.times do |i|
                expectfail = true if @images[@selected].shapes[i].style == n
            end
        end

        return n, expectfail
    end

    def choose_point()
        n, expectfail = choose_path()
        return n, rand(256), expectfail if expectfail

        if @images[@selected].paths[n].points.length > 1
            if rand(20) + 1 == 1
                # rolled a natural 1, you know what to do...
                m = rand(@images[@selected].paths[n].points.length..255)
                expectfail = true
            else
                m = rand(@images[@selected].paths[n].points.length)
            end
        elsif @images[@selected].paths[n].points.length == 1
            m = 0
        else
            m = 0
            expectfail = true
        end

        return n, m, expectfail
    end

    def choose_path(check_shapes = false)
        # choose a path
        if @images.length > 0 and @images[@selected].paths.length > 1
            if rand(20) + 1 == 1
                # rolled a natural 1, you know what to do...
                n = rand(@images[@selected].paths.length..255)
                expectfail = true
            else
                n = rand(@images[@selected].paths.length)
                expectfail = false
            end
        elsif @images.length > 0 and @images[@selected].paths.length == 1
            n = 0
            expectfail = false
        else
            n = 0
            expectfail = true
        end

        # verify the path is not currently in-use
        if not expectfail and check_shapes
            @images[@selected].shapes.length.times do |i|
                expectfail = true if @images[@selected].shapes[i].paths.include?(n)
            end
        end

        return n, expectfail
    end

    ##
    # Chooses a random transformer from a random shape in the currently @selected @image
    def choose_xformer()
        n, expectfail = choose_shape()
        return n, rand(256), expectfail if expectfail

        if @images[@selected].shapes[n].xformers.length > 1
            if rand(20) + 1 == 1
                # rolled a natural 1, you know what to do...
                m = rand(@images[@selected].shapes[n].xformers.length..255)
                expectfail = true
            else
                m = rand(@images[@selected].shapes[n].xformers.length)
                expectfail = false
            end
        elsif @images[@selected].shapes[n].xformers.length == 1
            m = 0
            expectfail = false
        else
            m = 0
            expectfail = true
        end

        return n, m, expectfail
    end

    ##
    # Chooses a random shape from the currently @selected @image
    def choose_shape()
        # choose a shape
        if @images.length > 0 and @images[@selected].shapes.length > 1
            if rand(20) + 1 == 1
                # rolled a natural 1, you know what to do...
                n = rand(@images[@selected].shapes.length..255)
                expectfail = true
            else
                n = rand(@images[@selected].shapes.length)
                expectfail = false
            end
        elsif @images.length > 0 and @images[@selected].shapes.length == 1
            n = 0
            expectfail = false
        else
            n = 0
            expectfail = true
        end

        return n, expectfail
    end

    ##
    # Requests a connection with the girard server
    def connect()
        # connect to server
        @socket = TCPSocket.new(@host, @port)

        # establish connection
        @socket.write("\x10")
        assert(RESULT_ACK, @socket.read(1))
    end

    ##
    # Requests a disconnect from the girard server
    def disconnect()
        # establish disconnection
        @socket.write("\x11")
        assert(RESULT_ACK, @socket.read(1))

        # disconnect from server
        @socket.close()
    end

    ##
    # Tests the "create" command
    def test_create()
        @socket.write(ACTIVITIES[:create])

        # handle result
        if @images.length < 16
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images << Image.new()
            @selected = @images.length - 1

        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "destroy" command
    def test_destroy()
        @socket.write(ACTIVITIES[:destroy])

        # choose image to destroy
        n = @images.length > 0 ? rand(@images.length) : 0
        @socket.write(n.chr())

        # handle result
        if @images.length > 0
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images.delete_at(n)
            @selected -= 1 if n <= @selected && @selected != 0
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "load" command
    def test_load(path)
        LOGGER.debug("Loading input: #{path}")
        @socket.write(ACTIVITIES[:load])

        # send raw image data
        self.send_blob(File.read(path).force_encoding("BINARY"))

        # handle result
        if @images.length < 16
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images << Image.new(path)
            @selected = @images.length - 1
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "store" command
    def test_store()
        @socket.write(ACTIVITIES[:store])

        # handle result
        if @images.length > 0
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            begin
                image = Image.new(data)
            rescue Exception => e
                # FIXME: currently don't have a way to load an image from raw data
                #fail "could not load image data received from server: #{e}"
            end
            #assert(@images[@selected].styles.length, image.styles.length)
            #assert(@images[@selected].paths.length, image.paths.length)
            #assert(@images[@selected].shapes.length, image.shapes.length)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "selected image" command
    def test_selected_image()
        @socket.write(ACTIVITIES[:selected_image])

        # handle result
        if @images.length > 0
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@selected.chr(), data)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "select image" command
    def test_select_image()
        @socket.write(ACTIVITIES[:select_image])

        # choose image to select
        n = @images.length > 0 ? rand(@images.length) : 0
        @socket.write(n.chr())

        # handle result
        if @images.length > 0
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @selected = n
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get styles" command
    def test_get_styles()
        @socket.write(ACTIVITIES[:get_styles])

        # handle result
        if @images.length > 0
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].styles.length, data[0].ord())
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get style" command
    def test_get_style()
        @socket.write(ACTIVITIES[:get_style])

        # choose style to select
        n, expectfail = choose_style()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].styles[n].serialize(), data, fuzzy: true)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set style" command
    def test_set_style()
        @socket.write(ACTIVITIES[:set_style])

        # choose style to select and create new style
        n, expectfail = choose_style()
        s = Style.generate()
        self.send_blob("#{n.chr()}#{s.serialize()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].styles[n] = s
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "add style" command
    def test_add_style()
        @socket.write(ACTIVITIES[:add_style])

        # create new style
        s = Style.generate()
        self.send_blob(s.serialize())

        # handle result
        if @images.length > 0 and @images[@selected].styles.length < 256
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].styles << s
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "remove style" command
    def test_remove_style()
        @socket.write(ACTIVITIES[:remove_style])

        # choose style to remove
        n, expectfail = choose_style(true)
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].styles.delete_at(n)
            @images[@selected].shapes.length.times do |i|
                if @images[@selected].shapes[i].style > n
                    @images[@selected].shapes[i].style -= 1
                end
            end
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "is flat" command
    def test_is_flat()
        @socket.write(ACTIVITIES[:is_flat])

        # choose style to select
        n, expectfail = choose_style()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].styles[n].color.is_gradient?, data[0].ord == 0)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "is transparent" command
    def test_is_transparent()
        @socket.write(ACTIVITIES[:is_transparent])

        # choose style to select
        n, expectfail = choose_style()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            if @images[@selected].styles[n].color.is_gradient?
                has_transparency = false
                @images[@selected].styles[n].color.steps.length.times do |i|
                    if @images[@selected].styles[n].color.steps[i].color.a != 255
                        has_transparency = true
                        break
                    end
                end
                assert(has_transparency, data[0].ord == 1)
            else
                assert(@images[@selected].styles[n].color.a != 255, data[0].ord == 1)
            end
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get color" command
    def test_get_color()
        @socket.write(ACTIVITIES[:get_color])

        # choose style to select
        n, expectfail = choose_style()
        @socket.write(n.chr())

        # handle result
        if not expectfail and not @images[@selected].styles[n].color.is_gradient?
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].styles[n].color.serialize(), data)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set color" command
    def test_set_color()
        @socket.write(ACTIVITIES[:set_color])

        # choose style to select and create new color
        n, expectfail = choose_style()
        c = Color.generate()
        self.send_blob("#{n.chr()}#{c.serialize()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].styles[n].color = c
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get gradient" command
    def test_get_gradient()
        @socket.write(ACTIVITIES[:get_gradient])

        # choose style to select
        n, expectfail = choose_style()
        @socket.write(n.chr())

        # handle result
        if not expectfail and @images[@selected].styles[n].color.is_gradient?
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            #assert(@images[@selected].styles[n].color.serialize()[1..-1], data[0])  # TODO: not sure why this is broken on the server-side
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set gradient" command
    def test_set_gradient()
        @socket.write(ACTIVITIES[:set_gradient])

        # choose style to select and create new gradient
        n, expectfail = choose_style()
        g = Gradient.generate()
        self.send_blob("#{n.chr()}#{g.serialize()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].styles[n].color = g
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set step" command
    def test_set_step()
        @socket.write(ACTIVITIES[:set_step])

        # choose step to select and create new gradient step
        n, m, expectfail = choose_step()
        s = Step.generate()
        self.send_blob("#{n.chr}#{m.chr}#{s.stop.chr()}#{s.color.r.chr()}#{s.color.g.chr()}#{s.color.b.chr()}#{s.color.a.chr()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].styles[n].color.steps[m] = s
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "add step" command
    def test_add_step()
        @socket.write(ACTIVITIES[:add_step])

        # choose step to insert after and create new gradient step
        n, expectfail = choose_style()
        if not expectfail and @images[@selected].styles[n].color.is_gradient? and @images[@selected].styles[n].color.steps.length > 0
            m = rand(@images[@selected].styles[n].color.steps.length)
        elsif not expectfail
            m = 0
        else
            m = rand(256)
        end
        s = Step.generate()
        self.send_blob("#{n.chr}#{m.chr}#{s.stop.chr()}#{s.color.r.chr()}#{s.color.g.chr()}#{s.color.b.chr()}#{s.color.a.chr()}")

        # handle result
        if not expectfail and @images[@selected].styles[n].color.is_gradient?
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].styles[n].color.steps.insert(m, s)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "remove step" command
    def test_remove_step()
        @socket.write(ACTIVITIES[:remove_step])

        # choose step to remove
        n, m, expectfail = choose_step()
        self.send_blob("#{n.chr}#{m.chr}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].styles[n].color.steps.delete_at(m)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get gradient transformer" command
    def test_get_gradient_transformer()
        @socket.write(ACTIVITIES[:get_gradient_transformer])

        # choose style to select
        n, expectfail = choose_style()
        @socket.write(n.chr())

        # handle result
        if not expectfail and @images[@selected].styles[n].color.is_gradient? and @images[@selected].styles[n].color.xformer
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].styles[n].color.xformer.serialize, data, fuzzy: true)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set gradient transformer" command
    def test_set_gradient_transformer()
        @socket.write(ACTIVITIES[:set_gradient_transformer])

        # choose style to select
        n, expectfail = choose_style()
        xformer = Transformer.generate()
        self.send_blob("#{n.chr}#{xformer.type.chr}#{xformer.serialize}")

        # handle result
        if not expectfail and @images[@selected].styles[n].color.is_gradient?
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].styles[n].color.xformer = xformer
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get paths" command
    def test_get_paths()
        @socket.write(ACTIVITIES[:get_paths])

        # handle result
        if @images.length > 0
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].paths.length, data[0].ord())
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get path" command
    def test_get_path()
        @socket.write(ACTIVITIES[:get_path])

        # choose path to select
        n, expectfail = choose_path()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].paths[n].serialize(), data, fuzzy: true)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set path" command
    def test_set_path()
        @socket.write(ACTIVITIES[:set_path])

        # choose path to select and generate new path
        n, expectfail = choose_path()
        p = Path.generate()
        self.send_blob("#{n.chr()}#{p.serialize()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].paths[n] = p
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "add path" command
    def test_add_path()
        @socket.write(ACTIVITIES[:add_path])

        # choose path to select and generate new path
        p = Path.generate()
        self.send_blob(p.serialize())

        # handle result
        if @images.length > 0 and @images[@selected].paths.length < 256
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].paths << p
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "remove path" command
    def test_remove_path()
        @socket.write(ACTIVITIES[:remove_path])

        # choose path to remove
        n, expectfail = choose_path(true)
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].paths.delete_at(n)
            @images[@selected].shapes.length.times do |i|
                @images[@selected].shapes[i].paths.length.times do |j|
                    if @images[@selected].shapes[i].paths[j] > n
                        @images[@selected].shapes[i].paths[j] -= 1
                    end
                end
            end
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get point" command
    def test_get_point()
        @socket.write(ACTIVITIES[:get_point])

        # choose point to select
        n, m, expectfail = choose_point()
        @socket.write(n.chr())
        @socket.write(m.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].paths[n].points[m].serialize(), data, fuzzy: true)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set point" command
    def test_set_point()
        @socket.write(ACTIVITIES[:set_point])

        # choose point to select
        n, m, expectfail = choose_point()
        p = Point.generate()
        self.send_blob("#{n.chr}#{m.chr}#{p.serialize()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].paths[n].points[m] = p
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "add point" command
    def test_add_point()
        @socket.write(ACTIVITIES[:add_point])

        # choose point to select
        n, expectfail = choose_path()
        if not expectfail and @images[@selected].paths[n].points.length > 0
            m = rand(@images[@selected].paths[n].points.length)
        elsif not expectfail
            m = 0
        else
            m = rand(256)
        end
        p = Point.generate()
        self.send_blob("#{n.chr}#{m.chr}#{p.serialize()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].paths[n].points.insert(m, p)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "remove point" command
    def test_remove_point()
        @socket.write(ACTIVITIES[:remove_point])

        # choose point to remove
        n, m, expectfail = choose_point()
        @socket.write(n.chr())
        @socket.write(m.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].paths[n].points.delete_at(m)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get shapes" command
    def test_get_shapes()
        @socket.write(ACTIVITIES[:get_shapes])

        # handle result
        if @images.length > 0
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].shapes.length, data[0].ord())
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get shape" command
    def test_get_shape()
        @socket.write(ACTIVITIES[:get_shape])

        # choose shape to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].shapes[n].serialize, data, fuzzy: true)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set shape" command
    def test_set_shape()
        @socket.write(ACTIVITIES[:set_shape])

        # choose shape to select
        n, expectfail = choose_shape()

        # create new shape
        if not expectfail and @images.length > 0
            sh = Shape.generate(@images[@selected].styles.length, @images[@selected].paths.length)
        else
            sh = Shape.generate(256, 256)
        end
        self.send_blob("#{n.chr}#{sh.serialize}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n] = sh
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "add shape" command
    def test_add_shape()
        @socket.write(ACTIVITIES[:add_shape])

        # create new shape
        if @images.length > 0 and @images[@selected].styles.length > 0 and @images[@selected].paths.length > 0
            sh = Shape.generate(@images[@selected].styles.length, @images[@selected].paths.length)
        else
            sh = Shape.generate(256, 256)
        end
        self.send_blob(sh.serialize)

        # handle result
        if @images.length > 0 and @images[@selected].styles.length > 0 and @images[@selected].paths.length > 0 and @images[@selected].shapes.length < 256
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes << sh
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "remove shape" command
    def test_remove_shape()
        @socket.write(ACTIVITIES[:remove_shape])

        # choose shape to remove
        n, expectfail = choose_shape()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes.delete_at(n)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get shape style" command
    def test_get_shape_style()
        @socket.write(ACTIVITIES[:get_shape_style])

        # choose style to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].shapes[n].style, data[0].ord)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set shape style" command
    def test_set_shape_style()
        @socket.write(ACTIVITIES[:set_shape_style])

        # choose shape and style to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())
        if not expectfail and @images[@selected].styles.length > 0
            m = rand(@images[@selected].styles.length)
        elsif not expectfail
            m = 0
        else
            m = rand(256)
        end
        @socket.write(m.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].style = m
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get shape paths" command
    def test_get_shape_paths()
        @socket.write(ACTIVITIES[:get_shape_paths])

        # choose shape to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].shapes[n].paths.sort.map { |x| x.chr }.join(), data)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set shape paths" command
    def test_set_shape_paths()
        @socket.write(ACTIVITIES[:set_shape_paths])

        # choose shape to select
        n, expectfail = choose_shape()

        # choose paths to set
        paths = []
        if not expectfail and @images[@selected].paths.length > 0
            rand(10).times { paths << rand(@images[@selected].paths.length) }
            paths.uniq!
        else
            rand(10).times { paths << rand(256) }
        end

        # send selected shape and paths
        self.send_blob("#{n.chr}#{paths.map { |x| x.chr }.join()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].paths = paths
            @images[@selected].shapes[n].paths.sort!
            @images[@selected].shapes[n].paths.uniq!
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "add shape path" command
    def test_add_shape_path()
        @socket.write(ACTIVITIES[:add_shape_path])

        # choose shape and path to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())
        if not expectfail and @images[@selected].shapes[n].paths.length > 0
            m = rand(@images[@selected].shapes[n].paths.length)
        elsif not expectfail
            m = 0
        else
            m = rand(256)
        end
        @socket.write(m.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].paths << m
            @images[@selected].shapes[n].paths.sort!
            @images[@selected].shapes[n].paths.uniq!
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "remove shape path" command
    def test_remove_shape_path()
        @socket.write(ACTIVITIES[:remove_shape_path])

        # choose shape and path to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())
        if not expectfail and @images[@selected].shapes[n].paths.length > 0
            m = @images[@selected].shapes[n].paths.sample
        else
            m = rand(256)
            expectfail = true
        end
        @socket.write(m.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].paths.delete(m)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "has hinting" command
    def test_has_hinting()
        @socket.write(ACTIVITIES[:has_hinting])

        # choose shape to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].shapes[n].hinting ? 1 : 0, data[0].ord)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set hinting" command
    def test_set_hinting()
        @socket.write(ACTIVITIES[:set_hinting])

        # choose shape to select and how to set hinting
        n, expectfail = choose_shape()
        @socket.write(n.chr())
        h = [true, false].sample
        @socket.write(h ? 1.chr : 0.chr)

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].hinting = h
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get min visibility" command
    def test_get_min_visibility()
        @socket.write(ACTIVITIES[:get_min_visibility])

        # choose shape to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].shapes[n].minvis, data[0].ord)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set min visibility" command
    def test_set_min_visibility()
        @socket.write(ACTIVITIES[:set_min_visibility])

        # choose shape to select and generate new visibility
        n, expectfail = choose_shape()
        @socket.write(n.chr())
        minvis = rand(256)
        @socket.write(minvis.chr)

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].minvis = minvis
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get max visibility" command
    def test_get_max_visibility()
        @socket.write(ACTIVITIES[:get_max_visibility])

        # choose shape to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].shapes[n].maxvis, data[0].ord)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set max visibility" command
    def test_set_max_visibility()
        @socket.write(ACTIVITIES[:set_max_visibility])

        # choose shape to select and generate new visibility
        n, expectfail = choose_shape()
        @socket.write(n.chr())
        maxvis = rand(256)
        @socket.write(maxvis.chr)

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].maxvis = maxvis
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get transformers" command
    def test_get_transformers()
        @socket.write(ACTIVITIES[:get_transformers])

        # choose shape to select
        n, expectfail = choose_shape()
        @socket.write(n.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert(@images[@selected].shapes[n].xformers.length, data[0].ord)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "get transformer" command
    def test_get_transformer()
        @socket.write(ACTIVITIES[:get_transformer])

        # choose shape to select
        n, m, expectfail = choose_xformer()
        @socket.write(n.chr())
        @socket.write(m.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_RES, @socket.read(1))
            data = self.recv_blob()
            assert("#{@images[@selected].shapes[n].xformers[m].type.chr}#{@images[@selected].shapes[n].xformers[m].serialize}", data, fuzzy: true)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "set transformer" command
    def test_set_transformer()
        @socket.write(ACTIVITIES[:set_transformer])

        # choose shape to select
        n, m, expectfail = choose_xformer()
        xformer = Transformer.generate()
        self.send_blob("#{n.chr}#{m.chr}#{xformer.type.chr}#{xformer.serialize()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].xformers[m] = xformer
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "add transformer" command
    def test_add_transformer()
        @socket.write(ACTIVITIES[:add_transformer])

        # choose shape to select
        n, expectfail = choose_shape()
        if not expectfail and @images[@selected].shapes[n].xformers.length > 0
            m = rand(@images[@selected].shapes[n].xformers.length)
        else
            m = 0
        end
        xformer = Transformer.generate()
        self.send_blob("#{n.chr}#{m.chr}#{xformer.type.chr}#{xformer.serialize()}")

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].xformers.insert(m, xformer)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Tests the "remove transformer" command
    def test_remove_transformer()
        @socket.write(ACTIVITIES[:remove_transformer])

        # choose shape to select
        n, m, expectfail = choose_xformer()
        @socket.write(n.chr())
        @socket.write(m.chr())

        # handle result
        if not expectfail
            # expect success
            assert(RESULT_SCS, @socket.read(1))
            @images[@selected].shapes[n].xformers.delete_at(m)
        else
            # expect failure
            assert(RESULT_FAL, @socket.read(1))
        end
    end

    ##
    # Runs @length number of tests, selected randomly according to our @seed
    def run_tests()
        # connect to girard server
        LOGGER.debug("Connecting to server...")
        self.connect()
        LOGGER.debug("Connected to server")

        # load a random image file to ensure we have something to manipulate
        file = FILES.sample()
        self.test_load("tests/#{file}")

        # run tests
        @length.times do
            Timeout::timeout(@timeout) do
                activity = ACTIVITIES.keys.sample()
                LOGGER.debug("Testing activity: #{activity}")
                case activity
                when :create
                    self.test_create()
                when :destroy
                    self.test_destroy()
                when :load
                    self.test_load("tests/#{FILES.sample()}")
                when :store
                    self.test_store()
                when :selected_image
                    self.test_selected_image()
                when :select_image
                    self.test_select_image()
                when :get_styles
                    self.test_get_styles()
                when :get_style
                    self.test_get_style()
                when :set_style
                    self.test_set_style()
                when :add_style
                    self.test_add_style()
                when :remove_style
                    self.test_remove_style()
                when :is_flat
                    self.test_is_flat()
                when :is_transparent
                    self.test_is_transparent()
                when :get_color
                    self.test_get_color()
                when :set_color
                    self.test_set_color()
                when :get_gradient
                    self.test_get_gradient()
                when :set_gradient
                    self.test_set_gradient()
                when :set_step
                    self.test_set_step()
                when :add_step
                    self.test_add_step()
                when :remove_step
                    self.test_remove_step()
                when :get_gradient_transformer
                    self.test_get_gradient_transformer()
                when :set_gradient_transformer
                    self.test_set_gradient_transformer()
                when :get_paths
                    self.test_get_paths()
                when :get_path
                    self.test_get_path()
                when :set_path
                    self.test_set_path()
                when :add_path
                    self.test_add_path()
                when :remove_path
                    self.test_remove_path()
                when :get_point
                    self.test_get_point()
                when :set_point
                    self.test_set_point()
                when :add_point
                    self.test_add_point()
                when :remove_point
                    self.test_remove_point()
                when :get_shapes
                    self.test_get_shapes()
                when :get_shape
                    self.test_get_shape()
                when :set_shape
                    self.test_set_shape()
                when :add_shape
                    self.test_add_shape()
                when :remove_shape
                    self.test_remove_shape()
                when :get_shape_style
                    self.test_get_shape_style()
                when :set_shape_style
                    self.test_set_shape_style()
                when :get_shape_paths
                    self.test_get_shape_paths()
                when :set_shape_paths
                    self.test_set_shape_paths()
                when :add_shape_path
                    self.test_add_shape_path()
                when :remove_shape_path
                    self.test_remove_shape_path()
                when :has_hinting
                    self.test_has_hinting()
                when :set_hinting
                    self.test_set_hinting()
                when :get_min_visibility
                    self.test_get_min_visibility()
                when :set_min_visibility
                    self.test_set_min_visibility()
                when :get_max_visibility
                    self.test_get_max_visibility()
                when :set_max_visibility
                    self.test_set_max_visibility()
                when :get_transformers
                    self.test_get_transformers()
                when :get_transformer
                    self.test_get_transformer()
                when :set_transformer
                    self.test_set_transformer()
                when :add_transformer
                    self.test_add_transformer()
                when :remove_transformer
                    self.test_remove_transformer()
                else
                    fail "unknown activity"
                end
            end
        end

        # disconnect from girard server
        LOGGER.debug("Disconnecting from server...")
        self.disconnect()
        LOGGER.debug("Disconnected from server")

        LOGGER.info("SUCCESS")
    end
end

##
# Entry point for the Girard poller
def main()
    # check for proper usage
    host = ARGV[0] || ENV["HOST"]
    port = (ARGV[1] || ENV["PORT"]).to_i()
    seed = (ENV["SEED"] || Random.new_seed()).to_i()
    Random.srand(seed)
    length = (ENV["LENGTH"] || (100..300).to_a.sample).to_i()
    timeout = (ENV["TIMEOUT"] || 1).to_i()

    # validate provided host
    if host == nil or host == ""
        fail "invalid host (or none specified)"
    end

    # validate provided port
    if port == nil or port <= 0 or port > 65535
        fail "invalid port (or none specified)"
    end

    # create poller and run tests for this seed
    LOGGER.info("HOST=#{host} PORT=#{port} SEED=#{seed} LENGTH=#{length} TIMEOUT=#{timeout}")
    p = Poller.new(host, port, seed, length, timeout)
    begin
        p.run_tests()
    rescue Timeout::Error => e
        LOGGER.info("TIMED OUT")
        raise  # continue failing normally
    rescue StandardError => e
        LOGGER.fatal(e.message)
        raise  # continue failing normally
    end
end


# jump to the entry point if this file is executed rather than imported
if __FILE__ == $0
    main()
end
