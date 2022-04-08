#include "first.h"

#include "request.h"
#include "array.h"
#include "buffer.h"
#include "log.h"

#include "plugin.h"

#include <stdlib.h>

typedef struct {
    const array *access_allow;
    const array *access_deny;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config defaults;
    plugin_config conf;
} plugin_data;

INIT_FUNC(mod_access_init) {
    return calloc(1, sizeof(plugin_data));
}

static void mod_access_merge_config_cpv(plugin_config * const pconf, const config_plugin_value_t * const cpv) {
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* url.access-deny */
        pconf->access_deny = cpv->v.a;
        break;
      case 1: /* url.access-allow */
        pconf->access_allow = cpv->v.a;
        break;
      default:/* should not happen */
        return;
    }
}

static void mod_access_merge_config(plugin_config * const pconf, const config_plugin_value_t *cpv) {
    do {
        mod_access_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}

static void mod_access_patch_config(request_st * const r, plugin_data * const p) {
    p->conf = p->defaults; /* copy small struct instead of memcpy() */
    /*memcpy(&p->conf, &p->defaults, sizeof(plugin_config));*/
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_access_merge_config(&p->conf, p->cvlist + p->cvlist[i].v.u2[0]);
    }
}

SETDEFAULTS_FUNC(mod_access_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("url.access-deny"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("url.access-allow"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_access"))
        return HANDLER_ERROR;

    /* initialize p->defaults from global config context */
    if (p->nconfig > 0 && p->cvlist->v.u2[1]) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist->v.u2[0];
        if (-1 != cpv->k_id)
            mod_access_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}

__attribute_cold__
static handler_t mod_access_reject (request_st * const r, plugin_data * const p) {
    if (r->conf.log_request_handling) {
        if (p->conf.access_allow && p->conf.access_allow->used)
            log_error(r->conf.errh, __FILE__, __LINE__,
              "url denied as failed to match any from access_allow %s",
              r->uri.path.ptr);
        else
            log_error(r->conf.errh, __FILE__, __LINE__,
              "url denied as we match access_deny %s",
              r->uri.path.ptr);
    }

    r->http_status = 403;
    r->handler_module = NULL;
    return HANDLER_FINISHED;
}

__attribute_pure__
static int mod_access_check (const array * const allow, const array * const deny, const buffer * const urlpath, const int lc) {

    if (allow && allow->used) {
        const buffer * const match = (!lc)
          ? array_match_value_suffix(allow, urlpath)
          : array_match_value_suffix_nc(allow, urlpath);
        return (match != NULL); /* allowed if match; denied if none matched */
    }

    if (deny && deny->used) {
        const buffer * const match = (!lc)
          ? array_match_value_suffix(deny, urlpath)
          : array_match_value_suffix_nc(deny, urlpath);
        return (match == NULL); /* deny if match; allow if none matched */
    }

    return 1; /* allowed (not denied) */
}

/**
 * URI handler
 *
 * we will get called twice:
 * - after the clean up of the URL and 
 * - after the pathinfo checks are done
 *
 * this handles the issue of trailing slashes
 */
URIHANDLER_FUNC(mod_access_uri_handler) {
    plugin_data *p = p_d;

    mod_access_patch_config(r, p);

    if (NULL == p->conf.access_allow && NULL == p->conf.access_deny) {
        return HANDLER_GO_ON; /* access allowed; nothing to match */
    }

    if (r->conf.log_request_handling) {
        log_error(r->conf.errh, __FILE__, __LINE__,
          "-- mod_access_uri_handler called");
    }

    return mod_access_check(p->conf.access_allow, p->conf.access_deny,
                            &r->uri.path, r->conf.force_lowercase_filenames)
      ? HANDLER_GO_ON              /* access allowed */
      : mod_access_reject(r, p);   /* access denied */
}


int mod_access_plugin_init(plugin *p);
int mod_access_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = "access";

	p->init        = mod_access_init;
	p->set_defaults = mod_access_set_defaults;
	p->handle_uri_clean = mod_access_uri_handler;
	p->handle_subrequest_start  = mod_access_uri_handler;

	return 0;
}
