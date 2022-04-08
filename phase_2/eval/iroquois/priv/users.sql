CREATE TABLE users (
  id integer PRIMARY KEY,
  name varchar NOT NULL,
  password_digest varchar NOT NULL,
  created_at datetime DEFAULT CURRENT_TIMESTAMP,
  UNIQUE (name)
  );
