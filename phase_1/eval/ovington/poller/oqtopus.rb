# support for serializing/deserializing oqtopus expressions to and from
# Ruby values

module Oqtopus
  def self.read(sock)
    first = read_one(sock)

    return first unless first.is_a? Openlist

    return read_list(sock)
  end

  def self.read_list(sock)
    acc = []
    loop do
      nxt = read_one(sock)
      case nxt
      when Openlist
        nxt = read_list(sock)
      when Closelist
        return acc
      end

      acc << nxt
    end
  end

  def self.read_one(sock)
    sigil = sock.getc
    case sigil
    when 's'
      return sock.read(8).unpack('q<').first
    when 'f'
      got = sock.read(8).unpack('E').first
      #LLL.info got
      return got
    when 'z'
      return read_stringz(sock)
    when 'x'
      len = sock.read(1).unpack('C').first
      return sock.read(len).k
    when '('
      return Openlist.new
    when ')'
      return Closelist.new
    when nil
      return nil
    else
      fail "unexpected oqtopus sigil #{sigil}"
    end
  end

  def self.read_stringz(sock)
    acc = ''

    while 0 != (c = sock.getc).ord
      acc << c
    end

    return acc
  end

  class Openlist
  end

  class Closelist
  end

  class Keyword < String
    def to_oqtopus
      fail "keyword too long" if length > 255

      "x" + [self.length].pack('C') + self
    end

    def inspect
      super + ".k"
    end
  end
end

class Array
  def to_oqtopus
    "(" + map(&:to_oqtopus).join + ")"
  end
end

class String
  def to_oqtopus
    "z" + self + "\0"
  end

  def k
    Oqtopus::Keyword.new(self)
  end

  def self.randomize
    WORDS.sample(5).join(' ')
  end
end

class Integer
  S64_RANGE = (0 - (2**63))..((2**63) - 1)

  def to_oqtopus
    fail "integer doesn't fit in s64" unless S64_RANGE.include? self

    "s" + [self].pack('q<')
  end

  def self.randomize
    rand S64_RANGE
  end
end

class Float
  def to_oqtopus
    "f" + [self].pack('E')
  end

  def self.randomize
    rand() + rand(-100..100)
  end
end
