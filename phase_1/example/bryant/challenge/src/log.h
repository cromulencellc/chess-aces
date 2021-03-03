#pragma once

#ifdef NO_LOG
#define lll(...) /* noop( __VA_ARGS__) */
#else
#define lll(...) fprintf(stderr, __VA_ARGS__)
#endif
