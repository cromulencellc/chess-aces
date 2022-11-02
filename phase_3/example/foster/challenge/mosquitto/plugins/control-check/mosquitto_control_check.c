/*
Copyright (c) 2021 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License 2.0
and Eclipse Distribution License v1.0 which accompany this distribution.

The Eclipse Public License is available at
   https://www.eclipse.org/legal/epl-2.0/
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.

SPDX-License-Identifier: EPL-2.0 OR EDL-1.0

Contributors:
   Roger Light - initial implementation and documentation.
*/

/*
 * This is an example plugin showing how to use the basic authentication
 * callback to allow/disallow client connections based on client IP addresses.
 *
 * This is an extremely basic type of access control, password based or similar
 * authentication is preferred.
 *
 * Compile with:
 *   gcc -I<path to mosquitto-repo/include> -fPIC -shared mosquitto_auth_by_ip.c -o mosquitto_auth_by_ip.so
 *
 * Use in config with:
 *
 *   plugin /path/to/mosquitto_auth_by_ip.so
 *
 * Note that this only works on Mosquitto 2.0 or later.
 */
#include "config.h"

#include <stdio.h>
#include <string.h>

#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"

char data[32] = {0};
static mosquitto_plugin_id_t *mosq_pid = NULL;

static int basic_control_callback(int event, void *event_data, void *userdata)
{
	struct mosquitto_evt_control *ed = event_data;
	
	UNUSED(event);
	UNUSED(userdata);

	if ( !strcmp(ed->topic, "$CONTROL/admin") && ed->qos == 2 ) {
		if (data[0] == '\0') {
			db__messages_easy_queue(NULL, "$CONTROL/admin", 2, 16, "start of control", 1, 60, NULL);
			memcpy(data, ed->payload, sizeof(data));
		} else {
			memcpy(data, ed->payload, sizeof(data));
			db__messages_easy_queue(NULL, "$CONTROL/admin", 2, strlen(data), data, 1, 60, NULL);
		}
		
		return MOSQ_ERR_SUCCESS;
	}

	return MOSQ_ERR_PLUGIN_DEFER;
}

static int basic_message_callback(int event, void *event_data, void *userdata)
{
	struct mosquitto_evt_message *ed = event_data;

	UNUSED(event);
	UNUSED(userdata);

	return MOSQ_ERR_SUCCESS;
}

int mosquitto_plugin_version(int supported_version_count, const int *supported_versions)
{
	int i;

	for(i=0; i<supported_version_count; i++){
		if(supported_versions[i] == 5){
			return 5;
		}
	}
	return -1;
}

int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);

	mosq_pid = identifier;

#ifdef PATCHED
	return mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, basic_message_callback, NULL, NULL);
#else
	mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, basic_message_callback, NULL, NULL);

	return mosquitto_callback_register(mosq_pid, MOSQ_EVT_CONTROL, basic_control_callback, "$CONTROL/admin", NULL);
#endif
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);

	mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, basic_message_callback, NULL);

	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_CONTROL, basic_control_callback, NULL);
}
