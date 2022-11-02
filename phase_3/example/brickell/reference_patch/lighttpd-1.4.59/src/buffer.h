#ifndef _BUFFER_H_
#define _BUFFER_H_
#include "first.h"

struct tm;              /* declaration */

/**
 * max size of a buffer which will just be reset
 * to ->used = 0 instead of really freeing the buffer
 */
#define BUFFER_MAX_REUSE_SIZE 4096

/* generic string + binary data container; contains a terminating 0 in both
 * cases
 *
 * used == 0 indicates a special "empty" state (unset config values); ptr
 * might be NULL too then. otherwise an empty string has used == 1 (and ptr[0]
 * == 0);
 *
 * copy/append functions will ensure used >= 1 (i.e. never leave it in the
 * special empty state); only buffer_copy_buffer will copy the special empty
 * state.
 */
typedef struct {
	char *ptr;

	/* "used" includes a terminating 0 */
	uint32_t used;
	/* size of allocated buffer at *ptr */
	uint32_t size;
} buffer;

/* create new buffer; either empty or copy given data */
__attribute_returns_nonnull__
buffer* buffer_init(void);

__attribute_returns_nonnull__
buffer* buffer_init_buffer(const buffer *src); /* src can  be NULL */

__attribute_returns_nonnull__
buffer* buffer_init_string(const char *str); /* str can  be NULL */

void buffer_free(buffer *b); /* b can be NULL */

/* reset b. if NULL != b && NULL != src, move src content to b. reset src. */
void buffer_move(buffer * restrict b, buffer * restrict src);

/* make sure buffer is large enough to store a string of given size
 * and a terminating zero.
 * sets b to an empty string, and may drop old content.
 * @return b->ptr
 */
__attribute_returns_nonnull__
char* buffer_string_prepare_copy(buffer *b, size_t size);

/* allocate buffer large enough to be able to append a string of given size
 * if b was empty (used == 0) it will contain an empty string (used == 1)
 * afterwards
 * "used" data is preserved; if not empty buffer must contain a
 * zero terminated string.
 */
__attribute_returns_nonnull__
char* buffer_string_prepare_append(buffer *b, size_t size);

/* use after prepare_(copy,append) when you have written data to the buffer
 * to increase the buffer length by size. also sets the terminating zero.
 * requires enough space is present for the terminating zero (prepare with the
 * same size to be sure).
 */
void buffer_commit(buffer *b, size_t size);

/* sets string length:
 * - always stores a terminating zero to terminate the "new" string
 * - does not modify the string data apart from terminating zero
 * - reallocates the buffer iff needed
 */
void buffer_string_set_length(buffer *b, uint32_t len);

/* clear buffer
 * - invalidate buffer contents
 * - unsets used chars but does not modify existing ptr contents
 *   (b->ptr *is not* set to an empty, '\0'-terminated string "")
 */
static inline void buffer_clear(buffer *b);

/* reset buffer
 * - invalidate buffer contents
 * - unsets used chars
 * - keeps smaller buffer (unmodified) for reuse
 *   (b->ptr *is not* set to an empty, '\0'-terminated string "")
 * - frees larger buffer (b->size > BUFFER_MAX_REUSE_SIZE)
 */
static inline void buffer_reset(buffer *b);

/* free buffer ptr
 * - invalidate buffer contents; free ptr; reset ptr, used, size to 0
 */
__attribute_cold__
void buffer_free_ptr(buffer *b);

void buffer_copy_string(buffer * restrict b, const char * restrict s);
void buffer_copy_string_len(buffer * restrict b, const char * restrict s, size_t s_len);
static inline void buffer_copy_buffer(buffer * restrict b, const buffer * restrict src);

void buffer_append_string(buffer * restrict b, const char * restrict s);
void buffer_append_string_len(buffer * restrict b, const char * restrict s, size_t s_len);
static inline void buffer_append_string_buffer(buffer * restrict b, const buffer * restrict src);

#define buffer_append_uint_hex(b,len) buffer_append_uint_hex_lc((b),(len))
void buffer_append_uint_hex_lc(buffer *b, uintmax_t len);
void buffer_append_int(buffer *b, intmax_t val);

void buffer_append_strftime(buffer * restrict b, const char * restrict format, const struct tm * restrict tm);

/* '-', log_10 (2^bits) = bits * log 2 / log 10 < bits * 0.31, terminating 0 */
#define LI_ITOSTRING_LENGTH (2 + (8 * sizeof(intmax_t) * 31 + 99) / 100)

size_t li_itostrn(char *buf, size_t buf_len, intmax_t val);
size_t li_utostrn(char *buf, size_t buf_len, uintmax_t val);

/* buf must be (at least) 2*s_len + 1 big. uses lower-case hex letters. */
#define li_tohex(buf,buf_len,s,s_len) li_tohex_lc((buf),(buf_len),(s),(s_len))
void li_tohex_lc(char * restrict buf, size_t buf_len, const char * restrict s, size_t s_len);
void li_tohex_uc(char * restrict buf, size_t buf_len, const char * restrict s, size_t s_len);

/* NULL buffer or empty buffer (used == 0);
 * unset "string" (buffer) config options are initialized to used == 0,
 * while setting an empty string leads to used == 1
 */
__attribute_pure__
static inline int buffer_is_empty(const buffer *b);
/* NULL buffer, empty buffer (used == 0) or empty string (used == 1) */
__attribute_pure__
static inline int buffer_string_is_empty(const buffer *b);

__attribute_pure__
int buffer_eq_icase_ssn(const char * const a, const char * const b, const size_t len);

__attribute_pure__
int buffer_eq_icase_ss(const char * const a, const size_t alen, const char * const b, const size_t blen);

__attribute_pure__
int buffer_eq_icase_slen(const buffer * const b, const char * const s, const size_t slen);
#define buffer_is_equal_caseless_string buffer_eq_icase_slen

__attribute_pure__
int buffer_eq_slen(const buffer * const b, const char * const s, const size_t slen);

__attribute_pure__
int buffer_is_equal(const buffer *a, const buffer *b);

__attribute_pure__
int buffer_is_equal_right_len(const buffer *a, const buffer *b, size_t len);

__attribute_pure__
int buffer_is_equal_string(const buffer *a, const char *s, size_t b_len);

void buffer_substr_replace (buffer * restrict b, size_t offset, size_t len, const buffer * restrict replace);

void buffer_append_string_encoded_hex_lc(buffer * restrict b, const char * restrict s, size_t len);
void buffer_append_string_encoded_hex_uc(buffer * restrict b, const char * restrict s, size_t len);

typedef enum {
	ENCODING_REL_URI, /* for coding a rel-uri (/with space/and%percent) nicely as part of a href */
	ENCODING_REL_URI_PART, /* same as ENC_REL_URL plus coding / too as %2F */
	ENCODING_HTML,         /* & becomes &amp; and so on */
	ENCODING_MINIMAL_XML   /* minimal encoding for xml */
} buffer_encoding_t;

void buffer_append_string_encoded(buffer * restrict b, const char * restrict s, size_t s_len, buffer_encoding_t encoding);

/* escape non-printable characters; simple escapes for \t, \r, \n; fallback to \xCC */
void buffer_append_string_c_escaped(buffer * restrict b, const char * restrict s, size_t s_len);

/* to upper case, replace non alpha-numerics with '_'; if is_http_header prefix with "HTTP_" unless s is "content-type" */
void buffer_copy_string_encoded_cgi_varnames(buffer * restrict b, const char * restrict s, size_t s_len, int is_http_header);

void buffer_urldecode_path(buffer *url);
void buffer_urldecode_query(buffer *url);
int buffer_is_valid_UTF8(const buffer *b);
void buffer_path_simplify(buffer *dest, buffer *src);

void buffer_to_lower(buffer *b);
void buffer_to_upper(buffer *b);


/** deprecated */
char hex2int(unsigned char c);
char int2hex(char i);

__attribute_pure__
static inline int light_isdigit(int c);
static inline int light_isdigit(int c) {
	return ((uint32_t)c-'0' <= '9'-'0');
}

__attribute_pure__
static inline int light_isxdigit(int c);
static inline int light_isxdigit(int c) {
	return light_isdigit(c) || (((uint32_t)c | 0x20)-'a' <= 'f'-'a');
}

__attribute_pure__
static inline int light_isalpha(int c);
static inline int light_isalpha(int c) {
	return (((uint32_t)c | 0x20)-'a' <= 'z'-'a');
}

__attribute_pure__
static inline int light_isalnum(int c);
static inline int light_isalnum(int c) {
	return light_isdigit(c) || light_isalpha(c);
}

#define light_isupper(c) ((uint32_t)(c)-'A' <= 'Z'-'A')
#define light_islower(c) ((uint32_t)(c)-'a' <= 'z'-'a')

#define light_bshift(b)           ((uint64_t)1uL << (b))
#define light_btst(a,b)  ((a) &   ((uint64_t)1uL << (b)))
#define light_bclr(a,b)  ((a) &= ~((uint64_t)1uL << (b)))
#define light_bset(a,b)  ((a) |=  ((uint64_t)1uL << (b)))


__attribute_pure__
static inline uint32_t buffer_string_length(const buffer *b); /* buffer string length without terminating 0 */

__attribute_pure__
static inline uint32_t buffer_string_space(const buffer *b); /* maximum length of string that can be stored without reallocating */

static inline void buffer_append_slash(buffer *b); /* append '/' no non-empty strings not ending in '/' */
void buffer_append_path_len(buffer * restrict b, const char * restrict a, size_t alen); /* join strings with '/', if '/' not present */

#define BUFFER_APPEND_STRING_CONST(x, y) \
	buffer_append_string_len(x, y, sizeof(y) - 1)

#define BUFFER_COPY_STRING_CONST(x, y) \
	buffer_copy_string_len(x, y, sizeof(y) - 1)

#define BUFFER_INTLEN_PTR(x) (x)->used ? (int)((x)->used - 1) : 0, (x)->ptr

#define CONST_LEN_STR(x) (uint32_t)sizeof(x)-1, x
#define CONST_STR_LEN(x) x, (uint32_t)sizeof(x) - 1
#define CONST_BUF_LEN(x) ((x) ? (x)->ptr : NULL), buffer_string_length(x)


#define LI_NORETURN __attribute_noreturn__

__attribute_cold__
void log_failed_assert(const char *filename, unsigned int line, const char *msg) LI_NORETURN;
#define force_assert(x) do { if (!(x)) log_failed_assert(__FILE__, __LINE__, "assertion failed: " #x); } while(0)
#define SEGFAULT() log_failed_assert(__FILE__, __LINE__, "aborted");

/* inline implementations */

static inline int buffer_is_empty(const buffer *b) {
	return NULL == b || 0 == b->used;
}
static inline int buffer_string_is_empty(const buffer *b) {
	return NULL == b || b->used < 2;
}

static inline uint32_t buffer_string_length(const buffer *b) {
	return NULL != b && 0 != b->used ? b->used - 1 : 0;
}

static inline uint32_t buffer_string_space(const buffer *b) {
	return NULL != b && b->size ? b->size - (b->used | (0 == b->used)) : 0;
}

static inline void buffer_copy_buffer(buffer * restrict b, const buffer * restrict src) {
	buffer_copy_string_len(b, CONST_BUF_LEN(src));
}

static inline void buffer_append_string_buffer(buffer * restrict b, const buffer * restrict src) {
	buffer_append_string_len(b, CONST_BUF_LEN(src));
}

static inline void buffer_append_slash(buffer *b) {
	uint32_t len = buffer_string_length(b);
	if (len > 0 && '/' != b->ptr[len-1]) BUFFER_APPEND_STRING_CONST(b, "/");
}

static inline void buffer_clear(buffer *b) {
	b->used = 0;
}

static inline void buffer_reset(buffer *b) {
	b->used = 0;
	/* release buffer larger than BUFFER_MAX_REUSE_SIZE bytes */
	if (b->size > BUFFER_MAX_REUSE_SIZE) buffer_free_ptr(b);
}


#endif
