#ifndef _MOD_MAGNET_CACHE_H_
#define _MOD_MAGNET_CACHE_H_
#include "first.h"

#include "base_decls.h"
#include "buffer.h"

#include <lua.h>

typedef struct {
	buffer *name;
	buffer *etag;

	lua_State *L;

	time_t last_used; /* LRU */
} script;

typedef struct {
	script **ptr;
	uint32_t used;
	uint32_t size;
} script_cache;

script_cache *script_cache_init(void);
void script_cache_free_data(script_cache *cache);

lua_State *script_cache_get_script(script_cache *cache, buffer *name, int etag_flags);

#endif
