# iroquois

This README includes details of challenge internals and
vulnerabilities. It should not be packaged for distribution to
evaluation participants. A separate `README-dist.md` will detail operation
for participants.

**Do not distribute this README to the control team or CHESS teams during
the evaluation event.**

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

## Vulnerabilities

### Access Control Vulnerability when Viewing Subscriptions

When viewing a subscription, the unpatched version does not check that
the subscription is owned by the current user.

```js
  // ifdef PATCH_ONLY_DELETE_OWN_SUBSCRIPTIONS
  let sub = current_user.findSubscription(sub_id)
  // else
  let sub = Subscription.find(sub_id)
  // endif
```

This allows a malicious user to discover which topics other users are
subscribed to, and messages that have been received for those topics.

This PoV's only two environment variables are `HOST` and `PORT`:
the host and port number the challenge is available
on. Defaults to `challenge` and `3035`, respectively.

### Code Injection Vulnerability when Receiving Messages

Messages received by MQTT do not escape their content before interpolating
it into a SQL statement. This allows a malicious user with MQTT access
to execute arbitrary
queries, including exfiltrating data from the database.

```js
    // ifdef PATCH_USE_PREPARED_STATEMENTS_FOR_MESSAGE_SAVE
    let stmt = global.iroquois.db.prepare(
      "INSERT INTO messages (topic_id, content) \
VALUES (CAST(:topic_id AS INT), :content)")
    let _got = stmt.run({topic_id: this.topic_id, content: this.content})
    // else
    global.iroquois.db.exec(
      `INSERT INTO messages (topic_id, content) \
VALUES (CAST(${this.topic_id}) AS INT),
${this.content});`)
    // endif
```

This allows a malicious user to dump all information from the database,
including password hashes, all the topics, and more.

This PoV has several environment variables for configuration:

* `HOST` and `PORT`: the host and port number the challenge is available
  on. Defaults to `challenge` and `3035`, respectively.
* `MQTT_URL`: the URL to the MQTT broker; it defaults to `hqtt://mosquitto:1883`