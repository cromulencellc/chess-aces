elements = File.read(File.join(__dir__, 'void_elements.txt')).split("\n").sort

# p elements

def load_element_into_hash(element, hash)
  return if '' == element
  hash[element[0]] ||= Hash.new
  load_element_into_hash(element[1..-1], hash[element[0]])
end

chars = Hash.new

elements.each do |el|
  load_element_into_hash(el, chars)
end

# p chars

def ifs_for_hash(hash, off=0)
  if 0 == hash.length
    puts 'return true;'
    return
  end

  hash.keys.each do |k|
    subhash = hash[k]

    puts "if ((e > (i + #{off})) && ('#{k}' == *(i + #{off}))) {"
    ifs_for_hash(subhash, off + 1)
    puts "}"
  end

  puts 'return false;'
end

ifs_for_hash(chars)
