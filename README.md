# CHESS Challenges

This is the collection of challenges developed by the
Assorted Challenges for Evaluation and Separation (ACES) effort of the
Computers and Humans Exploring Software Security (CHESS) program.

This research was developed with funding from the
Defense Advanced Research Projects Agency (DARPA).
Distribution Statement "A" (Approved for Public Release, Distribution Unlimited).

For more information about these challenges, please read our Phase 1 report
at XXXX

## Using These Challenges

**The software in this collection has known and intended vulnerabilities,
known and unintended vulnerabilities, and may have unknown and unintended
vulnerabiliies. They are suitable for use in research and educational contexts.
They are NOT suitable for production use.**

Challenges are separated by phase (as of this writing, only Phase 1 challenges
are public) and usage (example challenges are used during development of the
CHESS system, evaluation challenges are used to evaluate CHESS system progress).

Inside each challenge are multiple directories:

* `base_data` - challenges expect files from this directory to be loaded to
  `/data` in the challenge environment.
* `challenge` - this is the development directory for the challenge. It may or
  may not be present in this release.
* `poller` - this directory includes an integration test used to determine
  correct functioning of the challenge; it is expected to succeed for both
  unpatched and fully-patched versions of the challenge
* `pov` - this directory (or each of multiple `pov*` directories) contains a
  "Proof of Vulnerability" (PoV),
  an integration test used to demonstrate a specific vulerability in a
  challenge. It is expected to succeed (prove a vulnerability) on an unpatched
  challenge, and fail on a fully-patched challenge.
* `priv` - this directory may contain tools for building the challenge.
* `variants` - this directory includes an `unpatched` version of the challenge
  that all PoVs succeed on,
  a `fully_patched` version that no PoVs succeed on, and may include more
  variants if there are distinct PoVs.
  
Challenges each have a `README.md` that documents specifics about that
challenge. 

## License

Files in this repository are provided as-is under the MIT license unless
otherwise stated. See LICENSE.md for more details.

## Contact

Questions, comments, or concerns can be sent to `chess[at]cromulence.com` .