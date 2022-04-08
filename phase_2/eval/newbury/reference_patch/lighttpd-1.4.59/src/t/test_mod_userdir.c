#include "first.h"

#undef NDEBUG
#include <assert.h>
#include <stdlib.h>

#include "mod_userdir.c"

static void test_mod_userdir_reset(request_st * const r)
{
    r->http_status = 0;
    buffer_clear(&r->physical.basedir);
    buffer_clear(&r->physical.path);
}

static void
test_mod_userdir_docroot_handler(request_st * const r, plugin_data * const p)
{
    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN(""));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    p->defaults.active = 1;

    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/other"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~no-trailing-slash"));
    test_mod_userdir_reset(r);
    assert(HANDLER_FINISHED == mod_userdir_docroot_handler(r, p));
    assert(301 == r->http_status);
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~/"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~jan/"));
    buffer_copy_string_len(&r->physical.rel_path, CONST_STR_LEN("/~jan/"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_eq_slen(&r->physical.basedir,
                          CONST_STR_LEN("/web/u/jan/public_html")));
    assert(buffer_eq_slen(&r->physical.path,
                          CONST_STR_LEN("/web/u/jan/public_html/")));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~jan/more"));
    buffer_copy_string_len(&r->physical.rel_path, CONST_STR_LEN("/~jan/more"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_eq_slen(&r->physical.basedir,
                          CONST_STR_LEN("/web/u/jan/public_html")));
    assert(buffer_eq_slen(&r->physical.path,
                          CONST_STR_LEN("/web/u/jan/public_html/more")));

    p->defaults.letterhomes = 1;

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~.jan/"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~jan/"));
    buffer_copy_string_len(&r->physical.rel_path, CONST_STR_LEN("/~jan/"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_eq_slen(&r->physical.basedir,
                          CONST_STR_LEN("/web/u/j/jan/public_html")));
    assert(buffer_eq_slen(&r->physical.path,
                          CONST_STR_LEN("/web/u/j/jan/public_html/")));

    p->defaults.letterhomes = 0;

    array *include_user = array_init(2);
    array *exclude_user = array_init(2);

    array_insert_value(include_user, CONST_STR_LEN("notjan"));

    p->defaults.include_user = include_user;

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~jan/"));
    buffer_copy_string_len(&r->physical.rel_path, CONST_STR_LEN("/~jan/"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    array_insert_value(include_user, CONST_STR_LEN("jan"));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~jan/"));
    buffer_copy_string_len(&r->physical.rel_path, CONST_STR_LEN("/~jan/"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_eq_slen(&r->physical.basedir,
                          CONST_STR_LEN("/web/u/jan/public_html")));
    assert(buffer_eq_slen(&r->physical.path,
                          CONST_STR_LEN("/web/u/jan/public_html/")));

    p->defaults.exclude_user = exclude_user;

    array_insert_value(exclude_user, CONST_STR_LEN("notjan"));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~jan/"));
    buffer_copy_string_len(&r->physical.rel_path, CONST_STR_LEN("/~jan/"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_eq_slen(&r->physical.basedir,
                          CONST_STR_LEN("/web/u/jan/public_html")));
    assert(buffer_eq_slen(&r->physical.path,
                          CONST_STR_LEN("/web/u/jan/public_html/")));

    array_insert_value(exclude_user, CONST_STR_LEN("jan"));

    buffer_copy_string_len(&r->uri.path, CONST_STR_LEN("/~jan/"));
    buffer_copy_string_len(&r->physical.rel_path, CONST_STR_LEN("/~jan/"));
    test_mod_userdir_reset(r);
    assert(HANDLER_GO_ON == mod_userdir_docroot_handler(r, p));
    assert(buffer_is_empty(&r->physical.basedir));
    assert(buffer_is_empty(&r->physical.path));

    p->defaults.include_user = NULL;
    p->defaults.exclude_user = NULL;
    array_free(include_user);
    array_free(exclude_user);
}

int main (void)
{
    plugin_data * const p = mod_userdir_init();
    assert(NULL != p);

    buffer *basepath = buffer_init_string("/web/u/"); /*(skip getpwnam())*/
    buffer *path     = buffer_init_string("public_html");
    p->defaults.basepath = basepath;
    p->defaults.path = path;

    request_st r;

    memset(&r, 0, sizeof(request_st));
    r.tmp_buf                = buffer_init();
    r.conf.errh              = log_error_st_init();
    r.conf.errh->errorlog_fd = -1; /* (disable) */

    test_mod_userdir_docroot_handler(&r, p);

    free(r.uri.path.ptr);
    free(r.physical.basedir.ptr);
    free(r.physical.path.ptr);
    free(r.physical.rel_path.ptr);

    log_error_st_free(r.conf.errh);
    buffer_free(r.tmp_buf);

    buffer_free(basepath);
    buffer_free(path);
    free(p);
    return 0;
}

/*
 * stub functions
 */

int http_response_redirect_to_directory(request_st *r, int status) {
    r->http_status = status;
    return 0;
}

int config_plugin_values_init(server *srv, void *p_d, const config_plugin_keys_t *cpk, const char *mname) {
    UNUSED(srv);
    UNUSED(p_d);
    UNUSED(cpk);
    UNUSED(mname);
    return 0;
}

int config_check_cond(request_st *r, int context_ndx) {
    UNUSED(r);
    UNUSED(context_ndx);
    return 0;
}
