# Xenia (Very Fast Graph Database)

Start the challenge with:

```
docker-compose run --rm --name challenge challenge
```

Run the poller with:

```
docker-compose run --rm -e HOST=challenge -e PORT=3008 poller
```

## Running the PoV

The PoV needs access to the binary it's being tested against.

In this case, the binary is placed in `/variants/unpatched/xenia.bin`, and
running on `xenia_challenge_run_6` on port `3008`:

```
python3 pov.py /variants/unpatched/xenia.bin xenia_challenge_run_6 3008
```

A successful PoV prints out the TOKEN value.
