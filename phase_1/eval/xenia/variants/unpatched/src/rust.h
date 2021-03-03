#ifndef rust_HEADER
#define rust_HEADER
#include <stdio.h>
#include <unistd.h>
#include "challenge.h"
#define TRACE                                                                  \
  do {                                                                         \
    printf("[TRACE] %s:%s:%u\n", __FILE__, __FUNCTION__, __LINE__);            \
    fflush(stdout);                                                            \
  } while (0);
#define DEBUG(fmt, args...)                                                    \
  do {                                                                         \
    printf("[DEBUG] %s:%s:%d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__,    \
           args);                                                              \
    fflush(stdout);                                                            \
  } while (0)
#define panic(MESSAGE)                                                         \
  {                                                                            \
    printf("[PANIC] %s:%s:%d: %s\n", __FILE__, __FUNCTION__, __LINE__,         \
           MESSAGE);                                                           \
    fflush(stdout);                                                            \
    clean_shutdown();                                                          \
    exit(0);                                                                   \
  }                                                                            \
  while (0)                                                                    \
    ;
void unimplemented() __attribute__((noreturn));
extern void *null_pointer_check;
#define unwrap_null(XXX) unwrap_null_(XXX, __FUNCTION__, __LINE__)
void *unwrap_null_(void *, const char *function, unsigned int line);
#endif