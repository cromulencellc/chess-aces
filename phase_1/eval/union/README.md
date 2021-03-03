# Union

## What is Union?
Union is a HTTP server service that allows for session at a time to do lists. So when interacting with the server a session_id is created, you can then create lists, which inside of those lists keep track of to-dos for whatever is needed.

## Challenge
docker-compose run --rm --name challenge challenge bash spawns a shell in side docker container. Once inside you can do make run to run the service

## Poller
docker-compose run --rm --workdir /poller -e HOST=challenge -e PORT=3008 -e poller python union-poller.py

## Pov
docker-compose run --rm --workdir /pov -e HOST=challenge -e PORT=3008 -e pov python pov.py
The vulnerability is a buffer over through through the body that you can control via the Content-Length Header. This allows for register control of RBP and RIP