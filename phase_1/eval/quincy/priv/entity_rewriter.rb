require "json"
require 'set'

entities = JSON.parse(File.read(File.join(__dir__, 'entities.json')))

seen = Set.new

root = Hash.new

def load_entity_into_hash(hash, name, value)
  if '' == name
    hash[nil] = value
    return
  end

  hash[name[0]] ||= Hash.new
  load_entity_into_hash(hash[name[0]], name[1..-1], value)
end

def ifs_for_hash(hash, off=0)

  hash.each do |k, v|
    if k.nil?
      escaped_dinguses = v.map do |cp|
        "\\u%04xd" % cp
      end.join
      puts "if (e == (i + #{off})) {"
      puts "return \"#{escaped_dinguses}\";"
      puts "}"
      next
    end
    puts "if ((e > (i + #{off})) && ('#{k}' == *(i + #{off}))) {"
    ifs_for_hash(v, off + 1)
    puts "}"
  end

  puts 'return {std::nullopt};'
end

puts '#include "entities.hpp"'

puts 'std::optional<std::string> html::substitute_entity(const std::string candidate) {'
puts '  auto e = candidate.cend();'
puts '  auto i = candidate.cbegin();'

entities.each do |k, v|
  nombre = k.gsub('&', '').gsub(';', '')
  next if seen.include? nombre
  seen << nombre

  value = v['codepoints']

  load_entity_into_hash(root, nombre, value)
end

ifs_for_hash(root)

puts '}'
