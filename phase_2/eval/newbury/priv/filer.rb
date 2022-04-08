require 'fileutils'
require 'webrick/httpauth/htpasswd'
require 'webrick/httpauth/htdigest'

CONTENT_LENGTH = 16..(4 * 1_024)

WORDS = %w{
  alpha bravo charlie delta echo foxtrot golf hotel india
  juliet kilo lima mike november oscar papa quebec romeo
  sierra tango uniform victor whiskey xray yankee zulu
}

root_dir = ARGV[0]

if root_dir.nil?
    puts "pass in a root directory as argv0"
    exit(1)
end

dests = []

WORDS.each do |w|
  dir = File.join(root_dir, 'homes', w, 'webroot')
  FileUtils.mkdir_p(dir)
  dests << dir
end

auth_config = File.open(File.join(root_dir, 'auth_config.conf'), 'w')
auth_cheatsheet = File.open(File.join(root_dir, 'cheatsheet'), 'w')

WORDS.each do |w|
  dir = File.join(root_dir, 'webroot', w)
  FileUtils.mkdir_p(dir)
  dests << dir

  if rand() > 0.5
    htpasswd = WEBrick::HTTPAuth::Htpasswd.new File.join(dir, '.htpasswd')
    password = rand(36 ** 20).to_s(36)
    htpasswd.set_passwd w, w, password
    htpasswd.flush

    auth_cheatsheet.puts "#{dir}:#{w}:#{password}"

    auth_config.write %Q{
$HTTP["url"] =~ "^/#{w}/" {
  auth.backend = "htpasswd"
  auth.backend.htpasswd.userfile = "/#{dir}/.htpasswd"
  auth.require = ("/#{w}" => (
    "method" => "basic",
    "realm" => "#{w}",
    "require" => "valid-user"
  )) 
}
    }
  else
    htdigest = WEBrick::HTTPAuth::Htdigest.new File.join(dir, '.htdigest')
    password = rand(36 ** 20).to_s(36)
    htdigest.set_passwd w, w, password
    htdigest.flush
    
    auth_cheatsheet.puts "#{dir}:#{w}:#{password}"
    
    auth_config.write %Q{
$HTTP["url"] =~ "^/#{w}/" {
  auth.backend = "htdigest"
  auth.backend.htdigest.userfile = "/#{dir}/.htdigest"
  auth.require = ("/#{w}" => (
    "method" => "digest",
    "realm" => "#{w}",
    "require" => "valid-user"
  )) 
}
    }
  end
end

File.open(File.join(__dir__, 'books', 'moby-dick.txt'), 'r') do |moby|
  moby.seek 0x6f94

  until moby.eof?
    dest_path = File.join(dests.sample,
        "#{(Time.now.to_f * 1000).round}-#{rand(1_000_000)}.txt")
    File.open(dest_path, 'w'){|f| f.write moby.read(rand(CONTENT_LENGTH))}
  end
end

FileUtils.copy(File.join(__dir__, 'lighttpd.conf'), root_dir)