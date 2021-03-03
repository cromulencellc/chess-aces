# hamlin

* png (maybe not, DEFLATE looks hard?)
* ppm
* Hamlin Run-Length (hrl)

## development and testing

from `hamlin` (i.e. not from `hamlin/challenge`)

```sh
host> docker-compose run --name=hamlin1 challenge bash
challenge guest> make run
```

from the same `hamlin` directory

```sh
host> docker-compose run poller bash
poller guest> HOST=hamlin1 ruby poller.rb
```