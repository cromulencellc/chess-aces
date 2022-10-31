#include "first.h"

#include "base.h"
#include "fdevent.h"
#include "log.h"
#include "buffer.h"
#include "http_etag.h"
#include "http_header.h"
#include "stat_cache.h"

#include "plugin.h"

#include "response.h"

#include "mod_ssi.h"

#include "sys-socket.h"
#include "sys-time.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif

#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

static handler_ctx * handler_ctx_init(plugin_data *p, log_error_st *errh) {
	handler_ctx *hctx = calloc(1, sizeof(*hctx));
	force_assert(hctx);
	hctx->errh = errh;
	hctx->timefmt = p->timefmt;
	hctx->stat_fn = p->stat_fn;
	hctx->ssi_vars = p->ssi_vars;
	hctx->ssi_cgi_env = p->ssi_cgi_env;
	memcpy(&hctx->conf, &p->conf, sizeof(plugin_config));
	return hctx;
}

static void handler_ctx_free(handler_ctx *hctx) {
	free(hctx);
}

/* The newest modified time of included files for include statement */
static volatile time_t include_file_last_mtime = 0;

INIT_FUNC(mod_ssi_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));
	force_assert(p);

	p->timefmt = buffer_init();
	p->stat_fn = buffer_init();

	p->ssi_vars = array_init(8);
	p->ssi_cgi_env = array_init(32);

	return p;
}

FREE_FUNC(mod_ssi_free) {
	plugin_data *p = p_d;
	array_free(p->ssi_vars);
	array_free(p->ssi_cgi_env);
	buffer_free(p->timefmt);
	buffer_free(p->stat_fn);
}

static void mod_ssi_merge_config_cpv(plugin_config * const pconf, const config_plugin_value_t * const cpv) {
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* ssi.extension */
        pconf->ssi_extension = cpv->v.a;
        break;
      case 1: /* ssi.content-type */
        pconf->content_type = cpv->v.b;
        break;
      case 2: /* ssi.conditional-requests */
        pconf->conditional_requests = cpv->v.u;
        break;
      case 3: /* ssi.exec */
        pconf->ssi_exec = cpv->v.u;
        break;
      case 4: /* ssi.recursion-max */
        pconf->ssi_recursion_max = cpv->v.shrt;
        break;
      default:/* should not happen */
        return;
    }
}

static void mod_ssi_merge_config(plugin_config * const pconf, const config_plugin_value_t *cpv) {
    do {
        mod_ssi_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}

static void mod_ssi_patch_config(request_st * const r, plugin_data * const p) {
    memcpy(&p->conf, &p->defaults, sizeof(plugin_config));
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_ssi_merge_config(&p->conf, p->cvlist + p->cvlist[i].v.u2[0]);
    }
}

SETDEFAULTS_FUNC(mod_ssi_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("ssi.extension"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("ssi.content-type"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("ssi.conditional-requests"),
        T_CONFIG_BOOL,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("ssi.exec"),
        T_CONFIG_BOOL,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("ssi.recursion-max"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_ssi"))
        return HANDLER_ERROR;

    p->defaults.ssi_exec = 1;

    /* initialize p->defaults from global config context */
    if (p->nconfig > 0 && p->cvlist->v.u2[1]) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist->v.u2[0];
        if (-1 != cpv->k_id)
            mod_ssi_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}


static int ssi_env_add(void *venv, const char *key, size_t klen, const char *val, size_t vlen) {
	array_set_key_value((array *)venv, key, klen, val, vlen);
	return 0;
}

static int build_ssi_cgi_vars(request_st * const r, handler_ctx * const p) {
	http_cgi_opts opts = { 0, 0, NULL, NULL };
	/* temporarily remove Authorization from request headers
	 * so that Authorization does not end up in SSI environment */
	buffer *vb_auth = http_header_request_get(r, HTTP_HEADER_AUTHORIZATION, CONST_STR_LEN("Authorization"));
	buffer b_auth;
	if (vb_auth) {
		memcpy(&b_auth, vb_auth, sizeof(buffer));
		memset(vb_auth, 0, sizeof(buffer));
	}

	array_reset_data_strings(p->ssi_cgi_env);

	if (0 != http_cgi_headers(r, &opts, ssi_env_add, p->ssi_cgi_env)) {
		r->http_status = 400;
		return -1;
	}

	if (vb_auth) {
		memcpy(vb_auth, &b_auth, sizeof(buffer));
	}

	return 0;
}

static int mod_ssi_process_file(request_st *r, handler_ctx *p, struct stat *st);

static int process_ssi_stmt(request_st * const r, handler_ctx * const p, const char ** const l, size_t n, struct stat * const st) {

	/**
	 * <!--#element attribute=value attribute=value ... -->
	 *
	 * config       DONE
	 *   errmsg     -- missing
	 *   sizefmt    DONE
	 *   timefmt    DONE
	 * echo         DONE
	 *   var        DONE
	 *   encoding   -- missing
	 * exec         DONE
	 *   cgi        -- never
	 *   cmd        DONE
	 * fsize        DONE
	 *   file       DONE
	 *   virtual    DONE
	 * flastmod     DONE
	 *   file       DONE
	 *   virtual    DONE
	 * include      DONE
	 *   file       DONE
	 *   virtual    DONE
	 * printenv     DONE
	 * set          DONE
	 *   var        DONE
	 *   value      DONE
	 *
	 * if           DONE
	 * elif         DONE
	 * else         DONE
	 * endif        DONE
	 *
	 *
	 * expressions
	 * AND, OR      DONE
	 * comp         DONE
	 * ${...}       -- missing
	 * $...         DONE
	 * '...'        DONE
	 * ( ... )      DONE
	 *
	 *
	 *
	 * ** all DONE **
	 * DATE_GMT
	 *   The current date in Greenwich Mean Time.
	 * DATE_LOCAL
	 *   The current date in the local time zone.
	 * DOCUMENT_NAME
	 *   The filename (excluding directories) of the document requested by the user.
	 * DOCUMENT_URI
	 *   The (%-decoded) URL path of the document requested by the user. Note that in the case of nested include files, this is not then URL for the current document.
	 * LAST_MODIFIED
	 *   The last modification date of the document requested by the user.
	 * USER_NAME
	 *   Contains the owner of the file which included it.
	 *
	 */

	size_t i, ssicmd = 0;
	char buf[255];
	buffer *tb = NULL;

	static const struct {
		const char *var;
		enum { SSI_UNSET, SSI_ECHO, SSI_FSIZE, SSI_INCLUDE, SSI_FLASTMOD,
				SSI_CONFIG, SSI_PRINTENV, SSI_SET, SSI_IF, SSI_ELIF,
				SSI_ELSE, SSI_ENDIF, SSI_EXEC, SSI_COMMENT } type;
	} ssicmds[] = {
		{ "echo",     SSI_ECHO },
		{ "include",  SSI_INCLUDE },
		{ "flastmod", SSI_FLASTMOD },
		{ "fsize",    SSI_FSIZE },
		{ "config",   SSI_CONFIG },
		{ "printenv", SSI_PRINTENV },
		{ "set",      SSI_SET },
		{ "if",       SSI_IF },
		{ "elif",     SSI_ELIF },
		{ "endif",    SSI_ENDIF },
		{ "else",     SSI_ELSE },
		{ "exec",     SSI_EXEC },
		{ "comment",  SSI_COMMENT },

		{ NULL, SSI_UNSET }
	};

	for (i = 0; ssicmds[i].var; i++) {
		if (0 == strcmp(l[1], ssicmds[i].var)) {
			ssicmd = ssicmds[i].type;
			break;
		}
	}

	chunkqueue * const cq = &r->write_queue;

	switch(ssicmd) {
	case SSI_ECHO: {
		/* echo */
		int var = 0;
		/* int enc = 0; */
		const char *var_val = NULL;

		static const struct {
			const char *var;
			enum {
				SSI_ECHO_UNSET,
				SSI_ECHO_DATE_GMT,
				SSI_ECHO_DATE_LOCAL,
				SSI_ECHO_DOCUMENT_NAME,
				SSI_ECHO_DOCUMENT_URI,
				SSI_ECHO_LAST_MODIFIED,
				SSI_ECHO_USER_NAME,
				SSI_ECHO_SCRIPT_URI,
				SSI_ECHO_SCRIPT_URL,
			} type;
		} echovars[] = {
			{ "DATE_GMT",      SSI_ECHO_DATE_GMT },
			{ "DATE_LOCAL",    SSI_ECHO_DATE_LOCAL },
			{ "DOCUMENT_NAME", SSI_ECHO_DOCUMENT_NAME },
			{ "DOCUMENT_URI",  SSI_ECHO_DOCUMENT_URI },
			{ "LAST_MODIFIED", SSI_ECHO_LAST_MODIFIED },
			{ "USER_NAME",     SSI_ECHO_USER_NAME },
			{ "SCRIPT_URI",    SSI_ECHO_SCRIPT_URI },
			{ "SCRIPT_URL",    SSI_ECHO_SCRIPT_URL },

			{ NULL, SSI_ECHO_UNSET }
		};

/*
		static const struct {
			const char *var;
			enum { SSI_ENC_UNSET, SSI_ENC_URL, SSI_ENC_NONE, SSI_ENC_ENTITY } type;
		} encvars[] = {
			{ "url",          SSI_ENC_URL },
			{ "none",         SSI_ENC_NONE },
			{ "entity",       SSI_ENC_ENTITY },

			{ NULL, SSI_ENC_UNSET }
		};
*/

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "var")) {
				int j;

				var_val = l[i+1];

				for (j = 0; echovars[j].var; j++) {
					if (0 == strcmp(l[i+1], echovars[j].var)) {
						var = echovars[j].type;
						break;
					}
				}
			} else if (0 == strcmp(l[i], "encoding")) {
/*
				int j;

				for (j = 0; encvars[j].var; j++) {
					if (0 == strcmp(l[i+1], encvars[j].var)) {
						enc = encvars[j].type;
						break;
					}
				}
*/
			} else {
				log_error(r->conf.errh, __FILE__, __LINE__,
				  "ssi: unknown attribute for %s %s", l[1], l[i]);
			}
		}

		if (p->if_is_false) break;

		if (!var_val) {
			log_error(r->conf.errh, __FILE__, __LINE__,
			  "ssi: %s var is missing", l[1]);
			break;
		}

		switch(var) {
		case SSI_ECHO_USER_NAME: {
			tb = r->tmp_buf;
			buffer_clear(tb);
#ifdef HAVE_PWD_H
			struct passwd *pw;
			if (NULL == (pw = getpwuid(st->st_uid))) {
				buffer_append_int(tb, st->st_uid);
			} else {
				buffer_copy_string(tb, pw->pw_name);
			}
#else
			buffer_append_int(tb, st->st_uid);
#endif
			chunkqueue_append_mem(cq, CONST_BUF_LEN(tb));
			break;
		}
		case SSI_ECHO_LAST_MODIFIED:
		case SSI_ECHO_DATE_LOCAL:
		case SSI_ECHO_DATE_GMT: {
			struct tm tm;
			time_t t = (var == SSI_ECHO_LAST_MODIFIED)
			  ? st->st_mtime
			  : time(NULL);
			uint32_t len = strftime(buf, sizeof(buf), p->timefmt->ptr,
			                        (var != SSI_ECHO_DATE_GMT)
			                        ? localtime_r(&t, &tm)
			                        : gmtime_r(&t, &tm));

			if (len)
				chunkqueue_append_mem(cq, buf, len);
			else
				chunkqueue_append_mem(cq, CONST_STR_LEN("(none)"));
			break;
		}
		case SSI_ECHO_DOCUMENT_NAME: {
			char *sl;

			if (NULL == (sl = strrchr(r->physical.path.ptr, '/'))) {
				chunkqueue_append_mem(cq, CONST_BUF_LEN(&r->physical.path));
			} else {
				chunkqueue_append_mem(cq, sl + 1, strlen(sl + 1));
			}
			break;
		}
		case SSI_ECHO_DOCUMENT_URI: {
			chunkqueue_append_mem(cq, CONST_BUF_LEN(&r->uri.path));
			break;
		}
		case SSI_ECHO_SCRIPT_URI: {
			if (!buffer_string_is_empty(&r->uri.scheme) && !buffer_string_is_empty(&r->uri.authority)) {
				chunkqueue_append_mem(cq, CONST_BUF_LEN(&r->uri.scheme));
				chunkqueue_append_mem(cq, CONST_STR_LEN("://"));
				chunkqueue_append_mem(cq, CONST_BUF_LEN(&r->uri.authority));
				chunkqueue_append_mem(cq, CONST_BUF_LEN(&r->target));
				if (!buffer_string_is_empty(&r->uri.query)) {
					chunkqueue_append_mem(cq, CONST_STR_LEN("?"));
					chunkqueue_append_mem(cq, CONST_BUF_LEN(&r->uri.query));
				}
			}
			break;
		}
		case SSI_ECHO_SCRIPT_URL: {
			chunkqueue_append_mem(cq, CONST_BUF_LEN(&r->target));
			if (!buffer_string_is_empty(&r->uri.query)) {
				chunkqueue_append_mem(cq, CONST_STR_LEN("?"));
				chunkqueue_append_mem(cq, CONST_BUF_LEN(&r->uri.query));
			}
			break;
		}
		default: {
			const data_string *ds;
			/* check if it is a cgi-var or a ssi-var */

			if (NULL != (ds = (const data_string *)array_get_element_klen(p->ssi_cgi_env, var_val, strlen(var_val))) ||
			    NULL != (ds = (const data_string *)array_get_element_klen(p->ssi_vars, var_val, strlen(var_val)))) {
				chunkqueue_append_mem(cq, CONST_BUF_LEN(&ds->value));
			} else {
				chunkqueue_append_mem(cq, CONST_STR_LEN("(none)"));
			}

			break;
		}
		}
		break;
	}
	case SSI_INCLUDE:
	case SSI_FLASTMOD:
	case SSI_FSIZE: {
		const char * file_path = NULL, *virt_path = NULL;
		struct stat stb;

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "file")) {
				file_path = l[i+1];
			} else if (0 == strcmp(l[i], "virtual")) {
				virt_path = l[i+1];
			} else {
				log_error(r->conf.errh, __FILE__, __LINE__,
				  "ssi: unknown attribute for %s %s", l[1], l[i]);
			}
		}

		if (!file_path && !virt_path) {
			log_error(r->conf.errh, __FILE__, __LINE__,
			  "ssi: %s file or virtual is missing", l[1]);
			break;
		}

		if (file_path && virt_path) {
			log_error(r->conf.errh, __FILE__, __LINE__,
			  "ssi: %s only one of file and virtual is allowed here", l[1]);
			break;
		}


		if (p->if_is_false) break;

		tb = r->tmp_buf;

		if (file_path) {
			/* current doc-root */
			char *sl = strrchr(r->physical.path.ptr, '/');
			if (NULL == sl) break; /*(not expected)*/
			buffer_copy_string_len(p->stat_fn, r->physical.path.ptr, sl - r->physical.path.ptr + 1);

			buffer_copy_string(tb, file_path);
			buffer_urldecode_path(tb);
			if (!buffer_is_valid_UTF8(tb)) {
				log_error(r->conf.errh, __FILE__, __LINE__,
				  "SSI invalid UTF-8 after url-decode: %s", tb->ptr);
				break;
			}
			buffer_path_simplify(tb, tb);
			buffer_append_path_len(p->stat_fn, CONST_BUF_LEN(tb));
		} else {
			/* virtual */

			if (virt_path[0] == '/') {
				buffer_copy_string(tb, virt_path);
			} else {
				/* there is always a / */
				const char * const sl = strrchr(r->uri.path.ptr, '/');
				buffer_copy_string_len(tb, r->uri.path.ptr, sl - r->uri.path.ptr + 1);
				buffer_append_string(tb, virt_path);
			}

			buffer_urldecode_path(tb);
			if (!buffer_is_valid_UTF8(tb)) {
				log_error(r->conf.errh, __FILE__, __LINE__,
				  "SSI invalid UTF-8 after url-decode: %s", tb->ptr);
				break;
			}
			buffer_path_simplify(tb, tb);

			/* we have an uri */

			/* Destination physical path (similar to code in mod_webdav.c)
			 * src r->physical.path might have been remapped with mod_alias, mod_userdir.
			 *   (but neither modifies r->physical.rel_path)
			 * Find matching prefix to support relative paths to current physical path.
			 * Aliasing of paths underneath current r->physical.basedir might not work.
			 * Likewise, mod_rewrite URL rewriting might thwart this comparison.
			 * Use mod_redirect instead of mod_alias to remap paths *under* this basedir.
			 * Use mod_redirect instead of mod_rewrite on *any* parts of path to basedir.
			 * (Related, use mod_auth to protect this basedir, but avoid attempting to
			 *  use mod_auth on paths underneath this basedir, as target path is not
			 *  validated with mod_auth)
			 */

			/* find matching URI prefix
			 * check if remaining r->physical.rel_path matches suffix
			 *   of r->physical.basedir so that we can use it to
			 *   remap Destination physical path */
			{
				const char *sep, *sep2;
				sep = r->uri.path.ptr;
				sep2 = tb->ptr;
				for (i = 0; sep[i] && sep[i] == sep2[i]; ++i) ;
				while (i != 0 && sep[--i] != '/') ; /* find matching directory path */
			}
			if (r->conf.force_lowercase_filenames) {
				buffer_to_lower(tb);
			}
			uint32_t remain = buffer_string_length(&r->uri.path) - i;
			if (!r->conf.force_lowercase_filenames
			    ? buffer_is_equal_right_len(&r->physical.path, &r->physical.rel_path, remain)
			    :(buffer_string_length(&r->physical.path) >= remain
			      && buffer_eq_icase_ssn(r->physical.path.ptr+buffer_string_length(&r->physical.path)-remain, r->physical.rel_path.ptr+i, remain))) {
				buffer_copy_string_len(p->stat_fn, r->physical.path.ptr, buffer_string_length(&r->physical.path)-remain);
				buffer_append_path_len(p->stat_fn, tb->ptr+i, buffer_string_length(tb)-i);
			} else {
				/* unable to perform physical path remap here;
				 * assume doc_root/rel_path and no remapping */
				buffer_copy_buffer(p->stat_fn, &r->physical.doc_root);
				buffer_append_path_len(p->stat_fn, CONST_BUF_LEN(tb));
			}
		}

		if (!r->conf.follow_symlink
		    && 0 != stat_cache_path_contains_symlink(p->stat_fn, r->conf.errh)) {
			break;
		}

		int fd = stat_cache_open_rdonly_fstat(p->stat_fn, &stb, r->conf.follow_symlink);
		if (fd >= 0) {
			time_t t = stb.st_mtime;

			switch (ssicmd) {
			case SSI_FSIZE:
				buffer_clear(tb);
				if (p->sizefmt) {
					int j = 0;
					const char *abr[] = { " B", " kB", " MB", " GB", " TB", NULL };

					off_t s = stb.st_size;

					for (j = 0; s > 1024 && abr[j+1]; s /= 1024, j++);

					buffer_append_int(tb, s);
					buffer_append_string(tb, abr[j]);
				} else {
					buffer_append_int(tb, stb.st_size);
				}
				chunkqueue_append_mem(cq, CONST_BUF_LEN(tb));
				break;
			case SSI_FLASTMOD: {
				struct tm tm;
				uint32_t len = (uint32_t)strftime(buf, sizeof(buf), p->timefmt->ptr, localtime_r(&t, &tm));
				if (len)
					chunkqueue_append_mem(cq, buf, len);
				else
					chunkqueue_append_mem(cq, CONST_STR_LEN("(none)"));
				break;
			}
			case SSI_INCLUDE:
				/* Keep the newest mtime of included files */
				if (stb.st_mtime > include_file_last_mtime)
					include_file_last_mtime = stb.st_mtime;

				if (file_path || 0 == p->conf.ssi_recursion_max) {
					/* don't process if #include file="..." is used */
					chunkqueue_append_file_fd(cq, p->stat_fn, fd, 0, stb.st_size);
					fd = -1;
				} else {
					buffer upsave, ppsave, prpsave;

					/* only allow predefined recursion depth */
					if (p->ssi_recursion_depth >= p->conf.ssi_recursion_max) {
						chunkqueue_append_mem(cq, CONST_STR_LEN("(error: include directives recurse deeper than pre-defined ssi.recursion-max)"));
						break;
					}

					/* prevents simple infinite loop */
					if (buffer_is_equal(&r->physical.path, p->stat_fn)) {
						chunkqueue_append_mem(cq, CONST_STR_LEN("(error: include directives create an infinite loop)"));
						break;
					}

					/* save and restore r->physical.path, r->physical.rel_path, and r->uri.path around include
					 *
					 * tb contains url-decoded, path-simplified, and lowercased (if r->conf.force_lowercase) uri path of target.
					 * r->uri.path and r->physical.rel_path are set to the same since we only operate on filenames here,
					 * not full re-run of all modules for subrequest */
					upsave = r->uri.path;
					ppsave = r->physical.path;
					prpsave = r->physical.rel_path;

					r->physical.path = *p->stat_fn;
					memset(p->stat_fn, 0, sizeof(buffer));

					memset(&r->uri.path, 0, sizeof(buffer));
					buffer_copy_buffer(&r->uri.path, tb);
					r->physical.rel_path = r->uri.path;

					close(fd);
					fd = -1;

					/*(ignore return value; muddle along as best we can if error occurs)*/
					++p->ssi_recursion_depth;
					mod_ssi_process_file(r, p, &stb);
					--p->ssi_recursion_depth;

					free(r->uri.path.ptr);
					r->uri.path = upsave;
					r->physical.rel_path = prpsave;

					free(p->stat_fn->ptr);
					*p->stat_fn = r->physical.path;
					r->physical.path = ppsave;
				}

				break;
			}

			if (fd >= 0) close(fd);
		} else {
			log_perror(r->conf.errh, __FILE__, __LINE__,
			  "ssi: stating %s failed", p->stat_fn->ptr);
		}
		break;
	}
	case SSI_SET: {
		const char *key = NULL, *val = NULL;
		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "var")) {
				key = l[i+1];
			} else if (0 == strcmp(l[i], "value")) {
				val = l[i+1];
			} else {
				log_error(r->conf.errh, __FILE__, __LINE__,
				  "ssi: unknown attribute for %s %s", l[1], l[i]);
			}
		}

		if (p->if_is_false) break;

		if (key && val) {
			array_set_key_value(p->ssi_vars, key, strlen(key), val, strlen(val));
		} else if (key || val) {
			log_error(r->conf.errh, __FILE__, __LINE__,
			  "ssi: var and value have to be set in <!--#set %s=%s -->", l[1], l[2]);
		} else {
			log_error(r->conf.errh, __FILE__, __LINE__,
			  "ssi: var and value have to be set in <!--#set var=... value=... -->");
		}
		break;
	}
	case SSI_CONFIG:
		if (p->if_is_false) break;

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "timefmt")) {
				buffer_copy_string(p->timefmt, l[i+1]);
			} else if (0 == strcmp(l[i], "sizefmt")) {
				if (0 == strcmp(l[i+1], "abbrev")) {
					p->sizefmt = 1;
				} else if (0 == strcmp(l[i+1], "bytes")) {
					p->sizefmt = 0;
				} else {
					log_error(r->conf.errh, __FILE__, __LINE__,
					  "ssi: unknown value for attribute '%s' for %s %s",
					  l[i], l[1], l[i+1]);
				}
			} else {
				log_error(r->conf.errh, __FILE__, __LINE__,
				  "ssi: unknown attribute for %s %s", l[1], l[i]);
			}
		}
		break;
	case SSI_PRINTENV:
		if (p->if_is_false) break;

		tb = r->tmp_buf;
		buffer_clear(tb);
		for (i = 0; i < p->ssi_vars->used; i++) {
			data_string *ds = (data_string *)p->ssi_vars->sorted[i];

			buffer_append_string_buffer(tb, &ds->key);
			buffer_append_string_len(tb, CONST_STR_LEN("="));
			buffer_append_string_encoded(tb, CONST_BUF_LEN(&ds->value), ENCODING_MINIMAL_XML);
			buffer_append_string_len(tb, CONST_STR_LEN("\n"));
		}
		for (i = 0; i < p->ssi_cgi_env->used; i++) {
			data_string *ds = (data_string *)p->ssi_cgi_env->sorted[i];

			buffer_append_string_buffer(tb, &ds->key);
			buffer_append_string_len(tb, CONST_STR_LEN("="));
			buffer_append_string_encoded(tb, CONST_BUF_LEN(&ds->value), ENCODING_MINIMAL_XML);
			buffer_append_string_len(tb, CONST_STR_LEN("\n"));
		}
		chunkqueue_append_mem(cq, CONST_BUF_LEN(tb));
		break;
	case SSI_EXEC: {
		const char *cmd = NULL;
		pid_t pid;
		chunk *c;
		char *args[4];
		log_error_st *errh = p->errh;

		if (!p->conf.ssi_exec) { /* <!--#exec ... --> disabled by config */
			break;
		}

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "cmd")) {
				cmd = l[i+1];
			} else {
				log_error(errh, __FILE__, __LINE__,
				  "ssi: unknown attribute for %s %s", l[1], l[i]);
			}
		}

		if (p->if_is_false) break;

		/*
		 * as exec is assumed evil it is implemented synchronously
		 */

		if (!cmd) break;

		/* send cmd output to a temporary file */
		if (0 != chunkqueue_append_mem_to_tempfile(cq, "", 0, errh)) break;
		c = cq->last;

		*(const char **)&args[0] = "/bin/sh";
		*(const char **)&args[1] = "-c";
		*(const char **)&args[2] = cmd;
		args[3] = NULL;

		/*(expects STDIN_FILENO open to /dev/null)*/
		int serrh_fd = r->conf.serrh ? r->conf.serrh->errorlog_fd : -1;
		pid = fdevent_fork_execve(args[0], args, NULL, -1, c->file.fd, serrh_fd, -1);
		if (-1 == pid) {
			log_perror(errh, __FILE__, __LINE__, "spawning exec failed: %s", cmd);
		} else {
			struct stat stb;
			int status = 0;

			/* wait for the client to end */
			/* NOTE: synchronous; blocks entire lighttpd server */

			/*
			 * OpenBSD and Solaris send a EINTR on SIGCHILD even if we ignore it
			 */
			if (fdevent_waitpid(pid, &status, 0) < 0) {
				log_perror(errh, __FILE__, __LINE__, "waitpid failed");
				break;
			}
			if (!WIFEXITED(status)) {
				log_error(errh, __FILE__, __LINE__, "process exited abnormally: %s", cmd);
			}
			if (0 == fstat(c->file.fd, &stb)) {
				chunkqueue_update_file(cq, c, stb.st_size);
			}
		}

		break;
	}
	case SSI_IF: {
		const char *expr = NULL;

		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "expr")) {
				expr = l[i+1];
			} else {
				log_error(r->conf.errh, __FILE__, __LINE__,
				  "ssi: unknown attribute for %s %s", l[1], l[i]);
			}
		}

		if (!expr) {
			log_error(r->conf.errh, __FILE__, __LINE__,
			  "ssi: %s expr missing", l[1]);
			break;
		}

		if ((!p->if_is_false) &&
		    ((p->if_is_false_level == 0) ||
		     (p->if_level < p->if_is_false_level))) {
			switch (ssi_eval_expr(p, expr)) {
			case -1:
			case 0:
				p->if_is_false = 1;
				p->if_is_false_level = p->if_level;
				break;
			case 1:
				p->if_is_false = 0;
				break;
			}
		}

		p->if_level++;

		break;
	}
	case SSI_ELSE:
		p->if_level--;

		if (p->if_is_false) {
			if ((p->if_level == p->if_is_false_level) &&
			    (p->if_is_false_endif == 0)) {
				p->if_is_false = 0;
			}
		} else {
			p->if_is_false = 1;

			p->if_is_false_level = p->if_level;
		}
		p->if_level++;

		break;
	case SSI_ELIF: {
		const char *expr = NULL;
		for (i = 2; i < n; i += 2) {
			if (0 == strcmp(l[i], "expr")) {
				expr = l[i+1];
			} else {
				log_error(r->conf.errh, __FILE__, __LINE__,
				  "ssi: unknown attribute for %s %s", l[1], l[i]);
			}
		}

		if (!expr) {
			log_error(r->conf.errh, __FILE__, __LINE__,
			  "ssi: %s expr missing", l[1]);
			break;
		}

		p->if_level--;

		if (p->if_level == p->if_is_false_level) {
			if ((p->if_is_false) &&
			    (p->if_is_false_endif == 0)) {
				switch (ssi_eval_expr(p, expr)) {
				case -1:
				case 0:
					p->if_is_false = 1;
					p->if_is_false_level = p->if_level;
					break;
				case 1:
					p->if_is_false = 0;
					break;
				}
			} else {
				p->if_is_false = 1;
				p->if_is_false_level = p->if_level;
				p->if_is_false_endif = 1;
			}
		}

		p->if_level++;

		break;
	}
	case SSI_ENDIF:
		p->if_level--;

		if (p->if_level == p->if_is_false_level) {
			p->if_is_false = 0;
			p->if_is_false_endif = 0;
		}

		break;
	case SSI_COMMENT:
		break;
	default:
		log_error(r->conf.errh, __FILE__, __LINE__,
		  "ssi: unknown ssi-command: %s", l[1]);
		break;
	}

	return 0;

}

static int mod_ssi_parse_ssi_stmt_value(const unsigned char * const s, const int len) {
	int n;
	const int c = (s[0] == '"' ? '"' : s[0] == '\'' ? '\'' : 0);
	if (0 != c) {
		for (n = 1; n < len; ++n) {
			if (s[n] == c) return n+1;
			if (s[n] == '\\') {
				if (n+1 == len) return 0; /* invalid */
				++n;
			}
		}
		return 0; /* invalid */
	} else {
		for (n = 0; n < len; ++n) {
			if (isspace(s[n])) return n;
			if (s[n] == '\\') {
				if (n+1 == len) return 0; /* invalid */
				++n;
			}
		}
		return n;
	}
}

static int mod_ssi_parse_ssi_stmt_offlen(int o[10], const unsigned char * const s, const int len) {

	/**
	 * <!--#element attribute=value attribute=value ... -->
	 */

	/* s must begin "<!--#" and must end with "-->" */
	int n = 5;
	o[0] = n;
	for (; light_isalpha(s[n]); ++n) ; /*(n = 5 to begin after "<!--#")*/
	o[1] = n - o[0];
	if (0 == o[1]) return -1; /* empty token */

	if (n+3 == len) return 2; /* token only; no params */
	if (!isspace(s[n])) return -1;
	do { ++n; } while (isspace(s[n])); /* string ends "-->", so n < len */
	if (n+3 == len) return 2; /* token only; no params */

	o[2] = n;
	for (; light_isalpha(s[n]); ++n) ;
	o[3] = n - o[2];
	if (0 == o[3] || s[n++] != '=') return -1;

	o[4] = n;
	o[5] = mod_ssi_parse_ssi_stmt_value(s+n, len-n-3);
	if (0 == o[5]) return -1; /* empty or invalid token */
	n += o[5];

	if (n+3 == len) return 6; /* token and one param */
	if (!isspace(s[n])) return -1;
	do { ++n; } while (isspace(s[n])); /* string ends "-->", so n < len */
	if (n+3 == len) return 6; /* token and one param */

	o[6] = n;
	for (; light_isalpha(s[n]); ++n) ;
	o[7] = n - o[6];
	if (0 == o[7] || s[n++] != '=') return -1;

	o[8] = n;
	o[9] = mod_ssi_parse_ssi_stmt_value(s+n, len-n-3);
	if (0 == o[9]) return -1; /* empty or invalid token */
	n += o[9];

	if (n+3 == len) return 10; /* token and two params */
	if (!isspace(s[n])) return -1;
	do { ++n; } while (isspace(s[n])); /* string ends "-->", so n < len */
	if (n+3 == len) return 10; /* token and two params */
	return -1;
}

static void mod_ssi_parse_ssi_stmt(request_st * const r, handler_ctx * const p, char * const s, int len, struct stat * const st) {

	/**
	 * <!--#element attribute=value attribute=value ... -->
	 */

	int o[10];
	int m;
	const int n = mod_ssi_parse_ssi_stmt_offlen(o, (unsigned char *)s, len);
	char *l[6] = { s, NULL, NULL, NULL, NULL, NULL };
	if (-1 == n) {
		/* ignore <!--#comment ... --> */
		if (len >= 16
		    && 0 == memcmp(s+5, "comment", sizeof("comment")-1)
		    && (s[12] == ' ' || s[12] == '\t'))
			return;
		/* XXX: perhaps emit error comment instead of invalid <!--#...--> code to client */
		chunkqueue_append_mem(&r->write_queue, s, len); /* append stmt as-is */
		return;
	}

      #if 0
	/* dup s and then modify s */
	/*(l[0] is no longer used; was previously used in only one place for error reporting)*/
	l[0] = malloc((size_t)(len+1));
	memcpy(l[0], s, (size_t)len);
	(l[0])[len] = '\0';
      #endif

	/* modify s in-place to split string into arg tokens */
	for (m = 0; m < n; m += 2) {
		char *ptr = s+o[m];
		switch (*ptr) {
		case '"':
		case '\'': (++ptr)[o[m+1]-2] = '\0'; break;
		default:       ptr[o[m+1]] = '\0';   break;
		}
		l[1+(m>>1)] = ptr;
		if (m == 4 || m == 8) {
			/* XXX: removing '\\' escapes from param value would be
			 * the right thing to do, but would potentially change
			 * current behavior, e.g. <!--#exec cmd=... --> */
		}
	}

	process_ssi_stmt(r, p, (const char **)l, 1+(n>>1), st);

      #if 0
	free(l[0]);
      #endif
}

static int mod_ssi_stmt_len(const char *s, const int len) {
	/* s must begin "<!--#" */
	int n, sq = 0, dq = 0, bs = 0;
	for (n = 5; n < len; ++n) { /*(n = 5 to begin after "<!--#")*/
		switch (s[n]) {
		default:
			break;
		case '-':
			if (!sq && !dq && n+2 < len && s[n+1] == '-' && s[n+2] == '>') return n+3; /* found end of stmt */
			break;
		case '"':
			if (!sq && (!dq || !bs)) dq = !dq;
			break;
		case '\'':
			if (!dq && (!sq || !bs)) sq = !sq;
			break;
		case '\\':
			if (sq || dq) bs = !bs;
			break;
		}
	}
	return 0; /* incomplete directive "<!--#...-->" */
}

static void mod_ssi_read_fd(request_st * const r, handler_ctx * const p, struct stat * const st, int fd) {
	ssize_t rd;
	size_t offset, pretag;
	const size_t bufsz = 8192;
	char * const buf = malloc(bufsz); /* allocate to reduce chance of stack exhaustion upon deep recursion */
	chunkqueue * const cq = &r->write_queue;
	force_assert(buf);

	offset = 0;
	pretag = 0;
	while (0 < (rd = read(fd, buf+offset, bufsz-offset))) {
		char *s;
		size_t prelen = 0, len;
		offset += (size_t)rd;
		for (; (s = memchr(buf+prelen, '<', offset-prelen)); ++prelen) {
			prelen = s - buf;
			if (prelen + 5 <= offset) { /*("<!--#" is 5 chars)*/
				if (0 != memcmp(s+1, CONST_STR_LEN("!--#"))) continue; /* loop to loop for next '<' */

				if (prelen - pretag && !p->if_is_false) {
					chunkqueue_append_mem(cq, buf+pretag, prelen-pretag);
				}

				len = mod_ssi_stmt_len(buf+prelen, offset-prelen);
				if (len) { /* num of chars to be consumed */
					mod_ssi_parse_ssi_stmt(r, p, buf+prelen, len, st);
					prelen += (len - 1); /* offset to '>' at end of SSI directive; incremented at top of loop */
					pretag = prelen + 1;
					if (pretag == offset) {
						offset = pretag = 0;
						break;
					}
				} else if (0 == prelen && offset == bufsz) { /*(full buf)*/
					/* SSI statement is way too long
					 * NOTE: skipping this buf will expose *the rest* of this SSI statement */
					chunkqueue_append_mem(cq, CONST_STR_LEN("<!-- [an error occurred: directive too long] "));
					/* check if buf ends with "-" or "--" which might be part of "-->"
					 * (buf contains at least 5 chars for "<!--#") */
					if (buf[offset-2] == '-' && buf[offset-1] == '-') {
						chunkqueue_append_mem(cq, CONST_STR_LEN("--"));
					} else if (buf[offset-1] == '-') {
						chunkqueue_append_mem(cq, CONST_STR_LEN("-"));
					}
					offset = pretag = 0;
					break;
				} else { /* incomplete directive "<!--#...-->" */
					memmove(buf, buf+prelen, (offset -= prelen));
					pretag = 0;
					break;
				}
			} else if (prelen + 1 == offset || 0 == memcmp(s+1, "!--", offset - prelen - 1)) {
				if (prelen - pretag && !p->if_is_false) {
					chunkqueue_append_mem(cq, buf+pretag, prelen-pretag);
				}
				memcpy(buf, buf+prelen, (offset -= prelen));
				pretag = 0;
				break;
			}
			/* loop to look for next '<' */
		}
		if (offset == bufsz) {
			if (!p->if_is_false) {
				chunkqueue_append_mem(cq, buf+pretag, offset-pretag);
			}
			offset = pretag = 0;
		}
	}

	if (0 != rd) {
		log_perror(r->conf.errh, __FILE__, __LINE__,
		  "read(): %s", r->physical.path.ptr);
	}

	if (offset - pretag) {
		/* copy remaining data in buf */
		if (!p->if_is_false) {
			chunkqueue_append_mem(cq, buf+pretag, offset-pretag);
		}
	}

	free(buf);
}


static int mod_ssi_process_file(request_st * const r, handler_ctx * const p, struct stat * const st) {
	int fd = stat_cache_open_rdonly_fstat(&r->physical.path, st, r->conf.follow_symlink);
	if (-1 == fd) {
		log_perror(r->conf.errh, __FILE__, __LINE__,
		  "open(): %s", r->physical.path.ptr);
		return -1;
	}

	mod_ssi_read_fd(r, p, st, fd);

	close(fd);
	return 0;
}


static int mod_ssi_handle_request(request_st * const r, handler_ctx * const p) {
	struct stat st;

	/* get a stream to the file */

	array_reset_data_strings(p->ssi_vars);
	array_reset_data_strings(p->ssi_cgi_env);
	buffer_copy_string_len(p->timefmt, CONST_STR_LEN("%a, %d %b %Y %H:%M:%S %Z"));
	build_ssi_cgi_vars(r, p);

	/* Reset the modified time of included files */
	include_file_last_mtime = 0;

	if (mod_ssi_process_file(r, p, &st)) return -1;

	r->resp_body_started  = 1;
	r->resp_body_finished = 1;

	if (buffer_string_is_empty(p->conf.content_type)) {
		http_header_response_set(r, HTTP_HEADER_CONTENT_TYPE, CONST_STR_LEN("Content-Type"), CONST_STR_LEN("text/html"));
	} else {
		http_header_response_set(r, HTTP_HEADER_CONTENT_TYPE, CONST_STR_LEN("Content-Type"), CONST_BUF_LEN(p->conf.content_type));
	}

	if (p->conf.conditional_requests) {
		/* Generate "ETag" & "Last-Modified" headers */

		/* use most recently modified include file for ETag and Last-Modified */
		if (st.st_mtime < include_file_last_mtime)
			st.st_mtime = include_file_last_mtime;

		http_etag_create(&r->physical.etag, &st, r->conf.etag_flags);
		http_header_response_set(r, HTTP_HEADER_ETAG, CONST_STR_LEN("ETag"), CONST_BUF_LEN(&r->physical.etag));

		const buffer * const mtime = http_response_set_last_modified(r, st.st_mtime);
		if (HANDLER_FINISHED == http_response_handle_cachable(r, mtime, st.st_mtime)) {
			/* ok, the client already has our content,
			 * no need to send it again */

			chunkqueue_reset(&r->write_queue);
		}
	}

	/* Reset the modified time of included files */
	include_file_last_mtime = 0;

	/* reset physical.path */
	buffer_reset(&r->physical.path);

	return 0;
}

URIHANDLER_FUNC(mod_ssi_physical_path) {
	plugin_data *p = p_d;

	if (NULL != r->handler_module) return HANDLER_GO_ON;
	if (buffer_is_empty(&r->physical.path)) return HANDLER_GO_ON;

	mod_ssi_patch_config(r, p);
	if (NULL == p->conf.ssi_extension) return HANDLER_GO_ON;

	if (array_match_value_suffix(p->conf.ssi_extension, &r->physical.path)) {
		r->plugin_ctx[p->id] = handler_ctx_init(p, r->conf.errh);
		r->handler_module = p->self;
	}

	return HANDLER_GO_ON;
}

SUBREQUEST_FUNC(mod_ssi_handle_subrequest) {
	plugin_data *p = p_d;
	handler_ctx *hctx = r->plugin_ctx[p->id];
	if (NULL == hctx) return HANDLER_GO_ON;
	/*
	 * NOTE: if mod_ssi modified to use fdevents, HANDLER_WAIT_FOR_EVENT,
	 * instead of blocking to completion, then hctx->timefmt, hctx->ssi_vars,
	 * and hctx->ssi_cgi_env should be allocated and cleaned up per request.
	 */

			/* handle ssi-request */

			if (mod_ssi_handle_request(r, hctx)) {
				/* on error */
				r->http_status = 500;
				r->handler_module = NULL;
			}

			return HANDLER_FINISHED;
}

static handler_t mod_ssi_handle_request_reset(request_st * const r, void *p_d) {
	plugin_data *p = p_d;
	handler_ctx *hctx = r->plugin_ctx[p->id];
	if (hctx) {
		handler_ctx_free(hctx);
		r->plugin_ctx[p->id] = NULL;
	}

	return HANDLER_GO_ON;
}


int mod_ssi_plugin_init(plugin *p);
int mod_ssi_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = "ssi";

	p->init        = mod_ssi_init;
	p->handle_subrequest_start = mod_ssi_physical_path;
	p->handle_subrequest       = mod_ssi_handle_subrequest;
	p->handle_request_reset    = mod_ssi_handle_request_reset;
	p->set_defaults  = mod_ssi_set_defaults;
	p->cleanup     = mod_ssi_free;

	return 0;
}
