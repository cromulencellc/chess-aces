# tillary

This document discusses the architecture and vulnerabilities of this CHESS
challenge. It should not be shared with TA1, TA2, or TA5 performers prior
to {hase 1 evaluations.

Tillary is a web server built using [webmachine][1] techniques. To map REST
concepts on to HTTP concepts more effectively, the `Connection` class asks a
`Resource` class a bunch of questions to build a response, instead of the
Rack-style app more commonly seen (in which a responder builds its own
response).

1: https://github.com/webmachine/webmachine/wiki

The resources available are for serving static files
(`http::resources::Static`) and posting short
snippets of text (`http::resources::Tweets`).

## weakness and pov

The `Tweets` resource does not escape tweet IDs correctly, allowing a path
traversal to expose the contents of `/token`.

## patching it

A possible patch is to use the `std::filesystem::path` APIs to correctly
check if the disk path to the wanted tweet is in the appropriate directory.
