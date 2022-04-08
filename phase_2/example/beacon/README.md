# Beacon

This document discusses the architecture and vulnerabilities of this CHESS
challenge. If you would like to use this example without this understanding,
please only read the first section (stop before "weakness and pov")

## Running It

Beacon is the first node.js challenge introduced to the CHESS program. It
exposes an HTTP server.

```
host> docker-compose run --rm --service-ports challenge bash
guest> cd /mnt/challenge
guest> node src/main.js
```

### HTTP Endpoints

There's a basic banner at `http://challenge:3028/`, but most of the challenge
is at `http://challenge:3028/tweets/`, `http://challenge:3028/orders/`, and `http://challenge/3028/login/`.

### Dependencies

Beacon is built for Node.js 14 LTS. The `challenge/Dockerfile` sets this up on
Ubuntu 20.04.

The NPM (Node Package Manager) libraries Beacon depends on require some compiled
modules, including LevelDB. As such, it also depends on the `build-essential`
Ubuntu package for
installation.

#### ECMAScript 5 and Babel

The "Babel" JavaScript compiler has been added to convert the Node.js 14
compatible code to ECMAScript 5 (ES5) for analysis purposes. This is not
automated as part of a build process. From a `challenge`, `unpatched`, or
`fully_patched` directory:

```
guest> npx babel src/ --out-dir build
```

### Challenge Environment Variables

* `PORT` - the TCP port to listen on, defaults to 3028
* `STATIC_DIR` - the directory to find static files (images, css) in, defaults
  to `/static/static`
* `VIEW_DIR` - the directory to find templates (like, pug templates for
  html) in, defaults
  to `/static/view`
* `DONT_DELETE_CHEATSHEET` - the `/data/users_cheatsheet` directory is used
  by the poller, and should not be usable by the challenge or a PoV. The
  default behavior is to delete it, but you might want that to not happen.

The `challenge` service in `docker-compose.yml` may set these for dev work,
but the defaults are hard-coded.


### Files and Directories

Beacon stores a few different things in `/data`.

* `/data/tweets` is collection of status updates. It's stored as JSON blobs
  in a [LevelDB][leveldb-wp] collection.
* `/data/orders` is a collection of e-commerce orders, as JSON blobs in LevelDB.
* `/data/photos` is a directory of images that are optional attachments to
  status updates. Images are named by the ID of the status.
* `/data/users` is a LevelDB collection of users. Each user has a name and
  password digest (stored using `scrypt`); some users have an `is_admin` field.
* `/data/users_cheatsheet` is a LevelDB collection of users and their plaintext
  password. It is used by the poller to test logging in. The challenge should
  delete this file as part of its startup procedure. Proofs-of-vulnerability
  MUST NOT depend on knowledge of this data.

[leveldb-wp]: https://en.wikipedia.org/wiki/LevelDB

Beacon also has some static files:

* `/static/view` is mostly [Pug][pugjs] templates
* `/static/static` contains CSS files and other files that don't need processing
  prior to being served

[pugjs]: https://pugjs.org/

### Generating Data Directories

The `priv/leveler` directory contains a couple scripts for generating the
`orders` and `users`/`users_cheatsheet` directories. These run in the
`leveler` docker-compose environment, and output their data into the container's
`/tmp`; copy this directory somewhere for long-term keeping.

The `priv/poster.rb` file is a script for batch-posting tweets (with content
from `moby-dick.txt` and photos from the poller's collection.
It does require some work to point it at a running
challenge.

## Poller

It's entirely possible for the poller's data storage to get out of sync with the
challenges; it attempts to commit created objects to its own leveldb, but it
might fail. It's a good idea to destroy and re-provision the `/data` directories
on both the
challenge and poller after a poller run.

The poller uses "Playwright," a browser automation system, to request and
validate responses from the challenge. It's kind of slow, but also accurate.
When an assertion fails, it will try and save a screenshot into
a `tmp/` directory under where the poller is running. This is useful for
debugging, but may take up significant space if not cleaned out.

## Weaknesses and PoVs

### Path Traversal with Photo Tweets

Tweet photos are accessed via a GET to
path `/tweet/TWEET_ID/photo`, and served from
`/data/photos`. Performing a `GET` to `/tweets/%2e%2e%2f%2e%2e%2ftoken/photo`
can traverse to expose `/token`.

Several fixes are possible:

* restrict the tweet-id part of the URL to the format of a tweet id
* validate that the tweet exists and use its id to generate the file
  path
* storing image blobs in the database to preclude path traversal and also
  support database-like scaling of storage long-term

### Data Misuse to Decode Order PINs

UI and logic for filtering orders by price exists, but that same logic can
also be used to filter orders by their PIN, which is used to authenticate
access to privileged order information like the Primary Account Number (PAN)
used to make the purchase (the PIN is also privileged). By doing a binary search
using the filter on PIN, the exact PIN for an order can be calculated, allowing
authenticated access.

The fix for this is to only allow filtering by price, and not by arbitrary
fields.

### Data Injection to Create a New Admin User

The user creation process at `/login/new` puts all form parameters in the new
user's database object, including an `is_admin` parameter not exposed in the UI.

The fix for this is to only allow specific fields (`name`, and the two
password digest fields) to pass through into the new user object.
