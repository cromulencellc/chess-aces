# nevins

## Info

nevins is a proxy server that accepts incoming http requests and forwards them on the the necessary server.

nevins allows the creation of filters on both the upstream and the downstream

## Poller

The poller requires the following files in the test_files directory:

chess.jpg
faq.html
rr.html
test.doc
test.txt
test_del.txt
test_del2.txt
test_tr.txt

Prior to running the poller launch the python Simple HTTP server in the test_files directory:

python -m SimpleHTTPServer

This will launch a basic web server on port 8000 that we can use to serve up the files

## PoV

The pov expects access to the test_files directory. Currently it attempts to write to ../challenge/test_files/
