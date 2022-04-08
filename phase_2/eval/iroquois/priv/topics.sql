CREATE TABLE topics (
  id integer PRIMARY KEY,
  name varchar NOT NULL,
  created_at datetime DEFAULT CURRENT_TIMESTAMP,
  UNIQUE (name)
  );
