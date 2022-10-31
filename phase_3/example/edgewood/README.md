# Edgewood

This README includes details of example challenge internals and vulnerabilities.
If this challenge is used in an evaluation event, information about
the vulnerability and proof-of-vulnerability (PoV) should be removed.

## Description

This service implements a node application that creates a server for collecting, storing, and servicing requests for weather data.  It specifically supports the broadcast messages of the Tempest home weather station that sends out various message types on udp/50222.  This service collects those messages and processes them into several Mongodb Collections.  The service also implements a http front end to provide weather records in json format.  These weather data is provided unauthenticated to any requestor.  In addition, there is an admin interface that shows specific information about the weather station hardware that does require authentication.

### Scope for Analysis

The code outside of the `node_modules` directory is in scope.
The code in the `node_modules` directory does not have any intended weaknesses.

## Running the Challenge

The challenge was built with TypeScript. 
It can compiled to different flavors of JavaScript/ECMAScript.

```
# Compiling the Typescript, then running it
guest> npx tsc
guest> node ./build/main.js
```

The challenge has several environment variables for configuration:

* `CHESS`: this environment variable is checked on startup;
  it doesn't need to be a particular value, just there.
* `WEATHER_PORT`: configures which UDP port to listen for weather data on.
  Should match the values for the poller and PoV.
* `MONGO_URL`: sets connection parameters for MongoDb. 
  The default should work with the `docker-compose.yml` file as-is.
* `HTTP_PORT`: configures which TCP port to listen for HTTP traffic on.
  Should match the values for the poller and PoV.
* `ADMIN_DATA_PATH`: allows changing where admin data are read from
  on initial startup. See below.

### Admin Data

The poller expects to be able to log in to admin screens.
An `admin.json` file is in `base_data` and should be copied to `/data`
in the challenge and poller containers.
The challenge will read this file, set the admin in MongoDB, 
and delete it (it only needs to be used once).
The poller will read this file on every invocation, and does not delete it.

Proofs-of-vulnerability (including the reference PoV) must not depend
on a specific admin username and password.
The reference PoV creates its own random admin username and password.

### Poller Token Validation

The poller, in addition to the normal and above environment variables,
supports an `EXPECTED_TOKEN` environment variable for validating
admin logins. 
If it's left unset, a successful admin login will be checked,
and the found token will be printed out, but not asserted for a
specific value. 
If `EXPECTED_TOKEN` is set, the value returned from an admin login 
will be checked.

## Vulnerability

Data/code injection leading to structured information disclosure.

The Tempest weather message parsing includes a feature to save weather messages of an unexpected type by dynamically creating a document based on the schema of the incoming weather json message.  The vulnerability is due to Mongodb not enforcing schemas within a single Collection.  By crafting a weather message of type `admindatas` the attacker can insert a record into the admin user Collection to create a new admin user with a known password, provided the message also contains the fields needed to pass validation as a weather-type message.

A workable patch for this vulnerability is to disable the unexpected type saving. Adding a prefix to unexpected type names, or making a dedicated collection for unexpected types is also acceptable.