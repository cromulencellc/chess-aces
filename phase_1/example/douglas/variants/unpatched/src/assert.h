#pragma once

#undef assert

#define assert(expr)                                                  \
  (static_cast<bool>(expr) ? void(0)                                  \
                           : __assert_fail(#expr, __FILE__, __LINE__, \
                                           __extension__ __PRETTY_FUNCTION__))

void __assert_fail(const char *__assertion, const char *__file,

                   unsigned int __line, const char *__function)
    __attribute__((__noreturn__));
;
