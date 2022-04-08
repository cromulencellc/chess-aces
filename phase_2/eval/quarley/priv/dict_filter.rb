require 'iconv'
require "stringex/unidecoder"
require "stringex/core_ext"

p ARGV

src = File.open ARGV[0]
dest = File.open(ARGV[1], 'w')

conv = Iconv.new 'ascii//translit//ignore', 'utf-8'

src.each_line.each do |l|
  w = l.strip

  got = w.to_ascii
#  got = conv.iconv(w)

#  next unless got =~ /^[A-Z]$/

  dest.puts got.upcase
rescue => e
  p w
  p e
  exit
end