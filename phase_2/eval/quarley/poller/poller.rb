#!/usr/bin/env ruby

require "logger"
require "mechanize"
require "timeout"

# range of lengths to pick in the absence of a `LENGTH` environment variable
LENGTH_RANGE=(50..100)

# seconds to wait for a given activity
DEFAULT_TIME_LIMIT = 30

# need to sleep a bit to let wt sessions "settle" on the server
SESSION_SETTLE_SLEEP = (ENV['SETTLE_SLEEP'] || 1).to_f

LOGIN_RETRIES = (ENV['LOGIN_RETRIES'] || 100).to_i

KNOWN_DICTS = Dir.glob(File.join(__dir__, 'data', 'dict-*.txt'))

LLL = Logger.new(STDERR,
                 formatter: proc do |sev, datetime, progname, msg|
                   "#{sev}: #{msg}\n"
                 end,
                 level: Logger::DEBUG)

# for a less-noisy poller experience, set the `LOGGER_MIN_LEVEL` environment
# variable to `WARN`, `ERROR`, or `FATAL`
if ENV['LOGGER_MIN_LEVEL']
  LLL.level = Logger.const_get(ENV['LOGGER_MIN_LEVEL'])
end

LLL.info KNOWN_DICTS

# gathers statistics on how much time a given activity takes
class Timing
  attr_reader :count, :total, :min, :max

  def initialize
    @count = 0
    @total = 0.0
    @min = DEFAULT_TIME_LIMIT + 1
    @max = -1
  end

  def add(time)
    @count += 1
    @total += time
    @min = time if @min > time
    @max = time if @max < time
  end

  def mean
    @total / @count.to_f
  end
end

# the poller
class Poller
  attr_reader :seed, :length
  attr_reader :host, :port

  attr_reader :report, :assertions

  attr_reader :solves

  # activities that don't require data to have been loaded
  ACTIVITIES = %i{
    make_new_account
    fail_login
    fail_password

    try_to_solve
    try_to_fail_to_solve
  }

  def initialize
    @host = ENV['HOST'] || 'challenge'
    @port = (ENV['PORT'] || 3025).to_i

    @report = Hash[ACTIVITIES.map do |a|
                     [a, Timing.new]
                   end]

    @assertions = 0

    @accounts = {}
    @solves = Hash.new(0)

    seed_rng!
    pick_length!
    pick_timeout!
  end

  # actually runs the poller
  def run!
    make_new_account

    length.times do
      universe = ACTIVITIES
      activity = universe.sample
      unless respond_to?(activity, true)
        LLL.error "activity #{activity} not implemented"
        next
      end

      LLL.info "trying #{activity}"
      before_time = Time.now.to_f
      Timeout.timeout(@timeout) do
        send activity
      end
      after_time = Time.now.to_f

      @report[activity].add(after_time - before_time)
    end
  end

  private

  def make_new_account
    username = mk_name
    password = mk_password

    with_agent do |agent|      
      index = agent.get "http://#{@host}:#{@port}"
      real_index = agent.click index.link_with text: 'Load basic HTML'
      sleep SESSION_SETTLE_SLEEP
      fm = real_index.form_with id: 'form'

      reg_text = real_index.css('span[data-object-name="register"]').first
      assert_equal 'Register', reg_text.text
      sleep SESSION_SETTLE_SLEEP

      reg_btn = reg_text.parent
      reg_page = fm.click_button fm.button_with(name: reg_btn['name'])
      fm = reg_page.form_with id: 'form'
      popup = reg_page.css('.Wt-auth-registration')
      fm.field_with(name: object_name(popup, 'user-name')).value = username
      fm.field_with(name: object_name(popup, 'choose-password')).value = password
      fm.field_with(name: object_name(popup, 'repeat-password')).value = password
      did_reg_page = 
        fm.click_button fm.button_with(name: object_name(popup, 'ok-button'))

      assert_equal 'Ready to play ?', did_reg_page.css('h2').text
    end

    sleep SESSION_SETTLE_SLEEP

    @accounts[username] = password
  end
  
  def fail_login
    wrong_username = mk_name
    while @accounts.include? wrong_username
      wrong_username = mk_name
    end

    with_agent do |agent|
      index = agent.get "http://#{@host}:#{@port}"
      real_index = agent.click index.link_with text: 'Load basic HTML'
      sleep SESSION_SETTLE_SLEEP
      fm = real_index.form_with id: 'form'
      fm.field_with(name: object_name(fm.node, 'user-name')).value = wrong_username
      fm.field_with(name: object_name(fm.node, 'password')).value = mk_password
      sleep SESSION_SETTLE_SLEEP

      failed_login_page =
        fm.click_button fm.button_with(name: object_name(fm.node, 'login'))

      assert_equal 'Invalid', failed_login_page.css('form .Wt-error')[0].text
    end
  end

  def fail_password
    right_username = @accounts.keys.sample
    wrong_password = mk_password

    while @accounts[right_username] == wrong_password
      wrong_password = mk_password
    end

    LLL.info "wrong password #{wrong_password}"

    with_agent do |agent|
      index = agent.get "http://#{@host}:#{@port}"
      real_index = agent.click index.link_with text: 'Load basic HTML'
      sleep SESSION_SETTLE_SLEEP
      fm = real_index.form_with id: 'form'
      fm.field_with(name: object_name(fm.node, 'user-name')).value = right_username
      fm.field_with(name: object_name(fm.node, 'password')).value = wrong_password
      sleep SESSION_SETTLE_SLEEP

      failed_login_page =
        fm.click_button fm.button_with(name: object_name(fm.node, 'login'))
      fm = real_index.form_with id: 'form'

      assert_equal 'Valid', failed_login_page.css('form .Wt-info')[0].text

      LOGIN_RETRIES.times do |t|
        LLL.error "retrying fail_password login #{t} / #{LOGIN_RETRIES}"

        break if failed_login_page.css('form .Wt-error')[0]
        sleep SESSION_SETTLE_SLEEP
          
        fm.field_with(name: object_name(fm.node, 'user-name')).value = right_username
        fm.field_with(name: object_name(fm.node, 'password')).value = wrong_password
        failed_login_page =
          fm.click_button fm.button_with(name: object_name(fm.node, 'login'))
        fm = real_index.form_with id: 'form'
      end
      assert_equal 'Invalid password', failed_login_page.css('form .Wt-error')[0].text
    end
  end

  def try_to_solve
    username = @accounts.keys.sample
    password = @accounts[username]

    with_agent do |agent|
      index = agent.get "http://#{@host}:#{@port}"
      real_index = agent.click index.link_with text: 'Load basic HTML'
      fm = real_index.form_with id: 'form'
      fm.field_with(name: object_name(fm.node, 'user-name')).value = username
      fm.field_with(name: object_name(fm.node, 'password')).value = password
      sleep SESSION_SETTLE_SLEEP

      # x = fm.click_button fm.button_with(name: object_name(fm.node, 'login'))
      # fm = x.form_with id: 'form'

      pick_dict_page =
        fm.click_button fm.button_with(name: object_name(fm.node, 'login'))
      fm = pick_dict_page.form_with id: 'form'
      sleep SESSION_SETTLE_SLEEP

      LOGIN_RETRIES.times do |t|
        LLL.error "retrying try_to_solve login #{t} / #{LOGIN_RETRIES}"
        break if 'Ready to play ?' == pick_dict_page.css('h2').text
        fm.field_with(name: object_name(fm.node, 'user-name')).value = username
        fm.field_with(name: object_name(fm.node, 'password')).value = password
        pick_dict_page =
          fm.click_button fm.button_with(name: object_name(fm.node, 'login'))
        fm = pick_dict_page.form_with id: 'form' 
        sleep SESSION_SETTLE_SLEEP
      end

      assert_equal 'Ready to play ?', pick_dict_page.css('h2').text

      fm = pick_dict_page.form_with id: 'form'
      picker = fm.css('select').first
      app_dict_names = picker.css('option')

      dict_name = nil
      dict_idx = nil

      until KNOWN_DICTS.include? File.join(__dir__, 'data', dict_name.to_s) do
        dict_opt = app_dict_names.to_a.sample
        dict_name = dict_opt.text
        dict_idx = dict_opt['value']
      end

      fm[picker['name']] = dict_idx

      LLL.info "expecting #{dict_name} at #{dict_idx}"

      game_screen = fm.click_button fm.button_with(css: '.gamestack button')
      fm = game_screen.form_with id: 'form'
      sleep SESSION_SETTLE_SLEEP

      assert_equal "Guess the word, #{username}!", game_screen.css('h2').first.text

      words = File.read(File.join(__dir__, 'data', dict_name)).
        each_line.
        map(&:strip)

      loop do
        span_text = game_screen.css('.gamestack span').text
        if span_text.include? "You hang..."
          found_word_match = /The correct answer was: (\w+)/.match span_text

          LLL.info "Found word should've been #{found_word_match[1]}"
          assert(words.include?(found_word_match[1]), 
            "Poller internal error, word missing fron dict")

          @solves['tried but failed'] += 1
          return
        end

        if span_text.include? "You win!"
          @solves['tried and won'] += 1
          return
        end

        LLL.info game_screen.css('.wordcontainer').to_xml

        template = game_screen.css('.wordcontainer span').text
        template_exp = "^#{template.gsub('-', '.')}$"
        LLL.info "#{template.inspect} #{template_exp.inspect}"
        template_re = Regexp.new template_exp

        LLL.info "matching #{words.length} against #{template_re}"

        canary = words.detect{|w| w.length == template.length}
        canary_2 = words.detect{|w| template_re =~ w}
        LLL.info "#{canary.inspect} #{(template_re =~ canary).to_s}"
        LLL.info "#{canary_2.inspect}"

        words.select!{|w| template_re =~ w}

        assert(words.length > 0,
          "Poller internal error, couldn't find a word in #{dict_name} matching #{template_re}"
        )

        LLL.info "matched #{words.length}"

        letter_buttons = game_screen.css('.gamestack table button')
        all_letters = letter_buttons.map(&:text)
        used_letters = game_screen.css('.gamestack table button.Wt-disabled').map(&:text)

        remaining_letters = all_letters - used_letters
        LLL.info "remaining letters #{remaining_letters.inspect}"
        
        counts = words.inject(Hash.new(0)) do |cts, word|
          remaining_letters.each do |ltr|
            cts[ltr] += 1 if word.include? ltr
          end
          cts
        end

        best_count = counts.values.max
        best_letter = counts.keys.detect{|k| counts[k] == best_count}
        LLL.info "best letter in #{words.count} is #{best_letter}"

        best_btn = letter_buttons.detect{|btn| btn.text == best_letter}

        LLL.info best_btn.to_xml

        game_screen = fm.click_button fm.button_with(name: best_btn['name'])
        fm = game_screen.form_with id: 'form'
        sleep SESSION_SETTLE_SLEEP
      end
    end
  end

  def try_to_fail_to_solve
    username = @accounts.keys.sample
    password = @accounts[username]

    with_agent do |agent|
      index = agent.get "http://#{@host}:#{@port}"
      real_index = agent.click index.link_with text: 'Load basic HTML'
      fm = real_index.form_with id: 'form'
      fm.field_with(name: object_name(fm.node, 'user-name')).value = username
      fm.field_with(name: object_name(fm.node, 'password')).value = password
      sleep SESSION_SETTLE_SLEEP

      # x = fm.click_button fm.button_with(name: object_name(fm.node, 'login'))
      # fm = x.form_with id: 'form'

      pick_dict_page =
        fm.click_button fm.button_with(name: object_name(fm.node, 'login'))
      fm = pick_dict_page.form_with id: 'form'
      sleep SESSION_SETTLE_SLEEP

      LOGIN_RETRIES.times do |t|
        LLL.error "retrying try_to_fail_to_solve login #{t} / #{LOGIN_RETRIES}"
        break if 'Ready to play ?' == pick_dict_page.css('h2').text
        
        fm.field_with(name: object_name(fm.node, 'user-name')).value = username
        fm.field_with(name: object_name(fm.node, 'password')).value = password
        pick_dict_page =
          fm.click_button fm.button_with(name: object_name(fm.node, 'login'))
        fm = pick_dict_page.form_with id: 'form' 
        sleep SESSION_SETTLE_SLEEP
      end

      assert_equal 'Ready to play ?', pick_dict_page.css('h2').text

      fm = pick_dict_page.form_with id: 'form'
      picker = fm.css('select').first
      app_dict_names = picker.css('option')

      dict_name = nil
      dict_idx = nil

      until KNOWN_DICTS.include? File.join(__dir__, 'data', dict_name.to_s) do
        dict_opt = app_dict_names.to_a.sample
        dict_name = dict_opt.text
        dict_idx = dict_opt['value']
      end

      fm[picker['name']] = dict_idx

      LLL.info "expecting #{dict_name} at #{dict_idx}"

      game_screen = fm.click_button fm.button_with(css: '.gamestack button')
      fm = game_screen.form_with id: 'form'
      sleep SESSION_SETTLE_SLEEP

      assert_equal "Guess the word, #{username}!", game_screen.css('h2').first.text

      words = File.read(File.join(__dir__, 'data', dict_name)).
        each_line.
        map(&:strip)

      loop do
        span_text = game_screen.css('.gamestack span').text
        if span_text.include? "You hang..."
          found_word_match = /The correct answer was: (\w+)/.match span_text

          LLL.info "Found word should've been #{found_word_match[1]}"
          assert(words.include?(found_word_match[1]), 
            "Poller internal error, word missing fron dict")

          @solves['won at losing'] += 1
          return
        end

        if span_text.include? "You win!"
          @solves['failed to fail'] += 1
          return
        end

        LLL.info game_screen.css('.wordcontainer').to_xml

        template = game_screen.css('.wordcontainer span').text
        template_exp = "^#{template.gsub('-', '.')}$"
        LLL.info "#{template.inspect} #{template_exp.inspect}"
        template_re = Regexp.new template_exp

        LLL.info "matching #{words.length} against #{template_re}"

        canary = words.detect{|w| w.length == template.length}
        canary_2 = words.detect{|w| template_re =~ w}
        LLL.info "#{canary.inspect} #{(template_re =~ canary).to_s}"
        LLL.info "#{canary_2.inspect}"

        words.select!{|w| template_re =~ w}

        assert(words.length > 0,
          "Poller internal error, couldn't find a word in #{dict_name} matching #{template_re}"
        )

        LLL.info "matched #{words.length}"

        letter_buttons = game_screen.css('.gamestack table button')
        all_letters = letter_buttons.map(&:text)
        used_letters = game_screen.css('.gamestack table button.Wt-disabled').map(&:text)

        remaining_letters = all_letters - used_letters
        LLL.info "remaining letters #{remaining_letters.inspect}"
        
        counts = words.inject(Hash.new(0)) do |cts, word|
          remaining_letters.each do |ltr|
            cts[ltr] += 1 if word.include? ltr
          end
          cts
        end

        best_count = counts.values.min
        best_letter = counts.keys.detect{|k| counts[k] == best_count}
        LLL.info "best letter in #{words.count} is #{best_letter}"

        best_btn = letter_buttons.detect{|btn| btn.text == best_letter}

        LLL.info best_btn.to_xml

        game_screen = fm.click_button fm.button_with(name: best_btn['name'])
        fm = game_screen.form_with id: 'form'
        sleep SESSION_SETTLE_SLEEP
      end
    end
  end

  def with_agent
    Mechanize.start do |mech|
      begin
        mech.log = LLL
        mech.follow_redirect = true
#        mech.agent.allowed_error_codes = [404, 425]
        yield mech
      rescue

        dump_filename = File.join(__dir__, 'tmp',
                                  "#{Time.now.to_i}-s#{@seed}")

        File.open(dump_filename, 'w'){|d| d.write mech.current_page.body}

        LLL.fatal dump_filename
        raise
      end
    end
  end
  
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

  # assert two values are equal
  def assert_equal(expected, actual)
    if expected.is_a?(String) && actual.is_a?(String)
      ex_bin = expected.dup.force_encoding('BINARY')
      act_bin = actual.dup.force_encoding('BINARY')

      assert(ex_bin == act_bin,
             "expected string `#{ex_bin}`, got `#{act_bin}`")
    else
      assert(expected == actual, "expected `#{expected}`, got `#{actual}`")
    end
  end

  # assert a predicate is true
  def assert(predicate, message='failed assertion')
    @assertions += 1
    return if predicate
    fail message
  end

  def refute(predicate, message='failed refutation')
    @assertions += 1
    return unless predicate
    fail message
  end

  # get the seed from either the `SEED` environment variable or from urandom
  def seed_rng!
    @seed = (ENV['SEED'] || Random.new_seed).to_i
    $stdout.puts "SEED=#{@seed}"
    $stdout.flush
    srand(@seed)
  end

  # get the length (number of activities) from either `LENGTH` environment
  # variable or by sampling a length from the `LENGTH_RANGE` constant
  def pick_length!
    @length = (ENV['LENGTH'] || rand(LENGTH_RANGE)).to_i
    $stdout.puts "LENGTH=#{@length}"
    $stdout.flush
  end

  # pick a timeout (seconds to wait per activity) from either `TIMEOUT`
  # environment variable or the `DEFAULT_TIME_LIMIT` constant
  def pick_timeout!
    @timeout = (ENV['TIMEOUT'] || DEFAULT_TIME_LIMIT).to_i
    $stdout.puts "TIMEOUT=#{@timeout}"
    $stdout.flush
  end
  
  def pick_focus!
    return unless ENV['OOPS_ALL']

    focus = ENV['OOPS_ALL'].to_sym

    ACTIVITIES.keep_if{|e| e == focus}

    fail "Couldn't foucs on #{focus}" if ACTIVITIES.empty?
  end

  # make a random string with two random words and a random number
  def mk_name
    "quarley poller #{rand(1_000_000)} #{Time.now.to_i}"
  end

  def mk_password
    "#{rand(26**10).to_s(26)} #{rand 1_000_000}"
  end
end

poller = Poller.new
at_exit do
  LLL.info((["SEED=#{poller.seed}",
            "LENGTH=#{poller.length}",
           "ruby", $0] + ARGV).join(' '))
end
poller.run!

$stderr.puts "solve report (for grins)"
poller.solves.each do |k, v|
  $stderr.puts("%20s\t%d" % [k, v])
end

$stderr.puts("%20s\t%s\t%s\t%s\t%s" % %w{name count min mean max})
total_count = 0
total_min = DEFAULT_TIME_LIMIT + 1
total_max = -1
total_mean = 0
poller.report.each do |r|
  timing = r[1]
  total_count += timing.count
  total_min = timing.min if timing.min < total_min
  total_max = timing.max if timing.max > total_max
  total_mean += (timing.count * timing.mean)
  $stderr.puts("%20s\t%d\t%.4f\t%.4f\t%.4f" % [
                 r[0],
                 timing.count, timing.min, timing.mean, timing.max
               ])
end
$stderr.puts("%20s\t%d\t%.4f\t%.4f\t%.4f" % [
               'TOTAL',
               total_count, total_min, total_mean/total_count, total_max
             ])

LLL.info "#{poller.assertions} total assertions"

LLL.info "success :)"
