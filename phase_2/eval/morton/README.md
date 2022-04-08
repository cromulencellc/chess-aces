# morton
Morton uses Eclipse MQTT to allow communication between a client and server following a pub-sub model. Vulnerabilities have been injected to the client side only.

This challenge is different than previous ones in that the vulnerabilities are inserted into already existing code as opposed to writing the entire application from scratch. Remember that diffing is not a permissible method of bug finding for CHESS, so don't download the original and compare the two.

Both the client applications and their linked libraries are in scope.

# Client Usage
The client is an IoT Thermometer that accepts the following 3 different commands on the 'app/request' topic:
- temperature - returns the current temperature reading of the IoT device
- humidity - returns the current humidity reading of the IoT device
- UV - returns the current UV index reading of the IoT device

The topic must be 'app/request' and the payload must contain the following: "[command] [return topic]"
The command is one of the 3 commands above and the return topic can be any valid MQTT topic that the client will return the current reading on.

Here is an example of a valid publish message using the mosquitto_pub program:
```
./mosquitto_pub -t "app/request" -m "temperature return/topic"
```

This will return the temperature reading to 'return/topic' (make sure you are subscribed to that topic)

Any invalid commands or invalid return topics will generate an error that is posted to 'app/error' so ensure you are subscribed to this topic to determine errors in the commands you are sending.

# Using Docker
Currently, all the docker containers run on their own and the individual containers can be run with the following command:
```
docker run --rm -it -p 1883:1883 --name name container_name
```

# Poller
The poller runs through a list of possible commands and topics in order to show the different variations and errors that the client will return. The poller takes in environment variables for the following and the defaults are shown below:
```
HOST = 127.0.0.1  
PORT = 1883  
LENGTH = 20  
TIMEOUT = 60  
SEED = random number  
```

The poller is under morton/poller/poller_src and can be built by running 'make clean' and then 'make' inside that folder.

The poller can be run by simply going to morton/poller/poller_src and running ./poller

# Building the Clients
All the modified source has been provided under challenge/mosquitto_src and you just need to run 'make clean' to reset the build and then 'make' to build the client.

* Note the challenge/client and challenge/mosquitto_src are essentially the same thing but challenge/client only contains the vulnerable client where as challenge/mosquitto_src contains all the source and other programs as well. If you want to make changes to the mosquitto_sub vulnerable client then you will need to make changes to challenge/mosquitto_src/client/mosquitto_sub and then run 'make clean' and then 'make' inside of /challenge/mosquitto_src and then copy over the new mosquitto_sub to the challenge/client folder.

The original source code can be found at https://github.com/eclipse/mosquitto

Build Dependencies:  
c-ares (libc-ares-dev on Debian based systems) - only when compiled with make WITH_SRV=yes  
libwebsockets (libwebsockets-dev) - enable with make WITH_WEBSOCKETS=yes  
openssl (libssl-dev on Debian based systems) - disable with make WITH_TLS=no  
xsltproc (xsltproc and docbook-xsl on Debian based systems) - only needed when building from git sources - disable with make WITH_DOCS=no  
uthash / utlist - bundled versions of these headers are provided, disable their use with make WITH_BUNDLED_DEPS=no  
Equivalent options for enabling/disabling features are available when using the CMake build.  

# MQTT Client
use mosquitto_sub to subscribe to a topic:
```
mosquitto_sub -t 'test/topic' -v
```

And to publish a message:
```
mosquitto_pub -t 'test/topic' -m 'hello world'
```

Documentation:
Documentation for the broker, clients and client library API can be found in the man pages, which are available online at https://mosquitto.org/man/. There are also pages with an introduction to the features of MQTT, the mosquitto_passwd utility for dealing with username/passwords, and a description of the configuration file options available for the broker.

Detailed client library API documentation can be found at https://mosquitto.org/api/

# Apogee
## Docker
The docker-compose.yaml file works but the pov needs to be run after the poller as the poller commands might affect the pov since it sends a command and waits for a response to trigger the exploit.

## PoV
CWE-122: Heap-based Buffer Overflow

The pov can be built by going to morton/pov/pov_src and then running 'make clean' and then 'make'  
* Note that the pov is separate from the rest of the source so if you make changes to the morton/challenge/mosquitto_src folder then remember to copy those to the pov folder.

Inside of the mosq structure in mosquitto-internal.h there is a char array called print_string that is a buffer of size 600 that takes in error messages and then prints them out. This buffer is located above a char* userdata variable and a list of function pointers. An overflow of the print_string buffer would alow an attacker to overwrite a function pointer and the userdata variable (which ends up being a register value when these functions are called). There is only one string in the entire system that will write more than 600 bytes to print_string. It is shown below when one of the topic doesn't contain valid UTF-8 characters.

actions.c
```c
if(mosquitto_validate_utf8(topic, (int)tlen)) {
			log__printf(mosq, MOSQ_LOG_ERR, "Can't publish the following topic as it was not formatted correctly in UTF-8 and should be fixed before trying again: %s", topic);
			return MOSQ_ERR_MALFORMED_UTF8;
}
```

logging_mosq.c
```c
int log__printf(struct mosquitto *mosq, int priority, const char *fmt, ...)
{
	va_list va;
	size_t len;

	assert(mosq);
	assert(fmt);

	pthread_mutex_lock(&mosq->log_callback_mutex);
	if(mosq->on_log){
		len = strlen(fmt) + 500;

		va_start(va, fmt);
		vsnprintf(mosq->print_string, len, fmt, va);
		va_end(va);

		mosq->on_log(mosq, mosq->userdata, priority, mosq->print_string);
	}
	pthread_mutex_unlock(&mosq->log_callback_mutex);

	return MOSQ_ERR_SUCCESS;
}

```

The buffer overflow happens in the above function when the UTF-8 error message is printed and when the topic value is longer than 500 characters. The attacker can override the connect function pointer and then all they have to do is cause the vulnerable client to disconnect and reconnect to trigger the exploit. They can do this by registering the same client name which causes the broker to disconnect the old client. The vulnerable client always has the name Thermometer-1. A section of the mosq structure containing the vulnerable buffer is shown below:
```c
struct mosquitto {
	...
	char log_string[MQTT_MAX_STR];
	void *userdata;
	void (*on_connect)(struct mosquitto *, void *userdata, int rc);
	...
}
```
The buffer overflow is limited in the fact it can only overflow into these two variables. The userdata variable is never used and the connect function pointer is never used unless the client becomes disconnected.
