<!--
.. title: Using Let's Encrypt certificates with mosquitto
.. slug: using-lets-encrypt-certificates-with-mosquitto
.. date: 2015-12-13 19:53:37
.. tags: certificates,encryption,tls
.. category:
.. link:
.. description:
.. type: text
-->

If you want to use TLS certificates you've generated using the [Let's Encrypt]
service, this is how you should configure your listener (replace "example.com"
with your own domain of course):

Then use the following for your mosquitto.conf:

```
listener 8883
cafile /etc/ssl/certs/DST_Root_CA_X3.pem
certfile /etc/letsencrypt/live/example.com/fullchain.pem
keyfile /etc/letsencrypt/live/example.com/privkey.pem
```

You need to be aware that current versions of mosquitto never update listener
settings when running, so when you regenerate the server certificates you will
need to completely restart the broker.

[Let's Encrypt]: https://letsencrypt.org/
