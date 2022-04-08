# hackensack

Hackensack is an injection bug into the server portion of the open source mosquitto project from here: https://mosquitto.org/

The version being used is 2.0.4

Mosquitto is an implementation of the MQTT protocol. 

From MQTT.com:

"MQTT is an OASIS standard messaging protocol for the Internet of Things (IoT). It is designed as an extremely lightweight publish/subscribe messaging transport that is ideal for connecting remote devices with a small code footprint and minimal network bandwidth. MQTT today is used in a wide variety of industries, such as automotive, manufacturing, telecommunications, oil and gas, etc."

It uses a pub/sub model where clients subscribe to topics and other clients can publish messages to a given topic which is sent out to all subscribed clients.

With the server running a client can subscribe to a topic with the following command:

## Scope for Analysis

This challenge uses the MQTT but does not maintain a persistent set of data for storage. The only privileged data is stored in ``/token``.

The MQTT server is in scope and both clients ``mosquitto_sub`` and ``mosquitto_pub`` are not.

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
CWE-305 Authentication Bypass by Primary Weakness
CWE-697 Incorrect Comparison
Effects: Authentication, Information Disclosure, Logic Error

#ifndef PATCH_1
				if(!strncmp(u->password, ed->client->password, strlen(ed->client->password))) {
#else
				if(!strcmp(u->password, ed->client->password ) {
#endif
					return MOSQ_ERR_SUCCESS;
				}
#endif

The string comparison takes the size of the password sent by the user as the length to be compared. So, if an attacker only sends a single byte they only need to try each ascii character once to find the correct one which is trivial. Using this method an attacker can actually leak the admin password one byte at a time.

Once logged in as admin they can subscribe to the '$SYS/broker/store/token' topic which periodically publishes the token value.


## POV 2

CWE-121 Stack-based buffer overflow.

Effects: Memory Corruption

Users can unsubscribe from multiple topics at a time and initially the reason codes were stored in a heap buffer that would be reallocated on an as needed basis but this buffer was replaced with a static, stack-based buffer. If a user unsubscribes from greater than 10 topics simultaneously it is possible to overflow this buffer and overwrite the return address.

## POV 3

CWE-415 Double free
CWE-416 Use after free

Effects: Data Misuse, Information Disclosure

When a user subscribes to a topic they can specify the quality of service (qos) that they want associated with that subscription. If a user specifies greater than 2 it typically results in an error code and disconnection. However, by removing the return after the error it results in multiple frees of the 'sub' buffer which contains the topic name.

Multiple requests can result in leaking library addresses to a subscribed user.

This POV can take 2 minutes if it is going to fail.