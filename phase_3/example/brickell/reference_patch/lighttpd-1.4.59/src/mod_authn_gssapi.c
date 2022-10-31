#include "first.h"

/* mod_authn_gssapi
 *
 * - provides http_auth_backend_t "gssapi" for HTTP auth Basic realm="Kerberos"
 * - provides http_auth_scheme_t "Negotiate"
 * - (does not provide http_auth_backend_t for HTTP auth Digest)
 *
 * Note: Credentials cache (KRB5CCNAME) is exported into CGI and SSI environment
 *       as well as passed to FastCGI and SCGI (useful if on same machine
 *       and running under same user account with access to KRB5CCNAME file).
 *       Credentials are clean up at the end of each request.
 *
 * LIMITATIONS:
 * - no rate limiting of auth requests, so remote attacker can send many auth
 *   requests very quickly if attempting brute force password cracking attack
 *
 * FUTURE POTENTIAL PERFORMANCE ENHANCEMENTS:
 * - Kerberos auth is synchronous and blocks waiting for response
 *   TODO: attempt async?
 */

#include "plugin.h"

#include <krb5.h>
#include <gssapi.h>
#include <gssapi/gssapi_krb5.h>

#include "http_auth.h"
#include "http_header.h"
#include "base.h"
#include "log.h"
#include "base64.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    const buffer *auth_gssapi_keytab;
    const buffer *auth_gssapi_principal;
    unsigned int auth_gssapi_store_creds;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config defaults;
    plugin_config conf;
} plugin_data;

static handler_t mod_authn_gssapi_check(request_st *r, void *p_d, const struct http_auth_require_t *require, const struct http_auth_backend_t *backend);
static handler_t mod_authn_gssapi_basic(request_st *r, void *p_d, const http_auth_require_t *require, const buffer *username, const char *pw);

INIT_FUNC(mod_authn_gssapi_init) {
    static http_auth_scheme_t http_auth_scheme_gssapi =
      { "gssapi", mod_authn_gssapi_check, NULL };
    static http_auth_backend_t http_auth_backend_gssapi =
      { "gssapi", mod_authn_gssapi_basic, NULL, NULL };
    plugin_data *p = calloc(1, sizeof(*p));

    /* register http_auth_scheme_gssapi and http_auth_backend_gssapi */
    http_auth_scheme_gssapi.p_d = p;
    http_auth_scheme_set(&http_auth_scheme_gssapi);
    http_auth_backend_gssapi.p_d = p;
    http_auth_backend_set(&http_auth_backend_gssapi);

    return p;
}

static void mod_authn_gssapi_merge_config_cpv(plugin_config * const pconf, const config_plugin_value_t * const cpv) {
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* auth.backend.gssapi.keytab */
        pconf->auth_gssapi_keytab = cpv->v.b;
        break;
      case 1: /* auth.backend.gssapi.principal */
        pconf->auth_gssapi_principal = cpv->v.b;
        break;
      case 2: /* auth.backend.gssapi.store-creds */
        pconf->auth_gssapi_store_creds = cpv->v.u;
        break;
      default:/* should not happen */
        return;
    }
}

static void mod_authn_gssapi_merge_config(plugin_config * const pconf, const config_plugin_value_t *cpv) {
    do {
        mod_authn_gssapi_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}

static void mod_authn_gssapi_patch_config(request_st * const r, plugin_data * const p) {
    p->conf = p->defaults; /* copy small struct instead of memcpy() */
    /*memcpy(&p->conf, &p->defaults, sizeof(plugin_config));*/
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_authn_gssapi_merge_config(&p->conf,
                                        p->cvlist + p->cvlist[i].v.u2[0]);
    }
}

SETDEFAULTS_FUNC(mod_authn_gssapi_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("auth.backend.gssapi.keytab"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("auth.backend.gssapi.principal"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("auth.backend.gssapi.store-creds"),
        T_CONFIG_BOOL,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_authn_gssapi"))
        return HANDLER_ERROR;

    /* default enabled for backwards compatibility; disable in future */
    p->defaults.auth_gssapi_store_creds = 1;

    /* initialize p->defaults from global config context */
    if (p->nconfig > 0 && p->cvlist->v.u2[1]) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist->v.u2[0];
        if (-1 != cpv->k_id)
            mod_authn_gssapi_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}

__attribute_cold__
static handler_t mod_authn_gssapi_send_400_bad_request (request_st * const r)
{
    r->http_status = 400;
    r->handler_module = NULL;
    return HANDLER_FINISHED;
}

__attribute_cold__
static void mod_authn_gssapi_log_gss_error(log_error_st *errh, const char *file, unsigned int line, const char *func, const char *extra, OM_uint32 err_maj, OM_uint32 err_min)
{
    buffer * const msg = buffer_init_string(func);
    OM_uint32 maj_stat, min_stat;
    OM_uint32 msg_ctx = 0;
    gss_buffer_desc status_string;

    buffer_append_string_len(msg, CONST_STR_LEN("("));
    if (extra) buffer_append_string(msg, extra);
    buffer_append_string_len(msg, CONST_STR_LEN("):"));

    do {
        maj_stat = gss_display_status(&min_stat, err_maj, GSS_C_GSS_CODE,
                                      GSS_C_NO_OID, &msg_ctx, &status_string);
        if (GSS_ERROR(maj_stat))
            break;

        buffer_append_string(msg, status_string.value);
        gss_release_buffer(&min_stat, &status_string);

        maj_stat = gss_display_status(&min_stat, err_min, GSS_C_MECH_CODE,
                                      GSS_C_NULL_OID, &msg_ctx, &status_string);
        if (!GSS_ERROR(maj_stat)) {
            buffer_append_string_len(msg, CONST_STR_LEN(" ("));
            buffer_append_string(msg, status_string.value);
            buffer_append_string_len(msg, CONST_STR_LEN(")"));
            gss_release_buffer(&min_stat, &status_string);
        }
    } while (!GSS_ERROR(maj_stat) && msg_ctx != 0);

    log_error(errh, file, line, "%s", msg->ptr);
    buffer_free(msg);
}

__attribute_cold__
static void mod_authn_gssapi_log_krb5_error(log_error_st *errh, const char *file, unsigned int line, const char *func, const char *extra, krb5_context context, int code)
{
    UNUSED(context);
    /*(extra might be NULL)*/
    log_error(errh, file, line,
      "%s (%s): %s", func, extra ? extra : "", error_message(code));
}

static int mod_authn_gssapi_create_krb5_ccache(request_st * const r, plugin_data * const p, krb5_context kcontext, krb5_principal princ, krb5_ccache * const ccache)
{
    buffer * const kccname = buffer_init_string("FILE:/tmp/krb5cc_gssapi_XXXXXX");
    char * const ccname    = kccname->ptr + sizeof("FILE:")-1;
    const size_t ccnamelen = buffer_string_length(kccname)-(sizeof("FILE:")-1);
    /*(future: might consider using server.upload-dirs instead of /tmp)*/
  #ifdef __COVERITY__
    /* POSIX-2008 requires mkstemp create file with 0600 perms */
    umask(0600);
  #endif
    /* coverity[secure_temp : FALSE] */
    int fd = mkstemp(ccname);
    if (fd < 0) {
        log_perror(r->conf.errh, __FILE__, __LINE__, "mkstemp(): %s", ccname);
        buffer_free(kccname);
        return -1;
    }
    close(fd);

    do {
        krb5_error_code problem;

        problem = krb5_cc_resolve(kcontext, kccname->ptr, ccache);
        if (problem) {
            mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_cc_resolve", NULL, kcontext, problem);
            break;
        }

        problem = krb5_cc_initialize(kcontext, *ccache, princ);
        if (problem) {
            mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_cc_initialize", kccname->ptr, kcontext, problem);
            break;
        }

        r->plugin_ctx[p->id] = kccname;

        http_header_env_set(r, CONST_STR_LEN("KRB5CCNAME"), ccname, ccnamelen);
        http_header_request_set(r, HTTP_HEADER_OTHER, CONST_STR_LEN("X-Forwarded-Keytab"), ccname, ccnamelen);

        return 0;

    } while (0);

    if (*ccache) {
        krb5_cc_destroy(kcontext, *ccache);
        *ccache = NULL;
    }
    unlink(ccname);
    buffer_free(kccname);

    return -1;
}

/*
 * HTTP auth Negotiate
 */

__attribute_cold__
static handler_t mod_authn_gssapi_send_500_server_error (request_st * const r)
{
    r->http_status = 500;
    r->handler_module = NULL;
    return HANDLER_FINISHED;
}

static handler_t mod_authn_gssapi_send_401_unauthorized_negotiate (request_st * const r)
{
    r->http_status = 401;
    r->handler_module = NULL;
    http_header_response_set(r, HTTP_HEADER_WWW_AUTHENTICATE,
                             CONST_STR_LEN("WWW-Authenticate"),
                             CONST_STR_LEN("Negotiate"));
    return HANDLER_FINISHED;
}

static int mod_authn_gssapi_store_gss_creds(request_st * const r, plugin_data * const p, char * const princ_name, gss_cred_id_t delegated_cred)
{
    OM_uint32 maj_stat, min_stat;
    krb5_principal princ = NULL;
    krb5_ccache ccache   = NULL;
    krb5_error_code problem;
    krb5_context context;

    problem = krb5_init_context(&context);
    if (problem) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_init_context", NULL, context, problem);
        return 0;
    }

    problem = krb5_parse_name(context, princ_name, &princ);
    if (problem) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_parse_name", NULL, context, problem);
        goto end;
    }

    if (mod_authn_gssapi_create_krb5_ccache(r, p, context, princ, &ccache))
        goto end;

    maj_stat = gss_krb5_copy_ccache(&min_stat, delegated_cred, ccache);
    if (GSS_ERROR(maj_stat)) {
        mod_authn_gssapi_log_gss_error(r->conf.errh, __FILE__, __LINE__, "gss_krb5_copy_ccache", princ_name, maj_stat, min_stat);
        goto end;
    }

    krb5_cc_close(context, ccache);
    krb5_free_principal(context, princ);
    krb5_free_context(context);
    return 1;

    end:
        if (princ)
            krb5_free_principal(context, princ);
        if (ccache)
            krb5_cc_destroy(context, ccache);
        krb5_free_context(context);

        return 0;
}

static handler_t mod_authn_gssapi_check_spnego(request_st * const r, plugin_data * const p, const http_auth_require_t * const require, const char * const realm_str)
{
    OM_uint32 st_major, st_minor, acc_flags;
    gss_buffer_desc token_s   = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc token_in  = GSS_C_EMPTY_BUFFER;
    gss_buffer_desc token_out = GSS_C_EMPTY_BUFFER;
    gss_cred_id_t server_cred = GSS_C_NO_CREDENTIAL;
    gss_cred_id_t client_cred = GSS_C_NO_CREDENTIAL;
    gss_ctx_id_t context      = GSS_C_NO_CONTEXT;
    gss_name_t server_name    = GSS_C_NO_NAME;
    gss_name_t client_name    = GSS_C_NO_NAME;

    buffer *sprinc;
    handler_t rc = HANDLER_UNSET;

    buffer *t_in = buffer_init();
    if (!buffer_append_base64_decode(t_in, realm_str, strlen(realm_str), BASE64_STANDARD)) {
        log_error(r->conf.errh, __FILE__, __LINE__, "decoding GSSAPI authentication header failed %s", realm_str);
        buffer_free(t_in);
        return mod_authn_gssapi_send_400_bad_request(r);
    }

    mod_authn_gssapi_patch_config(r, p);

    {
        /* ??? Should code = krb5_kt_resolve(kcontext, p->conf.auth_gssapi_keytab->ptr, &keytab);
         *     be used, instead of putenv() of KRB5_KTNAME=...?  See mod_authn_gssapi_basic() */
        /* ??? Should KRB5_KTNAME go into r->env instead ??? */
        /* ??? Should KRB5_KTNAME be added to mod_authn_gssapi_basic(), too? */
        buffer ktname;
        memset(&ktname, 0, sizeof(ktname));
        buffer_copy_string(&ktname, "KRB5_KTNAME=");
        buffer_append_string_buffer(&ktname, p->conf.auth_gssapi_keytab);
        putenv(ktname.ptr);
        /* ktname.ptr becomes part of the environment, do not free */
    }

    sprinc = buffer_init_buffer(p->conf.auth_gssapi_principal);
    if (strchr(sprinc->ptr, '/') == NULL) {
        /*(copy HTTP Host, omitting port if port is present)*/
        /* ??? Should r->server_name be used if http_host not present?
         * ??? What if r->server_name is not set?
         * ??? Will this work below if IPv6 provided in Host?  probably not */
        if (!buffer_is_empty(r->http_host)) {
            buffer_append_string_len(sprinc, CONST_STR_LEN("/"));
            buffer_append_string_len(sprinc, r->http_host->ptr, strcspn(r->http_host->ptr, ":"));
        }
    }
    if (strchr(sprinc->ptr, '@') == NULL) {
        buffer_append_string_len(sprinc, CONST_STR_LEN("@"));
        buffer_append_string_buffer(sprinc, require->realm);
    }
    /*#define GSS_C_NT_USER_NAME gss_nt_user_name*/
    /*#define GSS_C_NT_HOSTBASED_SERVICE gss_nt_service_name*/
    #define GSS_KRB5_NT_PRINCIPAL_NAME gss_nt_krb5_name

    token_s.value = sprinc->ptr;
    token_s.length = buffer_string_length(sprinc);
    st_major = gss_import_name(&st_minor, &token_s, (gss_OID) GSS_KRB5_NT_PRINCIPAL_NAME, &server_name);
    if (GSS_ERROR(st_major)) {
        mod_authn_gssapi_log_gss_error(r->conf.errh, __FILE__, __LINE__, "gss_import_name", NULL, st_major, st_minor);
        goto end;
    }

    memset(&token_s, 0, sizeof(token_s));
    st_major = gss_display_name(&st_minor, server_name, &token_s, NULL);
    if (GSS_ERROR(st_major)) {
        mod_authn_gssapi_log_gss_error(r->conf.errh, __FILE__, __LINE__, "gss_display_name", NULL, st_major, st_minor);
        goto end;
    }

    /* acquire server's own credentials */
    st_major = gss_acquire_cred(&st_minor, server_name, GSS_C_INDEFINITE, GSS_C_NO_OID_SET, GSS_C_ACCEPT, &server_cred, NULL, NULL);
    if (GSS_ERROR(st_major)) {
        mod_authn_gssapi_log_gss_error(r->conf.errh, __FILE__, __LINE__, "gss_acquire_cred", sprinc->ptr, st_major, st_minor);
        goto end;
    }

    /* accept the user's context */
    token_in.length = buffer_string_length(t_in);
    token_in.value = t_in->ptr;
    st_major = gss_accept_sec_context(&st_minor, &context, server_cred, &token_in, GSS_C_NO_CHANNEL_BINDINGS,
                                      &client_name, NULL, &token_out, &acc_flags, NULL, &client_cred);
    if (GSS_ERROR(st_major)) {
        mod_authn_gssapi_log_gss_error(r->conf.errh, __FILE__, __LINE__, "gss_accept_sec_context", NULL, st_major, st_minor);
        goto end;
    }

    /* fetch the username */
    st_major = gss_display_name(&st_minor, client_name, &token_out, NULL);
    if (GSS_ERROR(st_major)) {
        mod_authn_gssapi_log_gss_error(r->conf.errh, __FILE__, __LINE__, "gss_display_name", NULL, st_major, st_minor);
        goto end;
    }

    if (!(acc_flags & GSS_C_CONF_FLAG)) {
        log_error(r->conf.errh, __FILE__, __LINE__, "No confidentiality for user: %s", (char *)token_out.value);
        goto end;
    }

    /* check the allow-rules */
    if (!http_auth_match_rules(require, token_out.value, NULL, NULL)) {
        goto end;
    }

    if (p->conf.auth_gssapi_store_creds) {
        if (!(acc_flags & GSS_C_DELEG_FLAG)) {
            log_error(r->conf.errh, __FILE__, __LINE__, "Unable to delegate credentials for user: %s", (char *)token_out.value);
            goto end;
        }
        else if (!mod_authn_gssapi_store_gss_creds(r, p, token_out.value, client_cred)) {
            rc = mod_authn_gssapi_send_500_server_error(r);
            goto end;
        }
    }

    http_auth_setenv(r, token_out.value, token_out.length, CONST_STR_LEN("GSSAPI"));
    rc = HANDLER_GO_ON; /* success */

    end:
        buffer_free(t_in);
        buffer_free(sprinc);

        if (context != GSS_C_NO_CONTEXT)
            gss_delete_sec_context(&st_minor, &context, GSS_C_NO_BUFFER);

        if (client_cred != GSS_C_NO_CREDENTIAL)
            gss_release_cred(&st_minor, &client_cred);
        if (server_cred != GSS_C_NO_CREDENTIAL)
            gss_release_cred(&st_minor, &server_cred);

        if (client_name != GSS_C_NO_NAME)
            gss_release_name(&st_minor, &client_name);
        if (server_name != GSS_C_NO_NAME)
            gss_release_name(&st_minor, &server_name);

        if (token_s.length)
            gss_release_buffer(&st_minor, &token_s);
        /* if (token_in.length)
         *    gss_release_buffer(&st_minor, &token_in); */
        if (token_out.length)
            gss_release_buffer(&st_minor, &token_out);

        return rc != HANDLER_UNSET ? rc : mod_authn_gssapi_send_401_unauthorized_negotiate(r);
}

static handler_t mod_authn_gssapi_check (request_st * const r, void *p_d, const struct http_auth_require_t * const require, const struct http_auth_backend_t * const backend)
{
    const buffer *vb = http_header_request_get(r, HTTP_HEADER_AUTHORIZATION, CONST_STR_LEN("Authorization"));

    UNUSED(backend);
    if (NULL == vb) {
        return mod_authn_gssapi_send_401_unauthorized_negotiate(r);
    }

    if (!buffer_eq_icase_ssn(vb->ptr, CONST_STR_LEN("Negotiate "))) {
        return mod_authn_gssapi_send_400_bad_request(r);
    }

    return mod_authn_gssapi_check_spnego(r, (plugin_data *)p_d, require, vb->ptr+sizeof("Negotiate ")-1);
}

/*
 * HTTP auth Basic realm="kerberos"
 */

static krb5_error_code mod_authn_gssapi_verify_krb5_init_creds(krb5_context context, krb5_creds *creds, krb5_principal ap_req_server, krb5_keytab ap_req_keytab, log_error_st *errh)
{
    krb5_error_code ret;
    krb5_data req;
    krb5_ccache local_ccache       = NULL;
    krb5_creds *new_creds          = NULL;
    krb5_auth_context auth_context = NULL;
    krb5_keytab keytab             = NULL;
    char *server_name;

    memset(&req, 0, sizeof(req));

    if (ap_req_keytab == NULL) {
        ret = krb5_kt_default(context, &keytab);
        if (ret)
            return ret;
    } else
        keytab = ap_req_keytab;

    ret = krb5_cc_resolve(context, "MEMORY:", &local_ccache);
    if (ret) {
        log_error(errh, __FILE__, __LINE__, "krb5_cc_resolve() failed when verifying KDC");
        /* return ret; */
        goto end;
    }

    ret = krb5_cc_initialize(context, local_ccache, creds->client);
    if (ret) {
        log_error(errh, __FILE__, __LINE__, "krb5_cc_initialize() failed when verifying KDC");
        goto end;
    }

    ret = krb5_cc_store_cred(context, local_ccache, creds);
    if (ret) {
        log_error(errh, __FILE__, __LINE__, "krb5_cc_store_cred() failed when verifying KDC");
        goto end;
    }

    ret = krb5_unparse_name(context, ap_req_server, &server_name);
    if (ret) {
        log_error(errh, __FILE__, __LINE__, "krb5_unparse_name() failed when verifying KDC");
        goto end;
    }
    krb5_free_unparsed_name(context, server_name);

    if (!krb5_principal_compare(context, ap_req_server, creds->server)) {
        krb5_creds match_cred;

        memset(&match_cred, 0, sizeof(match_cred));

        match_cred.client = creds->client;
        match_cred.server = ap_req_server;

        ret = krb5_get_credentials(context, 0, local_ccache, &match_cred, &new_creds);
        if (ret) {
            log_error(errh, __FILE__, __LINE__, "krb5_get_credentials() failed when verifying KDC");
            goto end;
        }
        creds = new_creds;
    }

    ret = krb5_mk_req_extended(context, &auth_context, 0, NULL, creds, &req);
    if (ret) {
        log_error(errh, __FILE__, __LINE__, "krb5_mk_req_extended() failed when verifying KDC");
        goto end;
    }

    krb5_auth_con_free(context, auth_context);
    auth_context = NULL;
    ret = krb5_auth_con_init(context, &auth_context);
    if (ret) {
        log_error(errh, __FILE__, __LINE__, "krb5_auth_con_init() failed when verifying KDC");
        goto end;
    }

    /* use KRB5_AUTH_CONTEXT_DO_SEQUENCE to skip replay cache checks */
    krb5_auth_con_setflags(context, auth_context, KRB5_AUTH_CONTEXT_DO_SEQUENCE);
    ret = krb5_rd_req(context, &auth_context, &req, ap_req_server, keytab, 0, NULL);
    if (ret) {
        log_error(errh, __FILE__, __LINE__, "krb5_rd_req() failed when verifying KDC");
        goto end;
    }

    end:
        krb5_free_data_contents(context, &req);
        if (auth_context)
            krb5_auth_con_free(context, auth_context);
        if (new_creds)
            krb5_free_creds(context, new_creds);
        if (ap_req_keytab == NULL && keytab)
            krb5_kt_close(context, keytab);
        if (local_ccache)
            krb5_cc_destroy(context, local_ccache);

    return ret;
}

static int mod_authn_gssapi_store_krb5_creds(request_st * const r, plugin_data * const p,
                                             krb5_context kcontext, krb5_ccache delegated_cred)
{
    krb5_error_code problem;
    krb5_principal princ = NULL;
    krb5_ccache ccache   = NULL;

    problem = krb5_cc_get_principal(kcontext, delegated_cred, &princ);
    if (problem) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_cc_get_principal", NULL, kcontext, problem);
        goto end;
    }

    if (mod_authn_gssapi_create_krb5_ccache(r, p, kcontext, princ, &ccache)) {
        goto end;
    }

    problem = krb5_cc_copy_creds(kcontext, delegated_cred, ccache);
    if (problem) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_cc_copy_creds", NULL, kcontext, problem);
        goto end;
    }

    krb5_free_principal(kcontext, princ);
    krb5_cc_close(kcontext, ccache);
    return 0;

    end:
        if (princ)
            krb5_free_principal(kcontext, princ);
        if (ccache)
            krb5_cc_destroy(kcontext, ccache);
        return -1;
}

static handler_t mod_authn_gssapi_send_401_unauthorized_basic (request_st * const r)
{
    r->http_status = 401;
    r->handler_module = NULL;
    http_header_response_set(r, HTTP_HEADER_WWW_AUTHENTICATE,
                             CONST_STR_LEN("WWW-Authenticate"),
                             CONST_STR_LEN("Basic realm=\"Kerberos\""));
    return HANDLER_FINISHED;
}

static handler_t mod_authn_gssapi_basic(request_st * const r, void *p_d, const http_auth_require_t * const require, const buffer * const username, const char * const pw)
{
    krb5_context kcontext  = NULL;
    krb5_keytab keytab     = NULL;
    krb5_principal s_princ = NULL;
    krb5_principal c_princ = NULL;
    krb5_creds c_creds;
    krb5_ccache c_ccache   = NULL;
    krb5_ccache ret_ccache = NULL;
    krb5_error_code code;
    int ret;
    buffer *sprinc;
    buffer *user_at_realm  = NULL;
    plugin_data * const p = (plugin_data *)p_d;

    if (*pw == '\0') {
        log_error(r->conf.errh, __FILE__, __LINE__, "Empty passwords are not accepted");
        return mod_authn_gssapi_send_401_unauthorized_basic(r);
    }

    mod_authn_gssapi_patch_config(r, p);

    code = krb5_init_context(&kcontext);
    if (code) {
        log_error(r->conf.errh, __FILE__, __LINE__, "krb5_init_context(): %d", code);
        return mod_authn_gssapi_send_401_unauthorized_basic(r); /*(well, should be 500)*/
    }

    if (buffer_string_is_empty(p->conf.auth_gssapi_keytab)) {
        log_error(r->conf.errh, __FILE__, __LINE__, "auth.backend.gssapi.keytab not configured");
        return mod_authn_gssapi_send_401_unauthorized_basic(r); /*(well, should be 500)*/
    }

    code = krb5_kt_resolve(kcontext, p->conf.auth_gssapi_keytab->ptr, &keytab);
    if (code) {
        log_error(r->conf.errh, __FILE__, __LINE__, "krb5_kt_resolve(): %d %s", code, p->conf.auth_gssapi_keytab->ptr);
        return mod_authn_gssapi_send_401_unauthorized_basic(r); /*(well, should be 500)*/
    }

    sprinc = buffer_init_buffer(p->conf.auth_gssapi_principal);
    if (strchr(sprinc->ptr, '/') == NULL) {
        /*(copy HTTP Host, omitting port if port is present)*/
        /* ??? Should r->server_name be used if http_host not present?
         * ??? What if r->server_name is not set?
         * ??? Will this work below if IPv6 provided in Host?  probably not */
        if (!buffer_is_empty(r->http_host)) {
            buffer_append_string_len(sprinc, CONST_STR_LEN("/"));
            buffer_append_string_len(sprinc, r->http_host->ptr, strcspn(r->http_host->ptr, ":"));
        }
    }

    /*(init c_creds before anything which might krb5_free_cred_contents())*/
    memset(&c_creds, 0, sizeof(c_creds));

    ret = krb5_parse_name(kcontext, sprinc->ptr, &s_princ);
    if (ret) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_parse_name", sprinc->ptr, kcontext, ret);
        ret = -1;
        goto end;
    }

    if (strchr(username->ptr, '@') == NULL) {
        user_at_realm = buffer_init_buffer(username);
        BUFFER_APPEND_STRING_CONST(user_at_realm, "@");
        buffer_append_string_buffer(user_at_realm, require->realm);
    }

    ret = krb5_parse_name(kcontext, (user_at_realm ? user_at_realm->ptr : username->ptr), &c_princ);
    if (ret) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_parse_name", (user_at_realm ? user_at_realm->ptr : username->ptr), kcontext, ret);
        if (user_at_realm) buffer_free(user_at_realm);
        ret = -1;
        goto end;
    }
    if (user_at_realm) buffer_free(user_at_realm);
    /* XXX: if the qualified username with @realm should be in REMOTE_USER,
     * then http_auth_backend_t basic interface needs to change to pass
     * modifiable buffer *username, but note that const char *pw follows
     * in the truncated buffer *username, so pw would need to be copied
     * before modifying buffer *username */

    /*
     * char *name = NULL;
     * ret = krb5_unparse_name(kcontext, c_princ, &name);
     * if (ret == 0) {
     *    log_error(r->conf.errh, __FILE__, __LINE__, "Trying to get TGT for user: %s password: %s", username->ptr, pw);
     * }
     * krb5_free_unparsed_name(kcontext, name);
     */

    ret = krb5_get_init_creds_password(kcontext, &c_creds, c_princ, pw, NULL, NULL, 0, NULL, NULL);
    if (ret) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_get_init_creds_password", NULL, kcontext, ret);
        goto end;
    }

    ret = mod_authn_gssapi_verify_krb5_init_creds(kcontext, &c_creds, s_princ, keytab, r->conf.errh);
    if (ret) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "mod_authn_gssapi_verify_krb5_init_creds", NULL, kcontext, ret);
        goto end;
    }

    if (!p->conf.auth_gssapi_store_creds) goto end;

    ret = krb5_cc_resolve(kcontext, "MEMORY:", &ret_ccache);
    if (ret) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_cc_resolve", NULL, kcontext, ret);
        goto end;
    }

    ret = krb5_cc_initialize(kcontext, ret_ccache, c_princ);
    if (ret) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_cc_initialize", NULL, kcontext, ret);
        goto end;
    }

    ret = krb5_cc_store_cred(kcontext, ret_ccache, &c_creds);
    if (ret) {
        mod_authn_gssapi_log_krb5_error(r->conf.errh, __FILE__, __LINE__, "krb5_cc_store_cred", NULL, kcontext, ret);
        goto end;
    }

    c_ccache = ret_ccache;
    ret_ccache = NULL;

    end:

        krb5_free_cred_contents(kcontext, &c_creds);
        if (ret_ccache)
            krb5_cc_destroy(kcontext, ret_ccache);

        if (!ret && c_ccache && (ret = mod_authn_gssapi_store_krb5_creds(r, p, kcontext, c_ccache))) {
            log_error(r->conf.errh, __FILE__, __LINE__, "mod_authn_gssapi_store_krb5_creds failed for %s", username->ptr);
        }

        buffer_free(sprinc);
        if (c_princ)
            krb5_free_principal(kcontext, c_princ);
        if (s_princ)
            krb5_free_principal(kcontext, s_princ);
        if (c_ccache)
            krb5_cc_destroy(kcontext, c_ccache);
        if (keytab)
            krb5_kt_close(kcontext, keytab);

        krb5_free_context(kcontext);

        if (0 == ret && http_auth_match_rules(require,username->ptr,NULL,NULL)){
            return HANDLER_GO_ON;
        }
        else {
            /* ret == KRB5KDC_ERR_C_PRINCIPAL_UNKNOWN or no authz rules match */
            log_error(r->conf.errh, __FILE__, __LINE__,
              "password doesn't match for %s username: %s IP: %s",
              r->uri.path.ptr, username->ptr, r->con->dst_addr_buf->ptr);
            return mod_authn_gssapi_send_401_unauthorized_basic(r);
        }
}


REQUEST_FUNC(mod_authn_gssapi_handle_reset) {
    plugin_data *p = (plugin_data *)p_d;
    buffer *kccname = (buffer *)r->plugin_ctx[p->id];
    if (NULL != kccname) {
        r->plugin_ctx[p->id] = NULL;
        unlink(kccname->ptr+sizeof("FILE:")-1);
        buffer_free(kccname);
    }

    return HANDLER_GO_ON;
}

int mod_authn_gssapi_plugin_init(plugin *p);
int mod_authn_gssapi_plugin_init(plugin *p) {
    p->version     = LIGHTTPD_VERSION_ID;
    p->name        = "authn_gssapi";
    p->init        = mod_authn_gssapi_init;
    p->set_defaults= mod_authn_gssapi_set_defaults;
    p->handle_request_reset = mod_authn_gssapi_handle_reset;

    return 0;
}
