/* mod_deflate
 *
 *
 * bug fix on Robert Jakabosky from alphatrade.com's lighttp 1.4.10 mod_deflate patch
 *
 * Bug fix and new features:
 * 1) fix loop bug when content-length is bigger than work-block-size*k
 *
 * -------
 *
 * lighttpd-1.4.26.mod_deflate.patch from
 *   https://redmine.lighttpd.net/projects/1/wiki/Docs_ModDeflate
 *
 * -------
 *
 * Patch further modified in this incarnation.
 *
 * Note: this patch only handles completed responses
 *         (r->resp_body_finished)
 *       this patch does not currently handle streaming dynamic responses,
 *       and therefore also does not worry about Transfer-Encoding: chunked
 *       (or having separate con->output_queue for chunked-encoded output)
 *       (or using separate buffers per connection instead of p->tmp_buf)
 *       (or handling interactions with block buffering and write timeouts)
 *
 * Bug fix:
 * - fixed major bug with compressing chunks with offset > 0
 *     x-ref:
 *       "Response breaking in mod_deflate"
 *       https://redmine.lighttpd.net/issues/986
 * - fix broken (in some cases) chunk accounting in deflate_compress_response()
 * - fix broken bzip2
 *     x-ref:
 *       "mod_deflate's bzip2 broken by default"
 *       https://redmine.lighttpd.net/issues/2035
 * - fix mismatch with current chunk interfaces
 *     x-ref:
 *       "Weird things in chunk.c (functions only handling specific cases, unexpected behaviour)"
 *       https://redmine.lighttpd.net/issues/1510
 *
 * Behavior changes from prior patch:
 * - deflate.mimetypes must now be configured to enable compression
 *     deflate.mimetypes = ( )          # compress nothing (disabled; default)
 *     deflate.mimetypes = ( "" )       # compress all mimetypes
 *     deflate.mimetypes = ( "text/" )  # compress text/... mimetypes
 *     x-ref:
 *       "mod_deflate enabled by default"
 *       https://redmine.lighttpd.net/issues/1394
 * - deflate.enabled directive removed (see new behavior of deflate.mimetypes)
 * - deflate.debug removed (was developer debug trace, not end-user debug)
 * - deflate.bzip2 replaced with deflate.allowed-encodings (like mod_compress)
 *     x-ref:
 *       "mod_deflate should allow limiting of compression algorithm from the configuration file"
 *       https://redmine.lighttpd.net/issues/996
 *       "mod_compress disabling methods"
 *       https://redmine.lighttpd.net/issues/1773
 * - deflate.nocompress-url removed since disabling compression for a URL
 *   can now easily be done by setting to a blank list either directive
 *   deflate.accept_encodings = () or deflate.mimetypes = () in a conditional
 *   block, e.g. $HTTP["url"] =~ "....." { deflate.mimetypes = ( ) }
 * - deflate.sync-flush removed; controlled by r->conf.stream_response_body
 *     (though streaming compression not currently implemented in mod_deflate)
 * - inactive directives in this patch
 *       (since r->resp_body_finished required)
 *     deflate.work-block-size
 *     deflate.output-buffer-size
 * - remove weak file size check; SIGBUS is trapped, file that shrink will error
 *     x-ref:
 *       "mod_deflate: filesize check is too weak"
 *       https://redmine.lighttpd.net/issues/1512
 * - change default deflate.min-compress-size from 0 to now be 256
 *   http://webmasters.stackexchange.com/questions/31750/what-is-recommended-minimum-object-size-for-gzip-performance-benefits
 *   Apache 2.4 mod_deflate minimum is 68 bytes
 *   Akamai recommends minimum 860 bytes
 *   Google recommends minimum be somewhere in range between 150 and 1024 bytes
 * - deflate.max-compress-size new directive (in kb like compress.max_filesize)
 * - deflate.mem-level removed (too many knobs for little benefit)
 * - deflate.window-size removed (too many knobs for little benefit)
 *
 * Future:
 * - config directives may be changed, renamed, or removed
 *   e.g. A set of reasonable defaults might be chosen
 *        instead of making them configurable.
 *     deflate.min-compress-size
 * - might add deflate.mimetypes-exclude = ( ... ) for list of mimetypes
 *   to avoid compressing, even if a broader deflate.mimetypes matched,
 *   e.g. to compress all "text/" except "text/special".
 *
 * Implementation notes:
 * - http_chunk_append_mem() used instead of http_chunk_append_buffer()
 *   so that p->tmp_buf can be large and re-used.  This results in an extra copy
 *   of compressed data before data is sent to network, though if the compressed
 *   size is larger than 64k, it ends up being sent to a temporary file on
 *   disk without suffering an extra copy in memory, and without extra chunk
 *   create and destroy.  If this is ever changed to give away buffers, then use
 *   a unique hctx->output buffer per hctx; do not reuse p->tmp_buf across
 *   multiple requests being handled in parallel.
 */
#include "first.h"

#include <sys/types.h>
#include <sys/stat.h>
#include "sys-mmap.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>     /* getpid() read() unlink() write() */

#include "base.h"
#include "fdevent.h"
#include "log.h"
#include "buffer.h"
#include "http_chunk.h"
#include "http_etag.h"
#include "http_header.h"
#include "response.h"
#include "stat_cache.h"

#include "plugin.h"

#if defined HAVE_ZLIB_H && defined HAVE_LIBZ
# define USE_ZLIB
# include <zlib.h>
#endif
#ifndef Z_DEFAULT_COMPRESSION
#define Z_DEFAULT_COMPRESSION -1
#endif
#ifndef MAX_WBITS
#define MAX_WBITS 15
#endif

#if defined HAVE_BZLIB_H && defined HAVE_LIBBZ2
# define USE_BZ2LIB
/* we don't need stdio interface */
# define BZ_NO_STDIO
# include <bzlib.h>
#endif

#if defined HAVE_BROTLI_ENCODE_H && defined HAVE_BROTLI
# define USE_BROTLI
# include <brotli/encode.h>
#endif

#if defined HAVE_ZSTD_H && defined HAVE_ZSTD
# define USE_ZSTD
# include <zstd.h>
#endif

#if defined HAVE_SYS_MMAN_H && defined HAVE_MMAP && defined ENABLE_MMAP
#define USE_MMAP

#include "sys-mmap.h"
#include <setjmp.h>
#include <signal.h>

static volatile int sigbus_jmp_valid;
static sigjmp_buf sigbus_jmp;

static void sigbus_handler(int sig) {
	UNUSED(sig);
	if (sigbus_jmp_valid) siglongjmp(sigbus_jmp, 1);
	log_failed_assert(__FILE__, __LINE__, "SIGBUS");
}
#endif

/* request: accept-encoding */
#define HTTP_ACCEPT_ENCODING_IDENTITY BV(0)
#define HTTP_ACCEPT_ENCODING_GZIP     BV(1)
#define HTTP_ACCEPT_ENCODING_DEFLATE  BV(2)
#define HTTP_ACCEPT_ENCODING_COMPRESS BV(3)
#define HTTP_ACCEPT_ENCODING_BZIP2    BV(4)
#define HTTP_ACCEPT_ENCODING_X_GZIP   BV(5)
#define HTTP_ACCEPT_ENCODING_X_BZIP2  BV(6)
#define HTTP_ACCEPT_ENCODING_BR       BV(7)
#define HTTP_ACCEPT_ENCODING_ZSTD     BV(8)

typedef struct {
	const array	*mimetypes;
	const buffer    *cache_dir;
	unsigned int	max_compress_size;
	unsigned short	min_compress_size;
	unsigned short	output_buffer_size;
	unsigned short	work_block_size;
	unsigned short	sync_flush;
	short		compression_level;
	short		allowed_encodings;
	double		max_loadavg;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config defaults;
    plugin_config conf;

    buffer tmp_buf;
} plugin_data;

typedef struct {
	union {
	      #ifdef USE_ZLIB
		z_stream z;
	      #endif
	      #ifdef USE_BZ2LIB
		bz_stream bz;
	      #endif
	      #ifdef USE_BROTLI
		BrotliEncoderState *br;
	      #endif
	      #ifdef USE_ZSTD
		ZSTD_CStream *cctx;
	      #endif
		int dummy;
	} u;
	off_t bytes_in;
	off_t bytes_out;
	buffer *output;
	plugin_data *plugin_data;
	request_st *r;
	int compression_type;
	int cache_fd;
	char *cache_fn;
	chunkqueue in_queue;
} handler_ctx;

static handler_ctx *handler_ctx_init() {
	handler_ctx *hctx;

	hctx = calloc(1, sizeof(*hctx));
	chunkqueue_init(&hctx->in_queue);
	hctx->cache_fd = -1;

	return hctx;
}

static void handler_ctx_free(handler_ctx *hctx) {
	if (hctx->cache_fn) {
		unlink(hctx->cache_fn);
		free(hctx->cache_fn);
	}
	if (-1 != hctx->cache_fd)
		close(hctx->cache_fd);
      #if 0
	if (hctx->output != &p->tmp_buf) {
		buffer_free(hctx->output);
	}
      #endif
	chunkqueue_reset(&hctx->in_queue);
	free(hctx);
}

INIT_FUNC(mod_deflate_init) {
    plugin_data * const p = calloc(1, sizeof(plugin_data));
  #ifdef USE_ZSTD
    buffer_string_prepare_copy(&p->tmp_buf, ZSTD_CStreamOutSize());
  #else
    buffer_string_prepare_copy(&p->tmp_buf, 65536);
  #endif
    return p;
}

FREE_FUNC(mod_deflate_free) {
    plugin_data *p = p_d;
    free(p->tmp_buf.ptr);
}

#if defined(_WIN32) && !defined(__CYGWIN__)
#define mkdir(x,y) mkdir(x)
#endif

static int mkdir_for_file (char *fn) {
    for (char *p = fn; (p = strchr(p + 1, '/')) != NULL; ) {
        if (p[1] == '\0') return 0; /* ignore trailing slash */
        *p = '\0';
        int rc = mkdir(fn, 0700);
        *p = '/';
        if (0 != rc && errno != EEXIST) return -1;
    }
    return 0;
}

static int mkdir_recursive (char *dir) {
    return 0 == mkdir_for_file(dir) && (0 == mkdir(dir,0700) || errno == EEXIST)
      ? 0
      : -1;
}

static buffer * mod_deflate_cache_file_name(request_st * const r, const buffer *cache_dir, const buffer * const etag) {
    /* XXX: future: for shorter paths into the cache, we could checksum path,
     *      and then shard it to avoid a huge single directory.
     *      Alternatively, could use &r->uri.path, minus any
     *      (matching) &r->pathinfo suffix, with result url-encoded */
    buffer * const tb = r->tmp_buf;
    buffer_copy_buffer(tb, cache_dir);
    buffer_append_string_len(tb, CONST_BUF_LEN(&r->physical.path));
    buffer_append_string_len(tb, CONST_STR_LEN("-"));
    buffer_append_string_len(tb, etag->ptr+1, buffer_string_length(etag)-2);
    return tb;
}

static void mod_deflate_cache_file_open (handler_ctx * const hctx, const buffer * const fn) {
    /* race exists whereby up to # workers might attempt to compress same
     * file at same time if requested at same time, but this is unlikely
     * and resolves itself by atomic rename into place when done */
    const uint32_t fnlen = buffer_string_length(fn);
    hctx->cache_fn = malloc(fnlen+1+LI_ITOSTRING_LENGTH+1);
    force_assert(hctx->cache_fn);
    memcpy(hctx->cache_fn, fn->ptr, fnlen);
    hctx->cache_fn[fnlen] = '.';
    const size_t ilen =
      li_itostrn(hctx->cache_fn+fnlen+1, LI_ITOSTRING_LENGTH, getpid());
    hctx->cache_fn[fnlen+1+ilen] = '\0';
    hctx->cache_fd = fdevent_open_cloexec(hctx->cache_fn, 1, O_RDWR|O_CREAT, 0600);
    if (-1 == hctx->cache_fd) {
        free(hctx->cache_fn);
        hctx->cache_fn = NULL;
    }
}

static int mod_deflate_cache_file_finish (request_st * const r, handler_ctx * const hctx, const buffer * const fn) {
    if (0 != fdevent_rename(hctx->cache_fn, fn->ptr))
        return -1;
    free(hctx->cache_fn);
    hctx->cache_fn = NULL;
    chunkqueue_reset(&r->write_queue);
    int rc = http_chunk_append_file_fd(r, fn, hctx->cache_fd, hctx->bytes_out);
    hctx->cache_fd = -1;
    return rc;
}

static void mod_deflate_merge_config_cpv(plugin_config * const pconf, const config_plugin_value_t * const cpv) {
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* deflate.mimetypes */
        pconf->mimetypes = cpv->v.a;
        break;
      case 1: /* deflate.allowed-encodings */
        pconf->allowed_encodings = (short)cpv->v.shrt;
        break;
      case 2: /* deflate.max-compress-size */
        pconf->max_compress_size = cpv->v.u;
        break;
      case 3: /* deflate.min-compress-size */
        pconf->min_compress_size = cpv->v.shrt;
        break;
      case 4: /* deflate.compression-level */
        pconf->compression_level = (short)cpv->v.shrt;
        break;
      case 5: /* deflate.output-buffer-size */
        pconf->output_buffer_size = cpv->v.shrt;
        break;
      case 6: /* deflate.work-block-size */
        pconf->work_block_size = cpv->v.shrt;
        break;
      case 7: /* deflate.max-loadavg */
        pconf->max_loadavg = cpv->v.d;
        break;
      case 8: /* deflate.cache-dir */
        pconf->cache_dir = cpv->v.b;
        break;
      case 9: /* compress.filetype */
        pconf->mimetypes = cpv->v.a;
        break;
      case 10:/* compress.allowed-encodings */
        pconf->allowed_encodings = (short)cpv->v.shrt;
        break;
      case 11:/* compress.cache-dir */
        pconf->cache_dir = cpv->v.b;
        break;
      case 12:/* compress.max-filesize */
        pconf->max_compress_size = cpv->v.u;
        break;
      case 13:/* compress.max-loadavg */
        pconf->max_loadavg = cpv->v.d;
        break;
      default:/* should not happen */
        return;
    }
}

static void mod_deflate_merge_config(plugin_config * const pconf, const config_plugin_value_t *cpv) {
    do {
        mod_deflate_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}

static void mod_deflate_patch_config(request_st * const r, plugin_data * const p) {
    memcpy(&p->conf, &p->defaults, sizeof(plugin_config));
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_deflate_merge_config(&p->conf, p->cvlist + p->cvlist[i].v.u2[0]);
    }
}

static short mod_deflate_encodings_to_flags(const array *encodings) {
    short allowed_encodings = 0;
    if (encodings->used) {
        for (uint32_t j = 0; j < encodings->used; ++j) {
          #if defined(USE_ZLIB) || defined(USE_BZ2LIB) || defined(USE_BROTLI) \
           || defined(USE_ZSTD)
            data_string *ds = (data_string *)encodings->data[j];
          #endif
          #ifdef USE_ZLIB
            if (NULL != strstr(ds->value.ptr, "gzip"))
                allowed_encodings |= HTTP_ACCEPT_ENCODING_GZIP
                                  |  HTTP_ACCEPT_ENCODING_X_GZIP;
            if (NULL != strstr(ds->value.ptr, "x-gzip"))
                allowed_encodings |= HTTP_ACCEPT_ENCODING_X_GZIP;
            if (NULL != strstr(ds->value.ptr, "deflate"))
                allowed_encodings |= HTTP_ACCEPT_ENCODING_DEFLATE;
            /*
            if (NULL != strstr(ds->value.ptr, "compress"))
                allowed_encodings |= HTTP_ACCEPT_ENCODING_COMPRESS;
            */
          #endif
          #ifdef USE_BZ2LIB
            if (NULL != strstr(ds->value.ptr, "bzip2"))
                allowed_encodings |= HTTP_ACCEPT_ENCODING_BZIP2
                                  |  HTTP_ACCEPT_ENCODING_X_BZIP2;
            if (NULL != strstr(ds->value.ptr, "x-bzip2"))
                allowed_encodings |= HTTP_ACCEPT_ENCODING_X_BZIP2;
          #endif
          #ifdef USE_BROTLI
            if (NULL != strstr(ds->value.ptr, "br"))
                allowed_encodings |= HTTP_ACCEPT_ENCODING_BR;
          #endif
          #ifdef USE_ZSTD
            if (NULL != strstr(ds->value.ptr, "zstd"))
                allowed_encodings |= HTTP_ACCEPT_ENCODING_ZSTD;
          #endif
        }
    }
    else {
        /* default encodings */
      #ifdef USE_ZLIB
        allowed_encodings |= HTTP_ACCEPT_ENCODING_GZIP
                          |  HTTP_ACCEPT_ENCODING_X_GZIP
                          |  HTTP_ACCEPT_ENCODING_DEFLATE;
      #endif
      #ifdef USE_BZ2LIB
        allowed_encodings |= HTTP_ACCEPT_ENCODING_BZIP2
                          |  HTTP_ACCEPT_ENCODING_X_BZIP2;
      #endif
      #ifdef USE_BROTLI
        allowed_encodings |= HTTP_ACCEPT_ENCODING_BR;
      #endif
      #ifdef USE_ZSTD
        allowed_encodings |= HTTP_ACCEPT_ENCODING_ZSTD;
      #endif
    }
    return allowed_encodings;
}

SETDEFAULTS_FUNC(mod_deflate_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("deflate.mimetypes"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("deflate.allowed-encodings"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("deflate.max-compress-size"),
        T_CONFIG_INT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("deflate.min-compress-size"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("deflate.compression-level"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("deflate.output-buffer-size"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("deflate.work-block-size"),
        T_CONFIG_SHORT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("deflate.max-loadavg"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("deflate.cache-dir"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("compress.filetype"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("compress.allowed-encodings"),
        T_CONFIG_ARRAY_VLIST,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("compress.cache-dir"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("compress.max-filesize"),
        T_CONFIG_INT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("compress.max-loadavg"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_deflate"))
        return HANDLER_ERROR;

    /* process and validate config directives
     * (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1]; i < p->nconfig; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        for (; -1 != cpv->k_id; ++cpv) {
            switch (cpv->k_id) {
              case 0: /* deflate.mimetypes */
                /* mod_deflate matches mimetype as prefix of Content-Type
                 * so ignore '*' at end of mimetype for end-user flexibility
                 * in specifying trailing wildcard to grouping of mimetypes */
                for (uint32_t m = 0; m < cpv->v.a->used; ++m) {
                    buffer *mimetype=&((data_string *)cpv->v.a->data[m])->value;
                    size_t len = buffer_string_length(mimetype);
                    if (len > 2 && mimetype->ptr[len-1] == '*')
                        buffer_string_set_length(mimetype, len-1);
                }
                if (0 == cpv->v.a->used) cpv->v.a = NULL;
                break;
              case 1: /* deflate.allowed-encodings */
                cpv->v.shrt = (unsigned short)
                  mod_deflate_encodings_to_flags(cpv->v.a);
                cpv->vtype = T_CONFIG_SHORT;
                break;
              case 2: /* deflate.max-compress-size */
              case 3: /* deflate.min-compress-size */
                break;
              case 4: /* deflate.compression-level */
                if ((cpv->v.shrt < 1 || cpv->v.shrt > 9)
                    && *(short *)&cpv->v.shrt != -1) {
                    log_error(srv->errh, __FILE__, __LINE__,
                      "compression-level must be between 1 and 9: %hu",
                      cpv->v.shrt);
                    return HANDLER_ERROR;
                }
                break;
              case 5: /* deflate.output-buffer-size */
              case 6: /* deflate.work-block-size */
                break;
              case 7: /* deflate.max-loadavg */
                cpv->v.d = (!buffer_string_is_empty(cpv->v.b))
                  ? strtod(cpv->v.b->ptr, NULL)
                  : 0.0;
                break;
              case 8: /* deflate.cache-dir */
                if (!buffer_string_is_empty(cpv->v.b)) {
                    buffer *b;
                    *(const buffer **)&b = cpv->v.b;
                    const uint32_t len = buffer_string_length(b);
                    if (len > 0 && '/' == b->ptr[len-1])
                        buffer_string_set_length(b, len-1); /*remove end slash*/
                    struct stat st;
                    if (0 != stat(b->ptr,&st) && 0 != mkdir_recursive(b->ptr)) {
                        log_perror(srv->errh, __FILE__, __LINE__,
                          "can't stat %s %s", cpk[cpv->k_id].k, b->ptr);
                        return HANDLER_ERROR;
                    }
                }
                break;
              case 9: /* compress.filetype */
                log_error(srv->errh, __FILE__, __LINE__,
                  "DEPRECATED: %s replaced with deflate.mimetypes",
                  cpk[cpv->k_id].k);
                break;
              case 10:/* compress.allowed-encodings */
                log_error(srv->errh, __FILE__, __LINE__,
                  "DEPRECATED: %s replaced with deflate.allowed-encodings",
                  cpk[cpv->k_id].k);
                cpv->v.shrt = (unsigned short)
                  mod_deflate_encodings_to_flags(cpv->v.a);
                cpv->vtype = T_CONFIG_SHORT;
                break;
              case 11:/* compress.cache-dir */
                log_error(srv->errh, __FILE__, __LINE__,
                  "DEPRECATED: %s replaced with deflate.cache-dir",
                  cpk[cpv->k_id].k);
                if (!buffer_string_is_empty(cpv->v.b)) {
                    buffer *b;
                    *(const buffer **)&b = cpv->v.b;
                    const uint32_t len = buffer_string_length(b);
                    if (len > 0 && '/' == b->ptr[len-1])
                        buffer_string_set_length(b, len-1); /*remove end slash*/
                    struct stat st;
                    if (0 != stat(b->ptr,&st) && 0 != mkdir_recursive(b->ptr)) {
                        log_perror(srv->errh, __FILE__, __LINE__,
                          "can't stat %s %s", cpk[cpv->k_id].k, b->ptr);
                        return HANDLER_ERROR;
                    }
                }
                break;
              case 12:/* compress.max-filesize */
                log_error(srv->errh, __FILE__, __LINE__,
                  "DEPRECATED: %s replaced with deflate.max-compress-size",
                  cpk[cpv->k_id].k);
                break;
              case 13:/* compress.max-loadavg */
                log_error(srv->errh, __FILE__, __LINE__,
                  "DEPRECATED: %s replaced with deflate.max-loadavg",
                  cpk[cpv->k_id].k);
                cpv->v.d = (!buffer_string_is_empty(cpv->v.b))
                  ? strtod(cpv->v.b->ptr, NULL)
                  : 0.0;
                break;
              default:/* should not happen */
                break;
            }
        }
    }

    p->defaults.allowed_encodings = 0;
    p->defaults.max_compress_size = 128*1024; /*(128 MB measured as num KB)*/
    p->defaults.min_compress_size = 256;
    p->defaults.compression_level = -1;
    p->defaults.output_buffer_size = 0;
    p->defaults.work_block_size = 2048;
    p->defaults.max_loadavg = 0.0;
    p->defaults.sync_flush = 0;

    /* initialize p->defaults from global config context */
    if (p->nconfig > 0 && p->cvlist->v.u2[1]) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist->v.u2[0];
        if (-1 != cpv->k_id)
            mod_deflate_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}


#if defined(USE_ZLIB) || defined(USE_BZ2LIB) || defined(USE_BROTLI) \
 || defined(USE_ZSTD)
static int mod_deflate_cache_file_append (handler_ctx * const hctx, const char *out, size_t len) {
    ssize_t wr;
    do {
        wr = write(hctx->cache_fd, out, len);
    } while (wr > 0 ? ((out += wr), (len -= (size_t)wr)) : errno == EINTR);
    return (0 == len) ? 0 : -1;
}

static int stream_http_chunk_append_mem(handler_ctx * const hctx, const char * const out, size_t len) {
    if (0 == len) return 0;
    return (-1 == hctx->cache_fd)
      ? http_chunk_append_mem(hctx->r, out, len)
      : mod_deflate_cache_file_append(hctx, out, len);
}
#endif


#ifdef USE_ZLIB

static int stream_deflate_init(handler_ctx *hctx) {
	z_stream * const z = &hctx->u.z;
	const plugin_data * const p = hctx->plugin_data;
	z->zalloc = Z_NULL;
	z->zfree = Z_NULL;
	z->opaque = Z_NULL;
	z->total_in = 0;
	z->total_out = 0;
	z->next_out = (unsigned char *)hctx->output->ptr;
	z->avail_out = hctx->output->size;

	if (Z_OK != deflateInit2(z,
				 p->conf.compression_level > 0
				  ? p->conf.compression_level
				  : Z_DEFAULT_COMPRESSION,
				 Z_DEFLATED,
				 (hctx->compression_type == HTTP_ACCEPT_ENCODING_GZIP)
				  ? (MAX_WBITS | 16) /*(0x10 flags gzip header, trailer)*/
				  : -MAX_WBITS,      /*(negate to suppress zlib header)*/
				 8, /* default memLevel */
				 Z_DEFAULT_STRATEGY)) {
		return -1;
	}

	return 0;
}

static int stream_deflate_compress(handler_ctx * const hctx, unsigned char * const start, off_t st_size) {
	z_stream * const z = &(hctx->u.z);
	size_t len;

	z->next_in = start;
	z->avail_in = st_size;
	hctx->bytes_in += st_size;

	/* compress data */
	do {
		if (Z_OK != deflate(z, Z_NO_FLUSH)) return -1;

		if (z->avail_out == 0 || z->avail_in > 0) {
			len = hctx->output->size - z->avail_out;
			hctx->bytes_out += len;
			if (0 != stream_http_chunk_append_mem(hctx, hctx->output->ptr, len))
				return -1;
			z->next_out = (unsigned char *)hctx->output->ptr;
			z->avail_out = hctx->output->size;
		}
	} while (z->avail_in > 0);

	return 0;
}

static int stream_deflate_flush(handler_ctx * const hctx, int end) {
	z_stream * const z = &(hctx->u.z);
	const plugin_data *p = hctx->plugin_data;
	size_t len;
	int rc = 0;
	int done;

	/* compress data */
	do {
		done = 1;
		if (end) {
			rc = deflate(z, Z_FINISH);
			if (rc == Z_OK) {
				done = 0;
			} else if (rc != Z_STREAM_END) {
				return -1;
			}
		} else {
			if (p->conf.sync_flush) {
				rc = deflate(z, Z_SYNC_FLUSH);
				if (rc != Z_OK) return -1;
			} else if (z->avail_in > 0) {
				rc = deflate(z, Z_NO_FLUSH);
				if (rc != Z_OK) return -1;
			}
		}

		len = hctx->output->size - z->avail_out;
		if (z->avail_out == 0 || (len > 0 && (end || p->conf.sync_flush))) {
			hctx->bytes_out += len;
			if (0 != stream_http_chunk_append_mem(hctx, hctx->output->ptr, len))
				return -1;
			z->next_out = (unsigned char *)hctx->output->ptr;
			z->avail_out = hctx->output->size;
		}
	} while (z->avail_in != 0 || !done);

	return 0;
}

static int stream_deflate_end(handler_ctx *hctx) {
	z_stream * const z = &(hctx->u.z);
	int rc = deflateEnd(z);
	if (Z_OK == rc || Z_DATA_ERROR == rc) return 0;

	if (z->msg != NULL) {
		log_error(hctx->r->conf.errh, __FILE__, __LINE__,
		  "deflateEnd error ret=%d, msg=%s", rc, z->msg);
	} else {
		log_error(hctx->r->conf.errh, __FILE__, __LINE__,
		  "deflateEnd error ret=%d", rc);
	}
	return -1;
}

#endif


#ifdef USE_BZ2LIB

static int stream_bzip2_init(handler_ctx *hctx) {
	bz_stream * const bz = &hctx->u.bz;
	const plugin_data * const p = hctx->plugin_data;
	bz->bzalloc = NULL;
	bz->bzfree = NULL;
	bz->opaque = NULL;
	bz->total_in_lo32 = 0;
	bz->total_in_hi32 = 0;
	bz->total_out_lo32 = 0;
	bz->total_out_hi32 = 0;
	bz->next_out = hctx->output->ptr;
	bz->avail_out = hctx->output->size;

	if (BZ_OK != BZ2_bzCompressInit(bz,
					p->conf.compression_level > 0
					 ? p->conf.compression_level
					 : 9, /* blocksize = 900k */
					0,    /* verbosity */
					0)) { /* workFactor: default */
		return -1;
	}

	return 0;
}

static int stream_bzip2_compress(handler_ctx * const hctx, unsigned char * const start, off_t st_size) {
	bz_stream * const bz = &(hctx->u.bz);
	size_t len;

	bz->next_in = (char *)start;
	bz->avail_in = st_size;
	hctx->bytes_in += st_size;

	/* compress data */
	do {
		if (BZ_RUN_OK != BZ2_bzCompress(bz, BZ_RUN)) return -1;

		if (bz->avail_out == 0 || bz->avail_in > 0) {
			len = hctx->output->size - bz->avail_out;
			hctx->bytes_out += len;
			if (0 != stream_http_chunk_append_mem(hctx, hctx->output->ptr, len))
				return -1;
			bz->next_out = hctx->output->ptr;
			bz->avail_out = hctx->output->size;
		}
	} while (bz->avail_in > 0);

	return 0;
}

static int stream_bzip2_flush(handler_ctx * const hctx, int end) {
	bz_stream * const bz = &(hctx->u.bz);
	const plugin_data *p = hctx->plugin_data;
	size_t len;
	int rc;
	int done;

	/* compress data */
	do {
		done = 1;
		if (end) {
			rc = BZ2_bzCompress(bz, BZ_FINISH);
			if (rc == BZ_FINISH_OK) {
				done = 0;
			} else if (rc != BZ_STREAM_END) {
				return -1;
			}
		} else if (bz->avail_in > 0) {
			/* p->conf.sync_flush not implemented here,
			 * which would loop on BZ_FLUSH while BZ_FLUSH_OK
			 * until BZ_RUN_OK returned */
			rc = BZ2_bzCompress(bz, BZ_RUN);
			if (rc != BZ_RUN_OK) {
				return -1;
			}
		}

		len = hctx->output->size - bz->avail_out;
		if (bz->avail_out == 0 || (len > 0 && (end || p->conf.sync_flush))) {
			hctx->bytes_out += len;
			if (0 != stream_http_chunk_append_mem(hctx, hctx->output->ptr, len))
				return -1;
			bz->next_out = hctx->output->ptr;
			bz->avail_out = hctx->output->size;
		}
	} while (bz->avail_in != 0 || !done);

	return 0;
}

static int stream_bzip2_end(handler_ctx *hctx) {
	bz_stream * const bz = &(hctx->u.bz);
	int rc = BZ2_bzCompressEnd(bz);
	if (BZ_OK == rc || BZ_DATA_ERROR == rc) return 0;

	log_error(hctx->r->conf.errh, __FILE__, __LINE__,
	  "BZ2_bzCompressEnd error ret=%d", rc);
	return -1;
}

#endif


#ifdef USE_BROTLI

static int stream_br_init(handler_ctx *hctx) {
    BrotliEncoderState * const br = hctx->u.br =
      BrotliEncoderCreateInstance(NULL, NULL, NULL);
    if (NULL == br) return -1;

    /* future: consider allowing tunables by encoder algorithm,
     * (i.e. not generic "compression_level") */
    /*(note: we ignore any errors while tuning parameters here)*/
    const plugin_data * const p = hctx->plugin_data;
    if (p->conf.compression_level >= 0) /* 0 .. 11 are valid values */
        BrotliEncoderSetParameter(br, BROTLI_PARAM_QUALITY,
                                  p->conf.compression_level);

    /* XXX: is this worth checking?
     * BROTLI_MODE_GENERIC vs BROTLI_MODE_TEXT or BROTLI_MODE_FONT */
    const buffer *vb =
      http_header_response_get(hctx->r, HTTP_HEADER_CONTENT_TYPE,
                               CONST_STR_LEN("Content-Type"));
    if (NULL != vb) {
        if (0 == strncmp(vb->ptr, "text/", sizeof("text/")-1)
            || (0 == strncmp(vb->ptr, "application/", sizeof("application/")-1)
                && (0 == strncmp(vb->ptr+12,"javascript",sizeof("javascript")-1)
                 || 0 == strncmp(vb->ptr+12,"ld+json",   sizeof("ld+json")-1)
                 || 0 == strncmp(vb->ptr+12,"json",      sizeof("json")-1)
                 || 0 == strncmp(vb->ptr+12,"xhtml+xml", sizeof("xhtml+xml")-1)
                 || 0 == strncmp(vb->ptr+12,"xml",       sizeof("xml")-1)))
            || 0 == strncmp(vb->ptr, "image/svg+xml", sizeof("image/svg+xml")-1))
            BrotliEncoderSetParameter(br, BROTLI_PARAM_MODE, BROTLI_MODE_TEXT);
        else if (0 == strncmp(vb->ptr, "font/", sizeof("font/")-1))
            BrotliEncoderSetParameter(br, BROTLI_PARAM_MODE, BROTLI_MODE_FONT);
    }

    return 0;
}

static int stream_br_compress(handler_ctx * const hctx, unsigned char * const start, off_t st_size) {
    const uint8_t *in = (uint8_t *)start;
    BrotliEncoderState * const br = hctx->u.br;
    hctx->bytes_in += st_size;
    while (st_size || BrotliEncoderHasMoreOutput(br)) {
        size_t insz = ((off_t)((~(uint32_t)0) >> 1) > st_size)
          ? (size_t)st_size
          : ((~(uint32_t)0) >> 1);
        size_t outsz = 0;
        BrotliEncoderCompressStream(br, BROTLI_OPERATION_PROCESS,
                                    &insz, &in, &outsz, NULL, NULL);
        const uint8_t *out = BrotliEncoderTakeOutput(br, &outsz);
        st_size -= (st_size - (off_t)insz);
        if (outsz) {
            hctx->bytes_out += (off_t)outsz;
            if (0 != stream_http_chunk_append_mem(hctx, (char *)out, outsz))
                return -1;
        }
    }
    return 0;
}

static int stream_br_flush(handler_ctx * const hctx, int end) {
    const int brmode = end ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_FLUSH;
    BrotliEncoderState * const br = hctx->u.br;
    do {
        size_t insz = 0;
        size_t outsz = 0;
        BrotliEncoderCompressStream(br, brmode,
                                    &insz, NULL, &outsz, NULL, NULL);
        const uint8_t *out = BrotliEncoderTakeOutput(br, &outsz);
        if (outsz) {
            hctx->bytes_out += (off_t)outsz;
            if (0 != stream_http_chunk_append_mem(hctx, (char *)out, outsz))
                return -1;
        }
    } while (BrotliEncoderHasMoreOutput(br));
    return 0;
}

static int stream_br_end(handler_ctx *hctx) {
    BrotliEncoderState * const br = hctx->u.br;
    BrotliEncoderDestroyInstance(br);
    return 0;
}

#endif


#ifdef USE_ZSTD

static int stream_zstd_init(handler_ctx *hctx) {
    ZSTD_CStream * const cctx = hctx->u.cctx = ZSTD_createCStream();
    if (NULL == cctx) return -1;
    hctx->output->used = 0;

    /* future: consider allowing tunables by encoder algorithm,
     * (i.e. not generic "compression_level" across all compression algos) */
    /*(note: we ignore any errors while tuning parameters here)*/
    const plugin_data * const p = hctx->plugin_data;
    if (p->conf.compression_level >= 0) { /* -1 is lighttpd default for "unset" */
        int level = p->conf.compression_level;
      #if ZSTD_VERSION_NUMBER >= 10000+400+0 /* v1.4.0 */
        ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, level);
      #else
        ZSTD_initCStream(cctx, level);
      #endif
    }
    return 0;
}

static int stream_zstd_compress(handler_ctx * const hctx, unsigned char * const start, off_t st_size) {
    ZSTD_CStream * const cctx = hctx->u.cctx;
    ZSTD_inBuffer zib = { start, (size_t)st_size, 0 };
    ZSTD_outBuffer zob = { hctx->output->ptr,
                           hctx->output->size,
                           hctx->output->used };
    hctx->output->used = 0;
    hctx->bytes_in += st_size;
    while (zib.pos < zib.size) {
      #if ZSTD_VERSION_NUMBER >= 10000+400+0 /* v1.4.0 */
        const size_t rv = ZSTD_compressStream2(cctx,&zob,&zib,ZSTD_e_continue);
      #else
        const size_t rv = ZSTD_compressStream(cctx, &zob, &zib);
      #endif
        if (ZSTD_isError(rv)) return -1;
        if (zib.pos == zib.size) break; /* defer flush */
        hctx->bytes_out += (off_t)zob.pos;
        if (0 != stream_http_chunk_append_mem(hctx, zob.dst, zob.pos))
            return -1;
        zob.pos = 0;
    }
    hctx->output->used = (uint32_t)zob.pos;
    return 0;
}

static int stream_zstd_flush(handler_ctx * const hctx, int end) {
    ZSTD_CStream * const cctx = hctx->u.cctx;
  #if ZSTD_VERSION_NUMBER >= 10000+400+0 /* v1.4.0 */
    const ZSTD_EndDirective endOp = end ? ZSTD_e_end : ZSTD_e_flush;
    ZSTD_inBuffer zib = { NULL, 0, 0 };
  #endif
    ZSTD_outBuffer zob = { hctx->output->ptr,
                           hctx->output->size,
                           hctx->output->used };
    size_t rv;
    do {
      #if ZSTD_VERSION_NUMBER >= 10000+400+0 /* v1.4.0 */
        rv = ZSTD_compressStream2(cctx, &zob, &zib, endOp);
      #else
        rv = end
           ? ZSTD_endStream(cctx, &zob)
           : ZSTD_flushStream(cctx, &zob);
      #endif
        if (ZSTD_isError(rv)) return -1;
        hctx->bytes_out += (off_t)zob.pos;
        if (0 != stream_http_chunk_append_mem(hctx, zob.dst, zob.pos))
            return -1;
        zob.pos = 0;
    } while (0 != rv);
    return 0;
}

static int stream_zstd_end(handler_ctx *hctx) {
    ZSTD_CStream * const cctx = hctx->u.cctx;
    ZSTD_freeCStream(cctx);
    return 0;
}

#endif


static int mod_deflate_stream_init(handler_ctx *hctx) {
	switch(hctx->compression_type) {
#ifdef USE_ZLIB
	case HTTP_ACCEPT_ENCODING_GZIP:
	case HTTP_ACCEPT_ENCODING_DEFLATE:
		return stream_deflate_init(hctx);
#endif
#ifdef USE_BZ2LIB
	case HTTP_ACCEPT_ENCODING_BZIP2:
		return stream_bzip2_init(hctx);
#endif
#ifdef USE_BROTLI
	case HTTP_ACCEPT_ENCODING_BR:
		return stream_br_init(hctx);
#endif
#ifdef USE_ZSTD
	case HTTP_ACCEPT_ENCODING_ZSTD:
		return stream_zstd_init(hctx);
#endif
	default:
		return -1;
	}
}

static int mod_deflate_compress(handler_ctx * const hctx, unsigned char * const start, off_t st_size) {
	if (0 == st_size) return 0;
	switch(hctx->compression_type) {
#ifdef USE_ZLIB
	case HTTP_ACCEPT_ENCODING_GZIP:
	case HTTP_ACCEPT_ENCODING_DEFLATE:
		return stream_deflate_compress(hctx, start, st_size);
#endif
#ifdef USE_BZ2LIB
	case HTTP_ACCEPT_ENCODING_BZIP2:
		return stream_bzip2_compress(hctx, start, st_size);
#endif
#ifdef USE_BROTLI
	case HTTP_ACCEPT_ENCODING_BR:
		return stream_br_compress(hctx, start, st_size);
#endif
#ifdef USE_ZSTD
	case HTTP_ACCEPT_ENCODING_ZSTD:
		return stream_zstd_compress(hctx, start, st_size);
#endif
	default:
		UNUSED(start);
		return -1;
	}
}

static int mod_deflate_stream_flush(handler_ctx * const hctx, int end) {
	if (0 == hctx->bytes_in) return 0;
	switch(hctx->compression_type) {
#ifdef USE_ZLIB
	case HTTP_ACCEPT_ENCODING_GZIP:
	case HTTP_ACCEPT_ENCODING_DEFLATE:
		return stream_deflate_flush(hctx, end);
#endif
#ifdef USE_BZ2LIB
	case HTTP_ACCEPT_ENCODING_BZIP2:
		return stream_bzip2_flush(hctx, end);
#endif
#ifdef USE_BROTLI
	case HTTP_ACCEPT_ENCODING_BR:
		return stream_br_flush(hctx, end);
#endif
#ifdef USE_ZSTD
	case HTTP_ACCEPT_ENCODING_ZSTD:
		return stream_zstd_flush(hctx, end);
#endif
	default:
		UNUSED(end);
		return -1;
	}
}

static void mod_deflate_note_ratio(request_st * const r, const off_t bytes_out, const off_t bytes_in) {
    /* store compression ratio in environment
     * for possible logging by mod_accesslog
     * (late in response handling, so not seen by most other modules) */
    /*(should be called only at end of successful response compression)*/
    char ratio[LI_ITOSTRING_LENGTH];
    if (0 == bytes_in) return;
    size_t len =
      li_itostrn(ratio, sizeof(ratio), bytes_out * 100 / bytes_in);
    http_header_env_set(r, CONST_STR_LEN("ratio"), ratio, len);
}

static int mod_deflate_stream_end(handler_ctx *hctx) {
	switch(hctx->compression_type) {
#ifdef USE_ZLIB
	case HTTP_ACCEPT_ENCODING_GZIP:
	case HTTP_ACCEPT_ENCODING_DEFLATE:
		return stream_deflate_end(hctx);
#endif
#ifdef USE_BZ2LIB
	case HTTP_ACCEPT_ENCODING_BZIP2:
		return stream_bzip2_end(hctx);
#endif
#ifdef USE_BROTLI
	case HTTP_ACCEPT_ENCODING_BR:
		return stream_br_end(hctx);
#endif
#ifdef USE_ZSTD
	case HTTP_ACCEPT_ENCODING_ZSTD:
		return stream_zstd_end(hctx);
#endif
	default:
		return -1;
	}
}

static int deflate_compress_cleanup(request_st * const r, handler_ctx * const hctx) {
	int rc = mod_deflate_stream_end(hctx);

      #if 1 /* unnecessary if deflate.min-compress-size is set to a reasonable value */
	if (0 == rc && hctx->bytes_in < hctx->bytes_out)
		log_error(r->conf.errh, __FILE__, __LINE__,
		  "uri %s in=%lld smaller than out=%lld", r->target.ptr,
		  (long long)hctx->bytes_in, (long long)hctx->bytes_out);
      #endif

	handler_ctx_free(hctx);
	return rc;
}


static int mod_deflate_file_chunk(request_st * const r, handler_ctx * const hctx, chunk * const c, off_t st_size) {
	off_t abs_offset;
	off_t toSend = -1;
	char *start;
#ifdef USE_MMAP
	/* use large chunks since server blocks while compressing, anyway
	 * (mod_deflate is not recommended for large files) */
	const off_t mmap_chunk_size = 16 * 1024 * 1024; /* must be power-of-2 */
	off_t we_want_to_send = st_size;
	volatile int mapped = 0;/* quiet warning: might be clobbered by 'longjmp' */
#else
	start = NULL;
#endif

	if (-1 == c->file.fd) {  /* open the file if not already open */
		if (-1 == (c->file.fd = fdevent_open_cloexec(c->mem->ptr, r->conf.follow_symlink, O_RDONLY, 0))) {
			log_perror(r->conf.errh, __FILE__, __LINE__, "open failed %s", c->mem->ptr);
			return -1;
		}
	}

	abs_offset = c->offset;

#ifdef USE_MMAP
	/* mmap the buffer
	 * - first mmap
	 * - new mmap as the we are at the end of the last one */
	if (c->file.mmap.start == MAP_FAILED ||
	    abs_offset == (off_t)(c->file.mmap.offset + c->file.mmap.length)) {

		/* Optimizations for the future:
		 *
		 * (comment is outdated since server blocks compressing file and then munmap()s file)
		 *
		 * adaptive mem-mapping
		 *   the problem:
		 *     we mmap() the whole file. If someone has a lot of large files and 32bit
		 *     machine the virtual address area will be exhausted and we will have a failing
		 *     mmap() call.
		 *   solution:
		 *     only mmap 16M in one chunk and move the window as soon as we have finished
		 *     the first 8M
		 *
		 * read-ahead buffering
		 *   the problem:
		 *     sending out several large files in parallel trashes the read-ahead of the
		 *     kernel leading to long wait-for-seek times.
		 *   solutions: (increasing complexity)
		 *     1. use madvise
		 *     2. use a internal read-ahead buffer in the chunk-structure
		 *     3. use non-blocking IO for file-transfers
		 *   */

		/* all mmap()ed areas are mmap_chunk_size except the last which might be smaller */
		off_t to_mmap;

		/* this is a remap, move the mmap-offset */
		if (c->file.mmap.start != MAP_FAILED) {
			munmap(c->file.mmap.start, c->file.mmap.length);
			c->file.mmap.offset += mmap_chunk_size;
		} else {
			/* in case the range-offset is after the first mmap()ed area we skip the area */
			c->file.mmap.offset = c->offset & ~(mmap_chunk_size-1);
		}

		to_mmap = c->file.length - c->file.mmap.offset;
		if (to_mmap > mmap_chunk_size) to_mmap = mmap_chunk_size;
		/* we have more to send than we can mmap() at once */
		if (we_want_to_send > to_mmap) we_want_to_send = to_mmap;

		if (MAP_FAILED == (c->file.mmap.start = mmap(0, (size_t)to_mmap, PROT_READ, MAP_SHARED, c->file.fd, c->file.mmap.offset))
		    && (errno != EINVAL || MAP_FAILED == (c->file.mmap.start = mmap(0, (size_t)to_mmap, PROT_READ, MAP_PRIVATE, c->file.fd, c->file.mmap.offset)))) {
			log_perror(r->conf.errh, __FILE__, __LINE__,
			  "mmap failed %s %d", c->mem->ptr, c->file.fd);

			return -1;
		}

		c->file.mmap.length = to_mmap;
#ifdef HAVE_MADVISE
		/* don't advise files <= 64Kb */
		if (c->file.mmap.length > 65536 &&
		    0 != madvise(c->file.mmap.start, c->file.mmap.length, MADV_WILLNEED)) {
			log_perror(r->conf.errh, __FILE__, __LINE__,
			  "madvise failed %s %d", c->mem->ptr, c->file.fd);
		}
#endif

		/* chunk_reset() or chunk_free() will cleanup for us */
	}

	/* to_send = abs_mmap_end - abs_offset */
	toSend = (c->file.mmap.offset + c->file.mmap.length) - abs_offset;
	if (toSend > we_want_to_send) toSend = we_want_to_send;

	if (toSend < 0) {
		log_error(r->conf.errh, __FILE__, __LINE__,
		  "toSend is negative: %lld %lld %lld %lld",
		  (long long)toSend,
		  (long long)c->file.mmap.length,
		  (long long)abs_offset,
		  (long long)c->file.mmap.offset);
		force_assert(toSend < 0);
	}

	start = c->file.mmap.start;
	mapped = 1;
#endif

	if (MAP_FAILED == c->file.mmap.start) {
		toSend = st_size;
		if (toSend > 2*1024*1024) toSend = 2*1024*1024;
		if (NULL == (start = malloc((size_t)toSend)) || -1 == lseek(c->file.fd, abs_offset, SEEK_SET) || toSend != read(c->file.fd, start, (size_t)toSend)) {
			log_perror(r->conf.errh, __FILE__, __LINE__, "reading %s failed", c->mem->ptr);

			free(start);
			return -1;
		}
		abs_offset = 0;
	}

#ifdef USE_MMAP
	if (mapped) {
		signal(SIGBUS, sigbus_handler);
		sigbus_jmp_valid = 1;
		if (0 != sigsetjmp(sigbus_jmp, 1)) {
			sigbus_jmp_valid = 0;

			log_error(r->conf.errh, __FILE__, __LINE__,
			  "SIGBUS in mmap: %s %d", c->mem->ptr, c->file.fd);
			return -1;
		}
	}
#endif

	if (mod_deflate_compress(hctx,
				(unsigned char *)start + (abs_offset - c->file.mmap.offset), toSend) < 0) {
		log_error(r->conf.errh, __FILE__, __LINE__, "compress failed.");
		toSend = -1;
	}

#ifdef USE_MMAP
	if (mapped)
		sigbus_jmp_valid = 0;
	else
#endif
		free(start);

	return toSend;
}


static handler_t deflate_compress_response(request_st * const r, handler_ctx * const hctx) {
	off_t len, max;
	int close_stream;

	/* move all chunk from write_queue into our in_queue, then adjust
	 * counters since r->write_queue is reused for compressed output */
	chunkqueue * const cq = &r->write_queue;
	len = chunkqueue_length(cq);
	chunkqueue_remove_finished_chunks(cq);
	chunkqueue_append_chunkqueue(&hctx->in_queue, cq);
	cq->bytes_in  -= len;
	cq->bytes_out -= len;

	max = chunkqueue_length(&hctx->in_queue);
      #if 0
	/* calculate max bytes to compress for this call */
	if (p->conf.sync_flush && max > (len = p->conf.work_block_size << 10)) {
		max = len;
	}
      #endif

	/* Compress chunks from in_queue into chunks for write_queue */
	while (max) {
		chunk *c = hctx->in_queue.first;

		switch(c->type) {
		case MEM_CHUNK:
			len = buffer_string_length(c->mem) - c->offset;
			if (len > max) len = max;
			if (mod_deflate_compress(hctx, (unsigned char *)c->mem->ptr+c->offset, len) < 0) {
				log_error(r->conf.errh, __FILE__, __LINE__, "compress failed.");
				return HANDLER_ERROR;
			}
			break;
		case FILE_CHUNK:
			len = c->file.length - c->offset;
			if (len > max) len = max;
			if ((len = mod_deflate_file_chunk(r, hctx, c, len)) < 0) {
				log_error(r->conf.errh, __FILE__, __LINE__, "compress file chunk failed.");
				return HANDLER_ERROR;
			}
			break;
		default:
			log_error(r->conf.errh, __FILE__, __LINE__, "%d type not known", c->type);
			return HANDLER_ERROR;
		}

		max -= len;
		chunkqueue_mark_written(&hctx->in_queue, len);
	}

	/*(currently should always be true)*/
	/*(current implementation requires response be complete)*/
	close_stream = (r->resp_body_finished
                        && chunkqueue_is_empty(&hctx->in_queue));
	if (mod_deflate_stream_flush(hctx, close_stream) < 0) {
		log_error(r->conf.errh, __FILE__, __LINE__, "flush error");
		return HANDLER_ERROR;
	}

	return close_stream ? HANDLER_FINISHED : HANDLER_GO_ON;
}


static int mod_deflate_choose_encoding (const char *value, plugin_data *p, const char **label) {
	/* get client side support encodings */
	int accept_encoding = 0;
      #if !defined(USE_ZLIB) && !defined(USE_BZ2LIB) && !defined(USE_BROTLI) \
       && !defined(USE_ZSTD)
	UNUSED(value);
	UNUSED(label);
      #else
        for (; *value; ++value) {
            const char *v;
            while (*value == ' ' || *value == ',') ++value;
            v = value;
            while (*value!=' ' && *value!=',' && *value!=';' && *value!='\0')
                ++value;
            switch (value - v) {
              case 2:
               #ifdef USE_BROTLI
                if (0 == memcmp(v, "br", 2))
                    accept_encoding |= HTTP_ACCEPT_ENCODING_BR;
               #endif
                break;
              case 4:
               #ifdef USE_ZLIB
                if (0 == memcmp(v, "gzip", 4))
                    accept_encoding |= HTTP_ACCEPT_ENCODING_GZIP;
               #endif
               #ifdef USE_ZSTD
                #ifdef USE_ZLIB
                else
                #endif
                if (0 == memcmp(v, "zstd", 4))
                    accept_encoding |= HTTP_ACCEPT_ENCODING_ZSTD;
               #endif
                break;
              case 5:
               #ifdef USE_BZ2LIB
                if (0 == memcmp(v, "bzip2", 5))
                    accept_encoding |= HTTP_ACCEPT_ENCODING_BZIP2;
               #endif
                break;
              case 6:
               #ifdef USE_ZLIB
                if (0 == memcmp(v, "x-gzip", 6))
                    accept_encoding |= HTTP_ACCEPT_ENCODING_X_GZIP;
               #endif
                break;
              case 7:
               #ifdef USE_ZLIB
                if (0 == memcmp(v, "deflate", 7))
                    accept_encoding |= HTTP_ACCEPT_ENCODING_DEFLATE;
               #endif
               #ifdef USE_BZ2LIB
                if (0 == memcmp(v, "x-bzip2", 7))
                    accept_encoding |= HTTP_ACCEPT_ENCODING_X_BZIP2;
               #endif
                break;
             #if 0
              case 8:
                if (0 == memcmp(v, "identity", 8))
                    accept_encoding |= HTTP_ACCEPT_ENCODING_IDENTITY;
                else if (0 == memcmp(v, "compress", 8))
                    accept_encoding |= HTTP_ACCEPT_ENCODING_COMPRESS;
                break;
             #endif
              default:
                break;
            }
            if (*value == ';') {
                while (*value != ',' && *value != '\0') ++value;
            }
            if (*value == '\0') break;
        }
      #endif

	/* mask to limit to allowed_encodings */
	accept_encoding &= p->conf.allowed_encodings;

	/* select best matching encoding */
#ifdef USE_ZSTD
	if (accept_encoding & HTTP_ACCEPT_ENCODING_ZSTD) {
		*label = "zstd";
		return HTTP_ACCEPT_ENCODING_ZSTD;
	} else
#endif
#ifdef USE_BROTLI
	if (accept_encoding & HTTP_ACCEPT_ENCODING_BR) {
		*label = "br";
		return HTTP_ACCEPT_ENCODING_BR;
	} else
#endif
#ifdef USE_BZ2LIB
	if (accept_encoding & HTTP_ACCEPT_ENCODING_BZIP2) {
		*label = "bzip2";
		return HTTP_ACCEPT_ENCODING_BZIP2;
	} else if (accept_encoding & HTTP_ACCEPT_ENCODING_X_BZIP2) {
		*label = "x-bzip2";
		return HTTP_ACCEPT_ENCODING_BZIP2;
	} else
#endif
#ifdef USE_ZLIB
	if (accept_encoding & HTTP_ACCEPT_ENCODING_GZIP) {
		*label = "gzip";
		return HTTP_ACCEPT_ENCODING_GZIP;
	} else if (accept_encoding & HTTP_ACCEPT_ENCODING_X_GZIP) {
		*label = "x-gzip";
		return HTTP_ACCEPT_ENCODING_GZIP;
	} else if (accept_encoding & HTTP_ACCEPT_ENCODING_DEFLATE) {
		*label = "deflate";
		return HTTP_ACCEPT_ENCODING_DEFLATE;
	} else
#endif
	if (0 == accept_encoding) {
		return 0;
	} else {
		return 0;
	}
}

REQUEST_FUNC(mod_deflate_handle_response_start) {
	plugin_data *p = p_d;
	const buffer *vbro;
	buffer *vb;
	handler_ctx *hctx;
	const char *label;
	off_t len;
	uint32_t etaglen;
	int compression_type;
	handler_t rc;
	int had_vary = 0;

	/*(current implementation requires response be complete)*/
	if (!r->resp_body_finished) return HANDLER_GO_ON;
	if (r->http_method == HTTP_METHOD_HEAD) return HANDLER_GO_ON;
	if (light_btst(r->resp_htags, HTTP_HEADER_TRANSFER_ENCODING))
		return HANDLER_GO_ON;
	if (light_btst(r->resp_htags, HTTP_HEADER_CONTENT_ENCODING))
		return HANDLER_GO_ON;

	/* disable compression for some http status types. */
	switch(r->http_status) {
	case 100:
	case 101:
	case 204:
	case 205:
	case 304:
		/* disable compression as we have no response entity */
		return HANDLER_GO_ON;
	default:
		break;
	}

	mod_deflate_patch_config(r, p);

	/* check if deflate configured for any mimetypes */
	if (NULL == p->conf.mimetypes) return HANDLER_GO_ON;

	/* check if size of response is below min-compress-size or exceeds max*/
	/* (r->resp_body_finished checked at top of routine) */
	len = chunkqueue_length(&r->write_queue);
	if (len <= (off_t)p->conf.min_compress_size) return HANDLER_GO_ON;
	if (p->conf.max_compress_size /*(max_compress_size in KB)*/
	    && len > ((off_t)p->conf.max_compress_size << 10)) {
		return HANDLER_GO_ON;
	}

	/* Check Accept-Encoding for supported encoding. */
	vbro = http_header_request_get(r, HTTP_HEADER_ACCEPT_ENCODING, CONST_STR_LEN("Accept-Encoding"));
	if (NULL == vbro) return HANDLER_GO_ON;

	/* find matching encodings */
	compression_type = mod_deflate_choose_encoding(vbro->ptr, p, &label);
	if (!compression_type) return HANDLER_GO_ON;

	/* Check mimetype in response header "Content-Type" */
	if (NULL != (vbro = http_header_response_get(r, HTTP_HEADER_CONTENT_TYPE, CONST_STR_LEN("Content-Type")))) {
		if (NULL == array_match_value_prefix(p->conf.mimetypes, vbro)) return HANDLER_GO_ON;
	} else {
		/* If no Content-Type set, compress only if first p->conf.mimetypes value is "" */
		data_string *mimetype = (data_string *)p->conf.mimetypes->data[0];
		if (!buffer_string_is_empty(&mimetype->value)) return HANDLER_GO_ON;
	}

	/* Vary: Accept-Encoding (response might change according to request Accept-Encoding) */
	if (NULL != (vb = http_header_response_get(r, HTTP_HEADER_VARY, CONST_STR_LEN("Vary")))) {
		had_vary = 1;
		if (NULL == strstr(vb->ptr, "Accept-Encoding")) {
			buffer_append_string_len(vb, CONST_STR_LEN(",Accept-Encoding"));
		}
	} else {
		http_header_response_append(r, HTTP_HEADER_VARY,
					    CONST_STR_LEN("Vary"),
					    CONST_STR_LEN("Accept-Encoding"));
	}

	/* check ETag as is done in http_response_handle_cachable()
	 * (slightly imperfect (close enough?) match of ETag "000000" to "000000-gzip") */
	vb = http_header_response_get(r, HTTP_HEADER_ETAG, CONST_STR_LEN("ETag"));
	etaglen = (NULL != vb) ? buffer_string_length(vb) : 0;
	if (NULL != vb && light_btst(r->rqst_htags, HTTP_HEADER_IF_NONE_MATCH)) {
		const buffer *if_none_match = http_header_response_get(r, HTTP_HEADER_IF_NONE_MATCH, CONST_STR_LEN("If-None-Match"));
		if (etaglen
		    && r->http_status < 300 /*(want 2xx only)*/
		    && NULL != if_none_match
		    && 0 == strncmp(if_none_match->ptr, vb->ptr, etaglen-1)
		    && if_none_match->ptr[etaglen-1] == '-'
		    && 0 == strncmp(if_none_match->ptr+etaglen, label, strlen(label))) {

			if (http_method_get_or_head(r->http_method)) {
				/* modify ETag response header in-place to remove '"' and append '-label"' */
				vb->ptr[etaglen-1] = '-'; /*(overwrite end '"')*/
				buffer_append_string(vb, label);
				buffer_append_string_len(vb, CONST_STR_LEN("\""));
				/*buffer_copy_buffer(&r->physical.etag, vb);*//*(keep in sync?)*/
				r->http_status = 304;
			} else {
				r->http_status = 412;
			}

			/* response_start hook occurs after error docs have been handled.
			 * For now, send back empty response body.
			 * In the future, might extract the error doc code so that it
			 * might be run again if response_start hooks return with
			 * changed http_status and r->handler_module = NULL */
			/* clear content length even if 304 since compressed length unknown */
			http_response_body_clear(r, 0);

			r->resp_body_finished = 1;
			r->handler_module = NULL;
			return HANDLER_GO_ON;
		}
	}

	if (0.0 < p->conf.max_loadavg && p->conf.max_loadavg < r->con->srv->loadavg[0]) {
		return HANDLER_GO_ON;
	}

	/* update ETag, if ETag response header is set */
	if (etaglen) {
		/* modify ETag response header in-place to remove '"' and append '-label"' */
		vb->ptr[etaglen-1] = '-'; /*(overwrite end '"')*/
		buffer_append_string(vb, label);
		buffer_append_string_len(vb, CONST_STR_LEN("\""));
		/*buffer_copy_buffer(&r->physical.etag, vb);*//*(keep in sync?)*/
	}

	/* set Content-Encoding to show selected compression type */
	http_header_response_set(r, HTTP_HEADER_CONTENT_ENCODING, CONST_STR_LEN("Content-Encoding"), label, strlen(label));

	/* clear Content-Length and r->write_queue if HTTP HEAD request
	 * (alternatively, could return original Content-Length with HEAD
	 *  request if ETag not modified and Content-Encoding not added) */
	if (HTTP_METHOD_HEAD == r->http_method) {
		/* ensure that uncompressed Content-Length is not sent in HEAD response */
		http_response_body_clear(r, 0);
		return HANDLER_GO_ON;
	}

	/* restrict items eligible for cache of compressed responses
	 * (This module does not aim to be a full caching proxy)
	 * response must be complete (not streaming response)
	 * must not have prior Vary response header (before Accept-Encoding added)
	 * must not have Range response header
	 * must have ETag
	 * must be file
	 * must be single FILE_CHUNK in chunkqueue
	 * must not be chunkqueue temporary file
	 * must be whole file, not partial content
	 * Note: small files (< 32k (see http_chunk.c)) will have been read into
	 *       memory (if streaming HTTP/1.1 chunked response) and will end up
	 *       getting stream-compressed rather than cached on disk as compressed
	 *       file
	 */
	buffer *tb = NULL;
	if (!buffer_is_empty(p->conf.cache_dir)
	    && !had_vary
	    && etaglen > 2
	    && r->resp_body_finished
	    && r->write_queue.first == r->write_queue.last
	    && r->write_queue.first->type == FILE_CHUNK
	    && r->write_queue.first->offset == 0
	    && !r->write_queue.first->file.is_temp
	    && !light_btst(r->resp_htags, HTTP_HEADER_RANGE)) {
		tb = mod_deflate_cache_file_name(r, p->conf.cache_dir, vb);
		/*(checked earlier and skipped if Transfer-Encoding had been set)*/
		stat_cache_entry *sce = stat_cache_get_entry_open(tb, 1);
		if (NULL != sce) {
			chunkqueue_reset(&r->write_queue);
			if (sce->fd < 0 || 0 != http_chunk_append_file_ref(r, sce))
				return HANDLER_ERROR;
			if (light_btst(r->resp_htags, HTTP_HEADER_CONTENT_LENGTH))
				http_header_response_unset(r, HTTP_HEADER_CONTENT_LENGTH,
				                           CONST_STR_LEN("Content-Length"));
			mod_deflate_note_ratio(r, sce->st.st_size, len);
			return HANDLER_GO_ON;
		}
		/* sanity check that response was whole file;
		 * (racy since using stat_cache, but cache file only if match) */
		sce = stat_cache_get_entry(r->write_queue.first->mem);
		if (NULL == sce || sce->st.st_size != len)
			tb = NULL;
		else if (0 != mkdir_for_file(tb->ptr))
			tb = NULL;
	}

	/* enable compression */
	p->conf.sync_flush =
	  ((r->conf.stream_response_body
	    & (FDEVENT_STREAM_RESPONSE | FDEVENT_STREAM_RESPONSE_BUFMIN))
	   && 0 == p->conf.output_buffer_size);
	hctx = handler_ctx_init();
	hctx->plugin_data = p;
	hctx->compression_type = compression_type;
	hctx->r = r;
	/* setup output buffer */
	buffer_clear(&p->tmp_buf);
	hctx->output = &p->tmp_buf;
	/* open cache file if caching compressed file */
	if (tb) mod_deflate_cache_file_open(hctx, tb);
	if (0 != mod_deflate_stream_init(hctx)) {
		/*(should not happen unless ENOMEM)*/
		handler_ctx_free(hctx);
		log_error(r->conf.errh, __FILE__, __LINE__,
		  "Failed to initialize compression %s", label);
		/* restore prior Etag and unset Content-Encoding */
		if (etaglen) {
			vb->ptr[etaglen-1] = '"'; /*(overwrite '-')*/
			buffer_string_set_length(vb, etaglen);
		}
		http_header_response_unset(r, HTTP_HEADER_CONTENT_ENCODING, CONST_STR_LEN("Content-Encoding"));
		return HANDLER_GO_ON;
	}

  #ifdef USE_BROTLI
	if (r->resp_body_finished
	    && (hctx->compression_type & HTTP_ACCEPT_ENCODING_BR)
	    && (len >> 1) < (off_t)((~(uint32_t)0u) >> 1))
		BrotliEncoderSetParameter(hctx->u.br, BROTLI_PARAM_SIZE_HINT, (uint32_t)len);
  #endif

	if (light_btst(r->resp_htags, HTTP_HEADER_CONTENT_LENGTH)) {
		http_header_response_unset(r, HTTP_HEADER_CONTENT_LENGTH, CONST_STR_LEN("Content-Length"));
	}
	r->plugin_ctx[p->id] = hctx;

	rc = deflate_compress_response(r, hctx);
	if (HANDLER_GO_ON == rc) return HANDLER_GO_ON;
	if (HANDLER_FINISHED == rc) {
	  #ifdef __COVERITY__
		/* coverity misses if hctx->cache_fd is not -1, then tb is not NULL */
		force_assert(-1 == hctx->cache_fd || NULL != tb);
	  #endif
		if (-1 == hctx->cache_fd
		    || 0 == mod_deflate_cache_file_finish(r, hctx, tb)) {
			mod_deflate_note_ratio(r, hctx->bytes_out, hctx->bytes_in);
			rc = HANDLER_GO_ON;
		}
		else
			rc = HANDLER_ERROR;
	}
	r->plugin_ctx[p->id] = NULL;
	if (deflate_compress_cleanup(r, hctx) < 0) return HANDLER_ERROR;
	return rc;
}

static handler_t mod_deflate_cleanup(request_st * const r, void *p_d) {
	plugin_data *p = p_d;
	handler_ctx *hctx = r->plugin_ctx[p->id];

	if (NULL != hctx) {
		r->plugin_ctx[p->id] = NULL;
		deflate_compress_cleanup(r, hctx);
	}

	return HANDLER_GO_ON;
}

int mod_deflate_plugin_init(plugin *p);
int mod_deflate_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = "deflate";

	p->init		= mod_deflate_init;
	p->cleanup	= mod_deflate_free;
	p->set_defaults	= mod_deflate_set_defaults;
	p->handle_request_reset = mod_deflate_cleanup;
	p->handle_response_start	= mod_deflate_handle_response_start;

	return 0;
}
