#include "first.h"

#include "plugin.h"
#include "base.h"
#include "array.h"
#include "log.h"

#include <string.h>
#include <stdlib.h>

array plugin_stats; /* global */

#ifdef HAVE_VALGRIND_VALGRIND_H
# include <valgrind/valgrind.h>
#endif

#if !defined(__WIN32) && !defined(LIGHTTPD_STATIC)
# include <dlfcn.h>
#endif
/*
 *
 * if you change this enum to add a new callback, be sure
 * - that PLUGIN_FUNC_SIZEOF is the last entry
 * - that you add:
 *   1. PLUGIN_CALL_... as callback-dispatcher
 *   2. count and assignment in plugins_call_init()
 *
 */

typedef enum {
	PLUGIN_FUNC_HANDLE_URI_CLEAN,
	PLUGIN_FUNC_HANDLE_URI_RAW,
	PLUGIN_FUNC_HANDLE_REQUEST_ENV,
	PLUGIN_FUNC_HANDLE_REQUEST_DONE,
	PLUGIN_FUNC_HANDLE_CONNECTION_ACCEPT,
	PLUGIN_FUNC_HANDLE_CONNECTION_SHUT_WR,
	PLUGIN_FUNC_HANDLE_CONNECTION_CLOSE,
	PLUGIN_FUNC_HANDLE_TRIGGER,
	PLUGIN_FUNC_HANDLE_SIGHUP,
	PLUGIN_FUNC_HANDLE_WAITPID,
	/* PLUGIN_FUNC_HANDLE_SUBREQUEST, *//* max one handler_module per req */
	PLUGIN_FUNC_HANDLE_SUBREQUEST_START,
	PLUGIN_FUNC_HANDLE_RESPONSE_START,
	PLUGIN_FUNC_HANDLE_DOCROOT,
	PLUGIN_FUNC_HANDLE_PHYSICAL,
	PLUGIN_FUNC_CONNECTION_RESET,
	/* PLUGIN_FUNC_INIT, *//* handled here in plugin.c */
	/* PLUGIN_FUNC_CLEANUP, *//* handled here in plugin.c */
	PLUGIN_FUNC_SET_DEFAULTS,
	PLUGIN_FUNC_WORKER_INIT,

	PLUGIN_FUNC_SIZEOF
} plugin_t;

static plugin *plugin_init(void) {
	plugin *p;

	p = calloc(1, sizeof(*p));
	force_assert(NULL != p);

	return p;
}

static void plugin_free(plugin *p) {
    if (NULL == p) return; /*(should not happen w/ current usage)*/
  #if !defined(LIGHTTPD_STATIC)
    if (p->lib) {
     #if defined(HAVE_VALGRIND_VALGRIND_H)
     /*if (!RUNNING_ON_VALGRIND) */
     #endif
      #if defined(__WIN32)
        FreeLibrary(p->lib);
      #else
        dlclose(p->lib);
      #endif
    }
  #endif

    free(p);
}

static void plugins_register(server *srv, plugin *p) {
	plugin **ps;
	if (srv->plugins.used == srv->plugins.size) {
		srv->plugins.size += 4;
		srv->plugins.ptr   = realloc(srv->plugins.ptr, srv->plugins.size * sizeof(*ps));
		force_assert(NULL != srv->plugins.ptr);
	}

	ps = srv->plugins.ptr;
	ps[srv->plugins.used++] = p;
}

/**
 *
 *
 *
 */

#if defined(LIGHTTPD_STATIC)

/* pre-declare functions, as there is no header for them */
#define PLUGIN_INIT(x)\
	int x ## _plugin_init(plugin *p);

#include "plugin-static.h"

#undef PLUGIN_INIT

/* build NULL-terminated table of name + init-function */

typedef struct {
	const char* name;
	int (*plugin_init)(plugin *p);
} plugin_load_functions;

static const plugin_load_functions load_functions[] = {
#define PLUGIN_INIT(x) \
	{ #x, &x ## _plugin_init },

#include "plugin-static.h"

	{ NULL, NULL }
#undef PLUGIN_INIT
};

int plugins_load(server *srv) {
	for (uint32_t i = 0; i < srv->srvconf.modules->used; ++i) {
		data_string *ds = (data_string *)srv->srvconf.modules->data[i];
		char *module = ds->value.ptr;

		uint32_t j;
		for (j = 0; load_functions[j].name; ++j) {
			if (0 == strcmp(load_functions[j].name, module)) {
				plugin * const p = plugin_init();
				if ((*load_functions[j].plugin_init)(p)) {
					log_error(srv->errh, __FILE__, __LINE__, "%s plugin init failed", module);
					plugin_free(p);
					return -1;
				}
				plugins_register(srv, p);
				break;
			}
		}
		if (!load_functions[j].name) {
			log_error(srv->errh, __FILE__, __LINE__, "%s plugin not found", module);
			return -1;
		}
	}

	return 0;
}
#else /* defined(LIGHTTPD_STATIC) */
int plugins_load(server *srv) {
	buffer * const tb = srv->tmp_buf;
	plugin *p;
	int (*init)(plugin *pl);

	for (uint32_t i = 0; i < srv->srvconf.modules->used; ++i) {
		const buffer * const module = &((data_string *)srv->srvconf.modules->data[i])->value;

		buffer_copy_buffer(tb, srv->srvconf.modules_dir);

		buffer_append_string_len(tb, CONST_STR_LEN("/"));
		buffer_append_string_buffer(tb, module);
#if defined(__WIN32) || defined(__CYGWIN__)
		buffer_append_string_len(tb, CONST_STR_LEN(".dll"));
#else
		buffer_append_string_len(tb, CONST_STR_LEN(".so"));
#endif

		p = plugin_init();
#ifdef __WIN32
		if (NULL == (p->lib = LoadLibrary(tb->ptr))) {
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, NULL);

			log_error(srv->errh, __FILE__, __LINE__,
			  "LoadLibrary() failed %s %s", lpMsgBuf, tb->ptr);

			plugin_free(p);

			return -1;

		}
#else
		if (NULL == (p->lib = dlopen(tb->ptr, RTLD_NOW|RTLD_GLOBAL))) {
			log_error(srv->errh, __FILE__, __LINE__,
			  "dlopen() failed for: %s %s", tb->ptr, dlerror());

			plugin_free(p);

			return -1;
		}

#endif
		buffer_copy_buffer(tb, module);
		buffer_append_string_len(tb, CONST_STR_LEN("_plugin_init"));

#ifdef __WIN32
		init = GetProcAddress(p->lib, tb->ptr);

		if (init == NULL) {
			LPVOID lpMsgBuf;
			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, NULL);

			log_error(srv->errh, __FILE__, __LINE__,
			  "getprocaddress failed: %s %s", tb->ptr, lpMsgBuf);

			plugin_free(p);
			return -1;
		}

#else
#if 1
		init = (int (*)(plugin *))(intptr_t)dlsym(p->lib, tb->ptr);
#else
		*(void **)(&init) = dlsym(p->lib, tb->ptr);
#endif
		if (NULL == init) {
			const char *error = dlerror();
			if (error != NULL) {
				log_error(srv->errh, __FILE__, __LINE__, "dlsym: %s", error);
			} else {
				log_error(srv->errh, __FILE__, __LINE__, "dlsym symbol not found: %s", tb->ptr);
			}

			plugin_free(p);
			return -1;
		}

#endif
		if ((*init)(p)) {
			log_error(srv->errh, __FILE__, __LINE__, "%s plugin init failed", module->ptr);

			plugin_free(p);
			return -1;
		}
#if 0
		log_error(srv->errh, __FILE__, __LINE__, "%s plugin loaded", module->ptr);
#endif
		plugins_register(srv, p);
	}

	return 0;
}
#endif /* defined(LIGHTTPD_STATIC) */

typedef struct {
  handler_t(*fn)();
  plugin_data_base *data;
} plugin_fn_data;

__attribute_hot__
static handler_t plugins_call_fn_req_data(request_st * const r, const int e) {
    const void * const plugin_slots = r->con->plugin_slots;
    const uint32_t offset = ((const uint16_t *)plugin_slots)[e];
    if (0 == offset) return HANDLER_GO_ON;
    const plugin_fn_data *plfd = (const plugin_fn_data *)
      (((uintptr_t)plugin_slots) + offset);
    handler_t rc = HANDLER_GO_ON;
    while (plfd->fn && (rc = plfd->fn(r, plfd->data)) == HANDLER_GO_ON)
        ++plfd;
    return rc;
}

__attribute_hot__
static handler_t plugins_call_fn_con_data(connection * const con, const int e) {
    const void * const plugin_slots = con->plugin_slots;
    const uint32_t offset = ((const uint16_t *)plugin_slots)[e];
    if (0 == offset) return HANDLER_GO_ON;
    const plugin_fn_data *plfd = (const plugin_fn_data *)
      (((uintptr_t)plugin_slots) + offset);
    handler_t rc = HANDLER_GO_ON;
    while (plfd->fn && (rc = plfd->fn(con, plfd->data)) == HANDLER_GO_ON)
        ++plfd;
    return rc;
}

static handler_t plugins_call_fn_srv_data(server * const srv, const int e) {
    const uint32_t offset = ((const uint16_t *)srv->plugin_slots)[e];
    if (0 == offset) return HANDLER_GO_ON;
    const plugin_fn_data *plfd = (const plugin_fn_data *)
      (((uintptr_t)srv->plugin_slots) + offset);
    handler_t rc = HANDLER_GO_ON;
    while (plfd->fn && (rc = plfd->fn(srv,plfd->data)) == HANDLER_GO_ON)
        ++plfd;
    return rc;
}

static void plugins_call_fn_srv_data_all(server * const srv, const int e) {
    const uint32_t offset = ((const uint16_t *)srv->plugin_slots)[e];
    if (0 == offset) return;
    const plugin_fn_data *plfd = (const plugin_fn_data *)
      (((uintptr_t)srv->plugin_slots) + offset);
    for (; plfd->fn; ++plfd)
        plfd->fn(srv, plfd->data);
}

/**
 * plugins that use
 *
 * - request_st *r
 * - void *p_d (plugin_data *)
 */

#define PLUGIN_CALL_FN_REQ_DATA(x, y) \
    handler_t plugins_call_##y(request_st * const r) {\
        return plugins_call_fn_req_data(r, x); \
    }

PLUGIN_CALL_FN_REQ_DATA(PLUGIN_FUNC_HANDLE_URI_CLEAN, handle_uri_clean)
PLUGIN_CALL_FN_REQ_DATA(PLUGIN_FUNC_HANDLE_URI_RAW, handle_uri_raw)
PLUGIN_CALL_FN_REQ_DATA(PLUGIN_FUNC_HANDLE_REQUEST_ENV, handle_request_env)
PLUGIN_CALL_FN_REQ_DATA(PLUGIN_FUNC_HANDLE_REQUEST_DONE, handle_request_done)
PLUGIN_CALL_FN_REQ_DATA(PLUGIN_FUNC_HANDLE_SUBREQUEST_START, handle_subrequest_start)
PLUGIN_CALL_FN_REQ_DATA(PLUGIN_FUNC_HANDLE_RESPONSE_START, handle_response_start)
PLUGIN_CALL_FN_REQ_DATA(PLUGIN_FUNC_HANDLE_DOCROOT, handle_docroot)
PLUGIN_CALL_FN_REQ_DATA(PLUGIN_FUNC_HANDLE_PHYSICAL, handle_physical)
PLUGIN_CALL_FN_REQ_DATA(PLUGIN_FUNC_CONNECTION_RESET, handle_request_reset)

/**
 * plugins that use
 *
 * - connection *con
 * - void *p_d (plugin_data *)
 */

#define PLUGIN_CALL_FN_CON_DATA(x, y) \
    handler_t plugins_call_##y(connection *con) {\
        return plugins_call_fn_con_data(con, x); \
    }

PLUGIN_CALL_FN_CON_DATA(PLUGIN_FUNC_HANDLE_CONNECTION_ACCEPT, handle_connection_accept)
PLUGIN_CALL_FN_CON_DATA(PLUGIN_FUNC_HANDLE_CONNECTION_SHUT_WR, handle_connection_shut_wr)
PLUGIN_CALL_FN_CON_DATA(PLUGIN_FUNC_HANDLE_CONNECTION_CLOSE, handle_connection_close)

#undef PLUGIN_CALL_FN_SRV_CON_DATA

/**
 * plugins that use
 *
 * - server *srv
 * - void *p_d (plugin_data *)
 */

handler_t plugins_call_set_defaults(server *srv) {
    return plugins_call_fn_srv_data(srv, PLUGIN_FUNC_SET_DEFAULTS);
}

handler_t plugins_call_worker_init(server *srv) {
    return plugins_call_fn_srv_data(srv, PLUGIN_FUNC_WORKER_INIT);
}

void plugins_call_handle_trigger(server *srv) {
    plugins_call_fn_srv_data_all(srv, PLUGIN_FUNC_HANDLE_TRIGGER);
}

void plugins_call_handle_sighup(server *srv) {
    plugins_call_fn_srv_data_all(srv, PLUGIN_FUNC_HANDLE_SIGHUP);
}

handler_t plugins_call_handle_waitpid(server *srv, pid_t pid, int status) {
    const uint32_t offset =
      ((const uint16_t *)srv->plugin_slots)[PLUGIN_FUNC_HANDLE_WAITPID];
    if (0 == offset) return HANDLER_GO_ON;
    const plugin_fn_data *plfd = (const plugin_fn_data *)
      (((uintptr_t)srv->plugin_slots) + offset);
    handler_t rc = HANDLER_GO_ON;
    while (plfd->fn&&(rc=plfd->fn(srv,plfd->data,pid,status))==HANDLER_GO_ON)
        ++plfd;
    return rc;
}

static void plugins_call_cleanup(server * const srv) {
    plugin ** const ps = srv->plugins.ptr;
    for (uint32_t i = 0; i < srv->plugins.used; ++i) {
        plugin *p = ps[i];
        if (NULL == p) continue;
        if (NULL != p->data) {
            plugin_data_base *pd = p->data;
            if (p->cleanup)
                p->cleanup(p->data);
            free(pd->cvlist);
            free(pd);
            p->data = NULL;
        }
    }
}

/**
 *
 * - call init function of all plugins to init the plugin-internals
 * - added each plugin that supports has callback to the corresponding slot
 *
 * - is only called once.
 */

__attribute_cold__
static void plugins_call_init_slot(server *srv, handler_t(*fn)(), void *data, const uint32_t offset) {
    if (fn) {
        plugin_fn_data *plfd = (plugin_fn_data *)
          (((uintptr_t)srv->plugin_slots) + offset);
        while (plfd->fn) ++plfd;
        plfd->fn = fn;
        plfd->data = data;
    }
}

handler_t plugins_call_init(server *srv) {
	plugin ** const ps = srv->plugins.ptr;
	uint16_t offsets[PLUGIN_FUNC_SIZEOF];
	memset(offsets, 0, sizeof(offsets));

	for (uint32_t i = 0; i < srv->plugins.used; ++i) {
		/* check which calls are supported */

		plugin *p = ps[i];

		if (p->init) {
			if (NULL == (p->data = p->init())) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "plugin-init failed for module %s", p->name);
				return HANDLER_ERROR;
			}

			((plugin_data_base *)(p->data))->self = p;
			((plugin_data_base *)(p->data))->id = i + 1;

			if (p->version != LIGHTTPD_VERSION_ID) {
				log_error(srv->errh, __FILE__, __LINE__,
				  "plugin-version doesn't match lighttpd-version for %s", p->name);
				return HANDLER_ERROR;
			}
		}

		if (p->priv_defaults && HANDLER_ERROR==p->priv_defaults(srv, p->data)) {
			return HANDLER_ERROR;
		}

		if (p->handle_uri_clean)
			++offsets[PLUGIN_FUNC_HANDLE_URI_CLEAN];
		if (p->handle_uri_raw)
			++offsets[PLUGIN_FUNC_HANDLE_URI_RAW];
		if (p->handle_request_env)
			++offsets[PLUGIN_FUNC_HANDLE_REQUEST_ENV];
		if (p->handle_request_done)
			++offsets[PLUGIN_FUNC_HANDLE_REQUEST_DONE];
		if (p->handle_connection_accept)
			++offsets[PLUGIN_FUNC_HANDLE_CONNECTION_ACCEPT];
		if (p->handle_connection_shut_wr)
			++offsets[PLUGIN_FUNC_HANDLE_CONNECTION_SHUT_WR];
		if (p->handle_connection_close)
			++offsets[PLUGIN_FUNC_HANDLE_CONNECTION_CLOSE];
		if (p->handle_trigger)
			++offsets[PLUGIN_FUNC_HANDLE_TRIGGER];
		if (p->handle_sighup)
			++offsets[PLUGIN_FUNC_HANDLE_SIGHUP];
		if (p->handle_waitpid)
			++offsets[PLUGIN_FUNC_HANDLE_WAITPID];
		if (p->handle_subrequest_start)
			++offsets[PLUGIN_FUNC_HANDLE_SUBREQUEST_START];
		if (p->handle_response_start)
			++offsets[PLUGIN_FUNC_HANDLE_RESPONSE_START];
		if (p->handle_docroot)
			++offsets[PLUGIN_FUNC_HANDLE_DOCROOT];
		if (p->handle_physical)
			++offsets[PLUGIN_FUNC_HANDLE_PHYSICAL];
		if (p->handle_request_reset)
			++offsets[PLUGIN_FUNC_CONNECTION_RESET];
		if (p->set_defaults)
			++offsets[PLUGIN_FUNC_SET_DEFAULTS];
		if (p->worker_init)
			++offsets[PLUGIN_FUNC_WORKER_INIT];
	}

	uint32_t nslots =
	  (sizeof(offsets)+sizeof(plugin_fn_data)-1) / sizeof(plugin_fn_data);
	for (uint32_t i = 0; i < PLUGIN_FUNC_SIZEOF; ++i) {
		if (offsets[i]) {
			uint32_t offset = nslots;
			nslots += offsets[i]+1; /* +1 to mark end of each list */
			force_assert(offset * sizeof(plugin_fn_data) <= USHRT_MAX);
			offsets[i] = (uint16_t)(offset * sizeof(plugin_fn_data));
		}
	}

	/* allocate and fill slots of two dimensional array */
	srv->plugin_slots = calloc(nslots, sizeof(plugin_fn_data));
	force_assert(NULL != srv->plugin_slots);
	memcpy(srv->plugin_slots, offsets, sizeof(offsets));

	for (uint32_t i = 0; i < srv->plugins.used; ++i) {
		plugin * const p = ps[i];

		plugins_call_init_slot(srv, p->handle_uri_clean, p->data,
					offsets[PLUGIN_FUNC_HANDLE_URI_CLEAN]);
		plugins_call_init_slot(srv, p->handle_uri_raw, p->data,
					offsets[PLUGIN_FUNC_HANDLE_URI_RAW]);
		plugins_call_init_slot(srv, p->handle_request_env, p->data,
					offsets[PLUGIN_FUNC_HANDLE_REQUEST_ENV]);
		plugins_call_init_slot(srv, p->handle_request_done, p->data,
					offsets[PLUGIN_FUNC_HANDLE_REQUEST_DONE]);
		plugins_call_init_slot(srv, p->handle_connection_accept, p->data,
					offsets[PLUGIN_FUNC_HANDLE_CONNECTION_ACCEPT]);
		plugins_call_init_slot(srv, p->handle_connection_shut_wr, p->data,
					offsets[PLUGIN_FUNC_HANDLE_CONNECTION_SHUT_WR]);
		plugins_call_init_slot(srv, p->handle_connection_close, p->data,
					offsets[PLUGIN_FUNC_HANDLE_CONNECTION_CLOSE]);
		plugins_call_init_slot(srv, p->handle_trigger, p->data,
					offsets[PLUGIN_FUNC_HANDLE_TRIGGER]);
		plugins_call_init_slot(srv, p->handle_sighup, p->data,
					offsets[PLUGIN_FUNC_HANDLE_SIGHUP]);
		plugins_call_init_slot(srv, p->handle_waitpid, p->data,
					offsets[PLUGIN_FUNC_HANDLE_WAITPID]);
		plugins_call_init_slot(srv, p->handle_subrequest_start, p->data,
					offsets[PLUGIN_FUNC_HANDLE_SUBREQUEST_START]);
		plugins_call_init_slot(srv, p->handle_response_start, p->data,
					offsets[PLUGIN_FUNC_HANDLE_RESPONSE_START]);
		plugins_call_init_slot(srv, p->handle_docroot, p->data,
					offsets[PLUGIN_FUNC_HANDLE_DOCROOT]);
		plugins_call_init_slot(srv, p->handle_physical, p->data,
					offsets[PLUGIN_FUNC_HANDLE_PHYSICAL]);
		plugins_call_init_slot(srv, p->handle_request_reset, p->data,
					offsets[PLUGIN_FUNC_CONNECTION_RESET]);
		plugins_call_init_slot(srv, p->set_defaults, p->data,
					offsets[PLUGIN_FUNC_SET_DEFAULTS]);
		plugins_call_init_slot(srv, p->worker_init, p->data,
					offsets[PLUGIN_FUNC_WORKER_INIT]);
	}

	return HANDLER_GO_ON;
}

void plugins_free(server *srv) {
	if (srv->plugin_slots) {
		plugins_call_cleanup(srv);
		free(srv->plugin_slots);
		srv->plugin_slots = NULL;
	}

	for (uint32_t i = 0; i < srv->plugins.used; ++i) {
		plugin_free(((plugin **)srv->plugins.ptr)[i]);
	}
	free(srv->plugins.ptr);
	srv->plugins.ptr = NULL;
	srv->plugins.used = 0;
	srv->plugins.size = 0;
	array_free_data(&plugin_stats);
}
