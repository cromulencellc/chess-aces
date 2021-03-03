# vernoa

(Originally "jitter_bugs")

Start the challenge with:

```
docker-compose run --rm --name jb challenge
```

Run the poller with:

```
docker-compose run --rm --name jbp -e HOST=jb -e PORT=3008 poller
```

If you need to exploit the service and get a shell in the docker, have the
`poller.py` script call `jitter_bugs.exploit_interactive()`.

# pmccabe

```
src % pmccabe -T -v *.c assembler/*.c container/*.c runtime/*.c
Modified McCabe Cyclomatic Complexity
|   Traditional McCabe Cyclomatic Complexity
|       |    # Statements in function
|       |        |   First line of function
|       |        |       |   # lines in function
|       |        |       |       |  filename(definition line number):function
|       |        |       |       |           |
401	657	1556	n/a	4068	Total
```
