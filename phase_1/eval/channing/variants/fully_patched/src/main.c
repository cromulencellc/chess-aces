#include "bst.h"
#include "config.h"
#include "debug.h"
#include "dictionary.h"
#include "http.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "testbed.h"
#define BUFFER_SIZE (MAX_URI + MAX_QUERY + 200)
dictionaryType *loadINI(char *filename);
dictionaryType *loadMIMEtypes(char *filename);
int loadConfig(serverConfigType *serverConfigData, dictionaryType *configDict);
dictionaryType *mimeDict;
bstType *hoststats;
bstType *ua_stats;
int readLine(int fd, char *buffer, int maxLen) {
  int writePos;
  writePos = 0;
  int count;
  if (buffer == 0)
    return -1;
  buffer[0] = 0;
  while (writePos < maxLen) {
    count = read(fd, buffer + writePos, 1);
    if (count == 0) {
      debug("No more data to read\n");
      return 0;
    }
    if (buffer[writePos] == '\n')
      break;
    ++writePos;
  }
  if (writePos == maxLen)
    return -1;
  else {
    buffer[writePos + 1] = 0;
    return writePos;
  }
}
int main(int argc, char **argv) {
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  serverConfigType serverInfo;
  dictionaryType *configDict;
  int s;
  hoststats = 0;
  ua_stats = 0;
  configDict = loadINI("/data/server.ini");
  printDict(configDict);
  mimeDict = loadMIMEtypes("/etc/mime.types");
  printDict(mimeDict);
  if (loadConfig(&serverInfo, configDict) != 0) {
    debug("Problem loading configuration data\n");
    return -1;
  }
  if (argc > 1) {
    if (strcmp(argv[1], "-f") == 0) {
      serverInfo.use_stdout = 1;
      handleRequest(&serverInfo, STDIN_FILENO, STDOUT_FILENO);
      return 0;
    }
  }
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    debug("Unable to create socket for server process\n");
    return -1;
  }
  int optval = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if (bind(s, serverInfo.listenAddr, serverInfo.addr_len) == -1) {
    debug("Unable to bind to socket %d\n", serverInfo.port);
    return -1;
  }
  if (listen(s, 1) == -1) {
    debug("Unable to listen for connections\n");
    return -1;
  }
  int ls;
  struct sockaddr_in peeraddr;
  unsigned int addrlen = sizeof(peeraddr);
  struct timespec request;
  struct timespec remaining;
  double difference;
  double min_request_interval;
  if (serverInfo.max_requests_per_second) {
    min_request_interval = 1.0 / serverInfo.max_requests_per_second;
  } else {
    min_request_interval = 0.0;
  }
  struct timespec current_time;
  struct timespec previous_request_time;
  previous_request_time.tv_nsec = 0;
  previous_request_time.tv_sec = 0;
  while (1) {
    ls = accept(s, (struct sockaddr *)&peeraddr, &addrlen);
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    debug("sec: %ld\nnsec: %ld\n", current_time.tv_sec, current_time.tv_nsec);
    difference = ((double)current_time.tv_sec +
                  (double)current_time.tv_nsec / 1000000000) -
                 ((double)previous_request_time.tv_sec +
                  (double)previous_request_time.tv_nsec / 1000000000);
    debug("Time difference = %f\n", difference);
    if (difference < min_request_interval) {
      debug("Request too soon, delaying for a bit\n");
      request.tv_nsec = 1000000000 * difference;
      request.tv_sec = 0;
      while (nanosleep(&request, &remaining) != 0) {
        request.tv_sec = remaining.tv_sec;
        request.tv_nsec = remaining.tv_nsec;
      }
    }
    clock_gettime(CLOCK_MONOTONIC, &previous_request_time);
    if (ls > 0) {
      debug("Received connection on socket\n");
      debug("Remote address is %s\n", inet_ntoa(peeraddr.sin_addr));
      fprintf(stderr, "Connection: %s\n", inet_ntoa(peeraddr.sin_addr));
      addBstNode(&hoststats, peeraddr.sin_addr.s_addr, 0);
    } else {
      debug("Some sort of socket error occurred\n");
      return -1;
    }
    if (handleRequest(&serverInfo, ls, ls) == -1) {
      close(ls);
      break;
    }
    close(ls);
  }
}
