\set ON_ERROR_STOP on
\set ECHO queries

DROP TABLE IF EXISTS satellites;
CREATE TABLE satellites
  (
    id UUID PRIMARY KEY,
    created_at TIMESTAMP DEFAULT now() NOT NULL
  );

DROP TABLE IF EXISTS searches;
CREATE TABLE searches
  (
    id UUID PRIMARY KEY default gen_random_uuid(),

    satellite_ids UUID[] NULL,
    tx_at_gt TIMESTAMP NULL,
    tx_at_lt TIMESTAMP NULL,

    bt_level_gt DOUBLE PRECISION NULL,
    bt_level_lt DOUBLE PRECISION NULL,

    created_at TIMESTAMP DEFAULT now() NOT NULL
  );

DROP TABLE IF EXISTS telemetry;
CREATE TABLE telemetry
  (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    satellite_id UUID REFERENCES satellites (id) NOT NULL,
    tx_at TIMESTAMP NOT NULL,

    pv_x DOUBLE PRECISION,
    pv_y DOUBLE PRECISION,
    pv_z DOUBLE PRECISION,

    pv_dx DOUBLE PRECISION,
    pv_dy DOUBLE PRECISION,
    pv_dz DOUBLE PRECISION,

    bt_level DOUBLE PRECISION,
    bt_charging_time BIGINT,
    bt_discharging_time BIGINT,

    created_at TIMESTAMP DEFAULT now() NOT NULL
  );
CREATE INDEX ON telemetry
  (created_at);
CREATE INDEX ON telemetry
  (satellite_id, created_at);

DROP TABLE IF EXISTS users;
CREATE TABLE users
  (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    name CHARACTER VARYING,
    password_digest BYTEA,
    is_admin BOOLEAN NOT NULL DEFAULT FALSE,

    created_at TIMESTAMP DEFAULT now() NOT NULL
  );
CREATE INDEX ON users
  (name);

DROP TABLE IF EXISTS satellites_users;
CREATE TABLE satellites_users
  (
    satellite_id UUID REFERENCES satellites (id) NOT NULL,
    user_id UUID REFERENCES users (id) NOT NULL,
    PRIMARY KEY (satellite_id, user_id)
  );
CREATE INDEX ON satellites_users
  (satellite_id);
CREATE INDEX ON satellites_users
  (user_id);

DROP TABLE IF EXISTS tokens;
CREATE TABLE tokens
  (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    token CHARACTER VARYING,

    created_at TIMESTAMP DEFAULT now() NOT NULL
  );

CREATE OR REPLACE FUNCTION telemetry_notify()
  RETURNS TRIGGER
  LANGUAGE plpgsql
  AS
  $$
    DECLARE
      body CONSTANT TEXT := (NEW.id || E'\x1f' || NEW.satellite_id);
    BEGIN
      PERFORM (
        SELECT pg_notify('telemetry_inserts', body)
      );
      RETURN NULL;
      END;
  $$;

CREATE TRIGGER telemetry_notify_trigger
  AFTER INSERT
  ON telemetry
  FOR EACH ROW
  EXECUTE PROCEDURE telemetry_notify();

CREATE OR REPLACE FUNCTION unsubscription_notify()
  RETURNS TRIGGER
  LANGUAGE plpgsql
  AS
  $$
    DECLARE
      body CONSTANT TEXT := (E'UNSUB\x1f' ||
        OLD.user_id || E'\x1f' || OLD.satellite_id);
    BEGIN
      PERFORM (
        SELECT pg_notify('user_satellite_changes', body)
      );
      RETURN NULL;
      END;
    $$;

CREATE TRIGGER unsubscription_notify_trigger
  AFTER DELETE
  ON satellites_users
  FOR EACH ROW
  EXECUTE PROCEDURE unsubscription_notify();

CREATE OR REPLACE FUNCTION subscription_notify()
  RETURNS TRIGGER
  LANGUAGE plpgsql
  AS
  $$
    DECLARE
      body CONSTANT TEXT := (E'SUB\x1f' ||
        NEW.user_id || E'\x1f' || NEW.satellite_id);
    BEGIN
      PERFORM (
        SELECT pg_notify('user_satellite_changes', body)
      );
      RETURN NULL;
      END;
    $$;

CREATE TRIGGER subscription_notify_trigger
  AFTER INSERT
  ON satellites_users
  FOR EACH ROW
  EXECUTE PROCEDURE subscription_notify();
