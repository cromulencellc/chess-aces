# Reference Patch
This patch can be applied from inside this folder but the vulnerable file is under morton/challenge/mosquitto_src/lib/logging_mosq.c. You can then rebuild the project by running 'make clean && make' under morton/challenge/mosquitto_src.

The patch can be applied by running the following:
```
patch ../challenge/mosquitto_src/lib/logging_mosq.c vulnerability.patch
```

The patch edits the following file which will eliminate the vulnerability in the program:
logging_mosq.c

Vulnerable Version:
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
		vsnprintf(mosq->log_string, len, fmt, va);
		va_end(va);

		mosq->on_log(mosq, mosq->userdata, priority, mosq->log_string);
	}
	pthread_mutex_unlock(&mosq->log_callback_mutex);

	return MOSQ_ERR_SUCCESS;
}
```

Patched Version:
```c
int log__printf(struct mosquitto *mosq, unsigned int priority, const char *fmt, ...)
{
	va_list va;
	char *s;
	size_t len;

	assert(mosq);
	assert(fmt);

	pthread_mutex_lock(&mosq->log_callback_mutex);
	if(mosq->on_log){
		len = strlen(fmt) + 500;
		s = mosquitto__malloc(len*sizeof(char));
		if(!s){
			pthread_mutex_unlock(&mosq->log_callback_mutex);
			return MOSQ_ERR_NOMEM;
		}

		va_start(va, fmt);
		vsnprintf(s, len, fmt, va);
		va_end(va);
		s[len-1] = '\0'; /* Ensure string is null terminated. */

		mosq->on_log(mosq, mosq->userdata, (int)priority, s);

		mosquitto__free(s);
	}
	pthread_mutex_unlock(&mosq->log_callback_mutex);

	return MOSQ_ERR_SUCCESS;
}
```

Note: The provided source is much different from the original MQTT source as some functionality was added to make it act like an IoT device.
Using the original MQTT source code will make the poller no longer work.
