# Lamartine

This README includes details of challenge internals and
vulnerabilities. It should not be packaged for distribution to
evaluation participants. A separate `README-dist.md` will detail operation
for participants.

**Do not distribute this README to the control team or CHESS teams during
the evaluation event.**

## Description

Lamartine creates SVG[^svg] images from Doom[^doom] maps. It supports the
original 1993
format with increased limits on some entities. It also supports UDMF[^udmf]
`TEXTMAP` maps.


[^svg]: Scalable Vector Graphics, or "SVG", is an file format
    describing graphics made of lines and polygons. When including pixel-based
    raster graphics, it supports embedding these graphics in the file.
[^doom]: Doom (often stylized "DOOM") and its sequel "Doom 2" are the foundation
    of modern shooter video games. Released in 1993 and 1994, they feature
    maps consisting of vertexes, lines ("linedefs") connecting vertexes,
    sides ("sidedefs") of lines that reference sectors, and sectors with
    ceilings and floors. Doom resource files are referred to as "WAD" files,
    which contain multiple "lumps" Where All the Data are.

[^udmf]: "Universal Doom Map Format," a text-based representation of Doom maps.
    It's less efficient than the classic format, but much more flexible.
    Instead of the "classic" format of
    storing vertexes, lines, etc. in separate places in the WAD file,
    it's a single lump containing text data that must be parsed.

### Scope for Analysis

The code outside of the `node_modules` directory is in scope. The code in the
`node_modules` directory does not have any intended weaknesses.

### Running It

Lamartine was built with TypeScript. It can either be run as-is, or compiled to
different flavors of JavaScript/ECMAScript.

```
# Running the TypeScript without a compile step
guest> npx ts-node-script src/main.ts

# Compiling the TypeScript, then running it
guest> npx tsc
guest> node ./build/main.js

# Compiling to ECMAScript 5, then running it
guest> npx babel src --out-dir build --extensions ".ts"
guest> node ./build/main.js
```

The challenge has several environment variables for configuration:

* `CHESS`: this environment variable is checked on startup; it doesn't need to
  be a particular value, just there.
* `LOG_LEVEL`: sets the log level; it defaults to
  `DEBUG`, but it can be made less noisy. The list of levels are in `logger.ts`
  and `logger.js`

### Notes about Output

The challenge will output errors to its stderr if a malformed map is uploaded.
This is intended behavior. The poller expects the challenge to handle
malformed maps. You can squelch these errors with the `LOG_LEVEL` environment
variable.

### Internals

The map conversion process is designed to keep the somewhat-slow conversion
process out of the HTTP request-response cycle.

1. User uploads map, which goes into `/data/enqueued`, and the user gets
  redirected to the finished-processing URL
2. The `Watcher`, which runs twice a second, notices a map in `/data/enqueued`,
  loads the map, generates the SVG, and puts the generated SVG in
  `/data/done`
3. The user, meanwhile sees a "still processing" message on the map page.
4. The user's browser refreshes the map page and sees the finished map.

Decoupling the conversion process from the HTTP cycle is similar to how much of
the modern web handles slow processes using an asynchronous job queue. Our goal
with this challenge is to see if CHESS analysis can identify a weakness that
doesn't happen in direct response to network traffic.

## Input Data

The `poller/classic` and `poller/udmf` directories contain well-formed WAD
files and the SVG files they're expected to generate. Feel free to upload these
WAD files yourself for testing.

Other WAD and SVG files have been held back and may be used for testing
patches to this challenge.

## Textures

Textures are loaded from `/static/flats`; these files are from
[the Freedoom project](https://github.com/freedoom/freedoom), and are PNG files
inlined into the SVG files.

Changing textures will require re-generating the SVG files that the poller
depends on. This is a somewhat manual process using shell automation and
`pwad_test.ts`.

## Poller

Poller errors will dump the last-fetched request body in `poller/tmp/`, named
with a timestamp and the poller seed. These
can be deleted if you're not using them for debugging.

### Persistent Data

Running the poller generates a lot of `/data` for the challenge. It's
preferable to
delete this between poller runs, since it takes disk space, inodes, and
processing time to generate the list of already-processed maps.
It is safe to leave it between poller runs, since the poller doesn't make any
assumptions about the presence or absence of data, and the files get random
UUIDv4 names.

## Vulnerabilities and Proofs

### Unsafe Texture Loading

When loading a floor texture ("flat") for inclusion into an SVG, the texture
name is unsafely concatenated into a path:

```typescript
let image_data = fs.readFileSync(`/static/flats/${floor.toLowerCase()}`)
```

This turns the texture name `../../token` (which is possible in UDMF/`TEXTMAP`
maps, but not classic maps) into `/static/flats/../../token`,
which becomes `/token`, which leads to a structured privilieged information
disclosure.  The patch is to only accept the base name "token" and format
it into a path:

```typescript
let image_path = path.format({
    dir: '/static/flats',
    name: path.basename(floor.toLowerCase())
})
let image_data = fs.readFileSync(image_path)
```

The `pov_flat_token/PRIV_pwad_contaminator.ts` tool
(move it into `challenge/src`) will
process a map into a UDMF map that references this file. When the texture is
inlined, the base64-encoded token will be embedded in the map.

This vulnerability happens outside a direct user request. The PoV goes through
the normal flow of uploading a map, waiting for it to be processed, and
reading the token out of the generated SVG.

```ruby
image_in_pattern = map_page.css('pattern image')
assert(image_in_pattern.first,
  "no patterns with images, it's probably the error svg")
  
assert((image = image_in_pattern.first['href']))
content = image.split('base64,').last
token = Base64.decode64(content)

puts "TOKEN=#{token}"
```

This PoV fails with the "no patterns with images" text when run against the
reference patch. Patches that do not trigger this message may be acceptable.

### Unsafe SVG Loading

When loading a processed SVG, the filename is not adequately validated by the
route specification:

```typescript
app.get(/^\/maps\/(.+)\.svg$/,
```

This route matches any URL beginning with `/maps/` and ending with `.svg`,
which includes `/maps/../../token.svg`, which loads `/token`, leading to another
unprivileged information disclosure. A similar mistake is made for the
HTML version of this route.

A possible fix is to only match UUIDs:

```typescript
app.get(/^\/maps\/([0-9a-f]{8}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{12})\.svg$/,
```

Other possible fixes include more thoughtfully building the load path,
rewriting more of the challenge to use `.svg` in paths to SVG files,
or rewriting the challenge to save the maps and html pages in `/data` and using
the `express.static` function to serve these files directly.

The reference PoV makes the request to both the HTML and SVG versions of the
weakness, and outputs tokens for both. If neither one appears to have worked,
it will fail with the message "expected at least one success."

```ruby
map_page_uri = @base_uri.dup
map_page_uri.path = '/maps/../../token.html'

map_page = agent.get map_page_uri
```

## Other Development Notes

### Generating the Parser

```
guest> npx pegjs --plugin ./node_modules/ts-pegjs/src/tspegjs \
  --extra-options-file ../priv/ts-pegjs-options.json \
  -o src/udmf.ts ../priv/udmf.peg
```