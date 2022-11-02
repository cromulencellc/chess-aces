#include "first.h"

#include "base.h"
#include "plugin.h"
#include "log.h"
#include "stat_cache.h"

#include <stdlib.h>
#include <string.h>

/**
 *
 * #
 * # define a pattern for the host url finding
 * # %% => % sign
 * # %0 => domain name + tld
 * # %1 => tld
 * # %2 => domain name without tld
 * # %3 => subdomain 1 name
 * # %4 => subdomain 2 name
 * # %_ => fqdn (without port info)
 * #
 * evhost.path-pattern = "/home/ckruse/dev/www/%3/htdocs/"
 *
 */

typedef struct {
    /* pieces for path creation */
    const buffer *path_pieces;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config defaults;
    plugin_config conf;

    buffer tmp_buf;
    array split_vals;
} plugin_data;

INIT_FUNC(mod_evhost_init) {
    return calloc(1, sizeof(plugin_data));
}

static void mod_evhost_free_path_pieces(const buffer *path_pieces) {
    buffer *b;
    *(const buffer **)&b = path_pieces;
    for (; path_pieces->ptr; ++path_pieces) free(path_pieces->ptr);
    free(b);
}

FREE_FUNC(mod_evhost_free) {
    plugin_data * const p = p_d;
    free(p->tmp_buf.ptr);
    array_free_data(&p->split_vals);
    if (NULL == p->cvlist) return;
    /* (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1], used = p->nconfig; i < used; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        for (; -1 != cpv->k_id; ++cpv) {
            if (cpv->vtype != T_CONFIG_LOCAL || NULL == cpv->v.v) continue;
            switch (cpv->k_id) {
              case 0: /* evhost.path-pattern */
                mod_evhost_free_path_pieces(cpv->v.v);
                break;
              default:
                break;
            }
        }
    }
}

__attribute_cold__
static buffer * mod_evhost_parse_pattern_err(buffer *bptr) {
    for (; bptr->ptr; ++bptr) free(bptr->ptr);
    return NULL;
}

static buffer * mod_evhost_parse_pattern(const char *ptr) {
	uint32_t used = 0;
	const uint32_t sz = 127;/* (sz+1 must match bptr[] num elts below) */
	const char *pos;
	buffer bptr[128]; /* (128 elements takes 2k on stack in 64-bit) */
	memset(bptr, 0, sizeof(bptr));

	for(pos=ptr;*ptr;ptr++) {
		if(*ptr == '%') {
			size_t len;
			if (used >= sz-1) /* (should not happen) */
				return mod_evhost_parse_pattern_err(bptr);

			/* "%%" "%_" "%x" "%{x.y}" where x and y are *single digit* 0 - 9 */
			if (ptr[1] == '%' || ptr[1] == '_' || light_isdigit(ptr[1])) {
				len = 2;
			} else if (ptr[1] == '{') {
				if (!light_isdigit(ptr[2]))
					return mod_evhost_parse_pattern_err(bptr);
				if (ptr[3] == '.') {
					if (!light_isdigit(ptr[4]))
						return mod_evhost_parse_pattern_err(bptr);
					if (ptr[5] != '}')
						return mod_evhost_parse_pattern_err(bptr);
					len = 6;
				} else if (ptr[3] == '}') {
					len = 4;
				} else {
					return mod_evhost_parse_pattern_err(bptr);
				}
			} else {
				return mod_evhost_parse_pattern_err(bptr);
			}

			buffer_copy_string_len(bptr+used, pos, ptr-pos);
			pos = ptr + len;

			buffer_copy_string_len(bptr+used+1, ptr, len);
			ptr += len - 1; /*(ptr++ in for() loop)*/

			used += 2;
		}
	}

	if(*pos != '\0') {
		if (used >= sz) /* (should not happen) */
			return mod_evhost_parse_pattern_err(bptr);
		buffer_copy_string_len(bptr+used, pos, ptr-pos);
		++used;
	}

	buffer * const path_pieces =
	  malloc(sizeof(bptr) + ((used+1) * sizeof(buffer)));
	force_assert(path_pieces);
	return memcpy(path_pieces, bptr, (used+1) * sizeof(buffer));
}

static void mod_evhost_merge_config_cpv(plugin_config * const pconf, const config_plugin_value_t * const cpv) {
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* evhost.path-pattern */
        if (cpv->vtype == T_CONFIG_LOCAL)
            pconf->path_pieces = cpv->v.v;
        break;
      default:/* should not happen */
        return;
    }
}

static void mod_evhost_merge_config(plugin_config * const pconf, const config_plugin_value_t *cpv) {
    do {
        mod_evhost_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}

static void mod_evhost_patch_config(request_st * const r, plugin_data * const p) {
    p->conf = p->defaults; /* copy small struct instead of memcpy() */
    /*memcpy(&p->conf, &p->defaults, sizeof(plugin_config));*/
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_evhost_merge_config(&p->conf, p->cvlist + p->cvlist[i].v.u2[0]);
    }
}

SETDEFAULTS_FUNC(mod_evhost_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("evhost.path-pattern"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_evhost"))
        return HANDLER_ERROR;

    /* process and validate config directives
     * (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1]; i < p->nconfig; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        for (; -1 != cpv->k_id; ++cpv) {
            switch (cpv->k_id) {
              case 0: /* evhost.path-pattern */
                if (!buffer_string_is_empty(cpv->v.b)) {
                    const char * const ptr = cpv->v.b->ptr;
                    cpv->v.v = mod_evhost_parse_pattern(ptr);
                    if (NULL == cpv->v.v) {
                        log_error(srv->errh, __FILE__, __LINE__,
                          "invalid evhost.path-pattern: %s", ptr);
                        return HANDLER_ERROR;
                    }
                    cpv->vtype = T_CONFIG_LOCAL;
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
            mod_evhost_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}

/**
 * assign the different parts of the domain to array-indezes (sub2.sub1.domain.tld)
 * - %0 - domain.tld
 * - %1 - tld
 * - %2 - domain
 * - %3 - sub1
 * - ...
 */

static void mod_evhost_parse_host(buffer *key, array *host, buffer *authority) {
	char *ptr = authority->ptr + buffer_string_length(authority);
	char *colon = ptr; /* needed to filter out the colon (if exists) */
	int first = 1;
	int i;

	/*if (ptr == authority->ptr) return;*//*(no authority checked earlier)*/

	if (*authority->ptr == '[') { /* authority is IPv6 literal address */
                colon = ptr;
                if (ptr[-1] != ']') {
			do { --ptr; } while (ptr > authority->ptr && ptr[-1] != ']');
			if (*ptr != ':') return; /*(should not happen for valid authority)*/
			colon = ptr;
		}
		ptr = authority->ptr;
		array_set_key_value(host,CONST_STR_LEN("%0"),ptr,colon-ptr);
		return;
	}

	/* first, find the domain + tld */
	for(; ptr > authority->ptr; --ptr) {
		if(*ptr == '.') {
			if(first) first = 0;
			else      break;
		} else if(*ptr == ':') {
			colon = ptr;
			first = 1;
		}
	}

	/* if we stopped at a dot, skip the dot */
	if (*ptr == '.') ptr++;
	array_set_key_value(host, CONST_STR_LEN("%0"), ptr, colon-ptr);

	/* if the : is not the start of the authority, go on parsing the hostname */

	if (colon != authority->ptr) {
		for(ptr = colon - 1, i = 1; ptr > authority->ptr; --ptr) {
			if(*ptr == '.') {
				if (ptr != colon - 1) {
					/* is something between the dots */
					buffer_copy_string_len(key, CONST_STR_LEN("%"));
					buffer_append_int(key, i++);
					array_set_key_value(host, CONST_BUF_LEN(key), ptr+1, colon-ptr-1);
				}
				colon = ptr;
			}
		}

		/* if the . is not the first character of the hostname */
		if (colon != ptr) {
			buffer_copy_string_len(key, CONST_STR_LEN("%"));
			buffer_append_int(key, i /* ++ */);
			array_set_key_value(host, CONST_BUF_LEN(key), ptr, colon-ptr);
		}
	}
}

static void mod_evhost_build_doc_root_path(buffer *b, array *parsed_host, buffer *authority, const buffer *path_pieces) {
	array_reset_data_strings(parsed_host);
	mod_evhost_parse_host(b, parsed_host, authority);
	buffer_clear(b);

	for (const char *ptr; (ptr = path_pieces->ptr); ++path_pieces) {
		if (*ptr == '%') {
			const data_string *ds;

			if (*(ptr+1) == '%') {
				/* %% */
				buffer_append_string_len(b, CONST_STR_LEN("%"));
			} else if (*(ptr+1) == '_' ) {
				/* %_ == full hostname */
				char *colon = strchr(authority->ptr, ':');

				if(colon == NULL) {
					buffer_append_string_buffer(b, authority); /* adds fqdn */
				} else {
					/* strip the port out of the authority-part of the URI scheme */
					buffer_append_string_len(b, authority->ptr, colon - authority->ptr); /* adds fqdn */
				}
			} else if (ptr[1] == '{' ) {
				char s[3] = "% ";
				s[1] = ptr[2]; /*(assumes single digit before '.', and, optionally, '.' and single digit after '.')*/
				if (NULL != (ds = (data_string *)array_get_element_klen(parsed_host, s, 2))) {
					if (ptr[3] != '.' || ptr[4] == '0') {
						buffer_append_string_buffer(b, &ds->value);
					} else {
						if ((size_t)(ptr[4]-'0') <= buffer_string_length(&ds->value)) {
							buffer_append_string_len(b, ds->value.ptr+(ptr[4]-'0')-1, 1);
						}
					}
				} else {
					/* unhandled %-sequence */
				}
			} else if (NULL != (ds = (data_string *)array_get_element_klen(parsed_host, CONST_BUF_LEN(path_pieces)))) {
				buffer_append_string_buffer(b, &ds->value);
			} else {
				/* unhandled %-sequence */
			}
		} else {
			buffer_append_string_buffer(b, path_pieces);
		}
	}

	buffer_append_slash(b);
}

static handler_t mod_evhost_uri_handler(request_st * const r, void *p_d) {
	plugin_data *p = p_d;

	/* not authority set */
	if (buffer_string_is_empty(&r->uri.authority)) return HANDLER_GO_ON;

	mod_evhost_patch_config(r, p);

	/* missing even default(global) conf */
	if (NULL == p->conf.path_pieces) return HANDLER_GO_ON;

	buffer * const b = &p->tmp_buf;
	mod_evhost_build_doc_root_path(b, &p->split_vals, &r->uri.authority, p->conf.path_pieces);

	if (!stat_cache_path_isdir(b)) {
		log_perror(r->conf.errh, __FILE__, __LINE__, "%s", b->ptr);
	} else {
		buffer_copy_buffer(&r->physical.doc_root, b);
	}

	return HANDLER_GO_ON;
}

int mod_evhost_plugin_init(plugin *p);
int mod_evhost_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = "evhost";
	p->init                    = mod_evhost_init;
	p->set_defaults            = mod_evhost_set_defaults;
	p->handle_docroot          = mod_evhost_uri_handler;
	p->cleanup                 = mod_evhost_free;

	return 0;
}
