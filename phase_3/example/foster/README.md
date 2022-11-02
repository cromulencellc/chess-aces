# foster

Foster is an injection bug into the server portion of the open source mosquitto project from here: https://mosquitto.org/

The version being used is 2.0.14

Mosquitto is an implementation of the MQTT protocol. 

From MQTT.com:

"MQTT is an OASIS standard messaging protocol for the Internet of Things (IoT). It is designed as an extremely lightweight publish/subscribe messaging transport that is ideal for connecting remote devices with a small code footprint and minimal network bandwidth. MQTT today is used in a wide variety of industries, such as automotive, manufacturing, telecommunications, oil and gas, etc."

It uses a pub/sub model where clients subscribe to topics and other clients can publish messages to a given topic which is sent out to all subscribed clients.

With the server running a client can subscribe to a topic with the following command:

## Overview
Externality: Mosquitto 2.0.14

Protocol: MQTT

Language: C

## Scope for Analysis

This challenge uses the MQTT but does not maintain a persistent set of data for storage. The only privileged data is stored in ``/token``.

The MQTT server and all plugins are in scope while both clients ``mosquitto_sub`` and ``mosquitto_pub`` are not.

## Subscribing

./mosquitto_sub -t chess/foo -t chess/bar -u chess -P chess -h <host>

Where each -t specifies topics to subscribe to, -u is the user and -P is the password

### If you get the following error:

./mosquitto_sub: /usr/lib/x86_64-linux-gnu/libmosquitto.so.1: version 'MOSQ_1.5' not found (required by ./mosquitto_sub)

You need to add the library to your path with:

export LD_LIBRARY_PATH=<path_to_mosquitto>/lib/

## Publishing

./mosquitto_pub -t chess/foo -m "hello world foo" -u chess -P chess

This sends the text "hello world foo" to the chess/foo topic

./mosquitto_pub -t chess/bar -m "hello world bar" -u chess -P chess

## POV 1
CWE-170: Improper Null Termination
Effects: Information Disclosure

The vulnerability for this challenge is relatively straight-forward but to trigger it there are number of requirements that must be met and it requires multiple connections.

If a user subscribes to the $CONTROL/admin topic and another user publishes a 32 byte message twice to the same topic with a QOS of 2 then the the server will leak an address to the subscriber.

This is a bit of a contrived vulnerability though since this is an example challenge I don't think it is a big deal. I designed it to make the CHESS teams analyze shared libraries and require them to maintain a state across multiple connections. Often when people fuzz they will launch, connect once, then restart a new test. 

