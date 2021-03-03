# evarts

Evarts is a burger construction kit with a SQL database of ingredients.

## running the challenge

The challenge defaults to running on port 3008; this is adjustable with the
`PORT` environment variable.

## running the poller

The poller expects `HOST` and `PORT` environment variables to be set.

## running the pov

The proof-of-vulnerability expects `HOST` and `PORT` environment variables to
be set.

In addition, the proof-of-vulnerability requires access to the currently-running
binary. It looks for it by default in `/target/evarts`, but this path can be
adjusted with the `BINARY_PATH` environment variable.
