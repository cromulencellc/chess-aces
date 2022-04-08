# quarley

This `README-dist.md` details operation for evaluation participants.

The `README.md` file includes details of challenge internals and
vulnerabilities. It should not be packaged for distribution to
evaluation participants.   

Quarley is a hangman game from the "wt" web toolkit example collection.

## Building

Quarley is built with `cmake`.

1. `cd challenge/wt-4.5.0`
2. `mkdir build`
3. `cd build`
4. `cmake -DMULTI_THREADED=false ../`
5. `make`
6. `make -C examples`

## Running

Quarley depends on the `/static` directory to be populated, and creates a
database in the `/data` directory (which can start present but empty).

1. `cd challenge/wt-4.5.0/build`
2. `./examples/hangman/hangman.wt --docroot /static/docroot --http-listen 0:3025`

## Scope for Analysis

The entire `wt` framework and `hangman` example are expected to be analyzed.
This includes the code in
`challenge/wt-4.5.0/src` and
`challenge/wt-4.5.0/examples/hangman`, or basically any code that goes in to
the `challenge/wt-4.5.0/build/examples/hangman/hangman.wt` executable.

## Poller

The Quarley poller creates accounts, tests the failure cases for logging in,
and can log in and play hangman, either to win or to lose.

Due to some uninvestigated internals of the "wt" toolkit, some kind of
settlement/propagation of session data has to take place, so the poller takes
efforts to allow this settlement. In particular, the "wrong password" case and
logging in may require retries and lots of `sleep`.

Environment variables are used to configure the poller:

* `HOST`: the hostname or IP address the poller will attempt to work with,
  defaults to `challenge`
* `PORT`: the port number the poller will attempt to work with, defaults to
  `3025`.
* `OOPS_ALL`: a poller activity to focus on, useful for testing a single code
  path, doesn't have a default
* `LENGTH`: how many activities to try, defaults to a random number between
  50 and 100
* `SEED`: RNG seed
* `LOGGER_MIN_LEVEL`: how much noise to output to stderr; defaults to `DEBUG`,
  but `INFO`, `WARN`, `ERROR`, and `FATAL` will reduce the amount of noise
* `SETTLE_SLEEP`: how many seconds to `sleep` between requests; defaults to
  1 second, but can be set to `0.5` or lower for faster but less reliable
  testing
* `LOGIN_RETRIES`: how many times to retry logging in when it does
  something weird; defaults to `100`, lower values will fail faster
* `TIMEOUT`: how many seconds a given activity is allowed;
  defaults to `30` seconds
