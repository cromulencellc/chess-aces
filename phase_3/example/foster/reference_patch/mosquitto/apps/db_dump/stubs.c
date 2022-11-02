#include <stdlib.h>
#include <string.h>

#include "misc_mosq.h"
#include "mosquitto_broker_internal.h"
#include "mosquitto_internal.h"
#include "util_mosq.h"

#ifndef UNUSED
#  define UNUSED(A) (void)(A)
#endif

struct mosquitto *context__init(mosq_sock_t sock)
{
	UNUSED(sock);

	return NULL;
}

void context__add_to_by_id(struct mosquitto *context)
{
	UNUSED(context);
}

int db__message_store(const struct mosquitto *source, struct mosquitto_msg_store *stored, uint32_t message_expiry_interval, dbid_t store_id, enum mosquitto_msg_origin origin)
{
	UNUSED(source);
	UNUSED(stored);
	UNUSED(message_expiry_interval);
	UNUSED(store_id);
	UNUSED(origin);
    return 0;
}

void db__msg_store_ref_inc(struct mosquitto_msg_store *store)
{
	UNUSED(store);
}

int handle__packet(struct mosquitto *context)
{
	UNUSED(context);
	return 0;
}

int log__printf(struct mosquitto *mosq, unsigned int level, const char *fmt, ...)
{
	UNUSED(mosq);
	UNUSED(level);
	UNUSED(fmt);
	return 0;
}

FILE *mosquitto__fopen(const char *path, const char *mode, bool restrict_read)
{
	UNUSED(path);
	UNUSED(mode);
	UNUSED(restrict_read);
	return NULL;
}

enum mosquitto_client_state mosquitto__get_state(struct mosquitto *mosq)
{
	UNUSED(mosq);
	return mosq_cs_new;
}

int mux__add_out(struct mosquitto *mosq)
{
	UNUSED(mosq);
	return 0;
}

int mux__remove_out(struct mosquitto *mosq)
{
	UNUSED(mosq);
	return 0;
}

ssize_t net__read(struct mosquitto *mosq, void *buf, size_t count)
{
	UNUSED(mosq);
	UNUSED(buf);
	UNUSED(count);
	return 0;
}

ssize_t net__write(struct mosquitto *mosq, const void *buf, size_t count)
{
	UNUSED(mosq);
	UNUSED(buf);
	UNUSED(count);
	return 0;
}

int retain__store(const char *topic, struct mosquitto_msg_store *stored, char **split_topics)
{
	UNUSED(topic);
	UNUSED(stored);
	UNUSED(split_topics);
	return 0;
}

int sub__add(struct mosquitto *context, const char *sub, uint8_t qos, uint32_t identifier, int options, struct mosquitto__subhier **root)
{
	UNUSED(context);
	UNUSED(sub);
	UNUSED(qos);
	UNUSED(identifier);
	UNUSED(options);
	UNUSED(root);
	return 0;
}

int sub__messages_queue(const char *source_id, const char *topic, uint8_t qos, int retain, struct mosquitto_msg_store **stored)
{
	UNUSED(source_id);
	UNUSED(topic);
	UNUSED(qos);
	UNUSED(retain);
	UNUSED(stored);
	return 0;
}

int keepalive__update(struct mosquitto *context)
{
	UNUSED(context);
	return 0;
}

void db__msg_add_to_inflight_stats(struct mosquitto_msg_data *msg_data, struct mosquitto_client_msg *msg)
{
	UNUSED(msg_data);
	UNUSED(msg);
}

void db__msg_add_to_queued_stats(struct mosquitto_msg_data *msg_data, struct mosquitto_client_msg *msg)
{
	UNUSED(msg_data);
	UNUSED(msg);
}
