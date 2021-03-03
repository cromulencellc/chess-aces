# channing

Channing is an HTTP/1.1 web server that supports GET, HEAD, and POST operations. PUT and DELETE
are recognized as legitimate commands but are reported as Not Implemented.  Channing also
supports directories in the webroot being protected with Basic authentication. Channing does 
not support CGI so any POST operation is handled by the server based upon the Content-Type
field in the HTTP headers in the request. Channing also support chunked Transfer-Encoding,
in both requests and responses.

## setting up

Channing uses the `/data` directory to hold its webroot (/data/www/html) and the server 
configuration file (/data/server.ini). These contents are not mofified by any operation
allowed by the server, so restoring them after each poll should not be required.

# poller

The poller accesses Channing as a typical client and performs all standard operations in both
successful and failing modes.  A GET of an existing file, as well as an attempt of a file 
that results in a 404 response, a POST of x-www-form-urlencoded data and application/octet-stream,
random, accessing a password protected file with a good password, and attempting to use an
incorrect password. Chunked transfers for requests and responses are also performed.

# pov

Channing attempts to protect against escaping the webroot and checks requests for attempts 
to use '..' in the reuest URI. However, the server also supports urlencoding, and the check
for '..' is performed before the URI has been decoded.  An attempt to access a higher directory
using /%2E%2E/ will succeed.  By iteratively accessing higher and higher in the directory 
structure, the /token file can eventually be retrieved.