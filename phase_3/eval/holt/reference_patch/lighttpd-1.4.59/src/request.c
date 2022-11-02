/*
 * request - HTTP request processing
 *
 * Fully-rewritten from original EXCEPT for request_check_hostname()
 * Copyright(c) 2018 Glenn Strauss gstrauss()gluelogic.com  All rights reserved
 * License: BSD 3-clause (same as lighttpd)
 */
#include "first.h"

#include "request.h"
#include "burl.h"
#include "http_header.h"
#include "http_kv.h"
#include "log.h"
#include "sock_addr.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int request_check_hostname(buffer * const host) {
	enum { DOMAINLABEL, TOPLABEL } stage = TOPLABEL;
	size_t i;
	int label_len = 0;
	size_t host_len, hostport_len;
	char *colon;
	int is_ip = -1; /* -1 don't know yet, 0 no, 1 yes */
	int level = 0;

	/*
	 *       hostport      = host [ ":" port ]
	 *       host          = hostname | IPv4address | IPv6address
	 *       hostname      = *( domainlabel "." ) toplabel [ "." ]
	 *       domainlabel   = alphanum | alphanum *( alphanum | "-" ) alphanum
	 *       toplabel      = alpha | alpha *( alphanum | "-" ) alphanum
	 *       IPv4address   = 1*digit "." 1*digit "." 1*digit "." 1*digit
	 *       IPv6address   = "[" ... "]"
	 *       port          = *digit
	 */

	/* IPv6 address */
	if (host->ptr[0] == '[') {
		char *c = host->ptr + 1;
		int colon_cnt = 0;

		/* check the address inside [...] */
		for (; *c && *c != ']'; c++) {
			if (*c == ':') {
				if (++colon_cnt > 7) {
					return -1;
				}
			} else if (!light_isxdigit(*c) && '.' != *c) {
				return -1;
			}
		}

		/* missing ] */
		if (!*c) {
			return -1;
		}

		/* check port */
		if (*(c+1) == ':') {
			for (c += 2; *c; c++) {
				if (!light_isdigit(*c)) {
					return -1;
				}
			}
		}
		else if ('\0' != *(c+1)) {
			/* only a port is allowed to follow [...] */
			return -1;
		}
		return 0;
	}

	hostport_len = host_len = buffer_string_length(host);

	if (NULL != (colon = memchr(host->ptr, ':', host_len))) {
		char *c = colon + 1;

		/* check portnumber */
		for (; *c; c++) {
			if (!light_isdigit(*c)) return -1;
		}

		/* remove the port from the host-len */
		host_len = colon - host->ptr;
	}

	/* Host is empty */
	if (host_len == 0) return -1;

	/* if the hostname ends in a "." strip it */
	if (host->ptr[host_len-1] == '.') {
		/* shift port info one left */
		if (NULL != colon) memmove(colon-1, colon, hostport_len - host_len);
		buffer_string_set_length(host, --hostport_len);
		if (--host_len == 0) return -1;
	}


	/* scan from the right and skip the \0 */
	for (i = host_len; i-- > 0; ) {
		const char c = host->ptr[i];

		switch (stage) {
		case TOPLABEL:
			if (c == '.') {
				/* only switch stage, if this is not the last character */
				if (i != host_len - 1) {
					if (label_len == 0) {
						return -1;
					}

					/* check the first character at right of the dot */
					if (is_ip == 0) {
						if (!light_isalnum(host->ptr[i+1])) {
							return -1;
						}
					} else if (!light_isdigit(host->ptr[i+1])) {
						is_ip = 0;
					} else if ('-' == host->ptr[i+1]) {
						return -1;
					} else {
						/* just digits */
						is_ip = 1;
					}

					stage = DOMAINLABEL;

					label_len = 0;
					level++;
				} else if (i == 0) {
					/* just a dot and nothing else is evil */
					return -1;
				}
			} else if (i == 0) {
				/* the first character of the hostname */
				if (!light_isalnum(c)) {
					return -1;
				}
				label_len++;
			} else {
				if (c != '-' && !light_isalnum(c)) {
					return -1;
				}
				if (is_ip == -1) {
					if (!light_isdigit(c)) is_ip = 0;
				}
				label_len++;
			}

			break;
		case DOMAINLABEL:
			if (is_ip == 1) {
				if (c == '.') {
					if (label_len == 0) {
						return -1;
					}

					label_len = 0;
					level++;
				} else if (!light_isdigit(c)) {
					return -1;
				} else {
					label_len++;
				}
			} else {
				if (c == '.') {
					if (label_len == 0) {
						return -1;
					}

					/* c is either - or alphanum here */
					if ('-' == host->ptr[i+1]) {
						return -1;
					}

					label_len = 0;
					level++;
				} else if (i == 0) {
					if (!light_isalnum(c)) {
						return -1;
					}
					label_len++;
				} else {
					if (c != '-' && !light_isalnum(c)) {
						return -1;
					}
					label_len++;
				}
			}

			break;
		}
	}

	/* a IP has to consist of 4 parts */
	if (is_ip == 1 && level != 3) {
		return -1;
	}

	if (label_len == 0) {
		return -1;
	}

	return 0;
}

int http_request_host_normalize(buffer * const b, const int scheme_port) {
    /*
     * check for and canonicalize numeric IP address and portnum (optional)
     * (IP address may be followed by ":portnum" (optional))
     * - IPv6: "[...]"
     * - IPv4: "x.x.x.x"
     * - IPv4: 12345678   (32-bit decimal number)
     * - IPv4: 012345678  (32-bit octal number)
     * - IPv4: 0x12345678 (32-bit hex number)
     *
     * allow any chars (except ':' and '\0' and stray '[' or ']')
     *   (other code may check chars more strictly or more pedantically)
     * ':'  delimits (optional) port at end of string
     * "[]" wraps IPv6 address literal
     * '\0' should have been rejected earlier were it present
     *
     * any chars includes, but is not limited to:
     * - allow '-' any where, even at beginning of word
     *     (security caution: might be confused for cmd flag if passed to shell)
     * - allow all-digit TLDs
     *     (might be mistaken for IPv4 addr by inet_aton()
     *      unless non-digits appear in subdomain)
     */

    /* Note: not using getaddrinfo() since it does not support "[]" around IPv6
     * and is not as lenient as inet_aton() and inet_addr() for IPv4 strings.
     * Not using inet_pton() (when available) on IPv4 for similar reasons. */

    const char * const p = b->ptr;
    const size_t blen = buffer_string_length(b);
    long port = 0;

    if (*p != '[') {
        char * const colon = (char *)memchr(p, ':', blen);
        if (colon) {
            if (*p == ':') return -1; /*(empty host then port, or naked IPv6)*/
            if (colon[1] != '\0') {
                char *e;
                port = strtol(colon+1, &e, 0); /*(allow decimal, octal, hex)*/
                if (0 < port && port <= USHRT_MAX && *e == '\0') {
                    /* valid port */
                } else {
                    return -1;
                }
            } /*(else ignore stray colon at string end)*/
            buffer_string_set_length(b, (size_t)(colon - p)); /*(remove port str)*/
        }

        if (light_isdigit(*p)) do {
            /* (IPv4 address literal or domain starting w/ digit (e.g. 3com))*/
            /* (check one-element cache of normalized IPv4 address string) */
            static struct { char s[INET_ADDRSTRLEN]; size_t n; } laddr;
            size_t n = colon ? (size_t)(colon - p) : blen;
            sock_addr addr;
            if (n == laddr.n && 0 == memcmp(p, laddr.s, n)) break;
            if (1 == sock_addr_inet_pton(&addr, p, AF_INET, 0)) {
                sock_addr_inet_ntop_copy_buffer(b, &addr);
                n = buffer_string_length(b);
                if (n < sizeof(laddr.s)) memcpy(laddr.s, b->ptr, (laddr.n = n));
            }
        } while (0);
    } else do { /* IPv6 addr */
      #if defined(HAVE_IPV6) && defined(HAVE_INET_PTON)

        /* (check one-element cache of normalized IPv4 address string) */
        static struct { char s[INET6_ADDRSTRLEN]; size_t n; } laddr;
        sock_addr addr;
        char *bracket = b->ptr+blen-1;
        char *percent = strchr(b->ptr+1, '%');
        size_t len;
        int rc;
        char buf[INET6_ADDRSTRLEN+16]; /*(+16 for potential %interface name)*/
        if (blen <= 2) return -1; /*(invalid "[]")*/
        if (*bracket != ']') {
            bracket = (char *)memchr(b->ptr+1, ']', blen-1);
            if (NULL == bracket || bracket[1] != ':'  || bracket - b->ptr == 1){
               return -1;
            }
            if (bracket[2] != '\0') { /*(ignore stray colon at string end)*/
                char *e;
                port = strtol(bracket+2, &e, 0); /*(allow decimal, octal, hex)*/
                if (0 < port && port <= USHRT_MAX && *e == '\0') {
                    /* valid port */
                } else {
                    return -1;
                }
            }
        }

        len = (size_t)((percent ? percent : bracket) - (b->ptr+1));
        if (laddr.n == len && 0 == memcmp(laddr.s, b->ptr+1, len)) {
            /* truncate after ']' and re-add normalized port, if needed */
            buffer_string_set_length(b, (size_t)(bracket - b->ptr + 1));
            break;
        }

        *bracket = '\0';/*(terminate IPv6 string)*/
        if (percent) *percent = '\0'; /*(remove %interface from address)*/
        rc = sock_addr_inet_pton(&addr, b->ptr+1, AF_INET6, 0);
        if (percent) *percent = '%'; /*(restore %interface)*/
        *bracket = ']'; /*(restore bracket)*/
        if (1 != rc) return -1;

        sock_addr_inet_ntop(&addr, buf, sizeof(buf));
        len = strlen(buf);
        if (percent) {
            if (percent > bracket) return -1;
            if (len + (size_t)(bracket - percent) >= sizeof(buf)) return -1;
            if (len < sizeof(laddr.s)) memcpy(laddr.s, buf, (laddr.n = len));
            memcpy(buf+len, percent, (size_t)(bracket - percent));
            len += (size_t)(bracket - percent);
        }
        buffer_string_set_length(b, 1); /* truncate after '[' */
        buffer_append_string_len(b, buf, len);
        buffer_append_string_len(b, CONST_STR_LEN("]"));

      #else

        return -1;

      #endif
    } while (0);

    if (0 != port && port != scheme_port) {
        buffer_append_string_len(b, CONST_STR_LEN(":"));
        buffer_append_int(b, (int)port);
    }

    return 0;
}

int http_request_host_policy (buffer * const b, const unsigned int http_parseopts, const int scheme_port) {
    return (((http_parseopts & HTTP_PARSEOPT_HOST_STRICT)
             && 0 != request_check_hostname(b))
            || ((http_parseopts & HTTP_PARSEOPT_HOST_NORMALIZE)
                && 0 != http_request_host_normalize(b, scheme_port)));
}

__attribute_pure__ /*(could be even more strict and use __attribute_const__)*/
static int request_uri_is_valid_char(const unsigned char c) {
	return (c > 32 && c != 127 && c != 255);
}

__attribute_cold__
__attribute_noinline__
static int http_request_header_line_invalid(request_st * const restrict r, const int status, const char * const restrict msg) {
    if (r->conf.log_request_header_on_error) {
        if (msg) log_error(r->conf.errh, __FILE__, __LINE__, "%s", msg);
    }
    return status;
}

__attribute_cold__
__attribute_noinline__
static int http_request_header_char_invalid(request_st * const restrict r, const char ch, const char * const restrict msg) {
    if (r->conf.log_request_header_on_error) {
        if ((unsigned char)ch > 32 && ch != 127) {
            log_error(r->conf.errh, __FILE__, __LINE__, "%s ('%c')", msg, ch);
        }
        else {
            log_error(r->conf.errh, __FILE__, __LINE__, "%s (0x%x)", msg, ch);
        }
    }
    return 400;
}


int64_t
li_restricted_strtoint64 (const char *v, const uint32_t vlen, const char ** const err)
{
    /* base 10 strtoll() parsing exactly vlen chars and requiring digits 0-9 */
    /* rejects negative numbers and considers values > INT64_MAX an error */
    /* note: errno is not set; detect error if *err != v+vlen upon return */
    /*(caller must check 0 == vlen if that is to be an error for caller)*/
    int64_t rv = 0;
    uint32_t i;
    for (i = 0; i < vlen; ++i) {
        const uint8_t c = ((uint8_t *)v)[i] - '0'; /*(unsigned; underflow ok)*/
        if (c > 9) break;
        if (rv > INT64_MAX/10) break;
        rv *= 10;
        if (rv > INT64_MAX - c) break;
        rv += c;
    }
    *err = v+i;
    return rv;
}


/* add header to list of headers
 * certain headers are also parsed
 * might drop a header if deemed unnecessary/broken
 *
 * returns 0 on success, HTTP status on error
 */
static int http_request_parse_single_header(request_st * const restrict r, const enum http_header_e id, const char * const restrict k, const size_t klen, const char * const restrict v, const size_t vlen) {
    buffer **saveb = NULL;

    /*
     * Note: k might not be '\0'-terminated
     * Note: v is not '\0'-terminated
     *   With lighttpd HTTP/1.1 parser, v ends with whitespace
     *     (one of '\r' '\n' ' ' '\t')
     *   With lighttpd HTTP/2 parser, v should not be accessed beyond vlen
     *     (care must be taken to avoid libc funcs which expect z-strings)
     */
    /*assert(vlen);*//*(caller must not call this func with 0 klen or 0 vlen)*/

    switch (id) {
      /*case HTTP_HEADER_OTHER:*/
      default:
        break;
      case HTTP_HEADER_HOST:
        if (!light_btst(r->rqst_htags, HTTP_HEADER_HOST)) {
            saveb = &r->http_host;
            if (vlen >= 1024) { /*(expecting < 256)*/
                return http_request_header_line_invalid(r, 400, "uri-authority too long -> 400");
            }
        }
        else if (NULL != r->http_host
                 && buffer_is_equal_string(r->http_host, v, vlen)) {
            /* ignore all Host: headers if match authority in request line */
            return 0; /* ignore header */
        }
        else {
            return http_request_header_line_invalid(r, 400, "duplicate Host header -> 400");
        }
        break;
      case HTTP_HEADER_CONNECTION:
        /* "Connection: close" is common case if header is present */
        if ((vlen == 5 && buffer_eq_icase_ssn(v, CONST_STR_LEN("close")))
            || http_header_str_contains_token(v,vlen,CONST_STR_LEN("close"))) {
            r->keep_alive = 0;
            break;
        }
        if (http_header_str_contains_token(v,vlen,CONST_STR_LEN("keep-alive"))){
            r->keep_alive = 1;
            break;
        }
        break;
      case HTTP_HEADER_CONTENT_TYPE:
        if (light_btst(r->rqst_htags, HTTP_HEADER_CONTENT_TYPE)) {
            return http_request_header_line_invalid(r, 400, "duplicate Content-Type header -> 400");
        }
        break;
      case HTTP_HEADER_IF_NONE_MATCH:
        /* if dup, only the first one will survive */
        if (light_btst(r->rqst_htags, HTTP_HEADER_IF_NONE_MATCH)) {
            return 0; /* ignore header */
        }
        break;
      case HTTP_HEADER_CONTENT_LENGTH:
        if (!light_btst(r->rqst_htags, HTTP_HEADER_CONTENT_LENGTH)) {
            /*(trailing whitespace was removed from vlen)*/
            /*(not using strtoll() since v might not be z-string)*/
            const char *err;
            off_t clen = (off_t)li_restricted_strtoint64(v, vlen, &err);
            if (err == v+vlen) {
                /* (set only if not set to -1 by Transfer-Encoding: chunked) */
                if (0 == r->reqbody_length) r->reqbody_length = clen;
            }
            else {
                return http_request_header_line_invalid(r, 400, "invalid Content-Length header -> 400");
            }
        }
        else {
            return http_request_header_line_invalid(r, 400, "duplicate Content-Length header -> 400");
        }
        break;
      case HTTP_HEADER_HTTP2_SETTINGS:
        if (light_btst(r->rqst_htags, HTTP_HEADER_HTTP2_SETTINGS)) {
            return http_request_header_line_invalid(r, 400, "duplicate HTTP2-Settings header -> 400");
        }
        break;
      case HTTP_HEADER_IF_MODIFIED_SINCE:
        if (light_btst(r->rqst_htags, HTTP_HEADER_IF_MODIFIED_SINCE)) {
            /* Proxies sometimes send dup headers
             * if they are the same we ignore the second
             * if not, we raise an error */
            const buffer *vb =
              http_header_request_get(r, HTTP_HEADER_IF_MODIFIED_SINCE,
                                      CONST_STR_LEN("If-Modified-Since"));
            if (vb && buffer_eq_icase_slen(vb, v, vlen)) {
                /* ignore it if they are the same */
                return 0; /* ignore header */
            }
            else {
                return http_request_header_line_invalid(r, 400, "duplicate If-Modified-Since header -> 400");
            }
        }
        break;
      case HTTP_HEADER_TRANSFER_ENCODING:
        if (HTTP_VERSION_1_1 != r->http_version) {
            return http_request_header_line_invalid(r, 400,
              HTTP_VERSION_1_0 == r->http_version
                ? "HTTP/1.0 with Transfer-Encoding (bad HTTP/1.0 proxy?) -> 400"
                : "HTTP/2 with Transfer-Encoding is invalid -> 400");
        }

        if (!buffer_eq_icase_ss(v, vlen, CONST_STR_LEN("chunked"))) {
            /* Transfer-Encoding might contain additional encodings,
             * which are not currently supported by lighttpd */
            return http_request_header_line_invalid(r, 501, NULL); /* Not Implemented */
        }
        r->reqbody_length = -1;

        /* Transfer-Encoding is a hop-by-hop header,
         * which must not be blindly forwarded to backends */
        return 0; /* skip header */
    }

    http_header_request_append(r, id, k, klen, v, vlen);

    if (saveb) {
        *saveb = http_header_request_get(r, id, k, klen);
    }

    return 0;
}

__attribute_cold__
static int http_request_parse_proto_loose(request_st * const restrict r, const char * const restrict ptr, const size_t len, const unsigned int http_parseopts) {
    const char * proto = memchr(ptr, ' ', len);
    if (NULL == proto)
        return http_request_header_line_invalid(r, 400, "incomplete request line -> 400");
    proto = memchr(proto+1, ' ', len - (proto+1 - ptr));
    if (NULL == proto)
        return http_request_header_line_invalid(r, 400, "incomplete request line -> 400");
    ++proto;

    if (proto[0]=='H' && proto[1]=='T' && proto[2]=='T' && proto[3]=='P' && proto[4] == '/') {
        if (proto[5] == '1' && proto[6] == '.' && (proto[7] == '1' || proto[7] == '0')) {
            /* length already checked before calling this routine */
            /* (len != (size_t)(proto - ptr + 8)) */
            if (http_parseopts & HTTP_PARSEOPT_HEADER_STRICT) /*(http_header_strict)*/
                return http_request_header_line_invalid(r, 400, "incomplete request line -> 400");
            r->http_version = (proto[7] == '1') ? HTTP_VERSION_1_1 : HTTP_VERSION_1_0;
        }
        else
            return http_request_header_line_invalid(r, 505, "unknown HTTP version -> 505");
    }
    else
        return http_request_header_line_invalid(r, 400, "unknown protocol -> 400");

    /* keep-alive default: HTTP/1.1 -> true; HTTP/1.0 -> false */
    r->keep_alive = (HTTP_VERSION_1_0 != r->http_version);

    return 0;
}

__attribute_cold__
static const char * http_request_parse_reqline_uri(request_st * const restrict r, const char * const restrict uri, const size_t len, const unsigned int http_parseopts) {
    const char *nuri;
    if ((len > 7 && buffer_eq_icase_ssn(uri, "http://", 7)
        && NULL != (nuri = memchr(uri + 7, '/', len-7)))
       ||
       (len > 8 && buffer_eq_icase_ssn(uri, "https://", 8)
        && NULL != (nuri = memchr(uri + 8, '/', len-8)))) {
        const char * const host = uri + (uri[4] == ':' ? 7 : 8);
        const size_t hostlen = nuri - host;
        if (0 == hostlen || hostlen >= 1024) { /*(expecting < 256)*/
            http_request_header_line_invalid(r, 400, "uri-authority empty or too long -> 400");
            return NULL;
        }
        /* Insert as host header */
        http_header_request_set(r, HTTP_HEADER_HOST, CONST_STR_LEN("Host"), host, hostlen);
        r->http_host = http_header_request_get(r, HTTP_HEADER_HOST, CONST_STR_LEN("Host"));
        return nuri;
    } else if (!(http_parseopts & HTTP_PARSEOPT_HEADER_STRICT) /*(!http_header_strict)*/
           || (HTTP_METHOD_CONNECT == r->http_method && (uri[0] == ':' || light_isdigit(uri[0])))
           || (HTTP_METHOD_OPTIONS == r->http_method && uri[0] == '*' && 1 == len)) {
        /* (permitted) */
        return uri;
    } else {
        http_request_header_line_invalid(r, 400, "request-URI parse error -> 400");
        return NULL;
    }
}


__attribute_cold__
__attribute_noinline__
static int http_request_parse_header_other(request_st * const restrict r, const char * const restrict k, const int klen, const unsigned int http_header_strict);


int
http_request_validate_pseudohdrs (request_st * const restrict r, const int scheme, const unsigned int http_parseopts)
{
    /* :method is required to indicate method
     * CONNECT method must have :method and :authority
     * All other methods must have at least :method :scheme :path */

    if (HTTP_METHOD_UNSET == r->http_method)
        return http_request_header_line_invalid(r, 400,
          "missing pseudo-header method -> 400");

    if (HTTP_METHOD_CONNECT != r->http_method) {
        if (!scheme)
            return http_request_header_line_invalid(r, 400,
              "missing pseudo-header scheme -> 400");

        if (buffer_string_is_empty(&r->target))
            return http_request_header_line_invalid(r, 400,
              "missing pseudo-header path -> 400");

        const char * const uri = r->target.ptr;
        if (*uri != '/') { /* (common case: (*uri == '/')) */
            if (uri[0] != '*' || uri[1] != '\0'
                || HTTP_METHOD_OPTIONS != r->http_method)
                return http_request_header_line_invalid(r, 400,
                  "invalid pseudo-header path -> 400");
        }
    }
    else { /* HTTP_METHOD_CONNECT */
        if (NULL == r->http_host)
            return http_request_header_line_invalid(r, 400,
              "missing pseudo-header authority -> 400");
        if (!buffer_string_is_empty(&r->target) || scheme)
            return http_request_header_line_invalid(r, 400,
              "invalid pseudo-header with CONNECT -> 400");
        /*(reuse uri and ulen to assign to r->target)*/
        buffer_copy_buffer(&r->target, r->http_host);
    }
    buffer_copy_buffer(&r->target_orig, &r->target);

    /* r->http_host, if set, is checked with http_request_host_policy()
     * in http_request_parse() */

    /* copied and modified from end of http_request_parse_reqline() */

    /* check uri for invalid characters */
    const unsigned int http_header_strict =
      (http_parseopts & HTTP_PARSEOPT_HEADER_STRICT);
    if (http_header_strict
        && (http_parseopts & HTTP_PARSEOPT_URL_NORMALIZE_CTRLS_REJECT))
        return 0; /* URI will be checked in http_request_parse_target() */

    const uint32_t ulen = buffer_string_length(&r->target);
    const uint8_t * const uri = (uint8_t *)r->target.ptr;
    if (http_header_strict) {
        for (uint32_t i = 0; i < ulen; ++i) {
            if (!request_uri_is_valid_char(uri[i]))
                return http_request_header_char_invalid(r, uri[i],
                  "invalid character in URI -> 400");
        }
    }
    else {
        if (NULL != memchr(uri, '\0', ulen))
            return http_request_header_char_invalid(r, '\0',
              "invalid character in header -> 400");
    }

    return 0;
}


int
http_request_parse_header (request_st * const restrict r, http_header_parse_ctx * const restrict hpctx)
{
    /* Note: k and v might not be '\0' terminated strings;
     * care must be taken to avoid libc funcs which expect z-strings */
    const char * const restrict k = hpctx->k;
    const char * const restrict v = hpctx->v;
    const uint32_t klen = hpctx->klen;
    const uint32_t vlen = hpctx->vlen;

    if (0 == klen)
        return http_request_header_line_invalid(r, 400,
          "invalid header key -> 400");
    if (0 == vlen)
        return http_request_header_line_invalid(r, 400,
          "invalid header value -> 400");

    if ((hpctx->hlen += klen + vlen + 4) > hpctx->max_request_field_size) {
        /*(configurable with server.max-request-field-size; default 8k)*/
      #if 1 /* emit to error log for people sending large headers */
        log_error(r->conf.errh, __FILE__, __LINE__,
                  "oversized request header -> 431");
        return 431; /* Request Header Fields Too Large */
      #else
        /* 431 Request Header Fields Too Large */
        return http_request_header_line_invalid(r, 431,
          "oversized request header -> 431");
      #endif
    }

    if (2 == klen && k[0] == 't' && k[1] == 'e'
        && !buffer_eq_icase_ss(v, vlen, CONST_STR_LEN("trailers")))
        return http_request_header_line_invalid(r, 400,
          "invalid TE header value with HTTP/2 -> 400");

    if (!hpctx->trailers) {
        if (*k == ':') {
            /* HTTP/2 request pseudo-header fields */
            if (!hpctx->pseudo) /*(pseudo header after non-pseudo header)*/
                return http_request_header_line_invalid(r, 400,
                  "invalid pseudo-header -> 400");
            switch (klen-1) {
              case 4:
                if (0 == memcmp(k+1, "path", 4)) {
                    if (!buffer_string_is_empty(&r->target))
                        return http_request_header_line_invalid(r, 400,
                          "repeated pseudo-header -> 400");
                    buffer_copy_string_len(&r->target, v, vlen);
                    return 0;
                }
                break;
              case 6:
                if (0 == memcmp(k+1, "method", 6)) {
                    if (HTTP_METHOD_UNSET != r->http_method)
                        return http_request_header_line_invalid(r, 400,
                          "repeated pseudo-header -> 400");
                    r->http_method = get_http_method_key(v, vlen);
                    if (HTTP_METHOD_UNSET >= r->http_method)
                        return http_request_header_line_invalid(r, 501,
                          "unknown http-method -> 501");
                    return 0;
                }
                else if (0 == memcmp(k+1, "scheme", 6)) {
                    if (hpctx->scheme)
                        return http_request_header_line_invalid(r, 400,
                          "repeated pseudo-header -> 400");
                    switch (vlen) {/*(validated, but then ignored)*/
                      case 5: /* "https" */
                        if (v[4]!='s') break;
                        __attribute_fallthrough__
                      case 4: /* "http" */
                        if (v[0]=='h' && v[1]=='t' && v[2]=='t' && v[3]=='p') {
                            hpctx->scheme = 1;
                            return 0;
                        }
                        break;
                      default:
                        break;
                    }
                    return http_request_header_line_invalid(r, 400,
                      "unknown pseudo-header scheme -> 400");
                }
                break;
              case 9:
                if (0 == memcmp(k+1, "authority", 9)) {
                    if (r->http_host)
                        return http_request_header_line_invalid(r, 400,
                          "repeated pseudo-header -> 400");
                    if (vlen >= 1024) /*(expecting < 256)*/
                        return http_request_header_line_invalid(r, 400,
                          "invalid pseudo-header authority too long -> 400");
                    /* insert as host header */
                    http_header_request_set(r, HTTP_HEADER_HOST,
                                            CONST_STR_LEN("host"), v, vlen);
                    r->http_host =
                      http_header_request_get(r, HTTP_HEADER_HOST,
                                              CONST_STR_LEN("Host"));
                    return 0;
                }
                break;
              default:
                break;
            }
            return http_request_header_line_invalid(r, 400,
              "invalid pseudo-header -> 400");
        }
        else { /*(non-pseudo headers)*/
            if (hpctx->pseudo) { /*(transition to non-pseudo headers)*/
                hpctx->pseudo = 0;
                int status =
                  http_request_validate_pseudohdrs(r, hpctx->scheme,
                                                   hpctx->http_parseopts);
                if (0 != status) return status;
            }

            const unsigned int http_header_strict =
              (hpctx->http_parseopts & HTTP_PARSEOPT_HEADER_STRICT);

            for (uint32_t j = 0; j < klen; ++j) {
                if (light_islower(k[j]) || k[j] == '-')
                    continue; /*(common cases)*/
                if (light_isupper(k[j]))
                    return 400;
                if (0 != http_request_parse_header_other(r, k+j, klen-j,
                                                         http_header_strict))
                    return 400;
                break;
            }

            if (http_header_strict) {
                for (uint32_t j = 0; j < vlen; ++j) {
                    if ((((uint8_t *)v)[j] < 32 && v[j] != '\t') || v[j]==127)
                        return http_request_header_char_invalid(r, v[j],
                          "invalid character in header -> 400");
                }
            }
            else {
                if (NULL != memchr(v, '\0', vlen))
                    return http_request_header_char_invalid(r, '\0',
                      "invalid character in header -> 400");
            }

            const enum http_header_e id =
              hpctx->id ? hpctx->id : http_header_hkey_get_lc(k, klen);
            return http_request_parse_single_header(r, id, k, klen, v, vlen);
        }
    }
    else { /*(trailers)*/
        /* ignore trailers (after required HPACK decoding) if streaming
         * request body to backend since headers have already been sent
         * to backend via Common Gateway Interface (CGI) (CGI, FastCGI,
         * SCGI, etc) or HTTP/1.1 (proxy) (mod_proxy does not currently
         * support using HTTP/2 to connect to backends) */
      #if 0 /* (if needed, save flag in hpctx instead of fdevent.h dependency)*/
        if (r->conf.stream_request_body & FDEVENT_STREAM_REQUEST)
            return 0;
      #endif
        /* Note: do not unconditionally merge into headers since if
         * headers had already been sent to backend, then mod_accesslog
         * logging of request headers might be inaccurate.
         * Many simple backends do not support HTTP/1.1 requests sending
         * Transfer-Encoding: chunked, and even those that do might not
         * handle trailers.  Some backends do not even support HTTP/1.1.
         * For all these reasons, ignore trailers if streaming request
         * body to backend.  Revisit in future if adding support for
         * connecting to backends using HTTP/2 (with explicit config
         * option to force connecting to backends using HTTP/2) */

        /* XXX: TODO: request trailers not handled if streaming reqbody
         * XXX: must ensure that trailers are not disallowed field-names
         */

        return 0;
    }
}


static int http_request_parse_reqline(request_st * const restrict r, const char * const restrict ptr, const unsigned short * const restrict hoff, const unsigned int http_parseopts) {
    size_t len = hoff[2];

    /* parse the first line of the request
     * <method> <uri> <protocol>\r\n
     * */
    if (len < 13) /* minimum len with (!http_header_strict): "x x HTTP/1.0\n" */
        return http_request_header_line_invalid(r, 400, "invalid request line (too short) -> 400");
    if (ptr[len-2] == '\r')
        len-=2;
    else if (!(http_parseopts & HTTP_PARSEOPT_HEADER_STRICT)) /*(!http_header_strict)*/
        len-=1;
    else
        return http_request_header_line_invalid(r, 400, "missing CR before LF in header -> 400");

    /*
     * RFC7230:
     *   HTTP-version  = HTTP-name "/" DIGIT "." DIGIT
     *   HTTP-name     = %x48.54.54.50 ; "HTTP", case-sensitive
     */

    /* protocol is expected to be " HTTP/1.1" or " HTTP/1.0" at end of line */
    union proto_un {
      char c[8];
      uint64_t u;
    };
    static const union proto_un http_1_1 = {{'H','T','T','P','/','1','.','1'}};
    static const union proto_un http_1_0 = {{'H','T','T','P','/','1','.','0'}};
    const char *p = ptr + len - 8;
    union proto_un proto8;
    proto8.c[0]=p[0]; proto8.c[1]=p[1]; proto8.c[2]=p[2]; proto8.c[3]=p[3];
    proto8.c[4]=p[4]; proto8.c[5]=p[5]; proto8.c[6]=p[6]; proto8.c[7]=p[7];
    if (p[-1] == ' ' && http_1_1.u == proto8.u) {
        r->http_version = HTTP_VERSION_1_1;
        r->keep_alive = 1; /* keep-alive default: HTTP/1.1 -> true */
    }
    else if (p[-1] == ' ' && http_1_0.u == proto8.u) {
        r->http_version = HTTP_VERSION_1_0;
        r->keep_alive = 0; /* keep-alive default: HTTP/1.0 -> false */
    }
    else {
        int status = http_request_parse_proto_loose(r,ptr,len,http_parseopts);
        if (0 != status) return status;
        /*(space char must exist if http_request_parse_proto_loose() succeeds)*/
        for (p = ptr + len - 9; p[-1] != ' '; --p) ;
    }

    /* method is expected to be a short string in the general case */
    size_t i = 0;
    while (ptr[i] != ' ') ++i;
  #if 0 /*(space must exist if protocol was parsed successfully)*/
    while (i < len && ptr[i] != ' ') ++i;
    if (ptr[i] != ' ')
        return http_request_header_line_invalid(r, 400, "incomplete request line -> 400");
  #endif

    r->http_method = get_http_method_key(ptr, i);
    if (HTTP_METHOD_UNSET >= r->http_method)
        return http_request_header_line_invalid(r, 501, "unknown http-method -> 501");

    const char *uri = ptr + i + 1;

    if (uri == p)
        return http_request_header_line_invalid(r, 400, "no uri specified -> 400");
    len = (size_t)(p - uri - 1);

    if (*uri != '/') { /* (common case: (*uri == '/')) */
        uri = http_request_parse_reqline_uri(r, uri, len, http_parseopts);
        if (NULL == uri) return 400;
        len = (size_t)(p - uri - 1);
    }

    if (0 == len)
        return http_request_header_line_invalid(r, 400, "no uri specified -> 400");

    /* check uri for invalid characters */
    if (http_parseopts & HTTP_PARSEOPT_HEADER_STRICT) { /* http_header_strict */
        if ((http_parseopts & HTTP_PARSEOPT_URL_NORMALIZE_CTRLS_REJECT)) {
            /* URI will be checked in http_request_parse_target() */
        }
        else {
            for (i = 0; i < len; ++i) {
                if (!request_uri_is_valid_char(uri[i]))
                    return http_request_header_char_invalid(r, uri[i], "invalid character in URI -> 400");
            }
        }
    }
    else {
        /* check entire set of request headers for '\0' */
        if (NULL != memchr(ptr, '\0', hoff[hoff[0]]))
            return http_request_header_char_invalid(r, '\0', "invalid character in header -> 400");
    }

    buffer_copy_string_len(&r->target, uri, len);
    buffer_copy_string_len(&r->target_orig, uri, len);
    return 0;
}

int http_request_parse_target(request_st * const r, int scheme_port) {
    /* URI is parsed into components at start of request and may
     * also be re-parsed upon HANDLER_COMEBACK during the request
     * r->target is expected to be a "/url-part?query-part"
     *   (and *not* a fully-qualified URI starting https://...)
     * r->uri.authority is expected to be parsed elsewhere into r->http_host
     */

    /**
     * prepare strings
     *
     * - uri.path
     * - uri.query
     *
     */

    /**
     * Name according to RFC 2396
     *
     * - scheme
     * - authority
     * - path
     * - query
     *
     * (scheme)://(authority)(path)?(query)#fragment
     *
     */

    /* take initial scheme value from connection-level state
     * (request r->uri.scheme can be overwritten for later,
     *  for example by mod_extforward or mod_magnet) */
    if (scheme_port == 443)
        buffer_copy_string_len(&r->uri.scheme, CONST_STR_LEN("https"));
    else
        buffer_copy_string_len(&r->uri.scheme, CONST_STR_LEN("http"));

    buffer * const target = &r->target;
    if (r->http_method == HTTP_METHOD_CONNECT
        || (r->http_method == HTTP_METHOD_OPTIONS
            && target->ptr[0] == '*'
            && target->ptr[1] == '\0')) {
        /* CONNECT ... (or) OPTIONS * ... */
        buffer_copy_buffer(&r->uri.path, target);
        buffer_clear(&r->uri.query);
        return 0;
    }

    char *qstr;
    if (r->conf.http_parseopts & HTTP_PARSEOPT_URL_NORMALIZE) {
        /*uint32_t len = (uint32_t)buffer_string_length(target);*/
        int qs = burl_normalize(target, r->tmp_buf, r->conf.http_parseopts);
        if (-2 == qs)
            return http_request_header_line_invalid(r, 400,
              "invalid character in URI -> 400"); /* Bad Request */
        qstr = (-1 == qs) ? NULL : target->ptr+qs;
      #if 0  /* future: might enable here, or below for all requests */
        /* (Note: total header size not recalculated on HANDLER_COMEBACK
         *  even if other request headers changed during processing)
         * (If (0 != r->loops_per_request), then the generated
         *  request is too large.  Should a different error be returned?) */
        r->rqst_header_len -= len;
        len = buffer_string_length(target);
        r->rqst_header_len += len;
        if (len > MAX_HTTP_REQUEST_URI) {
            return 414; /* 414 URI Too Long */
        }
        if (r->rqst_header_len > MAX_HTTP_REQUEST_HEADER) {
            log_error(r->conf.errh, __FILE__, __LINE__,
              "request header fields too large: %u -> 431",
              r->rqst_header_len);
            return 431; /* Request Header Fields Too Large */
        }
      #endif
    }
    else {
        size_t rlen = buffer_string_length(target);
        qstr = memchr(target->ptr, '#', rlen);/* discard fragment */
        if (qstr) {
            rlen = (size_t)(qstr - target->ptr);
            buffer_string_set_length(target, rlen);
        }
        qstr = memchr(target->ptr, '?', rlen);
    }

    /** extract query string from target */
    const char * const pstr = target->ptr;
    const uint32_t rlen = buffer_string_length(target);
    uint32_t plen;
    if (NULL != qstr) {
        plen = (uint32_t)(qstr - pstr);
        buffer_copy_string_len(&r->uri.query, qstr + 1, rlen - plen - 1);
    }
    else {
        plen = rlen;
        buffer_clear(&r->uri.query);
    }
    buffer_copy_string_len(&r->uri.path, pstr, plen);

    /* decode url to path
     *
     * - decode url-encodings  (e.g. %20 -> ' ')
     * - remove path-modifiers (e.g. /../)
     */

    buffer_urldecode_path(&r->uri.path);
    buffer_path_simplify(&r->uri.path, &r->uri.path);
    if (r->uri.path.ptr[0] != '/')
        return http_request_header_line_invalid(r, 400,
          "uri-path does not begin with '/' -> 400"); /* Bad Request */

    return 0;
}

__attribute_cold__
__attribute_noinline__
static int http_request_parse_header_other(request_st * const restrict r, const char * const restrict k, const int klen, const unsigned int http_header_strict) {
    for (int i = 0; i < klen; ++i) {
        if (light_isalpha(k[i]) || k[i] == '-') continue; /*(common cases)*/
        /**
         * 1*<any CHAR except CTLs or separators>
         * CTLs == 0-31 + 127, CHAR = 7-bit ascii (0..127)
         *
         */
        switch(k[i]) {
        case ' ':
        case '\t':
            return http_request_header_line_invalid(r, 400, "WS character in key -> 400");
        case '(':
        case ')':
        case '<':
        case '>':
        case '@':
        case ',':
        case ';':
        case '\\':
        case '\"':
        case '/':
        case '[':
        case ']':
        case '?':
        case '=':
        case '{':
        case '}':
            return http_request_header_char_invalid(r, k[i], "invalid character in header key -> 400");
        default:
            if (http_header_strict ? (k[i] < 32 || ((unsigned char *)k)[i] >= 127) : k[i] == '\0')
                return http_request_header_char_invalid(r, k[i], "invalid character in header key -> 400");
            break; /* ok */
        }
    }
    return 0;
}

static int http_request_parse_headers(request_st * const restrict r, char * const restrict ptr, const unsigned short * const restrict hoff, const unsigned int http_parseopts) {
    const unsigned int http_header_strict = (http_parseopts & HTTP_PARSEOPT_HEADER_STRICT);

  #if 0 /*(not checked here; will later result in invalid label for HTTP header)*/
    int i = hoff[2];

    if (ptr[i] == ' ' || ptr[i] == '\t') {
        return http_request_header_line_invalid(r, 400, "WS at the start of first line -> 400");
    }
  #endif

    for (int i = 2; i < hoff[0]; ++i) {
        const char *k = ptr + hoff[i];
        /* one past last line hoff[hoff[0]] is to final "\r\n" */
        char *end = ptr + hoff[i+1];

        const char *colon = memchr(k, ':', end - k);
        if (NULL == colon)
            return http_request_header_line_invalid(r, 400, "invalid header missing ':' -> 400");

        const char *v = colon + 1;

        /* RFC7230 Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
         * 3.2.4.  Field Parsing
         * [...]
         * No whitespace is allowed between the header field-name and colon.  In
         * the past, differences in the handling of such whitespace have led to
         * security vulnerabilities in request routing and response handling.  A
         * server MUST reject any received request message that contains
         * whitespace between a header field-name and colon with a response code
         * of 400 (Bad Request).  A proxy MUST remove any such whitespace from a
         * response message before forwarding the message downstream.
         */
        /* (line k[-1] is always preceded by a '\n',
         *  including first header after request-line,
         *  so no need to check colon != k) */
        if (colon[-1] == ' ' || colon[-1] == '\t') {
            if (http_header_strict) {
                return http_request_header_line_invalid(r, 400, "invalid whitespace between field-name and colon -> 400");
            }
            else {
                /* remove trailing whitespace from key(if !http_header_strict)*/
                do { --colon; } while (colon[-1] == ' ' || colon[-1] == '\t');
            }
        }

        const int klen = (int)(colon - k);
        if (0 == klen)
            return http_request_header_line_invalid(r, 400, "invalid header key -> 400");
        const enum http_header_e id = http_header_hkey_get(k, klen);

        if (id == HTTP_HEADER_OTHER) {
            for (int j = 0; j < klen; ++j) {
                if (light_isalpha(k[j]) || k[j] == '-') continue; /*(common cases)*/
                if (0 != http_request_parse_header_other(r, k+j, klen-j, http_header_strict))
                    return 400;
                break;
            }
        }

        /* remove leading whitespace from value */
        while (*v == ' ' || *v == '\t') ++v;

        for (; i+1 <= hoff[0]; ++i) {
            end = ptr + hoff[i+1];
            if (end[0] != ' ' && end[0] != '\t') break;

            /* line folding */
          #ifdef __COVERITY__
            force_assert(end - k >= 2);
          #endif
            if (end[-2] == '\r')
                end[-2] = ' ';
            else if (http_header_strict)
                return http_request_header_line_invalid(r, 400, "missing CR before LF in header -> 400");
            end[-1] = ' ';
        }
      #ifdef __COVERITY__
        /*(buf holding k has non-zero request-line, so end[-2] valid)*/
        force_assert(end >= k + 2);
      #endif
        if (end[-2] == '\r')
            --end;
        else if (http_header_strict)
            return http_request_header_line_invalid(r, 400, "missing CR before LF in header -> 400");
        /* remove trailing whitespace from value (+ remove '\r\n') */
        /* (line k[-1] is always preceded by a '\n',
         *  including first header after request-line,
         *  so no need to check (end != k)) */
        do { --end; } while (end[-1] == ' ' || end[-1] == '\t');

        const int vlen = (int)(end - v);
        /* empty header-fields are not allowed by HTTP-RFC, we just ignore them */
        if (vlen <= 0) continue; /* ignore header */

        if (http_header_strict) {
            for (int j = 0; j < vlen; ++j) {
                if ((((unsigned char *)v)[j] < 32 && v[j] != '\t') || v[j]==127)
                    return http_request_header_char_invalid(r, v[j], "invalid character in header -> 400");
            }
        } /* else URI already checked in http_request_parse_reqline() for any '\0' */

        int status = http_request_parse_single_header(r, id, k, (size_t)klen, v, (size_t)vlen);
        if (0 != status) return status;
    }

    return 0;
}


static int
http_request_parse (request_st * const restrict r, const int scheme_port)
{
    int status = http_request_parse_target(r, scheme_port);
    if (0 != status) return status;

    /*(r->http_host might not be set until after parsing request headers)*/
    buffer_copy_buffer(&r->uri.authority, r->http_host);/*(copy even if empty)*/
    buffer_to_lower(&r->uri.authority);

    /* post-processing */
    const unsigned int http_parseopts = r->conf.http_parseopts;

    /* check hostname field if it is set */
    if (r->http_host) {
        if (0 != http_request_host_policy(r->http_host,
                                          http_parseopts, scheme_port))
            return http_request_header_line_invalid(r, 400, "Invalid Hostname -> 400");
    }
    else {
        if (r->http_version >= HTTP_VERSION_1_1)
            return http_request_header_line_invalid(r, 400, "HTTP/1.1 but Host missing -> 400");
    }

    if (0 == r->reqbody_length) {
        /* POST requires Content-Length (or Transfer-Encoding)
         * (-1 == r->reqbody_length when Transfer-Encoding: chunked)*/
        if (HTTP_METHOD_POST == r->http_method
            && !light_btst(r->rqst_htags, HTTP_HEADER_CONTENT_LENGTH)) {
            return http_request_header_line_invalid(r, 411, "POST-request, but content-length missing -> 411");
        }
    }
    else {
        /* (-1 == r->reqbody_length when Transfer-Encoding: chunked)*/
        if (-1 == r->reqbody_length
            && light_btst(r->rqst_htags, HTTP_HEADER_CONTENT_LENGTH)) {
            /* RFC7230 Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
             * 3.3.3.  Message Body Length
             * [...]
             * If a message is received with both a Transfer-Encoding and a
             * Content-Length header field, the Transfer-Encoding overrides the
             * Content-Length.  Such a message might indicate an attempt to
             * perform request smuggling (Section 9.5) or response splitting
             * (Section 9.4) and ought to be handled as an error.  A sender MUST
             * remove the received Content-Length field prior to forwarding such
             * a message downstream.
             */
            const unsigned int http_header_strict =
              (http_parseopts & HTTP_PARSEOPT_HEADER_STRICT);
            if (http_header_strict) {
                return http_request_header_line_invalid(r, 400, "invalid Transfer-Encoding + Content-Length -> 400");
            }
            else {
                /* ignore Content-Length */
                http_header_request_unset(r, HTTP_HEADER_CONTENT_LENGTH, CONST_STR_LEN("Content-Length"));
            }
        }
        if (http_method_get_or_head(r->http_method)
            && !(http_parseopts & HTTP_PARSEOPT_METHOD_GET_BODY)) {
            return http_request_header_line_invalid(r, 400, "GET/HEAD with content-length -> 400");
        }
    }

    return 0;
}


static int
http_request_parse_hoff (request_st * const restrict r, char * const restrict hdrs, const unsigned short * const restrict hoff, const int scheme_port)
{
    /*
     * Request: "^(GET|POST|HEAD|...) ([^ ]+(\\?[^ ]+|)) (HTTP/1\\.[01])$"
     * Header : "^([-a-zA-Z]+): (.+)$"
     * End    : "^$"
     */

    int status;
    const unsigned int http_parseopts = r->conf.http_parseopts;

    status = http_request_parse_reqline(r, hdrs, hoff, http_parseopts);
    if (0 != status) return status;

    status = http_request_parse_headers(r, hdrs, hoff, http_parseopts);
    if (0 != status) return status;

    return http_request_parse(r, scheme_port);
}


static void
http_request_headers_fin (request_st * const restrict r)
{
    if (0 == r->http_status) {
      #if 0
        r->conditional_is_valid = (1 << COMP_SERVER_SOCKET)
                                | (1 << COMP_HTTP_SCHEME)
                                | (1 << COMP_HTTP_HOST)
                                | (1 << COMP_HTTP_REMOTE_IP)
                                | (1 << COMP_HTTP_REQUEST_METHOD)
                                | (1 << COMP_HTTP_URL)
                                | (1 << COMP_HTTP_QUERY_STRING)
                                | (1 << COMP_HTTP_REQUEST_HEADER);
      #else
        /* all config conditions are valid after parsing header
         * (set all bits; remove dependency on plugin_config.h) */
        r->conditional_is_valid = ~0u;
      #endif
    }
    else {
        r->keep_alive = 0;
        r->reqbody_length = 0;
    }
}


void
http_request_headers_process (request_st * const restrict r, char * const restrict hdrs, const unsigned short * const restrict hoff, const int scheme_port)
{
    r->http_status = http_request_parse_hoff(r, hdrs, hoff, scheme_port);

    http_request_headers_fin(r);

    if (0 != r->http_status) {
        if (r->conf.log_request_header_on_error) {
            /*(http_request_parse_headers() modifies hdrs only to
             * undo line-wrapping in-place using spaces)*/
            log_error(r->conf.errh, __FILE__, __LINE__,
              "request-header:\n%.*s", (int)r->rqst_header_len, hdrs);
        }
    }
}


void
http_request_headers_process_h2 (request_st * const restrict r, const int scheme_port)
{
    if (0 == r->http_status)
        r->http_status = http_request_parse(r, scheme_port);

    if (0 == r->http_status) {
        if (light_btst(r->rqst_htags, HTTP_HEADER_CONNECTION))
            r->http_status = http_request_header_line_invalid(r, 400,
              "invalid Connection header with HTTP/2 -> 400");
    }

    http_request_headers_fin(r);

    /* limited; headers not collected into a single buf for HTTP/2 */
    if (__builtin_expect( (0 != r->http_status), 0)) {
        if (r->conf.log_request_header_on_error) {
            log_error(r->conf.errh, __FILE__, __LINE__,
              "request-header:\n:authority: %s\n:method: %s\n:path: %s",
              r->http_host ? r->http_host->ptr : "",
              (HTTP_METHOD_UNSET != r->http_method)
                ? get_http_method_name(r->http_method)
                : "",
              !buffer_string_is_empty(&r->target) ? r->target.ptr : "");
        }
    }

    /* ignore Upgrade if using HTTP/2 */
    if (light_btst(r->rqst_htags, HTTP_HEADER_UPGRADE))
        http_header_request_unset(r, HTTP_HEADER_UPGRADE,
                                  CONST_STR_LEN("upgrade"));
    /* XXX: should filter out other hop-by-hop connection headers, too */
}
