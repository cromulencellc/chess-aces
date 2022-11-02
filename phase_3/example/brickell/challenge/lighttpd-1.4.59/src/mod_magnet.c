#include "first.h"

#include "base.h"
#include "log.h"
#include "buffer.h"
#include "http_chunk.h"
#include "http_header.h"
#include "response.h"   /* http_response_send_1xx() */

#include "plugin.h"

#include "mod_magnet_cache.h"
#include "sock_addr.h"
#include "stat_cache.h"
#include "status_counter.h"

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include <lua.h>
#include <lauxlib.h>

#define LUA_RIDX_LIGHTTPD_REQUEST "lighty.request"

#define MAGNET_RESTART_REQUEST      99

/* plugin config for all request/connections */

static jmp_buf exceptionjmp;

typedef struct {
    const array *url_raw;
    const array *physical_path;
    const array *response_start;
    int stage;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config defaults;
    plugin_config conf;

    script_cache cache;
} plugin_data;

INIT_FUNC(mod_magnet_init) {
    return calloc(1, sizeof(plugin_data));
}

FREE_FUNC(mod_magnet_free) {
    plugin_data *p = p_d;
    script_cache_free_data(&p->cache);
}

static void mod_magnet_merge_config_cpv(plugin_config * const pconf, const config_plugin_value_t * const cpv) {
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* magnet.attract-raw-url-to */
        pconf->url_raw = cpv->v.a;
        break;
      case 1: /* magnet.attract-physical-path-to */
        pconf->physical_path = cpv->v.a;
        break;
      case 2: /* magnet.attract-response-start-to */
        pconf->response_start = cpv->v.a;
        break;
      default:/* should not happen */
        return;
    }
}

static void mod_magnet_merge_config(plugin_config * const pconf, const config_plugin_value_t *cpv) {
    do {
        mod_magnet_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}

static void mod_magnet_patch_config(request_st * const r, plugin_data * const p) {
    p->conf = p->defaults; /* copy small struct instead of memcpy() */
    /*memcpy(&p->conf, &p->defaults, sizeof(plugin_config));*/
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_magnet_merge_config(&p->conf, p->cvlist + p->cvlist[i].v.u2[0]);
    }
}

SETDEFAULTS_FUNC(mod_magnet_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("magnet.attract-raw-url-to"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("magnet.attract-physical-path-to"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("magnet.attract-response-start-to"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_magnet"))
        return HANDLER_ERROR;

    /* process and validate config directives
     * (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1]; i < p->nconfig; ++i) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        for (; -1 != cpv->k_id; ++cpv) {
            switch (cpv->k_id) {
              case 0: /* magnet.attract-raw-url-to */
              case 1: /* magnet.attract-physical-path-to */
              case 2: /* magnet.attract-response-start-to */
                for (uint32_t j = 0; j < cpv->v.a->used; ++j) {
                    data_string *ds = (data_string *)cpv->v.a->data[j];
                    if (buffer_string_is_empty(&ds->value)) {
                        log_error(srv->errh, __FILE__, __LINE__,
                          "unexpected (blank) value for %s; "
                          "expected list of \"scriptpath\"", cpk[cpv->k_id].k);
                        return HANDLER_ERROR;
                    }
                }
                break;
              default:/* should not happen */
                break;
            }
        }
    }

    /* initialize p->defaults from global config context */
    if (p->nconfig > 0 && p->cvlist->v.u2[1]) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist->v.u2[0];
        if (-1 != cpv->k_id)
            mod_magnet_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
/* lua5.1 backward compat definition */
static void lua_pushglobaltable(lua_State *L) { /* (-0, +1, -) */
	lua_pushvalue(L, LUA_GLOBALSINDEX);
}
#endif

static void magnet_setfenv_mainfn(lua_State *L, int funcIndex) { /* (-1, 0, -) */
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 502
	/* set "_ENV" upvalue, which should be the first upvalue of a "main" lua
	 * function if it uses any global names
	 */

	const char* first_upvalue_name = lua_getupvalue(L, funcIndex, 1);
	if (NULL == first_upvalue_name) return; /* doesn't have any upvalues */
	lua_pop(L, 1); /* only need the name of the upvalue, not the value */

	if (0 != strcmp(first_upvalue_name, "_ENV")) return;

	if (NULL == lua_setupvalue(L, funcIndex, 1)) {
		/* pop value if lua_setupvalue didn't set the (not existing) upvalue */
		lua_pop(L, 1);
	}
#else
	lua_setfenv(L, funcIndex);
#endif
}

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
/* lua 5.2 already supports __pairs */

/* See http://lua-users.org/wiki/GeneralizedPairsAndIpairs for implementation details.
 * Override the default pairs() function to allow us to use a __pairs metakey
 */
static int magnet_pairs(lua_State *L) {
	luaL_checkany(L, 1); /* "self" */

	if (luaL_getmetafield(L, 1, "__pairs")) {
		/* call __pairs(self) */
		lua_pushvalue(L, 1);
		lua_call(L, 1, 3);
	} else {
		/* call <original-pairs-method>(self) */
		lua_pushvalue(L, lua_upvalueindex(1));
		lua_pushvalue(L, 1);
		lua_call(L, 1, 3);
	}
	return 3;
}
#endif

static void magnet_push_buffer(lua_State *L, const buffer *b) {
    if (!buffer_is_empty(b))
        lua_pushlstring(L, CONST_BUF_LEN(b));
    else
        lua_pushnil(L);
}

#if 0
static int magnet_array_get_element(lua_State *L, const array *a) {
    /* __index: param 1 is the (empty) table the value was not found in */
    size_t klen;
    const char * const k = luaL_checklstring(L, 2, &klen);
    const data_string * const ds = (const data_string *)
      array_get_element_klen(a, k, klen);
    magnet_push_buffer(L, NULL != ds ? &ds->value : NULL);
    return 1;
}
#endif

/* Define a function that will iterate over an array* (in upval 1) using current position (upval 2) */
static int magnet_array_next(lua_State *L) {
	data_unset *du;
	data_string *ds;
	data_integer *di;

	size_t pos = lua_tointeger(L, lua_upvalueindex(1));
	array *a = lua_touserdata(L, lua_upvalueindex(2));

	lua_settop(L, 0);

	if (pos >= a->used) return 0;
	if (NULL != (du = a->data[pos])) {
		lua_pushlstring(L, CONST_BUF_LEN(&du->key));
		switch (du->type) {
			case TYPE_STRING:
				ds = (data_string *)du;
				magnet_push_buffer(L, &ds->value);
				break;
			case TYPE_INTEGER:
				di = (data_integer *)du;
				lua_pushinteger(L, di->value);
				break;
			default:
				lua_pushnil(L);
				break;
		}

		/* Update our positional upval to reflect our new current position */
		pos++;
		lua_pushinteger(L, pos);
		lua_replace(L, lua_upvalueindex(1));

		/* Returning 2 items on the stack (key, value) */
		return 2;
	}
	return 0;
}

/* Create the closure necessary to iterate over the array *a with the above function */
static int magnet_array_pairs(lua_State *L, array *a) {
	lua_pushinteger(L, 0); /* Push our current pos (the start) into upval 1 */
	lua_pushlightuserdata(L, a); /* Push our array *a into upval 2 */
	lua_pushcclosure(L, magnet_array_next, 2); /* Push our new closure with 2 upvals */
	return 1;
}

static request_st * magnet_get_request(lua_State *L) {
	lua_getfield(L, LUA_REGISTRYINDEX, LUA_RIDX_LIGHTTPD_REQUEST);
	request_st * const r = lua_touserdata(L, -1);
	lua_pop(L, 1);
	return r;
}

typedef struct {
	const char *ptr;
	size_t len;
} const_buffer;

static const_buffer magnet_checkconstbuffer(lua_State *L, int idx) {
	const_buffer cb;
	cb.ptr = luaL_checklstring(L, idx, &cb.len);
	return cb;
}

static buffer* magnet_checkbuffer(lua_State *L, int idx) {
	const_buffer cb = magnet_checkconstbuffer(L, idx);
	buffer *b = buffer_init();
	buffer_copy_string_len(b, cb.ptr, cb.len);
	return b;
}

static int magnet_print(lua_State *L) {
	const_buffer cb = magnet_checkconstbuffer(L, 1);
	request_st * const r = magnet_get_request(L);
	log_error(r->conf.errh, __FILE__, __LINE__, "(lua-print) %s", cb.ptr);
	return 0;
}

static int magnet_stat(lua_State *L) {
	buffer * const sb = magnet_checkbuffer(L, 1);
	stat_cache_entry * const sce = stat_cache_get_entry(sb);
	buffer_free(sb);
	if (NULL == sce) {
		lua_pushnil(L);
		return 1;
	}

	lua_newtable(L); // return value

	lua_pushboolean(L, S_ISREG(sce->st.st_mode));
	lua_setfield(L, -2, "is_file");

	lua_pushboolean(L, S_ISDIR(sce->st.st_mode));
	lua_setfield(L, -2, "is_dir");

	lua_pushboolean(L, S_ISCHR(sce->st.st_mode));
	lua_setfield(L, -2, "is_char");

	lua_pushboolean(L, S_ISBLK(sce->st.st_mode));
	lua_setfield(L, -2, "is_block");

	lua_pushboolean(L, S_ISSOCK(sce->st.st_mode));
	lua_setfield(L, -2, "is_socket");

	lua_pushboolean(L, S_ISLNK(sce->st.st_mode));
	lua_setfield(L, -2, "is_link");

	lua_pushboolean(L, S_ISFIFO(sce->st.st_mode));
	lua_setfield(L, -2, "is_fifo");

	lua_pushinteger(L, sce->st.st_mtime);
	lua_setfield(L, -2, "st_mtime");

	lua_pushinteger(L, sce->st.st_ctime);
	lua_setfield(L, -2, "st_ctime");

	lua_pushinteger(L, sce->st.st_atime);
	lua_setfield(L, -2, "st_atime");

	lua_pushinteger(L, sce->st.st_uid);
	lua_setfield(L, -2, "st_uid");

	lua_pushinteger(L, sce->st.st_gid);
	lua_setfield(L, -2, "st_gid");

	lua_pushinteger(L, sce->st.st_size);
	lua_setfield(L, -2, "st_size");

	lua_pushinteger(L, sce->st.st_ino);
	lua_setfield(L, -2, "st_ino");

	request_st * const r = magnet_get_request(L);
	const buffer *etag = stat_cache_etag_get(sce, r->conf.etag_flags);
	if (!buffer_string_is_empty(etag)) {
		lua_pushlstring(L, CONST_BUF_LEN(etag));
	} else {
		lua_pushnil(L);
	}
	lua_setfield(L, -2, "etag");

	const buffer *content_type = stat_cache_content_type_get(sce, r);
	if (!buffer_string_is_empty(content_type)) {
		lua_pushlstring(L, CONST_BUF_LEN(content_type));
	} else {
		lua_pushnil(L);
	}
	lua_setfield(L, -2, "content-type");

	return 1;
}


static int magnet_atpanic(lua_State *L) {
	const_buffer cb = magnet_checkconstbuffer(L, 1);
	request_st * const r = magnet_get_request(L);
	log_error(r->conf.errh, __FILE__, __LINE__, "(lua-atpanic) %s", cb.ptr);
	longjmp(exceptionjmp, 1);
}

static int magnet_reqhdr_get(lua_State *L) {
    /* __index: param 1 is the (empty) table the value was not found in */
    size_t klen;
    const char * const k = luaL_checklstring(L, 2, &klen);
    request_st * const r = magnet_get_request(L);
    const int id = http_header_hkey_get(k, (uint32_t)klen);
    const buffer * const vb = http_header_request_get(r, id, k, klen);
    magnet_push_buffer(L, NULL != vb ? vb : NULL);
    return 1;
}

static int magnet_reqhdr_pairs(lua_State *L) {
	request_st * const r = magnet_get_request(L);
	return magnet_array_pairs(L, &r->rqst_headers);
}

static int magnet_status_get(lua_State *L) {
	/* __index: param 1 is the (empty) table the value was not found in */
	const_buffer key = magnet_checkconstbuffer(L, 2);
	int *i = status_counter_get_counter(key.ptr, key.len);
	lua_pushinteger(L, (lua_Integer)*i);

	return 1;
}

static int magnet_status_set(lua_State *L) {
	/* __newindex: param 1 is the (empty) table the value is supposed to be set in */
	const_buffer key = magnet_checkconstbuffer(L, 2);
	int counter = (int) luaL_checkinteger(L, 3);

	status_counter_set(key.ptr, key.len, counter);

	return 0;
}

static int magnet_status_pairs(lua_State *L) {
	return magnet_array_pairs(L, &plugin_stats);
}

typedef struct {
	const char *name;
	enum {
		MAGNET_ENV_UNSET,

		MAGNET_ENV_PHYSICAL_PATH,
		MAGNET_ENV_PHYSICAL_REL_PATH,
		MAGNET_ENV_PHYSICAL_DOC_ROOT,
		MAGNET_ENV_PHYSICAL_BASEDIR,

		MAGNET_ENV_URI_PATH,
		MAGNET_ENV_URI_PATH_RAW,
		MAGNET_ENV_URI_SCHEME,
		MAGNET_ENV_URI_AUTHORITY,
		MAGNET_ENV_URI_QUERY,

		MAGNET_ENV_REQUEST_METHOD,
		MAGNET_ENV_REQUEST_URI,
		MAGNET_ENV_REQUEST_ORIG_URI,
		MAGNET_ENV_REQUEST_PATH_INFO,
		MAGNET_ENV_REQUEST_REMOTE_IP,
		MAGNET_ENV_REQUEST_SERVER_ADDR,
		MAGNET_ENV_REQUEST_PROTOCOL,

		MAGNET_ENV_RESPONSE_HTTP_STATUS,
		MAGNET_ENV_RESPONSE_BODY_LENGTH,
		MAGNET_ENV_RESPONSE_BODY
	} type;
} magnet_env_t;

static const magnet_env_t magnet_env[] = {
	{ "physical.path", MAGNET_ENV_PHYSICAL_PATH },
	{ "physical.rel-path", MAGNET_ENV_PHYSICAL_REL_PATH },
	{ "physical.doc-root", MAGNET_ENV_PHYSICAL_DOC_ROOT },
	{ "physical.basedir", MAGNET_ENV_PHYSICAL_BASEDIR },

	{ "uri.path", MAGNET_ENV_URI_PATH },
	{ "uri.path-raw", MAGNET_ENV_URI_PATH_RAW },
	{ "uri.scheme", MAGNET_ENV_URI_SCHEME },
	{ "uri.authority", MAGNET_ENV_URI_AUTHORITY },
	{ "uri.query", MAGNET_ENV_URI_QUERY },

	{ "request.method", MAGNET_ENV_REQUEST_METHOD },
	{ "request.uri", MAGNET_ENV_REQUEST_URI },
	{ "request.orig-uri", MAGNET_ENV_REQUEST_ORIG_URI },
	{ "request.path-info", MAGNET_ENV_REQUEST_PATH_INFO },
	{ "request.remote-ip", MAGNET_ENV_REQUEST_REMOTE_IP },
	{ "request.remote-addr", MAGNET_ENV_REQUEST_REMOTE_IP },
	{ "request.server-addr", MAGNET_ENV_REQUEST_SERVER_ADDR },
	{ "request.protocol", MAGNET_ENV_REQUEST_PROTOCOL },

	{ "response.http-status", MAGNET_ENV_RESPONSE_HTTP_STATUS },
	{ "response.body-length", MAGNET_ENV_RESPONSE_BODY_LENGTH },
	{ "response.body", MAGNET_ENV_RESPONSE_BODY },

	{ NULL, MAGNET_ENV_UNSET }
};

static buffer *magnet_env_get_buffer_by_id(request_st * const r, int id) {
	buffer *dest = NULL;

	/**
	 * map all internal variables to lua
	 *
	 */

	switch (id) {
	case MAGNET_ENV_PHYSICAL_PATH: dest = &r->physical.path; break;
	case MAGNET_ENV_PHYSICAL_REL_PATH: dest = &r->physical.rel_path; break;
	case MAGNET_ENV_PHYSICAL_DOC_ROOT: dest = &r->physical.doc_root; break;
	case MAGNET_ENV_PHYSICAL_BASEDIR: dest = &r->physical.basedir; break;

	case MAGNET_ENV_URI_PATH: dest = &r->uri.path; break;
	case MAGNET_ENV_URI_PATH_RAW:
	    {
		dest = r->tmp_buf;
		buffer_clear(dest);
		uint32_t len = buffer_string_length(&r->target);
		char *qmark = memchr(r->target.ptr, '?', len);
		buffer_copy_string_len(dest, r->target.ptr, qmark ? (uint32_t)(qmark - r->target.ptr) : len);
		break;
	    }
	case MAGNET_ENV_URI_SCHEME: dest = &r->uri.scheme; break;
	case MAGNET_ENV_URI_AUTHORITY: dest = &r->uri.authority; break;
	case MAGNET_ENV_URI_QUERY: dest = &r->uri.query; break;

	case MAGNET_ENV_REQUEST_METHOD:
		dest = r->tmp_buf;
		buffer_clear(dest);
		http_method_append(dest, r->http_method);
		break;
	case MAGNET_ENV_REQUEST_URI:      dest = &r->target; break;
	case MAGNET_ENV_REQUEST_ORIG_URI: dest = &r->target_orig; break;
	case MAGNET_ENV_REQUEST_PATH_INFO: dest = &r->pathinfo; break;
	case MAGNET_ENV_REQUEST_REMOTE_IP: dest = r->con->dst_addr_buf; break;
	case MAGNET_ENV_REQUEST_SERVER_ADDR:
	    {
		const server_socket * const srv_socket = r->con->srv_socket;
		dest = r->tmp_buf;
		buffer_clear(dest);
		switch (sock_addr_get_family(&srv_socket->addr)) {
		case AF_INET:
		case AF_INET6:
			if (sock_addr_is_addr_wildcard(&srv_socket->addr)) {
				sock_addr addrbuf;
				socklen_t addrlen = sizeof(addrbuf);
				const int fd = r->con->fd;
				if (0 == getsockname(fd,(struct sockaddr *)&addrbuf,&addrlen)) {
					char buf[INET6_ADDRSTRLEN + 1];
					const char *s = sock_addr_inet_ntop(&addrbuf, buf, sizeof(buf)-1);
					if (NULL != s)
						buffer_copy_string_len(dest, s, strlen(s));
				}
			}
			else {
				buffer_copy_buffer(dest, srv_socket->srv_token);
				if (dest->ptr[0] != '[' || dest->ptr[buffer_string_length(dest)-1] != ']') {
					char *s = strrchr(dest->ptr, ':');
					if (s != NULL) /* local IP without port */
						buffer_string_set_length(dest, s - dest->ptr);
				}
			}
			break;
		default:
			break;
		}
		break;
	    }
	case MAGNET_ENV_REQUEST_PROTOCOL:
		dest = r->tmp_buf;
		buffer_clear(dest);
		http_version_append(dest, r->http_version);
		break;
	case MAGNET_ENV_RESPONSE_HTTP_STATUS:
		dest = r->tmp_buf;
		buffer_clear(dest);
		buffer_append_int(dest, r->http_status);
		break;
	case MAGNET_ENV_RESPONSE_BODY_LENGTH:
		dest = r->tmp_buf;
		buffer_clear(dest);
		if (!r->resp_body_finished)
			break;
		buffer_append_int(dest, chunkqueue_length(&r->write_queue));
		break;
	case MAGNET_ENV_RESPONSE_BODY:
		if (!r->resp_body_finished)
			break;
		else {
			chunkqueue * const cq = &r->write_queue;
			off_t len = chunkqueue_length(cq);
			if (0 == len) {
				dest = r->tmp_buf;
				buffer_copy_string_len(dest, CONST_STR_LEN(""));
				break;
			}
			dest = chunkqueue_read_squash(cq, r->conf.errh);
			if (NULL == dest) {
				dest = r->tmp_buf;
				buffer_clear(dest);
			}
		}
		break;

	case MAGNET_ENV_UNSET: break;
	}

	return dest;
}

static int magnet_env_get_id(const char * const key) {
	for (int i = 0; magnet_env[i].name; ++i) {
		if (0 == strcmp(key, magnet_env[i].name))
			return magnet_env[i].type;
	}
	return MAGNET_ENV_UNSET;
}

static buffer *magnet_env_get_buffer(request_st * const r, const char * const key) {
	return magnet_env_get_buffer_by_id(r, magnet_env_get_id(key));
}

static int magnet_env_get(lua_State *L) {
	/* __index: param 1 is the (empty) table the value was not found in */
	const char *key = luaL_checkstring(L, 2);
	request_st * const r = magnet_get_request(L);
	magnet_push_buffer(L, magnet_env_get_buffer(r, key));
	return 1;
}

static int magnet_env_set(lua_State *L) {
    /* __newindex: param 1 is the (empty) table the value is supposed to be set in */
    const char * const key = luaL_checkstring(L, 2);
    luaL_checkany(L, 3); /* nil or a string */

    request_st * const r = magnet_get_request(L);
    const int env_id = magnet_env_get_id(key);

    switch (env_id) {
      default:
        break;
      case MAGNET_ENV_URI_PATH_RAW:
      {
        /* modify uri-path of r->target; preserve query-part, if present */
        /* XXX: should we require that resulting path begin with '/' or %2F ? */
        const uint32_t len = buffer_string_length(&r->target);
        const char * const qmark = memchr(r->target.ptr, '?', len);
        const_buffer val = { NULL, 0 };
        if (!lua_isnil(L, 3))
            val = magnet_checkconstbuffer(L, 3);
        if (NULL != qmark)
            buffer_copy_string_len(r->tmp_buf, qmark,
                                   len - (uint32_t)(qmark - r->target.ptr));
        buffer_copy_string_len(&r->target, val.ptr, val.len);
        if (NULL != qmark)
            buffer_append_string_buffer(&r->target, r->tmp_buf);
        return 0;
      }
      case MAGNET_ENV_RESPONSE_HTTP_STATUS:
      case MAGNET_ENV_RESPONSE_BODY_LENGTH:
      case MAGNET_ENV_RESPONSE_BODY:
        return luaL_error(L, "lighty.env['%s'] is read-only", key);
    }

    buffer * const dest = magnet_env_get_buffer_by_id(r, env_id);
    if (NULL == dest)
        return luaL_error(L, "couldn't store '%s' in lighty.env[]", key);

    if (lua_isnil(L, 3)) {
        if (env_id==MAGNET_ENV_URI_QUERY || env_id==MAGNET_ENV_PHYSICAL_PATH)
            buffer_clear(dest);
        else
            buffer_string_set_length(dest, 0);
    }
    else {
        const_buffer val = magnet_checkconstbuffer(L, 3);
        buffer_copy_string_len(dest, val.ptr, val.len);
        /* NB: setting r->uri.query does not modify query-part in r->target
         * (r->uri.query is uri-decoded; r->target is not) */
    }

    switch (env_id) {
      case MAGNET_ENV_URI_SCHEME:
      case MAGNET_ENV_URI_AUTHORITY:
        buffer_to_lower(dest);
        if (env_id == MAGNET_ENV_URI_AUTHORITY)
            r->server_name = dest;
        break;
      default:
        break;
    }

    return 0;
}

static int magnet_env_next(lua_State *L) {
	const int pos = lua_tointeger(L, lua_upvalueindex(1));

	/* ignore previous key: use upvalue for current pos */
	lua_settop(L, 0);

	if (NULL == magnet_env[pos].name) return 0; /* end of list */
	/* Update our positional upval to reflect our new current position */
	lua_pushinteger(L, pos + 1);
	lua_replace(L, lua_upvalueindex(1));

	/* key to return */
	lua_pushstring(L, magnet_env[pos].name);

	/* get value */
	request_st * const r = magnet_get_request(L);
	magnet_push_buffer(L, magnet_env_get_buffer_by_id(r, magnet_env[pos].type));

	/* return 2 items on the stack (key, value) */
	return 2;
}

static int magnet_env_pairs(lua_State *L) {
	lua_pushinteger(L, 0); /* Push our current pos (the start) into upval 1 */
	lua_pushcclosure(L, magnet_env_next, 1); /* Push our new closure with 1 upvals */
	return 1;
}

static int magnet_cgi_get(lua_State *L) {
    /* __index: param 1 is the (empty) table the value was not found in */
    size_t klen;
    const char * const k = luaL_checklstring(L, 2, &klen);
    request_st * const r = magnet_get_request(L);
    const buffer * const vb = http_header_env_get(r, k, klen);
    magnet_push_buffer(L, NULL != vb ? vb : NULL);
    return 1;
}

static int magnet_cgi_set(lua_State *L) {
    /* __newindex: param 1 is the (empty) table the value is supposed to be set in */
    const_buffer key = magnet_checkconstbuffer(L, 2);
    const_buffer val = magnet_checkconstbuffer(L, 3);
    request_st * const r = magnet_get_request(L);
    http_header_env_set(r, key.ptr, key.len, val.ptr, val.len);
    return 0;
}

static int magnet_cgi_pairs(lua_State *L) {
	request_st * const r = magnet_get_request(L);
	return magnet_array_pairs(L, &r->env);
}


static int magnet_copy_response_header(request_st * const r, lua_State * const L, int lighty_table_ndx) {
	force_assert(lua_istable(L, lighty_table_ndx));

	lua_getfield(L, lighty_table_ndx, "header"); /* lighty.header */
	if (lua_istable(L, -1)) {
		/* header is found, and is a table */

		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			if (lua_isstring(L, -1) && lua_isstring(L, -2)) {
				const_buffer key = magnet_checkconstbuffer(L, -2);
				const_buffer val = magnet_checkconstbuffer(L, -1);
				enum http_header_e id = http_header_hkey_get(key.ptr, key.len);

				val.len
				  ? http_header_response_set(r, id, key.ptr, key.len, val.ptr, val.len)
				  : http_header_response_unset(r, id, key.ptr, key.len);
			}

			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1); /* pop lighty.header */

	return 0;
}

/**
 * walk through the content array
 *
 * content = { "<pre>", { file = "/content" } , "</pre>" }
 *
 * header["Content-Type"] = "text/html"
 *
 * return 200
 */
static int magnet_attach_content(request_st * const r, lua_State * const L, int lighty_table_ndx) {
	force_assert(lua_istable(L, lighty_table_ndx));

	lua_getfield(L, lighty_table_ndx, "content"); /* lighty.content */
	if (lua_istable(L, -1)) {
		int i;
		/* content is found, and is a table */

		http_response_body_clear(r, 0);

		for (i = 1; ; i++) {
			lua_rawgeti(L, -1, i);

			/* -1 is the value and should be the value ... aka a table */
			if (lua_isstring(L, -1)) {
				const_buffer data = magnet_checkconstbuffer(L, -1);

				chunkqueue_append_mem(&r->write_queue, data.ptr, data.len);
			} else if (lua_istable(L, -1)) {
				lua_getfield(L, -1, "filename");
				lua_getfield(L, -2, "length"); /* (0-based) end of range (not actually "length") */
				lua_getfield(L, -3, "offset"); /* (0-based) start of range */

				if (lua_isstring(L, -3)) { /* filename has to be a string */
					off_t off = (off_t) luaL_optinteger(L, -1, 0);
					off_t len = (off_t) luaL_optinteger(L, -2, -1); /*(-1 to http_chunk_append_file_range() uses file size minus offset)*/
					if (off < 0) {
						return luaL_error(L, "offset for '%s' is negative", lua_tostring(L, -3));
					}

					if (len >= off) {
						len -= off;
					} else if (-1 != len) {
						return luaL_error(L, "offset > length for '%s'", lua_tostring(L, -3));
					}

					if (0 != len) {
						buffer *fn = magnet_checkbuffer(L, -3);
						int rc = http_chunk_append_file_range(r, fn, off, len);
						buffer_free(fn);
						if (0 != rc) {
							return luaL_error(L, "error opening file content '%s' at offset %lld", lua_tostring(L, -3), (long long)off);
						}
					}
				} else {
					return luaL_error(L, "content[%d] is a table and requires the field \"filename\"", i);
				}

				lua_pop(L, 3);
			} else if (lua_isnil(L, -1)) {
				/* end of list */

				lua_pop(L, 1);

				break;
			} else {
				return luaL_error(L, "content[%d] is neither a string nor a table: ", i);
			}

			lua_pop(L, 1); /* pop the content[...] entry value */
		}
	} else {
		return luaL_error(L, "lighty.content has to be a table");
	}
	lua_pop(L, 1); /* pop lighty.content */

	return 0;
}

static int traceback(lua_State *L) {
	if (!lua_isstring(L, 1))  /* 'message' not a string? */
		return 1;  /* keep it intact */
	lua_getglobal(L, "debug");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1)) {
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);  /* pass error message */
	lua_pushinteger(L, 2);  /* skip this function and traceback */
	lua_call(L, 2, 1);  /* call debug.traceback */
	return 1;
}

/* push traceback function before calling lua_pcall after narg arguments
 * have been pushed (inserts it before the arguments). returns index for
 * traceback function ("msgh" in lua_pcall)
 */
static int push_traceback(lua_State *L, int narg) {
	int base = lua_gettop(L) - narg;  /* function index */
	lua_pushcfunction(L, traceback);
	lua_insert(L, base);
	return base;
}

static handler_t magnet_attract(request_st * const r, plugin_data * const p, buffer * const name) {
	lua_State *L;
	int lua_return_value;
	const int func_ndx = 1;
	const int lighty_table_ndx = 2;

	/* get the script-context */
	L = script_cache_get_script(&p->cache, name, r->conf.etag_flags);

	if (lua_isstring(L, -1)) {
		log_error(r->conf.errh, __FILE__, __LINE__,
		  "loading script %s failed: %s", name->ptr, lua_tostring(L, -1));

		lua_pop(L, 1);

		force_assert(lua_gettop(L) == 0); /* only the error should have been on the stack */

		if (p->conf.stage != -1) { /* skip for response-start */
			r->http_status = 500;
			r->handler_module = NULL;
		}

		return HANDLER_FINISHED;
	}

	force_assert(lua_gettop(L) == 1);
	force_assert(lua_isfunction(L, func_ndx));

	lua_pushlightuserdata(L, r);
	lua_setfield(L, LUA_REGISTRYINDEX, LUA_RIDX_LIGHTTPD_REQUEST);

	lua_atpanic(L, magnet_atpanic);

	/**
	 * we want to create empty environment for our script
	 *
	 * setmetatable({}, {__index = _G})
	 *
	 * if a function symbol is not defined in our env, __index will lookup
	 * in the global env.
	 *
	 * all variables created in the script-env will be thrown
	 * away at the end of the script run.
	 */
	lua_newtable(L); /* my empty environment aka {}              (sp += 1) */

	/* we have to overwrite the print function */
	lua_pushcfunction(L, magnet_print);                       /* (sp += 1) */
	lua_setfield(L, -2, "print"); /* -1 is the env we want to set(sp -= 1) */

	/**
	 * lighty.request[] (ro) has the HTTP-request headers
	 * lighty.env[] (rw) has various url/physical file paths and
	 *                   request meta data; might contain nil values
	 * lighty.req_env[] (ro) has the cgi environment
	 * lighty.status[] (ro) has the status counters
	 * lighty.content[] (rw) is a table of string/file
	 * lighty.header[] (rw) is a array to set response headers
	 */

	lua_newtable(L); /* lighty.*                                 (sp += 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_newtable(L); /* the meta-table for the request-table     (sp += 1) */
	lua_pushcfunction(L, magnet_reqhdr_get);                  /* (sp += 1) */
	lua_setfield(L, -2, "__index");                           /* (sp -= 1) */
	lua_pushcfunction(L, magnet_reqhdr_pairs);                /* (sp += 1) */
	lua_setfield(L, -2, "__pairs");                           /* (sp -= 1) */
	lua_setmetatable(L, -2); /* tie the metatable to request     (sp -= 1) */
	lua_setfield(L, -2, "request"); /* content = {}              (sp -= 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_newtable(L); /* the meta-table for the env-table         (sp += 1) */
	lua_pushcfunction(L, magnet_env_get);                     /* (sp += 1) */
	lua_setfield(L, -2, "__index");                           /* (sp -= 1) */
	lua_pushcfunction(L, magnet_env_set);                     /* (sp += 1) */
	lua_setfield(L, -2, "__newindex");                        /* (sp -= 1) */
	lua_pushcfunction(L, magnet_env_pairs);                   /* (sp += 1) */
	lua_setfield(L, -2, "__pairs");                           /* (sp -= 1) */
	lua_setmetatable(L, -2); /* tie the metatable to env         (sp -= 1) */
	lua_setfield(L, -2, "env"); /* content = {}                  (sp -= 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_newtable(L); /* the meta-table for the req_env-table     (sp += 1) */
	lua_pushcfunction(L, magnet_cgi_get);                     /* (sp += 1) */
	lua_setfield(L, -2, "__index");                           /* (sp -= 1) */
	lua_pushcfunction(L, magnet_cgi_set);                     /* (sp += 1) */
	lua_setfield(L, -2, "__newindex");                        /* (sp -= 1) */
	lua_pushcfunction(L, magnet_cgi_pairs);                   /* (sp += 1) */
	lua_setfield(L, -2, "__pairs");                           /* (sp -= 1) */
	lua_setmetatable(L, -2); /* tie the metatable to req_env     (sp -= 1) */
	lua_setfield(L, -2, "req_env"); /* content = {}              (sp -= 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_newtable(L); /* the meta-table for the status-table      (sp += 1) */
	lua_pushcfunction(L, magnet_status_get);                  /* (sp += 1) */
	lua_setfield(L, -2, "__index");                           /* (sp -= 1) */
	lua_pushcfunction(L, magnet_status_set);                  /* (sp += 1) */
	lua_setfield(L, -2, "__newindex");                        /* (sp -= 1) */
	lua_pushcfunction(L, magnet_status_pairs);                /* (sp += 1) */
	lua_setfield(L, -2, "__pairs");                           /* (sp -= 1) */
	lua_setmetatable(L, -2); /* tie the metatable to statzs      (sp -= 1) */
	lua_setfield(L, -2, "status"); /* content = {}               (sp -= 1) */

	/* add empty 'content' and 'header' tables */
	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_setfield(L, -2, "content"); /* content = {}              (sp -= 1) */

	lua_newtable(L); /*  {}                                      (sp += 1) */
	lua_setfield(L, -2, "header"); /* header = {}                (sp -= 1) */

	lua_pushinteger(L, MAGNET_RESTART_REQUEST);
	lua_setfield(L, -2, "RESTART_REQUEST");

	lua_pushcfunction(L, magnet_stat);                        /* (sp += 1) */
	lua_setfield(L, -2, "stat"); /* -1 is the env we want to set (sp -= 1) */

	/* insert lighty table at index 2 */
	lua_pushvalue(L, -1);
	lua_insert(L, lighty_table_ndx);

	lua_setfield(L, -2, "lighty"); /* lighty.*                   (sp -= 1) */

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
	/* override the default pairs() function to our __pairs capable version;
	 * not needed for lua 5.2+
	 */
	lua_getglobal(L, "pairs"); /* push original pairs()          (sp += 1) */
	lua_pushcclosure(L, magnet_pairs, 1);
	lua_setfield(L, -2, "pairs");                             /* (sp -= 1) */
#endif

	lua_newtable(L); /* the meta-table for the new env           (sp += 1) */
	lua_pushglobaltable(L);                                   /* (sp += 1) */
	lua_setfield(L, -2, "__index"); /* { __index = _G }          (sp -= 1) */
	lua_setmetatable(L, -2); /* setmetatable({}, {__index = _G}) (sp -= 1) */

	magnet_setfenv_mainfn(L, 1);                              /* (sp -= 1) */

	/* pcall will destroy the func value, duplicate it */     /* (sp += 1) */
	lua_pushvalue(L, func_ndx);
	{
		int errfunc = push_traceback(L, 0);
		int ret = lua_pcall(L, 0, 1, errfunc);
		lua_remove(L, errfunc);

		/* reset environment */
		lua_pushglobaltable(L);                               /* (sp += 1) */
		magnet_setfenv_mainfn(L, 1);                          /* (sp -= 1) */

		if (0 != ret) {
			log_error(r->conf.errh, __FILE__, __LINE__,
			  "lua_pcall(): %s", lua_tostring(L, -1));
			lua_pop(L, 2); /* remove the error-msg and the lighty table at index 2 */

			force_assert(lua_gettop(L) == 1); /* only the function should be on the stack */

			if (p->conf.stage != -1) { /* skip for response-start */
				r->http_status = 500;
				r->handler_module = NULL;
			}

			return HANDLER_FINISHED;
		}
	}

	/* we should have the function, the lighty table and the return value on the stack */
	force_assert(lua_gettop(L) == 3);

	switch (lua_type(L, -1)) {
	case LUA_TNUMBER:
	case LUA_TNIL:
		lua_return_value = (int) luaL_optinteger(L, -1, -1);
		break;
	default:
		log_error(r->conf.errh, __FILE__, __LINE__,
		  "lua_pcall(): unexpected return type: %s", luaL_typename(L, -1));
		lua_return_value = -1;
		break;
	}

	lua_pop(L, 1); /* pop return value */

	magnet_copy_response_header(r, L, lighty_table_ndx);

	{
		handler_t result = HANDLER_GO_ON;

		if (lua_return_value >= 200) {
			r->http_status = lua_return_value;
			r->resp_body_finished = 1;

			if (0 == setjmp(exceptionjmp)) {
				magnet_attach_content(r, L, lighty_table_ndx);
				if (!chunkqueue_is_empty(&r->write_queue)) {
					r->handler_module = p->self;
				}
			} else {
				lua_settop(L, 2); /* remove all but function and lighty table */
				r->http_status = 500;
				r->handler_module = NULL;
				http_response_body_clear(r, 0);
			}

			result = HANDLER_FINISHED;
		} else if (lua_return_value >= 100) {
			/*(custom lua code should not return 101 Switching Protocols)*/
			r->http_status = lua_return_value;
			result = http_response_send_1xx(r)
			  ? HANDLER_GO_ON
			  : HANDLER_ERROR;
		} else if (MAGNET_RESTART_REQUEST == lua_return_value) {
			result = HANDLER_COMEBACK;
		}

		lua_pop(L, 1); /* pop the lighty table */
		force_assert(lua_gettop(L) == 1); /* only the function should remain on the stack */

		return result;
	}
}

static handler_t magnet_attract_array(request_st * const r, plugin_data * const p, int stage) {
	mod_magnet_patch_config(r, p);
	p->conf.stage = stage;

	const array *files;
	switch (stage) {
	  case  1: files = p->conf.url_raw; break;
	  case  0: files = p->conf.physical_path; break;
	  case -1: files = p->conf.response_start; break;
	  default: files = NULL; break;
	}

	/* no filename set */
	if (NULL == files || files->used == 0) return HANDLER_GO_ON;

	r->con->srv->request_env(r);

	/**
	 * execute all files and jump out on the first !HANDLER_GO_ON
	 */
	handler_t ret = HANDLER_GO_ON;
	for (uint32_t i = 0; i < files->used && ret == HANDLER_GO_ON; ++i) {
		data_string *ds = (data_string *)files->data[i];
		ret = magnet_attract(r, p, &ds->value);
	}

	if (r->error_handler_saved_status) {
		/* retrieve (possibly modified) REDIRECT_STATUS and store as number */
		int x;
		const buffer * const vb = http_header_env_get(r, CONST_STR_LEN("REDIRECT_STATUS"));
		if (vb && (x = http_header_str_to_code(vb->ptr)) != -1)
			r->error_handler_saved_status =
			  r->error_handler_saved_status > 0 ? (int)x : -(int)x;
	}

	return ret;
}

URIHANDLER_FUNC(mod_magnet_uri_handler) {
	return magnet_attract_array(r, p_d, 1);
}

URIHANDLER_FUNC(mod_magnet_physical) {
	return magnet_attract_array(r, p_d, 0);
}

URIHANDLER_FUNC(mod_magnet_response_start) {
	return magnet_attract_array(r, p_d, -1);
}


int mod_magnet_plugin_init(plugin *p);
int mod_magnet_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = "magnet";

	p->init        = mod_magnet_init;
	p->handle_uri_clean  = mod_magnet_uri_handler;
	p->handle_physical   = mod_magnet_physical;
	p->handle_response_start = mod_magnet_response_start;
	p->set_defaults  = mod_magnet_set_defaults;
	p->cleanup     = mod_magnet_free;

	return 0;
}
