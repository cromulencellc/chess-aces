# iroquois

This `README-dist.md` details operation
for participants.

The `README.md` file includes details of challenge internals and
vulnerabilities. It should not be packaged for distribution to
evaluation participants. 

## Description

"Iroquois" is a web service that provides an interactive MQTT[^mqtt] client
environment, allowing users to register, log in, publish, subscribe, and
view messages.

[^mqtt]: "MQ Telemetry Transport" (MQTT) is a network protocol that transports
  messages from publishers, through a broker, to subscribers. It's often used
  in Internet of Things scenarios.

### Scope for Analysis

This service processes HTTP requests and MQTT messages. Both of these interfaces
are in-scope for analysis.

This service uses a SQLite3 database for storing state. The structure of this
database is guaranteed (see the "Persistent Data" section below), but the
content may vary. Proofs-of-Vulnerability (PoVs) MAY expect these tables to be
consistent, but MUST NOT expect specific contents in these tables.
PoVs MUST NOT depend on the existence of a `users_cheatsheet` table; the
challenge attempts to delete this table on startup, and it's
only used by the poller.

Users are expected to treat their subscriptions as their own. They shouldn't
know what other users are on the system, what topics other users subscribe to that they don't know about.

### Running It

Iroquois was built with TypeScript. It can either be run as-is, or compiled to
different flavors of JavaScript/ECMAScript.

```
# Running the TypeScript without a compile step
guest> npx ts-node-script src/main.ts

# Compiling the TypeScript, then running it
guest> npx tsc
guest> node ./build/main.js

# Compiling to ECMAScript 5, then running it
guest> npx babel src --out-dir build --extensions ".ts"
guest> node ./build/main.js
```

The challenge has several environment variables for configuration:

* `CHESS`: this environment variable is checked on startup; it doesn't need to
  be a particular value, just there.
* `MQTT_URL`: the URL to the MQTT broker; it
  defaults to `tcp://mosquitto:1883`
* `LOG_LEVEL`: sets the log level; it defaults to
  `DEBUG`, but it can be made less noisy. The list of levels are in `logger.ts`
  and `logger.js`

### MQTT Access

This service needs access to an MQTT server. The `docker-compose.yml` runs
one with the name `mosquitto` using the `eclipse-mosquitto` image. The
challenge, poller, and PoVs are expected to have access to the same MQTT
server.

### Persistent Data

Data is stored in `/data/iroquois.sqlite3`, in the following tables:

* `users`: `id` (integer pk), `name` (varchar),
  `password_digest` (scrypt),
  `created_at` (datetime)
* `topics`: `id` (integer pk), `name` (varchar)
* `subscriptions`: `id` (integer pk), `user_id` (integer fk),
  `topic_id` (integer fk),
  `created_at` (datetime)
* `messages`: `id` (integer pk), `topic_id` (integer fk),
  `content` (text),
  `created_at` (datetime)
* `users_cheatsheet`: `id` (integer pk), `name` (varchar),
  `password` (varchar)

The `users_cheatsheet` table will be deleted during the challenge
launch; the poller needs plaintext passwords, but the challenge does not, and
they're not in scope for proofs-of-vulnerability.

## Poller Operation

The poller is implemented in Ruby. It uses the `mechanize` gem (library)
to simulate
a web browser, and the `mqtt` gem to perform MQTT operations.

The poller has several environment variables for configuration:

* `HOST` and `PORT`: the host and port number the challenge is available
  on. Defaults to `challenge` and `3035`, respectively.
* `MQTT_URL`: the URL to the MQTT broker; it defaults to `hqtt://mosquitto:1883`