CREATE TABLE messages (
  id integer primary key,
  topic_id integer REFERENCES topics (id) NOT NULL,
  content text,
  created_at datetime DEFAULT CURRENT_TIMESTAMP
  );
CREATE INDEX messages_topic_id ON messages(topic_id);