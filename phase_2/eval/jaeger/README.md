# Jaeger

This `README.md` MUST NOT be shared with performers or evaluation teams before
or during the Phase 2 evaluation event. The `README-dist.md` file SHOULD be
shared with evaluation teams during the Phase 2 evaluation event.

## Description

Simulator of multiple RISC-V devices connected via a bus.

## Build instructions

The best way to build this service is probably to let Gitlab CI do it.

### Building Devices

You need to build the `cross-rv32im` docker container. This takes a long time.

```bash
docker build -t cross-rv32im cross-rv32im
```

However, if you are at Cromulence, just use the already built container availble
for you in the Gitlab registry.

```
docker pull registry.mlb.cromulence.com/chess/jaeger/cross-rv32im:latest
docker tag registry.mlb.cromulence.com/chess/jaeger/cross-rv32im:latest cross-rv32im:latest
```

Now build the devices using the docker container:

```
docker run --rm -ti -v $(pwd):/jaeger cross-rv32im
$ cd challenge/devices
$ ./build.sh
```

### Being lazy and just running things right now

(This is a little out of date)

Everything is built and thrown in a docker container using Gitlab CI. You can
just pull it down and run it.

```
docker run --rm -ti registry.mlb.cromulence.com/chess/jaeger/jaeger:master
```

For example:

```
> docker run --rm -ti registry.mlb.cromulence.com/chess/jaeger/jaeger:master
Unable to find image 'registry.mlb.cromulence.com/chess/jaeger/jaeger:master' locally
master: Pulling from chess/jaeger/jaeger
57df1a1f1ad8: Already exists
4d2fb8d6cf1b: Pull complete
3a42fe972bc1: Pull complete
f4b6b310117a: Pull complete
f9f22abc49e7: Pull complete
65a4b91b3e61: Pull complete
60166d085e36: Pull complete
394daa2263d3: Pull complete
407a4eaffdd4: Pull complete
b23f12752b21: Pull complete
cf5e124cc1cb: Pull complete
75dc7c201313: Pull complete
Digest: sha256:3858050a5c8e61e5f30449b72544601f3d2f996d62f885b4a712971e5f8c32f0
Status: Downloaded newer image for registry.mlb.cromulence.com/chess/jaeger/jaeger:master
root@c5bf572e97e5:/challenge# ./test-0
[.] (system_create) vm 0 is enabled, devices/test-0-d0
[+] (system_create) vm 0 (devices/test-0-d0) successfully initialized
[.] (system_create) vm 1 is enabled, devices/test-0-d1
[+] (system_create) vm 1 (devices/test-0-d1) successfully initialized
[.] (system_create) vm 2 is not enabled, skipping
[.] (system_create) vm 3 is not enabled, skipping
[.] (main) System Tick
[.] (system_tick) vm 0 is waiting for interrupt step 47
[.] (system_tick) vm 1 is waiting for interrupt step 47
[.] (main) System Tick
...
```

### Documentation

```
cd docs
python3 assemble.py
pandoc --template template.tex README.md --output documentation.pdf -V geometry:margin=1in
```

## Device Memory Map

```
0x00000000 - 64kb of code
0x00010000 - 64kb of RAM
0x02000000 - uint64_t mtime
0x02000008 - uint64_t mtimecmp 
0x40000000 - Mapped I/O device 1
0x40010000 - Mapped I/O device 2
0x40020000 - Mapped I/O device 3
0x40030000 - Mapped I/O device 4
0x80000000 - Memory-mapped byte to stdout (for debug purposes)
```

## Weakness and Proof of Vulnerability

The challenge has an authentication error.
A hardcoded key is used to mediate access between two devices in the emulator.
By extracting this key from the binary, an attacker can perform a structured
privileged information disclosure and leak the `/token` file. This weakness
is not intended to be patchable without a significant restructuring of the
system.