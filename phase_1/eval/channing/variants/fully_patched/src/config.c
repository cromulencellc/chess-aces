#include "config.h"
#include "debug.h"
#include "dictionary.h"
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
#include <unistd.h>
#define MAX_LINE_SIZE 1024
dictionaryType *loadINI(char *filename) {
  FILE *fp;
  char line[MAX_LINE_SIZE];
  dictionaryType *configDict = NULL;
  fp = fopen(filename, "r");
  if (fp == NULL)
    return NULL;
  while (fgets(line, MAX_LINE_SIZE, fp)) {
    if (!isalpha(*line))
      continue;
    if (line[0] == '#')
      continue;
    addDictbyLine(&configDict, line, ':');
  }
  fclose(fp);
  return configDict;
}
dictionaryType *loadMIMEtypes(char *filename) {
  FILE *fp;
  char line[MAX_LINE_SIZE];
  char *linePtr;
  dictionaryType *mimeDict = NULL;
  char *value;
  char *key;
  fp = fopen(filename, "r");
  if (fp == NULL)
    return NULL;
  while (fgets(line, MAX_LINE_SIZE, fp)) {
    if (!isalpha(*line))
      continue;
    if (!strchr(line, '\t') && !strchr(line, ' '))
      continue;
    value = line;
    linePtr = line;
    while (*linePtr != 0 && !isblank(*linePtr))
      ++linePtr;
    *linePtr = 0;
    ++linePtr;
    while (*linePtr != 0 && isblank(*linePtr))
      ++linePtr;
    if (*linePtr == 0)
      continue;
    while (*linePtr != 0) {
      key = linePtr;
      while (*linePtr != 0 && *linePtr != ' ' && *linePtr != '\t' &&
             *linePtr != '\r' && *linePtr != '\n')
        ++linePtr;
      if (*linePtr == 0 || *linePtr == '\r' || *linePtr == '\n') {
        *linePtr = 0;
      } else {
        *linePtr = 0;
        ++linePtr;
        while (*linePtr != 0 && (*linePtr == ' ' || *linePtr == '\t'))
          ++linePtr;
      }
      addDictEntry(&mimeDict, key, value);
    }
  }
  fclose(fp);
  return mimeDict;
}
int loadConfig(serverConfigType *serverConfigData, dictionaryType *configDict) {
  char *value;
  char *listeningPort;
  char *request_sec;
  char *envPort;
  if (serverConfigData == 0)
    return -1;
  if (configDict == 0)
    return -1;
  request_sec = findDict(configDict, "maxRequestsPerSec");
  if (request_sec == 0) {
    serverConfigData->max_requests_per_second = 0;
  } else {
    serverConfigData->max_requests_per_second = atoi(request_sec);
  }
  serverConfigData->use_stdout = 0;
  listeningPort = findDict(configDict, "listenPort");
  envPort = getenv("PORT");
  if (envPort) {
    serverConfigData->port = atoi(envPort);
    listeningPort = envPort;
  } else if (listeningPort) {
    serverConfigData->port = atoi(listeningPort);
    if (serverConfigData->port <= 0)
      return -1;
    if (serverConfigData->port > 65535)
      return -1;
  } else {
    debug("listenPort not defined\n");
    return -1;
  }
  value = findDict(configDict, "maxEntitySize");
  if (value) {
    serverConfigData->maxEntitySize = atoi(value);
    if (serverConfigData->maxEntitySize <= 0)
      serverConfigData->maxEntitySize = 0;
  } else {
    debug("maxEntity not defined--using default value\n");
    serverConfigData->maxEntitySize = 10000;
  }
  value = findDict(configDict, "docRoot");
  if (value) {
    if (strlen(value) < MAX_PATH_SIZE) {
      strcpy(serverConfigData->docRoot, value);
    } else {
      return -1;
    }
  } else {
    debug("docRoot not defined\n");
    return -1;
  }
  value = findDict(configDict, "defaultFile");
  if (value) {
    if (strlen(value) < MAX_FILENAME_LENGTH) {
      strcpy(serverConfigData->defaultFile, value);
    } else {
      return -1;
    }
  } else {
    debug("defaultFile not defined\n");
    return -1;
  }
  struct addrinfo hints;
  struct addrinfo *results;
  int retcode;
  value = findDict(configDict, "listenAddr");
  if (value) {
    hints.ai_flags = AI_ADDRCONFIG;
  } else {
    debug("listenAddr not defined so using default\n");
    hints.ai_flags = AI_PASSIVE;
  }
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  retcode = getaddrinfo(value, listeningPort, &hints, &results);
  if (retcode != 0) {
    debug("Unable to resolve configured listenAddr\n");
    return -1;
  }
  struct addrinfo *temp;
  int temp_socket;
  for (temp = results; temp; temp = temp->ai_next) {
    struct sockaddr_in *addr;
    addr = (struct sockaddr_in *)temp->ai_addr;
    debug("address: %s\n", inet_ntoa((struct in_addr)addr->sin_addr));
    temp_socket = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);
    if (temp_socket == -1) {
      debug("Unable to create a socket with that info\n");
      continue;
    }
    retcode = bind(temp_socket, temp->ai_addr, temp->ai_addrlen);
    if (retcode != 0) {
      debug("bind() failed\n");
      close(temp_socket);
      continue;
    } else {
      debug("bind() succeeded\n");
      close(temp_socket);
      break;
    }
  }
  if (temp == NULL) {
    debug("Unable to bind to listenAddr\n");
    freeaddrinfo(results);
    return -1;
  }
  serverConfigData->listenAddr = malloc(sizeof(struct sockaddr));
  if (serverConfigData->listenAddr == 0) {
    debug("Unable to malloc memory!\n");
    return (-1);
  }
  memcpy(serverConfigData->listenAddr, temp->ai_addr, sizeof(struct sockaddr));
  serverConfigData->addr_len = temp->ai_addrlen;
  freeaddrinfo(results);
  value = findDict(configDict, "serverName");
  if (value) {
    if (strlen(value) < MAX_SERVER_NAME_LENGTH) {
      strcpy(serverConfigData->serverName, value);
    } else {
      strncpy(serverConfigData->serverName, value, MAX_SERVER_NAME_LENGTH - 1);
    }
  } else {
    debug("serverName is not defined\n");
    strcpy(serverConfigData->serverName, "ACES-HTTP-1.1");
  }
  return 0;
}
