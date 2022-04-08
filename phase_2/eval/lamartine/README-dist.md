# Lamartine

This `README-dist.md` details operation for evaluation participants.

The `README.md` file includes details of challenge internals and
vulnerabilities. It should not be packaged for distribution to
evaluation participants.   

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
