#ifndef rust_HEADER
#define rust_HEADER
#include "error.h"
#include <stdio.h>
extern int try_err;
#define try(XXX) if ((try_err = (XXX)) != 0){                                  \
    printf("%s:%u %s\n", __FILE__, __LINE__, describe_error(try_err));         \
    if (error_description[0] != '\0'){                                         \
        printf("%s\n", error_description); } return try_err; }
#define try_cleanup_start(XXX)                                                 \
  if ((try_err = (XXX)) != 0) {                                                \
    printf("%s:%u %s\n", __FILE__, __LINE__, describe_error(try_err));         \
    if (error_description[0] != '\0') {                                        \
      printf("%s\n", error_description);                                       \
    }
#define try_cleanup_end                                                        \
  return try_err;                                                              \
  }
#define DEBUG_TRACE                                                            \
  do {                                                                         \
    printf("TRACE %s:%u %s\n", __FILE__, __LINE__, __func__);                  \
    fflush(stdout);                                                            \
  } while (0);
#endif