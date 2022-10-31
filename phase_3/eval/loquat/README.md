# Loquat

This README includes details of challenge internals and
vulnerabilities. It should not be packaged for distribution to
evaluation participants. A separate `README-dist.md` will detail operation
for participants.

**Do not distribute this README to the control team or CHESS teams during
the evaluation event.**

## Description

Loquat is a satellite telemetry storage and querying system.
It receives telemetry over UDP, 
stores it in PostgreSQL,
allows users to stream real-time telemetry over websockets,
and also allows users to search for telemetry using
a persistent stored search.

The app requires basic authentication for users.
A user should, under no circumnstances, 
see information about a satellite they
have not been granted access to by an admin,
or if an admin has revoked their access.

Proofs-of-vulnerability may use
`ADMIN_USERNAME` and `ADMIN_PASSWORD` to set up required
conditions for demonstrating a vulnerability.

### Scope for Analysis

The code outside of the `node_modules` directory is in scope. 
The code in the
`node_modules` directory does not have any intended weaknesses.

### Requirements Before Running

Loquat depends on `/token` existing, 
and the poller may benignly check the contents of this file.
It can be initialized as such:

```
guest> echo asdf > /token
```

The poller can be configured to check this with
the `EXPECT_TOKEN` environment variable. 
Whitespace is trimmed automatically,
so the above token (which has a trailing newline)
will work with a poller invocation:
`EXPECT_TOKEN=asdf node build/poller.js`

Loquat also depends on the files in `static`,
which should be copied to `/static`:

```
guest> cp -R /mnt/static /static
```

### Running It

Loquat was built with TypeScript. It can either be run as-is, or compiled to
different flavors of JavaScript/ECMAScript.

```
# Compiling the TypeScript, then running it
guest> npx tsc && node ./build/main.js
```

The challenge has several environment variables for configuration:

* `CHESS`: this environment variable is checked on startup; it doesn't need to
  be a particular value, just there.
* `LOG_LEVEL`: sets the log level; it defaults to
  `DEBUG`, but it can be made less noisy. The list of levels are in `logger.ts`
  and `logger.js` . `WARN` is a good choice.
* `HTTP_PORT`: defaults to 3080
* `TLM_PORT`: defaults to 3081
* `DATABASE_URL`: URL of the postgres server the challenge
  connects to. The poller and proofs-of-vulnerability do not
  have access to this server.
* `CLOBBER_ADMIN`: set this environment variable to
  delete all admin users and 
  create a new admin user with known password. 
  See the "Persistent Data" section below.

### Notes about Output

The challenge will output errors and other diagnostic
information to its stderr.
This is intended behavior.
You can squelch these errors with the `LOG_LEVEL` environment
variable.

### Persistent Data

This challenge stores data in PostgreSQL, which `docker-compose`
should be configured to start and initialize automatically,
using the `priv/docker-entrypoint-initdb` directory.

The poller depends on an administrator user,
which the challenge will create automatically if one doesn't exist:

```
[WARN] created new admin user ADMIN_USERNAME='admin 29886' ADMIN_PASSWORD='85273243-2bb9-4523-84cc-f3784de76fbb'
```

These variables must be set on the poller for it to work.

To force all admin users to be deleted,
and a new admin user created with a known password,
set the `CLOBBER_ADMIN` environment variable when starting
the challenge.


## Poller

The poller depends on an admin username and password
being passed in via the
`ADMIN_USERNAME` and `ADMIN_PASSWORD` environment
variables. 
See the above "Persistent Data" section for more
information.

The poller requires `static` to be present at `/static`.

The poller generates a lot of data in
the challenge during normal operation.
The challenge and poller have been tested extensively with stale
data lingering, and it shouldn't cause problems.

```
guest> npx tsc && \
  HOST=loquat_challenge_run_d9d3284f31c7 \
  ADMIN_USERNAME='admin 82290' \
  ADMIN_PASSWORD='d7b7464a-0136-4f8a-8782-8be481cb00c3' \
  node build/main.js
```

## Vulnerabilities and Proofs

### TOCTOU During Telemetry Connection

* CWE-367 TOCTOU Race Condition

A user making an authenticated request for telemetry normally
has their access (via the `satellites_users` table)
checked when starting the connection.
This means that if their access is later revoked,
they'll receive updates they shouldn't for the duration of
that connection.

`pov_still_subscribed` demonstrates this.
This POV requires 
`ADMIN_USERNAME` and `ADMIN_PASSWORD` environment
variables
(similar to the poller) to demonstrate the access revocation.

### Parameter Injection on User Creation

* CWE-233 Improper Handling of Parameters

When a user creates an unprivileged account,
they're able to add an `is_admin` field tothe POST request
and become an admin.
This is bad.

`pov_user_parameter_injection` demonstrates this.

### TOCTOU When Using a Shared Search Link

* CWE-200 Exposure of Sensitive Information to an Unauthorized Actor
* CWE-306 Missing Authentication for Critical Function
* CWE-862 Missing Authorization


When anyone creates a search link,
they're able to access telemetry even after their
access is revoked.

`pov_shared_search` demonstrates this.

Since satellite IDs aren't predictable, 
the risk of an unauthenticated user guessing a satellite ID
is mitigated.

## Development Notes

### Processing TLE.json from keeptrack.space

```sh
jq \
  '[.[] | select(has("ON")) | select(.ON | test("NAVSTAR"))]' \
  keeptrack.space/src/tle/TLE.json \
> loquat/priv/navstar_tle.json
```
