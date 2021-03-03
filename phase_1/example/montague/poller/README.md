# montague poller

This poller must have the admin password and may validate the token configured
with the challenge. These are configured with environment variables:

* `ADMIN_PASSWORD` - this defaults to `asdfasdf`, and must match the challenge's
  `/data/password` file.
* `TOKEN` - this may match the `/token` file the challenge has access to. If it
  is set, it will be validated after logging in through the admin interface. If
  left unset, the present token will be echoed to the poller's stderr, but it
  will not be validated.
