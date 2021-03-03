# livingston

This document discusses the architecture and vulnerabilities of this CHESS
challenge. It should not be shared with TA1, TA2, or TA5 performers prior
to Phase 1 evaluations.

livingston is a music synthesizer.

## running the poller

The poller is implemented as a C++ program (it shares much of the codebase with
the challenge).

It should build cleanly with the `Makefile` in the `poller` folder, and connects
to the challenge using the normal `HOST` and `PORT` environment variables.

It's extremely fast, so a high `LENGTH` value is acceptable.

## vulnerability

The `StateMachine::setLogin` method, given a larger-than-expected loginType, can
clobber another client's login metadata. Using this memory corruption weakness
allows the PoV to obtain the contents of `/token`.

The PoV works the same way as the poller: it should build cleanly with the
`Makefile` and run with the appropriate environment variables.
