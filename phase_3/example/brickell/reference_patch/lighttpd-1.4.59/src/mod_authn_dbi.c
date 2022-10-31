/*
 * mod_authn_dbi - authn backend employing DBI interface
 *
 * Copyright(c) 2020 Glenn Strauss gstrauss()gluelogic.com  All rights reserved
 * License: BSD 3-clause (same as lighttpd)
 */
/*
 * authn backend employing DBI
 *
 * e.g.
 *   auth.backend.dbi = (
 *     "sql" => "SELECT passwd FROM users WHERE user='?' AND realm='?'"
 *     "dbtype" => "sqlite3",
 *     "dbname" => "mydb.sqlite",
 *     "sqlite_dbdir" => "/path/to/sqlite/dbs/"
 *   )
 *
 *   SQL samples (change table and column names for your database schema):
 *     "sql" => "SELECT passwd FROM users WHERE user='?'"
 *     "sql" => "SELECT passwd FROM users WHERE user='?' AND realm='?'"
 *     "sql" => "SELECT passwd FROM users WHERE user='?' AND realm='?' AND algorithm='?'"
 *     "sql" => "SELECT passwd FROM users WHERE user='?' AND realm='?' AND algorithm='?' AND group IN ('groupA','groupB','groupC')"
 */
#include "first.h"

#if defined(HAVE_CRYPT_R) || defined(HAVE_CRYPT)
#ifndef _XOPEN_CRYPT
#define _XOPEN_CRYPT 1
#endif
#include <unistd.h>     /* crypt() */
#endif
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include <dbi/dbi.h>

#include <string.h>
#include <stdlib.h>

#include "sys-crypto-md.h"
#include "safe_memclear.h"
#include "base.h"
#include "http_auth.h"
#include "fdevent.h"
#include "log.h"
#include "plugin.h"

typedef struct {
    dbi_conn dbconn;
    dbi_inst dbinst;
    const buffer *sqlquery;
    log_error_st *errh;
    short reconnect_count;
} dbi_config;

typedef struct {
    void *vdata;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config defaults;
    plugin_config conf;
} plugin_data;


/* used to reconnect to the database when we get disconnected */
static void
mod_authn_dbi_error_callback (dbi_conn dbconn, void *vdata)
{
    dbi_config *dbconf = (dbi_config *)vdata;
    const char *errormsg = NULL;
    /*assert(dbconf->dbconn == dbconn);*/

    while (++dbconf->reconnect_count <= 3) { /* retry */
        if (0 == dbi_conn_connect(dbconn)) {
            fdevent_setfd_cloexec(dbi_conn_get_socket(dbconn));
            return;
        }
    }

    dbi_conn_error(dbconn, &errormsg);
    log_error(dbconf->errh,__FILE__,__LINE__,"dbi_conn_connect(): %s",errormsg);
}


static void
mod_authn_dbi_dbconf_free (void *vdata)
{
    dbi_config *dbconf = (dbi_config *)vdata;
    if (!dbconf) return;
    dbi_conn_close(dbconf->dbconn);
    dbi_shutdown_r(dbconf->dbinst);
    free(dbconf);
}


static int
mod_authn_dbi_dbconf_setup (server *srv, const array *opts, void **vdata)
{
    const buffer *sqlquery = NULL;
    const buffer *dbtype=NULL, *dbname=NULL;

    for (size_t i = 0; i < opts->used; ++i) {
        const data_string *ds = (data_string *)opts->data[i];
        if (ds->type == TYPE_STRING) {
            if (buffer_eq_icase_slen(&ds->key, CONST_STR_LEN("sql")))
                sqlquery = &ds->value;
            else if (buffer_eq_icase_slen(&ds->key, CONST_STR_LEN("dbname")))
                dbname = &ds->value;
            else if (buffer_eq_icase_slen(&ds->key, CONST_STR_LEN("dbtype")))
                dbtype = &ds->value;
        }
    }

    /* required:
     * - sql    (sql query)
     * - dbtype
     * - dbname
     *
     * optional:
     * - username, some databases don't require this (sqlite)
     * - password, default: empty
     * - socket, default: database type default
     * - hostname, if set overrides socket
     * - port, default: database default
     * - encoding, default: database default
     */

    if (!buffer_string_is_empty(sqlquery)
        && !buffer_is_empty(dbname) && !buffer_is_empty(dbtype)) {
        /* create/initialise database */
        dbi_config *dbconf;
        dbi_inst dbinst = NULL;
        dbi_conn dbconn;
        if (dbi_initialize_r(NULL, &dbinst) < 1) {
            log_error(srv->errh, __FILE__, __LINE__,
              "dbi_initialize_r() failed.  "
              "Do you have the DBD for this db type installed?");
            return -1;
        }
        dbconn = dbi_conn_new_r(dbtype->ptr, dbinst);
        if (NULL == dbconn) {
            log_error(srv->errh, __FILE__, __LINE__,
              "dbi_conn_new_r() failed.  "
              "Do you have the DBD for this db type installed?");
            dbi_shutdown_r(dbinst);
            return -1;
        }

        /* set options */
        for (size_t j = 0; j < opts->used; ++j) {
            data_unset *du = opts->data[j];
            const buffer *opt = &du->key;
            if (!buffer_string_is_empty(opt)) {
                if (du->type == TYPE_INTEGER) {
                    data_integer *di = (data_integer *)du;
                    dbi_conn_set_option_numeric(dbconn, opt->ptr, di->value);
                }
                else if (du->type == TYPE_STRING) {
                    data_string *ds = (data_string *)du;
                    if (&ds->value != sqlquery && &ds->value != dbtype) {
                        dbi_conn_set_option(dbconn, opt->ptr, ds->value.ptr);
                    }
                }
            }
        }

        dbconf = (dbi_config *)calloc(1, sizeof(*dbconf));
        dbconf->dbinst = dbinst;
        dbconf->dbconn = dbconn;
        dbconf->sqlquery = sqlquery;
        dbconf->errh = srv->errh;
        dbconf->reconnect_count = 0;
        *vdata = dbconf;

        /* used to automatically reconnect to the database */
        dbi_conn_error_handler(dbconn, mod_authn_dbi_error_callback, dbconf);

        /* connect to database */
        mod_authn_dbi_error_callback(dbconn, dbconf);
        if (dbconf->reconnect_count >= 3) {
            mod_authn_dbi_dbconf_free(dbconf);
            return -1;
        }
    }

    return 0;
}


static handler_t mod_authn_dbi_basic(request_st *r, void *p_d, const http_auth_require_t *require, const buffer *username, const char *pw);
static handler_t mod_authn_dbi_digest(request_st *r, void *p_d, http_auth_info_t *dig);

INIT_FUNC(mod_authn_dbi_init) {
    static http_auth_backend_t http_auth_backend_dbi =
      { "dbi", mod_authn_dbi_basic, mod_authn_dbi_digest, NULL };
    plugin_data *p = calloc(1, sizeof(*p));

    /* register http_auth_backend_dbi */
    http_auth_backend_dbi.p_d = p;
    http_auth_backend_set(&http_auth_backend_dbi);

    return p;
}


FREE_FUNC(mod_authn_dbi_cleanup) {
    plugin_data * const p = p_d;
    if (NULL == p->cvlist) return;
    /* (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1], used = p->nconfig; i < used; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        for (; -1 != cpv->k_id; ++cpv) {
            if (cpv->vtype != T_CONFIG_LOCAL || NULL == cpv->v.v) continue;
            switch (cpv->k_id) {
              case 0: /* auth.backend.dbi */
                mod_authn_dbi_dbconf_free(cpv->v.v);
                break;
              default:
                break;
            }
        }
    }
}


static void
mod_authn_dbi_merge_config_cpv (plugin_config * const pconf, const config_plugin_value_t * const cpv)
{
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* auth.backend.dbi */
        if (cpv->vtype == T_CONFIG_LOCAL)
            pconf->vdata = cpv->v.v;
        break;
      default:/* should not happen */
        return;
    }
}


static void
mod_authn_dbi_merge_config (plugin_config * const pconf, const config_plugin_value_t *cpv)
{
    do {
        mod_authn_dbi_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}


static void
mod_authn_dbi_patch_config(request_st * const r, plugin_data * const p)
{
    p->conf = p->defaults; /* copy small struct instead of memcpy() */
    /*memcpy(&p->conf, &p->defaults, sizeof(plugin_config));*/
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_authn_dbi_merge_config(&p->conf,
                                       p->cvlist + p->cvlist[i].v.u2[0]);
    }
}


SETDEFAULTS_FUNC(mod_authn_dbi_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("auth.backend.dbi"),
        T_CONFIG_ARRAY_KVANY,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_authn_dbi"))
        return HANDLER_ERROR;

    /* process and validate config directives
     * (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1]; i < p->nconfig; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        for (; -1 != cpv->k_id; ++cpv) {
            switch (cpv->k_id) {
              case 0: /* auth.backend.dbi */
                if (cpv->v.a->used) {
                    if (0 != mod_authn_dbi_dbconf_setup(srv,cpv->v.a,&cpv->v.v))
                        return HANDLER_ERROR;
                    if (NULL != cpv->v.v)
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
            mod_authn_dbi_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}


/* improved and diverged from mod_authn_mysql_password_cmp()
 *   If Basic Auth and storing MD5 or SHA-256, the hash is of user:realm:passwd
 *   with unique user:realm: combination acting as salt, rather than salt-less
 *   hash of passwd as was implemented in (deprecated) mod_authn_mysql.
 *   Conveniently, hash digest of user:realm:passwd is the same value used by
 *   Digest Auth, so transition to Digest Auth is simple change in lighttpd.conf
 * future: might move to mod_auth.c and have only mod_auth.so depend on -lcrypt
 * references:
 *   https://en.wikipedia.org/wiki/Crypt_(C)
 *   Modular Crypt Format (MCF)
 *   https://passlib.readthedocs.io/en/stable/modular_crypt_format.html
 *   PHC string format
 *   https://github.com/P-H-C/phc-string-format/blob/master/phc-sf-spec.md
 * Note: (struct crypt_data) is large and might not fit on the stack
 *   On some systems it is 32k, on others 128k or more.
 *   Therefore, since lighttpd is single-threaded, simply use crypt(),
 *     which is not thread-safe but fine for single-threaded lighttpd.
 *   If crypt_r() is preferred, then safest method is to allocate from heap
 *   rather than using (struct crypt_data) on stack.
 */
#if defined(HAVE_CRYPT_R) || defined(HAVE_CRYPT)
static int
mod_authn_crypt_cmp (const char *reqpw, const char *userpw, unsigned long userpwlen)
{
 #if 1

    char *crypted = crypt(reqpw, userpw);
    size_t crypwlen = (NULL != crypted) ? strlen(crypted) : 0;
    int rc = (crypwlen == userpwlen) ? memcmp(crypted, userpw, crypwlen) : -1;
    if (crypwlen) safe_memclear(crypted, crypwlen);
    return rc;

 #else

  #if defined(HAVE_CRYPT_R)
   #if 1 /* (must free() before returning if allocated here) */
    struct crypt_data *crypt_tmp_data = malloc(sizeof(struct crypt_data));
    force_assert(crypt_tmp_data);
   #else /* safe if sizeof(struct crypt_data) <= 32768 */
    struct crypt_data crypt_tmp_data_stack;
    struct crypt_data *crypt_tmp_data = &crypt_tmp_data_stack;
   #endif
   #ifdef _AIX
    memset(&crypt_tmp_data_stack, 0, sizeof(struct crypt_data));
   #else
    crypt_tmp_data_stack.initialized = 0;
   #endif
  #endif

  #if defined(HAVE_CRYPT_R)
    char *crypted = crypt_r(reqpw, userpw, crypt_tmp_data);
  #else
    char *crypted = crypt(reqpw, userpw);
  #endif
    size_t crypwlen = (NULL != crypted) ? strlen(crypted) : 0;
    int rc = (crypwlen == userpwlen) ? memcmp(crypted, userpw, crypwlen) : -1;

    safe_memclear(crypted, crypwlen);
  #if defined(HAVE_CRYPT_R)
   #if 1 /* (must free() if allocated above) */
    free(crypt_tmp_data);
   #else /* safe if sizeof(struct crypt_data) <= 32768 */
   #endif
  #endif
    return rc;

 #endif
}
#endif


static int
mod_authn_dbi_password_cmp (const char *userpw, unsigned long userpwlen, http_auth_info_t * const ai, const char *reqpw)
{
  #if defined(HAVE_CRYPT_R) || defined(HAVE_CRYPT)
    if (userpwlen >= 3 && userpw[0] == '$')
        return mod_authn_crypt_cmp(reqpw, userpw, userpwlen);
    else
  #endif
    if (32 == userpwlen) {
        /* plain md5 */
        MD5_CTX ctx;
        unsigned char HA1[16];
        unsigned char md5pw[16];

        MD5_Init(&ctx);
        MD5_Update(&ctx, (unsigned char *)ai->username, ai->ulen);
        MD5_Update(&ctx, CONST_STR_LEN(":"));
        MD5_Update(&ctx, (unsigned char *)ai->realm, ai->rlen);
        MD5_Update(&ctx, CONST_STR_LEN(":"));
        MD5_Update(&ctx, (unsigned char *)reqpw, strlen(reqpw));
        MD5_Final(HA1, &ctx);

        /*(compare 16-byte MD5 binary instead of converting to hex strings
         * in order to then have to do case-insensitive hex str comparison)*/
        return (0 == http_auth_digest_hex2bin(userpw, 32, md5pw, sizeof(md5pw)))
          ? http_auth_const_time_memeq(HA1, md5pw, sizeof(md5pw)) ? 0 : 1
          : -1;
    }
  #ifdef USE_LIB_CRYPTO
    else if (64 == userpwlen) {
        SHA256_CTX ctx;
        unsigned char HA1[32];
        unsigned char shapw[32];

        SHA256_Init(&ctx);
        SHA256_Update(&ctx, (unsigned char *)ai->username, ai->ulen);
        SHA256_Update(&ctx, CONST_STR_LEN(":"));
        SHA256_Update(&ctx, (unsigned char *)ai->realm, ai->rlen);
        SHA256_Update(&ctx, CONST_STR_LEN(":"));
        SHA256_Update(&ctx, (unsigned char *)reqpw, strlen(reqpw));
        SHA256_Final(HA1, &ctx);

        /*(compare 32-byte binary digest instead of converting to hex strings
         * in order to then have to do case-insensitive hex str comparison)*/
        return (0 == http_auth_digest_hex2bin(userpw, 64, shapw, sizeof(shapw)))
          ? http_auth_const_time_memeq(HA1, shapw, sizeof(shapw)) ? 0 : 1
          : -1;
    }
  #endif

    return -1;
}


static buffer *
mod_authn_dbi_query_build (buffer * const sqlquery, dbi_config * const dbconf, http_auth_info_t * const ai)
{
    buffer_clear(sqlquery);
    int qcount = 0;
    for (char *b = dbconf->sqlquery->ptr, *d; *b; b = d+1) {
        if (NULL != (d = strchr(b, '?'))) {
            /* Substitute for up to three question marks (?)
             *   substitute username for first question mark
             *   substitute realm for second question mark
             *   substitute digest algorithm for third question mark */
            const char *v;
            switch (++qcount) {
              case 1:
                v = ai->username;
                break;
              case 2:
                v = ai->realm;
                break;
              case 3:
                if (ai->dalgo & HTTP_AUTH_DIGEST_SHA256)
                    v = "SHA-256";
                else if (ai->dalgo & HTTP_AUTH_DIGEST_MD5)
                    v = "MD5";
                else if (ai->dalgo == HTTP_AUTH_DIGEST_NONE)
                    v = "NONE";
                else
                    return NULL;
                break;
              default:
                return NULL;
            }
            /* escape the value */
            char *esc = NULL;
            size_t elen =
              dbi_conn_escape_string_copy(dbconf->dbconn, v, &esc);
            if (0 == elen) return NULL; /*('esc' must not be freed if error)*/
            buffer_append_string_len(sqlquery, b, (size_t)(d - b));
            buffer_append_string_len(sqlquery, esc, elen);
            free(esc);
        }
        else {
            d = dbconf->sqlquery->ptr + buffer_string_length(dbconf->sqlquery);
            buffer_append_string_len(sqlquery, b, (size_t)(d - b));
            break;
        }
    }

    return sqlquery;
}


static handler_t
mod_authn_dbi_query (request_st * const r, void *p_d, http_auth_info_t * const ai, const char * const pw)
{
    plugin_data * const p = (plugin_data *)p_d;
    mod_authn_dbi_patch_config(r, p);
    if (NULL == p->conf.vdata) return HANDLER_ERROR; /*(should not happen)*/
    dbi_config * const dbconf = (dbi_config *)p->conf.vdata;

    buffer * const sqlquery = mod_authn_dbi_query_build(r->tmp_buf, dbconf, ai);
    if (NULL == sqlquery)
        return HANDLER_ERROR;

    /* reset our reconnect-attempt counter, this is a new query. */
    dbconf->reconnect_count = 0;

    dbi_result result;
    int retry_count = 0;
    do {
        result = dbi_conn_query(dbconf->dbconn, sqlquery->ptr);
    } while (!result && ++retry_count < 2);

    if (!result) {
        const char *errmsg;
        dbi_conn_error(dbconf->dbconn, &errmsg);
        log_error(r->conf.errh, __FILE__, __LINE__, "%s", errmsg);
        return HANDLER_ERROR;
    }

    handler_t rc = HANDLER_ERROR;
    unsigned long long nrows = dbi_result_get_numrows(result);
    if (nrows && nrows != DBI_ROW_ERROR && dbi_result_next_row(result)) {
        size_t len = dbi_result_get_field_length_idx(result, 1);
        const char *rpw = dbi_result_get_string_idx(result, 1);
        /*("ERROR" indicates an error and should not be permitted as passwd)*/
        if (len != DBI_LENGTH_ERROR
            && rpw && (len != 5 || 0 != memcmp(rpw, "ERROR", 5))) {
            if (pw) {  /* used with HTTP Basic auth */
                if (0 == mod_authn_dbi_password_cmp(rpw, len, ai, pw))
                    rc = HANDLER_GO_ON;
            }
            else {     /* used with HTTP Digest auth */
                /*(currently supports only single row, single digest algorithm)*/
                if (len == (ai->dlen << 1)
                    && 0 == http_auth_digest_hex2bin(rpw, len, ai->digest,
                                                     sizeof(ai->digest)))
                    rc = HANDLER_GO_ON;
            }
        }
    } /* else not found */

    dbi_result_free(result);
    return rc;
}


static handler_t
mod_authn_dbi_basic (request_st * const r, void *p_d, const http_auth_require_t * const require, const buffer * const username, const char * const pw)
{
    handler_t rc;
    http_auth_info_t ai;
    ai.dalgo    = HTTP_AUTH_DIGEST_NONE;
    ai.dlen     = 0;
    ai.username = username->ptr;
    ai.ulen     = buffer_string_length(username);
    ai.realm    = require->realm->ptr;
    ai.rlen     = buffer_string_length(require->realm);
    rc = mod_authn_dbi_query(r, p_d, &ai, pw);
    if (HANDLER_GO_ON != rc) return rc;
    return http_auth_match_rules(require, username->ptr, NULL, NULL)
      ? HANDLER_GO_ON  /* access granted */
      : HANDLER_ERROR;
}


static handler_t
mod_authn_dbi_digest (request_st * const r, void *p_d, http_auth_info_t * const ai)
{
    return mod_authn_dbi_query(r, p_d, ai, NULL);
}


int mod_authn_dbi_plugin_init (plugin *p);
int mod_authn_dbi_plugin_init (plugin *p)
{
    p->version          = LIGHTTPD_VERSION_ID;
    p->name             = "authn_dbi";
    p->init             = mod_authn_dbi_init;
    p->cleanup          = mod_authn_dbi_cleanup;
    p->set_defaults     = mod_authn_dbi_set_defaults;

    return 0;
}
