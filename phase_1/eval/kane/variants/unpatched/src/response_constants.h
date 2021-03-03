#ifndef RESPONSE_CONSTANTS_H
#define RESPONSE_CONSTANTS_H
#define SUPPED_METHOD_COUNT 1
#define UNSUPPED_METHOD_COUNT 2
const char OK[] = "200 OK\r\n";
const char NOT_FOUND[] = "404 Not Found\r\n";
const char METHOD_NOT_ALLOWED[] = "405 Method Not Allowed\r\n";
const char HTTP_VER[] = "HTTP/1.1";
const char SUPP_METHODS[SUPPED_METHOD_COUNT][5] = {"GET"};
const char NOT_SUPPED_METHODS[UNSUPPED_METHOD_COUNT][6] = {"HEAD", "POST"};
#endif
