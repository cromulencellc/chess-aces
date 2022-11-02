#include "first.h"

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "gw_backend.h"
typedef gw_plugin_config plugin_config;
typedef gw_plugin_data   plugin_data;
typedef gw_handler_ctx   handler_ctx;

#include "base.h"
#include "buffer.h"
#include "fdevent.h"
#include "http_chunk.h"
#include "log.h"
#include "status_counter.h"

#include "compat/fastcgi.h"

#if GW_RESPONDER  != FCGI_RESPONDER
#error "mismatched defines: (GW_RESPONDER != FCGI_RESPONDER)"
#endif
#if GW_AUTHORIZER != FCGI_AUTHORIZER
#error "mismatched defines: (GW_AUTHORIZER != FCGI_AUTHORIZER)"
#endif
#if GW_FILTER     != FCGI_FILTER
#error "mismatched defines: (GW_FILTER != FCGI_FILTER)"
#endif

static void mod_fastcgi_merge_config_cpv(plugin_config * const pconf, const config_plugin_value_t * const cpv) {
    switch (cpv->k_id) { /* index into static config_plugin_keys_t cpk[] */
      case 0: /* fastcgi.server */
        if (cpv->vtype == T_CONFIG_LOCAL) {
            gw_plugin_config * const gw = cpv->v.v;
            pconf->exts      = gw->exts;
            pconf->exts_auth = gw->exts_auth;
            pconf->exts_resp = gw->exts_resp;
        }
        break;
      case 1: /* fastcgi.balance */
        /*if (cpv->vtype == T_CONFIG_LOCAL)*//*always true here for this param*/
            pconf->balance = (int)cpv->v.u;
        break;
      case 2: /* fastcgi.debug */
        pconf->debug = (int)cpv->v.u;
        break;
      case 3: /* fastcgi.map-extensions */
        pconf->ext_mapping = cpv->v.a;
        break;
      default:/* should not happen */
        return;
    }
}

static void mod_fastcgi_merge_config(plugin_config * const pconf, const config_plugin_value_t *cpv) {
    do {
        mod_fastcgi_merge_config_cpv(pconf, cpv);
    } while ((++cpv)->k_id != -1);
}

static void mod_fastcgi_patch_config(request_st * const r, plugin_data * const p) {
    memcpy(&p->conf, &p->defaults, sizeof(plugin_config));
    for (int i = 1, used = p->nconfig; i < used; ++i) {
        if (config_check_cond(r, (uint32_t)p->cvlist[i].k_id))
            mod_fastcgi_merge_config(&p->conf,p->cvlist + p->cvlist[i].v.u2[0]);
    }
}

SETDEFAULTS_FUNC(mod_fastcgi_set_defaults) {
    static const config_plugin_keys_t cpk[] = {
      { CONST_STR_LEN("fastcgi.server"),
        T_CONFIG_ARRAY_KVARRAY,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("fastcgi.balance"),
        T_CONFIG_STRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("fastcgi.debug"),
        T_CONFIG_INT,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ CONST_STR_LEN("fastcgi.map-extensions"),
        T_CONFIG_ARRAY_KVSTRING,
        T_CONFIG_SCOPE_CONNECTION }
     ,{ NULL, 0,
        T_CONFIG_UNSET,
        T_CONFIG_SCOPE_UNSET }
    };

    plugin_data * const p = p_d;
    if (!config_plugin_values_init(srv, p, cpk, "mod_fastcgi"))
        return HANDLER_ERROR;

    /* process and validate config directives
     * (init i to 0 if global context; to 1 to skip empty global context) */
    for (int i = !p->cvlist[0].v.u2[1]; i < p->nconfig; ++i) {
        config_plugin_value_t *cpv = p->cvlist + p->cvlist[i].v.u2[0];
        for (; -1 != cpv->k_id; ++cpv) {
            switch (cpv->k_id) {
              case 0:{/* fastcgi.server */
                gw_plugin_config *gw = calloc(1, sizeof(gw_plugin_config));
                force_assert(gw);
                if (!gw_set_defaults_backend(srv, p, cpv->v.a, gw, 0,
                                             cpk[cpv->k_id].k)) {
                    gw_plugin_config_free(gw);
                    return HANDLER_ERROR;
                }
                cpv->v.v = gw;
                cpv->vtype = T_CONFIG_LOCAL;
                break;
              }
              case 1: /* fastcgi.balance */
                cpv->v.u = (unsigned int)gw_get_defaults_balance(srv, cpv->v.b);
                break;
              case 2: /* fastcgi.debug */
              case 3: /* fastcgi.map-extensions */
                break;
              default:/* should not happen */
                break;
            }
        }
    }

    /* default is 0 */
    /*p->defaults.balance = (unsigned int)gw_get_defaults_balance(srv, NULL);*/

    /* initialize p->defaults from global config context */
    if (p->nconfig > 0 && p->cvlist->v.u2[1]) {
        const config_plugin_value_t *cpv = p->cvlist + p->cvlist->v.u2[0];
        if (-1 != cpv->k_id)
            mod_fastcgi_merge_config(&p->defaults, cpv);
    }

    return HANDLER_GO_ON;
}


static int fcgi_env_add(void *venv, const char *key, size_t key_len, const char *val, size_t val_len) {
	buffer *env = venv;
	size_t len;
	char len_enc[8];
	size_t len_enc_len = 0;
	char *dst;

	if (!key || (!val && val_len)) return -1;

	len = key_len + val_len;

	len += key_len > 127 ? 4 : 1;
	len += val_len > 127 ? 4 : 1;

	if (buffer_string_length(env) + len >= FCGI_MAX_LENGTH + sizeof(FCGI_BeginRequestRecord) + sizeof(FCGI_Header)) {
		/**
		 * we can't append more headers, ignore it
		 */
		return -1;
	}

	/**
	 * field length can be 31bit max
	 *
	 * HINT: this can't happen as FCGI_MAX_LENGTH is only 16bit
	 */
	force_assert(key_len < 0x7fffffffu);
	force_assert(val_len < 0x7fffffffu);

	if (buffer_string_space(env) < len) {
		size_t extend = env->size * 2 - buffer_string_length(env);
		extend = extend > len ? extend : len + 4095;
		buffer_string_prepare_append(env, extend);
	}

	if (key_len > 127) {
		len_enc[len_enc_len++] = ((key_len >> 24) & 0xff) | 0x80;
		len_enc[len_enc_len++] = (key_len >> 16) & 0xff;
		len_enc[len_enc_len++] = (key_len >> 8) & 0xff;
		len_enc[len_enc_len++] = (key_len >> 0) & 0xff;
	} else {
		len_enc[len_enc_len++] = (key_len >> 0) & 0xff;
	}

	if (val_len > 127) {
		len_enc[len_enc_len++] = ((val_len >> 24) & 0xff) | 0x80;
		len_enc[len_enc_len++] = (val_len >> 16) & 0xff;
		len_enc[len_enc_len++] = (val_len >> 8) & 0xff;
		len_enc[len_enc_len++] = (val_len >> 0) & 0xff;
	} else {
		len_enc[len_enc_len++] = (val_len >> 0) & 0xff;
	}

	dst = buffer_string_prepare_append(env, len);
	memcpy(dst, len_enc, len_enc_len);
	memcpy(dst + len_enc_len, key, key_len);
	if (val_len) memcpy(dst + len_enc_len + key_len, val, val_len);
	buffer_commit(env, len);

	return 0;
}

static void fcgi_header(FCGI_Header * header, unsigned char type, int request_id, int contentLength, unsigned char paddingLength) {
	force_assert(contentLength <= FCGI_MAX_LENGTH);
	
	header->version = FCGI_VERSION_1;
	header->type = type;
	header->requestIdB0 = request_id & 0xff;
	header->requestIdB1 = (request_id >> 8) & 0xff;
	header->contentLengthB0 = contentLength & 0xff;
	header->contentLengthB1 = (contentLength >> 8) & 0xff;
	header->paddingLength = paddingLength;
	header->reserved = 0;
}

static handler_t fcgi_stdin_append(handler_ctx *hctx) {
	FCGI_Header header;
	chunkqueue * const req_cq = &hctx->r->reqbody_queue;
	off_t offset, weWant;
	off_t req_cqlen = chunkqueue_length(req_cq);
	int request_id = hctx->request_id;
	if (req_cqlen > MAX_WRITE_LIMIT) req_cqlen = MAX_WRITE_LIMIT;

	/* something to send ? */
	for (offset = 0; offset != req_cqlen; offset += weWant) {
		weWant = req_cqlen - offset > FCGI_MAX_LENGTH ? FCGI_MAX_LENGTH : req_cqlen - offset;

		if (-1 != hctx->wb_reqlen) {
			if (hctx->wb_reqlen >= 0) {
				hctx->wb_reqlen += sizeof(header);
			} else {
				hctx->wb_reqlen -= sizeof(header);
			}
		}

		fcgi_header(&(header), FCGI_STDIN, request_id, weWant, 0);
		(chunkqueue_is_empty(&hctx->wb) || hctx->wb.first->type == MEM_CHUNK) /* else FILE_CHUNK for temp file */
		  ? chunkqueue_append_mem(&hctx->wb, (const char *)&header, sizeof(header))
		  : chunkqueue_append_mem_min(&hctx->wb, (const char *)&header, sizeof(header));
		chunkqueue_steal(&hctx->wb, req_cq, weWant);
		/*(hctx->wb_reqlen already includes reqbody_length)*/
	}

	if (hctx->wb.bytes_in == hctx->wb_reqlen) {
		/* terminate STDIN */
		/* (future: must defer ending FCGI_STDIN
		 *  if might later upgrade protocols
		 *  and then have more data to send) */
		fcgi_header(&(header), FCGI_STDIN, request_id, 0, 0);
		chunkqueue_append_mem(&hctx->wb, (const char *)&header, sizeof(header));
		hctx->wb_reqlen += (int)sizeof(header);
	}

	return HANDLER_GO_ON;
}

static handler_t fcgi_create_env(handler_ctx *hctx) {
	FCGI_BeginRequestRecord beginRecord;
	FCGI_Header header;
	int request_id;

	gw_host *host = hctx->host;
	request_st * const r = hctx->r;

	http_cgi_opts opts = {
	  (hctx->gw_mode == FCGI_AUTHORIZER),
	  host->break_scriptfilename_for_php,
	  host->docroot,
	  host->strip_request_uri
	};

	size_t rsz = (size_t)(r->read_queue.bytes_out - hctx->wb.bytes_in);
	if (rsz >= 65536) rsz = r->rqst_header_len;
	buffer * const b = chunkqueue_prepend_buffer_open_sz(&hctx->wb, rsz);

	/* send FCGI_BEGIN_REQUEST */

	if (hctx->request_id == 0) {
		hctx->request_id = 1; /* always use id 1 as we don't use multiplexing */
	} else {
		log_error(r->conf.errh, __FILE__, __LINE__,
		  "fcgi-request is already in use: %d", hctx->request_id);
	}
	request_id = hctx->request_id;

	fcgi_header(&(beginRecord.header), FCGI_BEGIN_REQUEST, request_id, sizeof(beginRecord.body), 0);
	beginRecord.body.roleB0 = hctx->gw_mode;
	beginRecord.body.roleB1 = 0;
	beginRecord.body.flags = 0;
	memset(beginRecord.body.reserved, 0, sizeof(beginRecord.body.reserved));

	buffer_copy_string_len(b, (const char *)&beginRecord, sizeof(beginRecord));
	fcgi_header(&header, FCGI_PARAMS, request_id, 0, 0); /*(set aside space to fill in later)*/
	buffer_append_string_len(b, (const char *)&header, sizeof(header));

	/* send FCGI_PARAMS */

	if (0 != http_cgi_headers(r, &opts, fcgi_env_add, b)) {
		r->http_status = 400;
		r->handler_module = NULL;
		buffer_clear(b);
		chunkqueue_remove_finished_chunks(&hctx->wb);
		return HANDLER_FINISHED;
	} else {
		fcgi_header(&(header), FCGI_PARAMS, request_id,
			    buffer_string_length(b) - sizeof(FCGI_BeginRequestRecord) - sizeof(FCGI_Header), 0);
		memcpy(b->ptr+sizeof(FCGI_BeginRequestRecord), (const char *)&header, sizeof(header));

		fcgi_header(&(header), FCGI_PARAMS, request_id, 0, 0);
		buffer_append_string_len(b, (const char *)&header, sizeof(header));

		hctx->wb_reqlen = buffer_string_length(b);
		chunkqueue_prepend_buffer_commit(&hctx->wb);
	}

	if (r->reqbody_length) {
		/*chunkqueue_append_chunkqueue(&hctx->wb, &r->reqbody_queue);*/
		if (r->reqbody_length > 0)
			hctx->wb_reqlen += r->reqbody_length;/* (eventual) (minimal) total request size, not necessarily including all fcgi_headers around content length yet */
		else /* as-yet-unknown total request size (Transfer-Encoding: chunked)*/
			hctx->wb_reqlen = -hctx->wb_reqlen;
	}
	fcgi_stdin_append(hctx);

	status_counter_inc(CONST_STR_LEN("fastcgi.requests"));
	return HANDLER_GO_ON;
}

typedef struct {
	unsigned int len;
	int      type;
	int      padding;
	int      request_id;
} fastcgi_response_packet;

static int fastcgi_get_packet(handler_ctx *hctx, fastcgi_response_packet *packet) {
	FCGI_Header header;
	off_t rblen = chunkqueue_length(hctx->rb);
	if (rblen < (off_t)sizeof(FCGI_Header)) {
		/* no header */
		if (hctx->conf.debug && 0 != rblen) {
			log_error(hctx->r->conf.errh, __FILE__, __LINE__,
			  "FastCGI: header too small: %lld bytes < %zu bytes, "
			  "waiting for more data", (long long)rblen, sizeof(FCGI_Header));
		}
		return -1;
	}
	char *ptr = (char *)&header;
	uint32_t rd = sizeof(FCGI_Header);
	if (chunkqueue_peek_data(hctx->rb, &ptr, &rd, hctx->r->conf.errh) < 0)
		return -1;
	if (rd != sizeof(FCGI_Header))
		return -1;
	if (ptr != (char *)&header) /* copy into aligned struct */
		memcpy(&header, ptr, sizeof(FCGI_Header));

	/* we have at least a header, now check how much we have to fetch */
	packet->len = (header.contentLengthB0 | (header.contentLengthB1 << 8)) + header.paddingLength;
	packet->request_id = (header.requestIdB0 | (header.requestIdB1 << 8));
	packet->type = header.type;
	packet->padding = header.paddingLength;

	if (packet->len > (unsigned int)rblen-sizeof(FCGI_Header)) {
		return -1; /* we didn't get the full packet */
	}

	chunkqueue_mark_written(hctx->rb, sizeof(FCGI_Header));
	return 0;
}

static void fastcgi_get_packet_body(buffer * const b, handler_ctx * const hctx, const fastcgi_response_packet * const packet) {
    /* copy content; hctx->rb must contain at least packet->len content */
    /* (read entire packet and then truncate padding, if present) */
    const uint32_t blen = buffer_string_length(b);
    if (chunkqueue_read_data(hctx->rb,
                             buffer_string_prepare_append(b, packet->len),
                             packet->len, hctx->r->conf.errh) < 0)
        return; /*(should not happen; should all be in memory)*/
    buffer_string_set_length(b, blen + packet->len - packet->padding);
}

__attribute_cold__
__attribute_noinline__
static int
mod_fastcgi_chunk_decode_transfer_cqlen (request_st * const r, chunkqueue * const src, const unsigned int len)
{
    if (0 == len) return 0;

    /* specialized for mod_fastcgi to decode chunked encoding;
     * FastCGI packet data is all type MEM_CHUNK
     * entire src cq is processed, minus packet.padding at end
     * (This extra work can be avoided if FastCGI backend does not send
     *  Transfer-Encoding: chunked, which FastCGI is not supposed to do) */
    uint32_t remain = len, wr;
    for (const chunk *c = src->first; c && remain; c = c->next, remain -= wr) {
        /*assert(c->type == MEM_CHUNK);*/
        wr = buffer_string_length(c->mem) - c->offset;
        if (wr > remain) wr = remain;
        if (0 != http_chunk_decode_append_mem(r, c->mem->ptr+c->offset, wr))
            return -1;
    }
    chunkqueue_mark_written(src, len);
    return 0;
}

static int
mod_fastcgi_transfer_cqlen (request_st * const r, chunkqueue * const src, const unsigned int len)
{
    return (!r->resp_decode_chunked)
      ? http_chunk_transfer_cqlen(r, src, len)
      : mod_fastcgi_chunk_decode_transfer_cqlen(r, src, len);
}

static handler_t fcgi_recv_parse(request_st * const r, struct http_response_opts_t *opts, buffer *b, size_t n) {
	handler_ctx *hctx = (handler_ctx *)opts->pdata;
	int fin = 0;

	if (0 == n) {
		if (-1 == hctx->request_id) return HANDLER_FINISHED; /*(flag request ended)*/
		if (!(fdevent_fdnode_interest(hctx->fdn) & FDEVENT_IN)
		    && !(r->conf.stream_response_body & FDEVENT_STREAM_RESPONSE_POLLRDHUP))
			return HANDLER_GO_ON;
		log_error(r->conf.errh, __FILE__, __LINE__,
		  "unexpected end-of-file (perhaps the fastcgi process died):"
		  "pid: %d socket: %s",
		  hctx->proc->pid, hctx->proc->connection_name->ptr);

		return HANDLER_ERROR;
	}

	chunkqueue_append_buffer(hctx->rb, b);

	/*
	 * parse the fastcgi packets and forward the content to the write-queue
	 *
	 */
	while (fin == 0) {
		fastcgi_response_packet packet;

		/* check if we have at least one packet */
		if (0 != fastcgi_get_packet(hctx, &packet)) {
			/* no full packet */
			break;
		}

		switch(packet.type) {
		case FCGI_STDOUT:
			if (packet.len == 0) break;

			/* is the header already finished */
			if (0 == r->resp_body_started) {
				/* split header from body */
				buffer *hdrs = hctx->response;
				if (NULL == hdrs) {
					hdrs = r->tmp_buf;
					buffer_clear(hdrs);
				}
				fastcgi_get_packet_body(hdrs, hctx, &packet);
				if (HANDLER_GO_ON != http_response_parse_headers(r, &hctx->opts, hdrs)) {
					hctx->send_content_body = 0;
					fin = 1;
					break;
				}
				if (0 == r->resp_body_started) {
					if (!hctx->response) {
						hctx->response = chunk_buffer_acquire();
						buffer_copy_buffer(hctx->response, hdrs);
					}
				}
				else if (hctx->gw_mode == GW_AUTHORIZER &&
					 (r->http_status == 0 || r->http_status == 200)) {
					/* authorizer approved request; ignore the content here */
					hctx->send_content_body = 0;
				}
			} else if (hctx->send_content_body) {
				if (0 != mod_fastcgi_transfer_cqlen(r, hctx->rb, packet.len - packet.padding)) {
					/* error writing to tempfile;
					 * truncate response or send 500 if nothing sent yet */
					fin = 1;
				}
				if (packet.padding) chunkqueue_mark_written(hctx->rb, packet.padding);
			} else {
				chunkqueue_mark_written(hctx->rb, packet.len);
			}
			break;
		case FCGI_STDERR:
			if (packet.len) {
				buffer * const tb = r->tmp_buf;
				buffer_clear(tb);
				fastcgi_get_packet_body(tb, hctx, &packet);
				log_error_multiline_buffer(r->conf.errh, __FILE__, __LINE__, tb,
				  "FastCGI-stderr:");
			}
			break;
		case FCGI_END_REQUEST:
			hctx->request_id = -1; /*(flag request ended)*/
			fin = 1;
			break;
		default:
			log_error(r->conf.errh, __FILE__, __LINE__,
			  "FastCGI: header.type not handled: %d", packet.type);
			chunkqueue_mark_written(hctx->rb, packet.len);
			break;
		}
	}

	return 0 == fin ? HANDLER_GO_ON : HANDLER_FINISHED;
}

static handler_t fcgi_check_extension(request_st * const r, void *p_d, int uri_path_handler) {
	plugin_data *p = p_d;
	handler_t rc;

	if (NULL != r->handler_module) return HANDLER_GO_ON;

	mod_fastcgi_patch_config(r, p);
	if (NULL == p->conf.exts) return HANDLER_GO_ON;

	rc = gw_check_extension(r, p, uri_path_handler, 0);
	if (HANDLER_GO_ON != rc) return rc;

	if (r->handler_module == p->self) {
		handler_ctx *hctx = r->plugin_ctx[p->id];
		hctx->opts.backend = BACKEND_FASTCGI;
		hctx->opts.parse = fcgi_recv_parse;
		hctx->opts.pdata = hctx;
		hctx->stdin_append = fcgi_stdin_append;
		hctx->create_env = fcgi_create_env;
		if (!hctx->rb) {
			hctx->rb = chunkqueue_init(NULL);
		}
		else {
			chunkqueue_reset(hctx->rb);
		}
	}

	return HANDLER_GO_ON;
}

/* uri-path handler */
static handler_t fcgi_check_extension_1(request_st * const r, void *p_d) {
	return fcgi_check_extension(r, p_d, 1);
}

/* start request handler */
static handler_t fcgi_check_extension_2(request_st * const r, void *p_d) {
	return fcgi_check_extension(r, p_d, 0);
}


int mod_fastcgi_plugin_init(plugin *p);
int mod_fastcgi_plugin_init(plugin *p) {
	p->version      = LIGHTTPD_VERSION_ID;
	p->name         = "fastcgi";

	p->init         = gw_init;
	p->cleanup      = gw_free;
	p->set_defaults = mod_fastcgi_set_defaults;
	p->handle_request_reset    = gw_handle_request_reset;
	p->handle_uri_clean        = fcgi_check_extension_1;
	p->handle_subrequest_start = fcgi_check_extension_2;
	p->handle_subrequest       = gw_handle_subrequest;
	p->handle_trigger          = gw_handle_trigger;
	p->handle_waitpid          = gw_handle_waitpid_cb;

	return 0;
}
