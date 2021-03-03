# Kane

## What is Kane
Kane is a HTTP server that accepts GET requests that returns wiki pages. With each request there is an optional header wherein you can define user level, ADMIN, ANONYMOUS, USER. By adding an optional Role header to the HTTP request you can define user level. This is checked by a master list which keeps USER and ADMIN level accounts usernames and passwords. This master list can have new users added to it and current users edited and deleted from it through the function of an admin console that is behind the curtain, listening on a support port from how it handles HTTP requests. In this admin console you are also able to manipulate the origin files of the server. These text files follow a structure in such a way where the server can convert them to html files. This is done as the server is launched it will convert the files in `/data/wiki/origins/` to html files and store them in `/data/wiki/cache/`.

## Challenge
Through docker you can go into a bash shell of the challenge with the following command:  
docker-compose run --rm --name challenge challenge bash

## Poller
It is important that between runs of the poller that you delete `/data` and copy `/base_content` to a folder called  `/data`. This allows for the poller to know the initial state of the challenge.
For running the poller you can do:  
docker-compose run --rm --workdir /poller -e HOST=challenge -e HTTP_PORT=1080 -e ADMIN_PORT=1234 poller python kane-poller.py

## Pov
For running the pov you can do:  
docker-compose run --rm --workdir /pov -e HOST='name_of_docker_container' -e HTTP_PORT=1080 -e ADMIN_PORT=1234 -e URI=/wiki/cache/Fellowship.html pov python pov.py  
The vulnerability for Kane is one of access control. If you make a GET request this will generate a role table. In this role numbers are associated with the various user levels, Anonymous 0, User 1, Admin 2. This role table is then stored on the server. Where this is vulnerable is that there are cookies associated to each user level, and the buffer that holds the name in the role entry struct is right above role number struct variable that tracks role numbers. Due to name being a 10 character static buffer that is intialized with strlen you can fill up the buffer with whatever you from the role header value. From here it is possible to overwrite the role number to which user level you want, in the case of this Proof of Vulnerability Admin. You can take the cookie that is returned in the response and send that back up in a HTTP Request and get access to the admin console, thus the bug.
