
#define _XOPEN_SOURCE
#include "base64.h"
#include "bst.h"
#include "crc.h"
#include "debug.h"
#include "dictionary.h"
#include "http.h"
#include "http_errors.h"
#include "url_decode.h"
#include <crypt.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
extern dictionaryType *mimeDict;
extern bstType *ua_stats;
int readLine(int fd, char *buffer, int maxLen);
int handleRequest(serverConfigType *config, int in, int out) {
  char method[10];
  char query[MAX_QUERY];
  char uri[MAX_URI];
  char version[10];
  dictionaryType *headers = 0;
  char buffer[HTTP_BUFFER_SIZE];
  int ret;
  int readIndex = 0;
  int writeIndex;
  enum state readState;
  int methodToken = 0;
  readState = METHOD;
  readIndex = 0;
  writeIndex = 0;
  while (1) {
  readMore:
    debug("at readmore\n");
    ret = readLine(in, buffer, HTTP_BUFFER_SIZE - 1);
    if (ret == -1) {
      debug("Line too long!\n");
      debug("%s\n", buffer);
      sendError(out, 414);
      return 0;
    }
    if (ret == 0) {
      debug("out of data from socket\n");
      return 0;
    }
    debug("%d chars ready in this line\n", ret);
    debug("Line: %s\n", buffer);
    readIndex = 0;
    switch (readState) {
    case METHOD:
      debug("At Method\n");
      while (buffer[readIndex] >= 'A' && buffer[readIndex] <= 'Z') {
        method[writeIndex] = buffer[readIndex];
        ++writeIndex;
        ++readIndex;
        if (readIndex == ret)
          goto readMore;
        if (writeIndex >= 6) {
          debug("Bad method length\n");
          sendError(out, 405);
          return 0;
        }
      }
      method[writeIndex] = 0;
      debug("Method = %s\n", method);
      if (strcmp(method, "GET") == 0) {
        methodToken = METHOD_GET;
      } else if (strcmp(method, "PUT") == 0) {
        methodToken = METHOD_PUT;
      } else if (strcmp(method, "POST") == 0) {
        methodToken = METHOD_POST;
      } else if (strcmp(method, "HEAD") == 0) {
        methodToken = METHOD_HEAD;
      } else {
        debug("Bad Method %s received\n", method);
        sendError(out, 405);
        return 0;
      }
      writeIndex = 0;
      readState = URI;
    case URI:
      debug("At URI\n");
      while (isblank(buffer[readIndex])) {
        ++readIndex;
        if (readIndex == ret)
          goto readMore;
      }
      while (isgraph(buffer[readIndex]) && buffer[readIndex] != '?') {
        uri[writeIndex] = buffer[readIndex];
        ++writeIndex;
        ++readIndex;
        if (readIndex == ret)
          goto readMore;
        if (writeIndex >= MAX_URI - 1) {
          debug("Bad URI length\n");
          sendError(out, 414);
          return 0;
        }
      }
      if (buffer[readIndex] == '\r' || buffer[readIndex] == '\n') {
        debug("Bad request\n");
        sendError(out, 405);
        return 0;
      }
      uri[writeIndex] = 0;
      debug("URI: %s\n", uri);
      readState = QUERY;
      writeIndex = 0;
      debug("len of URI = %lu\n", strlen(uri));
    case QUERY:
      debug("At Query\n");
      if (buffer[readIndex] != '?') {
        readState = VERSION;
        goto version;
      } else {
        debug("Getting a query string\n");
      }
      ++readIndex;
      if (readIndex == ret)
        goto readMore;
      while (isgraph(buffer[readIndex])) {
        debug("Adding char: %c to query string\n", buffer[readIndex]);
        query[writeIndex] = buffer[readIndex];
        if (readIndex == ret) {
          debug("going back to read more for query string\n");
          goto readMore;
        }
        ++writeIndex;
        ++readIndex;
        if (writeIndex == MAX_QUERY - 1) {
          debug("Bad Query length\n");
          return 0;
        }
      }
      debug("End of line for query string\n");
      query[writeIndex] = 0;
      if (strlen(query)) {
        debug("Received a query string\n");
      }
      readState = VERSION;
      writeIndex = 0;
    version:
    case VERSION:
      debug("At Version\n");
      while (isspace(buffer[readIndex])) {
        ++readIndex;
        if (readIndex == ret)
          goto readMore;
      }
      while (isgraph(buffer[readIndex])) {
        if (writeIndex >= 9) {
          debug("Bad Version length\n");
          sendError(out, 505);
          return 0;
        }
        version[writeIndex] = buffer[readIndex];
        ++writeIndex;
        ++readIndex;
        if (readIndex == ret) {
          debug("going back to read more\n");
          goto readMore;
        }
      }
      debug("End of line for vesion\n");
      version[writeIndex] = 0;
      debug("Version: %s\n", version);
      while (buffer[readIndex] != '\n' && readIndex < ret)
        ++readIndex;
      debug("Found the end of the request line\n");
      break;
    default:
      break;
    }
    break;
  }
  int count;
  count = readLine(in, buffer, HTTP_BUFFER_SIZE);
  if (count == -1) {
    debug("Got error response from readLine()\n");
    return -1;
  }
  if (count == 0) {
    debug("No more data to read\n");
    return 0;
  }
  while (strncmp(buffer, "\r\n", 2) != 0 && strncmp(buffer, "\n", 1) != 0) {
    addDictbyLine(&headers, buffer, ':');
    count = readLine(in, buffer, HTTP_BUFFER_SIZE);
    if (count == -1) {
      debug("Got error response from readLine2()\n");
      return -1;
    } else if (count == 0) {
      debug("No more data to read\n");
      return 0;
    }
    debug("Read another line %s\n", buffer);
  }
  debug("No more lines in the header\n");
  fprintf(stderr, "%s %s %s\n", method, uri, version);
  char *useragent_string = 0;
  useragent_string = findDict(headers, "User-Agent");
  if (useragent_string) {
    unsigned long crc =
        crc32(0L, (unsigned char *)useragent_string, strlen(useragent_string));
    debug("useragent hash: %lu\n", crc);
    if (!findBstNode(ua_stats, crc)) {
      char *ua_string = 0;
      ua_string = malloc(strlen(useragent_string) + 1);
      if (ua_string) {
        ua_string[0] = 0;
        memcpy(ua_string, useragent_string, strlen(useragent_string) + 1);
        addBstNode(&ua_stats, crc, ua_string);
      } else {
        debug("unable to allocate memory for useragent string\n");
        return -1;
      }
    } else {
      incBstNode(&ua_stats, crc);
    }
  }
  switch (methodToken) {
  case METHOD_GET:
    serveFile(out, config, uri, headers);
    break;
  case METHOD_PUT:
    sendError(out, 501);
    break;
  case METHOD_POST:
    handlePost(in, out, config, uri, headers, version);
    break;
  case METHOD_HEAD:
    headFile(out, config, uri, headers);
    break;
  case METHOD_DELETE:
    sendError(out, 501);
    break;
  default:
    sendError(out, 501);
    break;
  }
  deleteDict(&headers);
  if (headers != NULL) {
    debug("headers should be null\n");
  }
  return 0;
}
