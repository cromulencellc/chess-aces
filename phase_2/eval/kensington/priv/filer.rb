require 'fileutils'

CONTENT_LENGTH = 16..(10 * 1_024)

WORDS = %w{
  alpha bravo charlie delta echo foxtrot golf hotel india
  juliet kilo lima mike november oscar papa quebec romeo
  sierra tango uniform victor whiskey xray yankee zulu
  _anonymous
}

root_dir = ARGV[0]

if root_dir.nil?
    puts "pass in a root directory as argv0"
    exit(1)
end

WORDS.each do |w|
    FileUtils.mkdir_p(File.join(root_dir, w))
end

File.open(File.join(__dir__, 'moby-dick.txt'), 'r') do |moby|
  moby.seek 0x6f94

  until moby.eof?
    dest_path = File.join(root_dir,
        WORDS.sample,
        "#{(Time.now.to_f * 1000).round}-#{rand(1_000_000)}")
    File.open(dest_path, 'w'){|f| f.write moby.read(rand(CONTENT_LENGTH))}
  end
end
