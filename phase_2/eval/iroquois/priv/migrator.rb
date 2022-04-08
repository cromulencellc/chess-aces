require 'fileutils'

require 'scrypt'
require 'sqlite3'

USER_COUNT = 10
SUBSCRIPTION_PROBABILITY = 0.5
MESSAGE_COUNT = 500

WORDS = %w{
  alpha bravo charlie delta echo foxtrot golf hotel india
  juliet kilo lima mike november oscar papa quebec romeo
  sierra tango uniform victor whiskey xray yankee zulu
}

SEPS = [' ', '-', '_']

unless ARGV[0]
  puts <<EOS
pass in a filename as an argument; this script will delete (if exists) and
re-create that file as a sqlite3 database for iroquois

for example:
    ruby #{__FILE__} tmp.sqlite3
EOS
  exit 1
end

FileUtils.rm_f ARGV[0]
db = SQLite3::Database.new ARGV[0]

Dir.chdir(__dir__)

# schema

db.execute 'PRAGMA foreign_keys = ON;'

db.execute File.read 'users.sql'
db.execute File.read 'users_cheatsheet.sql'
db.execute File.read 'topics.sql'
db.execute File.read 'subscriptions.sql'
db.execute File.read 'messages.sql'

def bulk_insert(db, objs, delay = nil)
  db.execute 'BEGIN TRANSACTION;'

  stmts = {}

  objs.each_with_index do |obj, i|
    unless stmts[obj.class]
      stmts[obj.class] = db.prepare obj.class.statement
    end

    stmts[obj.class].execute *obj.to_statement_parameters
    obj.id = db.last_insert_row_id

    if delay
      print "\r#{i} / #{objs.length}"
      sleep delay
    end
  end
  db.execute 'COMMIT;'
  if delay
    puts "\r#{objs.length} / #{objs.length}"
  end
end

# users

User = Struct.new(:id, :name, :password,
                  keyword_init: true) do
  def self.statement
    <<SQL
INSERT INTO USERS (name, password_digest) VALUES (?, ?)
SQL
  end

  def password_digest
    SCrypt::Password.create password
  end

  def to_statement_parameters
    [name, password_digest]
  end
end

users = USER_COUNT.times.map do
  name = [*(WORDS.sample(3)),
          rand(9_999_999)].join(SEPS.sample)
  password = [*(WORDS.sample(5)),
              rand(9_999_999_999)].join(SEPS.sample)

  User.new(name: name, password: password)
end

bulk_insert(db, users)

UserCheat = Struct.new(:id, :name, :password,
                       keyword_init: true) do
  def self.statement
    <<SQL
INSERT INTO users_cheatsheet (name, password) VALUES (?, ?)
SQL
  end

  def to_statement_parameters
    [name, password]
  end
end

user_cheatsheet = users.map do |u|
  UserCheat.new name: u.name, password: u.password
end
bulk_insert(db, user_cheatsheet)

# topics

Topic = Struct.new(:id, :name,
                   keyword_init: true) do
  def self.statement
    "INSERT INTO topics (name) VALUES (?)"
  end

  def to_statement_parameters
    [name]
  end
end

topics = WORDS.map do |word|
  Topic.new(name: word)
end

bulk_insert(db, topics)

# subscriptions

Subscription = Struct.new(:id, :topic_id, :user_id,
                          keyword_init: true) do
  def self.statement
    "INSERT INTO subscriptions (user_id, topic_id) VALUES (?, ?)"
  end

  def to_statement_parameters
    [user_id, topic_id]
  end
end

subscriptions = users.map do |u|
  topics.map do |t|
    next nil if SUBSCRIPTION_PROBABILITY < rand()

    Subscription.new(topic_id: t.id, user_id: u.id)
  end.compact
end.flatten

# deferring subscription insert

# # messages

Message = Struct.new(:id, :topic_id, :content,
                     keyword_init: true) do
  def self.statement
    "INSERT INTO messages (topic_id, content) VALUES (?, ?)"
  end

  def to_statement_parameters
    [topic_id, content]
  end
end

messages = MESSAGE_COUNT.times.map do
  Message.new(topic_id: topics.sample.id,
              content: WORDS.sample(5).join(' '))
end

pile = (subscriptions + messages).shuffle

bulk_insert(db, pile, 0.01)
