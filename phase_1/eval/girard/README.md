# girard #

Girard is a server backend for manipulating Haiku Vector Image Format (HVIF)
files. HVIF is a real image format whose only implementation is within the
Haiku operating system. Using resources like Leah Hanson's awesome
[blog post](http://blog.leahhanson.us/post/recursecenter2016/haiku_icons.html)
outlining how the format works, we now have a basic, network-ready version of
Haiku's Icon-O-Matic tool.


## Building ##

To build Girard, you will need the following packages:

```sh
sudo apt install build-essential clang pmccabe
```

Building Girard should be as simple as running:

```sh
cd challenge
make all
```

This should build Girard and place it at `challenge/build/girard`.

Girard does support a `DEBUG` mode (which can be built with `make debug`), but
it was only used for very early testing and is no longer up-to-date or used.


## Running ##

Girard can be run in two different configurations. The first is as a service
using standard input/output for comms:

```sh
challenge/build/girard
```

The second is as a network server that binds and opens a port and accepts a
single connection:

```sh
PORT=12345 challenge/build/girard
```


## Polling and Testing ##

Girard's poller doubles as a unit test suite that should cover *almost* all of
the logic within the service. It can be run like so:

```sh
poller/girard-poller.rb localhost 12345
```

Rather than specify the host/port of the server as arguments, you may instead
provide them (along with other options) as environment variables. Girard's
poller supports the following environment variables:

* `HOST` - The remote host to connect to
* `PORT` - The remote port to connect on
* `SEED` - The seed for the poller's random number generator
* `LENGTH` - The number of messages to send in a single poll
* `TIMEOUT` - The maximum length of time a poll is allowed to last

Specifying host and port (via arguments or environment) is not optional. All
other options are optional.

The poller will pull test cases from the `poller/tests` directory, which
contains HVIF images that ship with the Haiku operating system. The poller
will always select a single image from this directory to load as its first
action after successfully negotiating a connection with the server. After
that, all tests are chosen randomly according to the `SEED` that was
specified or generated for that poll.


## Proof of Vulnerability ##

Due to time constraints while developing the challenge, the vulnerability is a
single, _very_ vulnerable "function" call inside of
`Girard::PerspectiveTransformer::toBytes` (see `transform.cxx`) that will
crash with user-controlled data inside of both `rip` and `rdi`. The
vulnerability should be obvious when it is encountered by either a human or a
computer.

A proof for the vulnerability can be set up in one of two ways:

1. Request a `load` of a valid `Image` file that contains a
   `PerspectiveTransformer`.
2. Request a new image be `create`d and add a valid `Shape` (which requires
   having a valid `Style` and `Path`) that contains a `PerspectiveTransformer`.

Triggering the vulnerability can be done by:

1. Requesting a `store` of the `Image` containing the `PerspectiveTransformer`.
2. Requesting `get_transformer` for the `PerspectiveTransformer`.

A proof of vulnerability using method 2 of both options above has been
provided in `proof/girard-pov.rb`. It is run similar to the poller.

Because `PerspectiveTransformer`s don't actually work in the original HVIF
implementation in the Haiku operating system, none of the test cases provided
with the poller will trigger this bug on their own. The poller will also not
generate polls that will create a `PerspectiveTransformer`.

### Expected Patch ###

A successful patch should simply remove the bad "function" call and leave the
rest of the service intact with no changes. It would also be acceptable to
remove the ability to process a `PerspectiveTransformer`, since they are not
actually supported in the original implementation. As a result, the poller
only tests for functionality that would be expected by a hypothetical
end user.
