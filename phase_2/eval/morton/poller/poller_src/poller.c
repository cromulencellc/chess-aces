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
#include <signal.h>

#define PUB_QOS 1
#define SUB_QOS 1

static atomic_int message_count = 0;
static atomic_int err_msg_count = 0;
static atomic_int normal_msg_count = 0;
char* HOST = "127.0.0.1";
int PORT = 1883;
int LENGTH = 20;
int TIMEOUT = 60;
unsigned long long SEED = 0;
int return_val = 0;

void my_signal_handler(int signum)
{
	if(signum == SIGALRM || signum == SIGTERM || signum == SIGINT){
		fprintf(stderr, "Timed out or Terminated by user\n");
		exit(-1);
	}
}

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc;
	/* Print out the connection result. mosquitto_connack_string() produces an
	 * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
	 * clients is mosquitto_reason_string().
	 */
	fprintf(stderr, "on_connect: %s\n", mosquitto_connack_string(reason_code));

	if(reason_code != 0){
		/* If the connection fails for any reason, we don't want to keep on
		 * retrying in this example, so disconnect. Without this, the client
		 * will attempt to reconnect. */
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

	if(!strcmp(msg->topic, "app/error")){
		fprintf(stderr,"received message on '%s'\n", msg->topic);
    	fprintf(stderr,"message payload: %s\n\n", msg->payload);
		err_msg_count++;
	}else if(strcmp(msg->topic, "app/request")){
		fprintf(stderr,"received message on '%s'\n", msg->topic);
    	fprintf(stderr,"message payload: %s\n\n", msg->payload);
		normal_msg_count++;
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
	char* request_topic = "app/request";
	char* string_topic = NULL;

	
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
		TIMEOUT = 60;
	}
	if(getenv("LENGTH")){
		LENGTH = atoi(getenv("LENGTH"));
	}else {
		LENGTH = 20;
	}
	if(getenv("SEED")){
		SEED = atoi(getenv("SEED"));
	}else {
		// Generate random 64-bit SEED number
		SEED = (((uint64_t) rand() <<  0) & 0x00000000FFFFFFFFull) | 
  			   (((uint64_t) rand() << 32) & 0xFFFFFFFF00000000ull);
	}

	printf("HOST: %s\n", HOST);
	printf("PORT: %i\n", PORT);
	printf("TIMEOUT: %i\n", TIMEOUT);
	printf("LENGTH: %i\n", LENGTH);
	printf("SEED: %llu\n", SEED);

	srand(SEED);

	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
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

	sleep(LENGTH/10);

    /* Publish to app/request with the temperature command and set app/respond as the topic to send the response on */
    string_topic = "temperature app/respond";
    fprintf(stderr,"Publishing '%s' to '%s'\n\n", string_topic, request_topic);

	rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

	sleep(LENGTH/10);

    /* Publish to app/request with the humidity command and set app/respond as the topic to send the response on*/
    string_topic = "humidity app/respond";
    fprintf(stderr,"Publishing '%s' to '%s'\n\n", string_topic, request_topic);

    rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

	sleep(LENGTH/10);

    /* Publish to app/request with the UV command and set app/respond as the topic to send the response on*/
    string_topic = "UV app/random respond topic";
    fprintf(stderr,"Publishing '%s' to '%s'\n\n", string_topic, request_topic);

    rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

	sleep(LENGTH/10);

    /* Publish to app/request with an invalid command and it should respond using the app/error topic*/
    string_topic = "error app/respond";
    fprintf(stderr,"Publishing '%s' to '%s'\n\n", string_topic, request_topic);

    rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

	sleep(LENGTH/10);

	/* Publish to app/request with a valid command but invalid MQTT topic*/
    string_topic = "UV app/\x01";
    fprintf(stderr,"Publishing '%s' to '%s'\n\n", string_topic, request_topic);

    rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

	sleep(LENGTH/10);

    /* Publish to app/request with an invalid response topic and it should respond using the app/error topic*/
    string_topic = "error app/#";
    fprintf(stderr,"Publishing '%s' to '%s'\n\n", string_topic, request_topic);

    rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

	sleep(LENGTH/10);

    /* Publish to app/request with an invalid response topic and it should respond using the app/error topic*/
    string_topic = "invalid";
    fprintf(stderr,"Publishing '%s' to '%s'\n\n", string_topic, request_topic);

    rc = mosquitto_publish(mosq, NULL, request_topic, strlen(string_topic), string_topic, PUB_QOS, 0);
	if(rc){
		fprintf(stderr,"Error Publishing rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}


    sleep(LENGTH/10);

    /* Ensure Client id is unique or the broker will disconnect a client if ids are the same */
    char* client_id = "new_client_id";
    fprintf(stderr,"Changed the client id to '%s'\n\n", client_id);
    mosquitto_reinitialise(mosq, client_id, true, NULL);
	rc = mosquitto_connect(mosq, HOST, PORT, 60);
    if(rc){
		fprintf(stderr,"Error Connecting rc = %i\n", rc);
		return_val = 1;
		goto cleanup;
	}

    sleep(LENGTH/5);

	if(err_msg_count == 4 && normal_msg_count == 3){
		printf("SUCCESS\n");
	}else{
		printf("FAILURE\n");
		return_val = 1;
	}


cleanup:
	mosquitto_disconnect(mosq);
	mosquitto_loop_stop(mosq, false);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return return_val;
}
