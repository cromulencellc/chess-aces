CREATE TABLE subscriptions (
  id integer PRIMARY KEY,
  user_id integer REFERENCES users (id) NOT NULL,
  topic_id integer REFERENCES topics (id) NOT NULL,
  created_at datetime DEFAULT CURRENT_TIMESTAMP,
  UNIQUE (user_id, topic_id) ON CONFLICT ROLLBACK
  );
CREATE INDEX subscriptions_user_id ON subscriptions(user_id);
CREATE INDEX subscriptions_topic_id ON subscriptions(topic_id);