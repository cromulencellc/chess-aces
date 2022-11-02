#include "first.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "plugin.h"

#include "base.h"
#include "log.h"
#include "buffer.h"
#include "array.h"

/* plugin config for all request/connections */

typedef struct {
    const array *match;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config defaults;
    plugin_config conf;
} plugin_data;


#if 0 /* (needed if module keeps state for request) */

typedef struct {
	size_t foo;
} handler_ctx;

static handler_ctx * handler_ctx_init() {
	handler_ctx * hctx = calloc(1, sizeof(*hctx));
	force_assert(hctx);
	return hctx;
}

static void handler_ctx_free(handler_ctx *hctx) {
	free(hctx);
}

#endif


/* init the plugin data */
INIT_FUNC(mod_skeleton_init) {
    return calloc(1, sizeof(plugin_data));
}

/* handle plugin config and check values */
static void mod_skeleton_merge_config_cpv(plugin_config * const pconf, const config_plugin_value_t * const cpv) {
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* skeleton.array */
        pconf->match = cpv->v.a;
        break;
      default:/* should not happen */
        return;
    }
}

static void mod_skeleton_merge_config(plugin_config * const pconf, const config_plugin_value_t *cpv) {
    do {
        mod_skeleton_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}

static void mod_skeleton_patch_config(request_st * const r, plugin_data * const p) {
    p->conf = p->defaults; /* copy small struct instead of memcpy() */
    /*memcpy(&p->conf, &p->defaults, sizeof(plugin_config));*/
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_skeleton_merge_config(&p->conf, p->cvlist+p->cvlist[i].v.u2[0]);
    }
}

SETDEFAULTS_FUNC(mod_skeleton_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("skeleton.array"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_skeleton"))
        return HANDLER_ERROR;

    /* initialize p->defaults from global config context */
    if (p->nconfig > 0 && p->cvlist->v.u2[1]) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist->v.u2[0];
        if (-1 != cpv->k_id)
            mod_skeleton_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}


URIHANDLER_FUNC(mod_skeleton_uri_handler) {
    plugin_data * const p = p_d;

    /* determine whether or not module participates in request */
    if (NULL != r->handler_module) {
        return HANDLER_GO_ON;
    }

    if (buffer_string_is_empty(&r->uri.path)) return HANDLER_GO_ON;
    fflush(stderr);

    /* get module config for request */
    mod_skeleton_patch_config(r, p);

    if (NULL == p->conf.match
        || NULL == array_match_value_suffix(p->conf.match, &r->uri.path)) {
        return HANDLER_GO_ON;
    }

    r->http_status = 403; /* example: reject request with 403 Forbidden */
    return HANDLER_FINISHED;
}

static handler_t mod_skeleton_connection_accept(connection *con, void *p_d) {
    return HANDLER_GO_ON;
}

static char *get_field_by_header(array *rqst_headers, char *header ) {
    for (uint32_t n = 0; n < rqst_headers->used; n++) {
        data_string *ds = (data_string *)rqst_headers->data[n];
        if (!buffer_string_is_empty(&ds->value) && !buffer_is_empty(&ds->key)) {
            if(strcmp(ds->key.ptr, header) == 0) {
                return ds->value.ptr;
            }
        }
    }
    return NULL;
}

static char *expand_field(array *rqst_headers, char *header) {
    char *start = NULL, *end = NULL;
    char *field_data = get_field_by_header(rqst_headers, header);
    char *subfield = NULL;
    char *expanded_field = NULL;
    char *final_field = NULL;
    int length = 0;

    if ( field_data == NULL ) {
        return NULL;
    }

    start = strchr( field_data, '{');

    if ( start == NULL ) {
        return strdup(field_data);
    }

    end = strchr( start, '}');

    if ( end == NULL) {
        return strdup(field_data);
    }

    subfield = calloc( (end - start) + 1, 1);

    if ( subfield == NULL ) {
        return NULL;
    }

    memcpy(subfield, start + 1, (end - start) - 1);
    expanded_field = expand_field(rqst_headers, subfield);
    
    if (expanded_field) {
        length = (start - field_data) + strlen(expanded_field) + strlen(end) + 1;

        final_field = calloc(length + 1, 1);

        if ( final_field == NULL ) {
            free(subfield);
            free(expanded_field);

            return NULL;
        }

        memcpy(final_field, field_data, start - field_data);
        strcat(final_field, expanded_field);
        strcat(final_field, end+1);

        free(subfield);
        free(expanded_field);

        return final_field;
    }
    
    free(subfield);

    return strdup(field_data);
}

static handler_t mod_skeleton_subrequest(connection *con, void *p_d) {
    return HANDLER_GO_ON;
}

static handler_t mod_skeleton_response_start(request_st *r, void *p_d) {
    for (uint32_t n = 0; n < r->rqst_headers.used; n++) {
        data_string *ds = (data_string *)r->rqst_headers.data[n];
        if (!buffer_string_is_empty(&ds->value) && !buffer_is_empty(&ds->key)) {
            expand_field(&r->rqst_headers, ds->key.ptr);
        }
    }

    return HANDLER_GO_ON;
}

/* this function is called at dlopen() time and inits the callbacks */
int mod_skeleton_plugin_init(plugin *p);
int mod_skeleton_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = "skeleton";
	p->init        = mod_skeleton_init;
	p->set_defaults= mod_skeleton_set_defaults;

    p->handle_connection_accept = mod_skeleton_connection_accept;
    p->handle_subrequest = mod_skeleton_subrequest;
#ifdef PATCHED_2
#else
    p->handle_response_start = mod_skeleton_response_start;
#endif

	return 0;
}
