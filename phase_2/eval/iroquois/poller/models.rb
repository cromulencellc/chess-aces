require 'active_record'

class Message < ActiveRecord::Base
  belongs_to :topic

  after_save :prune

  def prune
    self.class.connection.execute <<SQL
        WITH recent_messages AS (
      SELECT id,
        row_number()
          OVER(PARTITION BY topic_id ORDER BY id DESC) AS most_recent
        FROM messages
      ),
    to_delete AS (
      SELECT id FROM recent_messages WHERE most_recent > 100)
    DELETE FROM messages WHERE id IN to_delete;
SQL
  end
end
class Subscription < ActiveRecord::Base
  belongs_to :topic
  belongs_to :user

  def messages
    topic.messages.
      where('created_at >= datetime(:newer_than)',
            newer_than: created_at.iso8601).
      order('created_at desc')
  end
end
class Topic < ActiveRecord::Base
  has_many :subscriptions
  has_many :messages

  def self.prune
    connection.execute 'reindex'
    transaction do 
      connection.execute("CREATE TEMP TABLE prune_topics AS \
      SELECT id, name FROM topics \
      WHERE id NOT IN (SELECT DISTINCT topic_id FROM subscriptions);")

      connection.execute("DELETE FROM topics WHERE id IN (SELECT id FROM prune_topics);")
      
      connection.execute("DROP TABLE prune_topics")
    end
  end
end
class User < ActiveRecord::Base
  has_many :subscriptions
end
class UserCheatsheet < ActiveRecord::Base
  self.table_name = 'users_cheatsheet'
end
