#include "first.h"

#include "base.h"
#include "array.h"
#include "buffer.h"
#include "log.h"
#include "http_header.h"

#include "plugin.h"
#include "stat_cache.h"

#include "sys-time.h"
#include <stdlib.h>
#include <string.h>

/**
 * set HTTP headers Cache-Control and Expires
 */

typedef struct {
    const array *expire_url;
    const array *expire_mimetypes;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config defaults;
    plugin_config conf;
    time_t *toffsets;
    uint32_t tused;
} plugin_data;

INIT_FUNC(mod_expire_init) {
    return calloc(1, sizeof(plugin_data));
}

FREE_FUNC(mod_expire_free) {
    plugin_data * const p = p_d;
    free(p->toffsets);
}

static time_t mod_expire_get_offset(log_error_st *errh, const buffer *expire, time_t *offset) {
	char *ts;
	time_t type = -1;
	time_t retts = 0;

	/*
	 * parse
	 *
	 * '(access|now|modification) [plus] {<num> <type>}*'
	 *
	 * e.g. 'access 1 years'
	 */

	if (buffer_string_is_empty(expire)) {
		log_error(errh, __FILE__, __LINE__, "mod_expire empty string");
		return -1;
	}

	ts = expire->ptr;

	if (0 == strncmp(ts, "access ", 7)) {
		type  = 0;
		ts   += 7;
	} else if (0 == strncmp(ts, "now ", 4)) {
		type  = 0;
		ts   += 4;
	} else if (0 == strncmp(ts, "modification ", 13)) {
		type  = 1;
		ts   += 13;
	} else {
		/* invalid type-prefix */
		log_error(errh, __FILE__, __LINE__, "invalid <base>: %s", ts);
		return -1;
	}

	if (0 == strncmp(ts, "plus ", 5)) {
		/* skip the optional plus */
		ts   += 5;
	}

	/* the rest is just <number> (years|months|weeks|days|hours|minutes|seconds) */
	while (1) {
		char *space, *err;
		int num;

		if (NULL == (space = strchr(ts, ' '))) {
			log_error(errh, __FILE__, __LINE__,
			  "missing space after <num>: %s", ts);
			return -1;
		}

		num = strtol(ts, &err, 10);
		if (*err != ' ') {
			log_error(errh, __FILE__, __LINE__,
			  "missing <type> after <num>: %s", ts);
			return -1;
		}

		ts = space + 1;

		if (NULL != (space = strchr(ts, ' '))) {
			int slen;
			/* */

			slen = space - ts;

			if (slen == 5 &&
			    0 == strncmp(ts, "years", slen)) {
				num *= 60 * 60 * 24 * 30 * 12;
			} else if (slen == 6 &&
				   0 == strncmp(ts, "months", slen)) {
				num *= 60 * 60 * 24 * 30;
			} else if (slen == 5 &&
				   0 == strncmp(ts, "weeks", slen)) {
				num *= 60 * 60 * 24 * 7;
			} else if (slen == 4 &&
				   0 == strncmp(ts, "days", slen)) {
				num *= 60 * 60 * 24;
			} else if (slen == 5 &&
				   0 == strncmp(ts, "hours", slen)) {
				num *= 60 * 60;
			} else if (slen == 7 &&
				   0 == strncmp(ts, "minutes", slen)) {
				num *= 60;
			} else if (slen == 7 &&
				   0 == strncmp(ts, "seconds", slen)) {
				num *= 1;
			} else {
				log_error(errh, __FILE__, __LINE__, "unknown type: %s", ts);
				return -1;
			}

			retts += num;

			ts = space + 1;
		} else {
			if (0 == strcmp(ts, "years")) {
				num *= 60 * 60 * 24 * 30 * 12;
			} else if (0 == strcmp(ts, "months")) {
				num *= 60 * 60 * 24 * 30;
			} else if (0 == strcmp(ts, "weeks")) {
				num *= 60 * 60 * 24 * 7;
			} else if (0 == strcmp(ts, "days")) {
				num *= 60 * 60 * 24;
			} else if (0 == strcmp(ts, "hours")) {
				num *= 60 * 60;
			} else if (0 == strcmp(ts, "minutes")) {
				num *= 60;
			} else if (0 == strcmp(ts, "seconds")) {
				num *= 1;
			} else {
				log_error(errh, __FILE__, __LINE__, "unknown type: %s", ts);
				return -1;
			}

			retts += num;

			break;
		}
	}

	*offset = retts;

	return type;
}

static void mod_expire_merge_config_cpv(plugin_config * const pconf, const config_plugin_value_t * const cpv) {
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* expire.url */
        pconf->expire_url = cpv->v.a;
        break;
      case 1: /* expire.mimetypes */
        pconf->expire_mimetypes = cpv->v.a;
        break;
      default:/* should not happen */
        return;
    }
}

static void mod_expire_merge_config(plugin_config * const pconf, const config_plugin_value_t *cpv) {
    do {
        mod_expire_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}

static void mod_expire_patch_config(request_st * const r, plugin_data * const p) {
    p->conf = p->defaults; /* copy small struct instead of memcpy() */
    /*memcpy(&p->conf, &p->defaults, sizeof(plugin_config));*/
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_expire_merge_config(&p->conf, p->cvlist + p->cvlist[i].v.u2[0]);
    }
}

SETDEFAULTS_FUNC(mod_expire_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("expire.url"),
        T_CONFIG_ARRAY_KVSTRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("expire.mimetypes"),
        T_CONFIG_ARRAY_KVSTRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_expire"))
        return HANDLER_ERROR;

    /* process and validate config directives
     * (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1]; i < p->nconfig; ++i) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        for (; -1 != cpv->k_id; ++cpv) {
            const array *a = NULL;
            switch (cpv->k_id) {
              case 0: /* expire.url */
                a = cpv->v.a;
                break;
              case 1: /* expire.mimetypes */
                for (uint32_t k = 0; k < cpv->v.a->used; ++k) {
                    data_string *ds = (data_string *)cpv->v.a->data[k];
                    /*(omit trailing '*', if present, from prefix match)*/
                    /*(not usually a good idea to modify array keys
                     * since doing so might break array_get_element_klen()
                     * search; config should be consistent in using * or not)*/
                    size_t klen = buffer_string_length(&ds->key);
                    if (klen && ds->key.ptr[klen-1] == '*')
                        buffer_string_set_length(&ds->key, klen-1);
                }
                a = cpv->v.a;
                break;
              default:/* should not happen */
                break;
            }

            /* parse array values into structured data */
            if (NULL != a && a->used) {
                p->toffsets =
                  realloc(p->toffsets, sizeof(time_t) * (p->tused + a->used*2));
                time_t *toff = p->toffsets + p->tused;
                for (uint32_t k = 0; k < a->used; ++k, toff+=2, p->tused+=2) {
                    buffer *v = &((data_string *)a->data[k])->value;
                    *toff = mod_expire_get_offset(srv->errh, v, toff+1);
                    if (-1 == *toff) {
                        log_error(srv->errh, __FILE__, __LINE__,
                          "parsing %s failed: %s", cpk[cpv->k_id].k, v->ptr);
                        return HANDLER_ERROR;
                    }
                    /* overwrite v->used with offset int p->toffsets
                     * as v->ptr is not used by this module after config */
                    v->used = (uint32_t)p->tused;
                }
            }
        }
    }

    /* initialize p->defaults from global config context */
    if (p->nconfig > 0 && p->cvlist->v.u2[1]) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist->v.u2[0];
        if (-1 != cpv->k_id)
            mod_expire_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}

REQUEST_FUNC(mod_expire_handler) {
	plugin_data *p = p_d;
	const buffer *vb;
	const data_string *ds;

	/* Add caching headers only to http_status 200 OK or 206 Partial Content */
	if (r->http_status != 200 && r->http_status != 206) return HANDLER_GO_ON;
	/* Add caching headers only to GET or HEAD requests */
	if (!http_method_get_or_head(r->http_method)) return HANDLER_GO_ON;
	/* Add caching headers only if not already present */
	vb = http_header_response_get(r, HTTP_HEADER_CACHE_CONTROL, CONST_STR_LEN("Cache-Control"));
	if (NULL != vb) return HANDLER_GO_ON;

	mod_expire_patch_config(r, p);

	/* check expire.url */
	ds = p->conf.expire_url
	  ? (const data_string *)array_match_key_prefix(p->conf.expire_url, &r->uri.path)
	  : NULL;
	/* check expire.mimetypes (if no match with expire.url) */
	if (NULL == ds) {
		if (NULL == p->conf.expire_mimetypes) return HANDLER_GO_ON;
		vb = http_header_response_get(r, HTTP_HEADER_CONTENT_TYPE, CONST_STR_LEN("Content-Type"));
		ds = (NULL != vb)
		   ? (const data_string *)array_match_key_prefix(p->conf.expire_mimetypes, vb)
		   : (const data_string *)array_get_element_klen(p->conf.expire_mimetypes, CONST_STR_LEN(""));
		if (NULL == ds) return HANDLER_GO_ON;
	}

	const time_t * const off = p->toffsets + ds->value.used;
	const time_t cur_ts = log_epoch_secs;
	time_t expires = off[1];
	if (0 == off[0]) { /* access */
		expires += cur_ts;
	}
	else {             /* modification */
		const stat_cache_st * const st = stat_cache_path_stat(&r->physical.path);
		/* can't set modification-based expire if mtime is not available */
		if (NULL == st) return HANDLER_GO_ON;
		expires += st->st_mtime;
	}

			/* expires should be at least cur_ts */
			if (expires < cur_ts) expires = cur_ts;

			buffer * const tb = r->tmp_buf;
			struct tm tm;

			/* HTTP/1.0 */
			buffer_clear(tb);
			buffer_append_strftime(tb, "%a, %d %b %Y %H:%M:%S GMT", gmtime_r(&expires, &tm));
			http_header_response_set(r, HTTP_HEADER_EXPIRES,
			                         CONST_STR_LEN("Expires"),
			                         CONST_BUF_LEN(tb));

			/* HTTP/1.1 */
			buffer_copy_string_len(tb, CONST_STR_LEN("max-age="));
			buffer_append_int(tb, expires - cur_ts); /* as expires >= cur_ts the difference is >= 0 */
			http_header_response_set(r, HTTP_HEADER_CACHE_CONTROL, CONST_STR_LEN("Cache-Control"), CONST_BUF_LEN(tb));

	return HANDLER_GO_ON;
}


int mod_expire_plugin_init(plugin *p);
int mod_expire_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = "expire";

	p->init        = mod_expire_init;
	p->cleanup     = mod_expire_free;
	p->set_defaults= mod_expire_set_defaults;
	p->handle_response_start = mod_expire_handler;

	return 0;
}
