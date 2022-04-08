#include "first.h"

#include <stdlib.h>
#include <string.h>

#include "plugin.h"

#include "base.h"
#include "log.h"
#include "buffer.h"
#include "array.h"

/**
 * this is a skeleton for a lighttpd plugin
 *
 * just replaces every occurrence of 'skeleton' by your plugin name
 *
 * e.g. in vim:
 *
 *   :%s/skeleton/myhandler/
 *
 */


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

    if (NULL != r->handler_module) return HANDLER_GO_ON;
    if (buffer_string_is_empty(&r->uri.path)) return HANDLER_GO_ON;

    /* get module config for request */
    mod_skeleton_patch_config(r, p);

    if (NULL == p->conf.match
        || NULL == array_match_value_suffix(p->conf.match, &r->uri.path)) {
        return HANDLER_GO_ON;
    }

    /* module participates in request; business logic here */

    r->http_status = 403; /* example: reject request with 403 Forbidden */
    return HANDLER_FINISHED;
}

/* this function is called at dlopen() time and inits the callbacks */
int mod_skeleton_plugin_init(plugin *p);
int mod_skeleton_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = "skeleton";
	p->init        = mod_skeleton_init;
	p->set_defaults= mod_skeleton_set_defaults;

	p->handle_uri_clean = mod_skeleton_uri_handler;

	return 0;
}
