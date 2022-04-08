#ifndef log_HEADER
#define log_HEADER

#include <stdio.h>

#ifndef LOG_LEVEL_TRACE
#ifndef LOG_LEVEL_DEBUG
#ifndef LOG_LEVEL_INFO
#ifndef LOG_LEVEL_SUCCESS
#ifndef LOG_LEVEL_WARN
#ifndef LOG_LEVEL_ERROR
#define LOG_LEVEL_INFO
#endif
#endif
#endif
#endif
#endif
#endif


#ifdef LOG_LEVEL_TRACE
#define LOG_LEVEL_DEBUG
#endif

#ifdef LOG_LEVEL_DEBUG
#define LOG_LEVEL_INFO
#endif

#ifdef LOG_LEVEL_INFO
#define LOG_LEVEL_SUCCESS
#endif

#ifdef LOG_LEVEL_SUCCESS
#define LOG_LEVEL_WARN
#endif

#ifdef LOG_LEVEL_WARN
#define LOG_LEVEL_ERROR
#endif


#ifdef LOG_LEVEL_TRACE
#define LOG_TRACE(FMT, ...) printf("[T] (%s) " FMT "\n", __FUNCTION__, ##__VA_ARGS__)
#define LOG_TRACEB(FMT, ...) \
    printf("\e[0;34m[T] (%s) " FMT "\e[0m\n", __FUNCTION__, ##__VA_ARGS__)
#define LOG_TRACEW(FMT, ...) \
    printf("\e[0;37m[T] (%s) " FMT "\e[0m\n", __FUNCTION__, ##__VA_ARGS__)
#define LOG_TRACEC(FMT, ...) \
    printf("\e[0;36m[T] (%s) " FMT "\e[0m\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_TRACE(FMT, ...) do {} while (0)
#define LOG_TRACEB(FMT, ...) do {} while (0)
#define LOG_TRACEW(FMT, ...) do {} while (0)
#define LOG_TRACEC(FMT, ...) do {} while (0)
#endif

#ifdef LOG_LEVEL_DEBUG
#define LOG_DEBUG(FMT, ...) printf("[D] (%s) " FMT "\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(FMT, ...) do {} while (0)
#endif

#ifdef LOG_LEVEL_INFO
#define LOG_INFO(FMT, ...) printf("[.] (%s) " FMT "\n", __FUNCTION__, ##__VA_ARGS__)
#define LOG_INFOB(FMT, ...) \
    printf("\e[1;94m[.] (%s) " FMT "\e[0m\n", __FUNCTION__, ##__VA_ARGS__)
#define LOG_INFOW(FMT, ...) \
    printf("\e[1;97m[.] (%s) " FMT "\e[0m\n", __FUNCTION__, ##__VA_ARGS__)
#define LOG_INFOC(FMT, ...) \
    printf("\e[1;96m[.] (%s) " FMT "\e[0m\n", __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_INFO(FMT, ...) do {} while (0)
#define LOG_INFOB(FMT, ...) do {} while (0)
#define LOG_INFOW(FMT, ...) do {} while (0)
#define LOG_INFOC(FMT, ...) do {} while (0)
#endif

#define LOG_SUCCESS(FMT, ...) \
    printf("\e[0;32m[+] (%s) " FMT "\e[0m\n", __FUNCTION__, ##__VA_ARGS__)

#define LOG_WARN(FMT, ...) \
    printf("\e[0;33m[!] (%s) " FMT "\e[0m\n", __FUNCTION__, ##__VA_ARGS__)

#define LOG_ERROR(FMT, ...) \
    printf("\e[0;31m[-] (%s) " FMT "\e[0m\n", __FUNCTION__, ##__VA_ARGS__)

#endif