#define _XOPEN_SOURCE
#include "base64.h"
#include "dictionary.h"
#include "http.h"
#include "url_decode.h"
#include <crypt.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "bst.h"
#include "debug.h"
#include "stats.h"
extern dictionaryType *mimeDict;
extern bstType *hoststats;
extern bstType *ua_stats;
extern int verboseLogging;
int read_body(int in, dictionaryType *headers, unsigned char **body_data,
              int maxBodySize, int *received_size);
int checkFile(char *fileName) {
  char *tmpname;
  if (!fileName) {
    return -1;
  }
  tmpname = malloc(strlen(fileName) + 1);
  if (!tmpname) {
    return -1;
  }
  strcpy(tmpname, fileName);
  char *baseFile = basename(tmpname);
  if (baseFile[0] == '.') {
    debug("File starts with dot, which is no bueno\n");
    free(tmpname);
    return 1;
  }
  free(tmpname);
  if (access(fileName, F_OK)) {
    debug("Requested file does not exist\n");
    return 1;
  }
  if (access(fileName, R_OK)) {
    debug("Server cannot read that file\n");
    return 2;
  }
  return 0;
}
int checkAuth(int s, char *filepath, dictionaryType *headers) {
  FILE *fp;
  char accessFile[MAX_PATH_LEN];
  char *authCode;
  char *dir;
  char *decodedCreds;
  char realm[200];
  if (filepath == 0)
    return -1;
  if (headers == 0)
    return -1;
  if (strlen(filepath) + 8 > MAX_PATH_LEN - 1) {
    return -1;
  }
  dir = malloc(strlen(filepath) + 1);
  if (dir == 0) {
    return -1;
  }
  strcpy(dir, filepath);
  sprintf(accessFile, "%s/.access", dirname(dir));
  fp = fopen(accessFile, "r");
  free(dir);
  if (fp == NULL) {
    debug("No .access file found\n");
    return 0;
  }
  if (fgets(realm, sizeof(realm), fp) == 0) {
    debug("Error reading realm from .access file\n");
    send401Error(s, "");
    return -1;
  }
  debug("Realm = %s\n", realm);
  authCode = findDict(headers, "Authorization");
  if (authCode == 0) {
    debug("There was no Auth in the request\n");
    fclose(fp);
    if (strlen(realm)) {
      send401Error(s, realm);
      return -1;
    } else {
      send401Error(s, "");
      return 0;
    }
  }
  decodedCreds = decode_base64(authCode + 6);
  debug("Decoded creds = %s\n", decodedCreds);
  char *separator;
  separator = strrchr(decodedCreds, ':');
  if (separator == 0) {
    free(decodedCreds);
    fclose(fp);
    return 0;
  }
  *separator = 0;
  char *username = decodedCreds;
  char *password = ++separator;
  debug("username: %s\npassword: %s\n", username, password);
  char *encryptedPassword;
  char line[200];
  while (1) {
    if (fgets(line, sizeof(line), fp) == 0)
      break;
    if (feof(fp)) {
      break;
    }
    if (line[0] == '#')
      continue;
    separator = strchr(line, ':');
    if (separator == 0) {
      debug("Badly formatted line in .access file\n");
      continue;
    }
    *separator = 0;
    encryptedPassword = ++separator;
    if (strlen(encryptedPassword) < 13) {
      debug("Badly formatted password in .access file\n");
      continue;
    }
    if (strcmp(line, username) != 0) {
      debug("%s name didn't match\n", line);
      continue;
    }
    fclose(fp);
    debug("We must have found a match, now to check the password\n");
    debug("re-crypted password = %s\n", crypt(password, encryptedPassword));
    if (strncmp(encryptedPassword, crypt(password, encryptedPassword), 13) ==
        0) {
      debug("The password matched!!");
      free(decodedCreds);
      return 0;
    } else {
      send401Error(s, realm);
      debug("the password did not match\n");
      free(decodedCreds);
      return -1;
    }
  }
  free(decodedCreds);
  fclose(fp);
  return 0;
}
void handlePost(int in, int out, serverConfigType *config, char *uri,
                dictionaryType *headers, char *version) {
  int content_length = 0;
  char *content_length_header;
  char *transfer_encoding_header;
  unsigned char *body_data;
  char *expect_header;
  char response_header[400];
  char response_data[400];
  struct tm serverTime;
  char serverDateString[31];
  char *message;
  if (config == 0)
    return;
  if (headers == 0)
    return;
  if (uri == 0)
    return;
  expect_header = findDict(headers, "Expect");
  if (expect_header) {
    if (strcmp(expect_header, "100-continue") == 0) {
      debug("Got a 100-continue header so sending that response if we can\n");
      if (content_length <= config->maxEntitySize) {
        sendError(out, 100);
      }
    }
  } else {
    if (version && strcmp(version, "HTTP/1.1") == 0) {
      debug("need to send a continue message to this client\n");
      if (content_length <= config->maxEntitySize) {
        sendError(out, 100);
      }
    }
  }
  content_length_header = 0;
  transfer_encoding_header = 0;
  content_length_header = findDict(headers, "Content-Length");
  if (content_length_header) {
    content_length = atoi(content_length_header);
  } else {
    debug("No Content-Length header found\n");
    transfer_encoding_header = findDict(headers, "Transfer-Encoding");
    if (!transfer_encoding_header) {
      debug("No Transfer-Encoding header found in Post\n");
      sendError(out, 411);
      return;
    }
  }
  if (transfer_encoding_header &&
      strcmp(transfer_encoding_header, "chunked") == 0 &&
      content_length_header) {
    debug("Content-Length should not be used with Chunked encoding\n");
    sendError(out, 400);
    return;
  }
  if (!content_length_header &&
      strcmp(transfer_encoding_header, "chunked") != 0) {
    debug(
        "Content-Length or chunked transfer encoding is required for a POST\n");
    sendError(out, 400);
    return;
  }
  if (content_length > config->maxEntitySize) {
    debug("The content is too large for this server to handle\n");
    sendError(out, 413);
    return;
  }
  int result;
  int received_body_size;
  body_data = 0;
  result = read_body(in, headers, &body_data, config->maxEntitySize,
                     &received_body_size);
  if (result != 200) {
    sendError(out, result);
    if (body_data) {
      free(body_data);
      body_data = 0;
    }
    return;
  }
  content_length = received_body_size;
  char *content_type = findDict(headers, "Content-Type");
  if (content_type == 0) {
    debug("Content-Type header not found, assuming application/octet-stream\n");
    sprintf(response_data, "<html>You submitted data w/o a Content-Type field, "
                           "assuming its binary</br></html>");
    message = response_data;
  } else if (strcmp(content_type, "application/x-www-form-urlencoded") == 0) {
    sprintf(response_data, "<html>You submitted form data</br></html>");
    message = handle_encoded_form_data((char *)body_data);
  } else if (strcmp(content_type, "text/html") == 0) {
    sprintf(response_data, "<html>You submitted text/html</br></html>");
    message = response_data;
  } else if (strcmp(content_type, "application/octet-stream") == 0) {
    message = handle_binary_data((unsigned char *)body_data, content_length);
  } else if (strcmp(content_type, "text/plain") == 0) {
    message = handle_text_data((char *)body_data, content_length);
  } else {
    snprintf(response_data, sizeof(response_data),
             "<html>You submitted %s data</br></html>", content_type);
    message = response_data;
  }
  if (body_data)
    free(body_data);
  if (!message) {
    sendError(out, 500);
    return;
  }
  memset(&serverTime, 0, sizeof(struct tm));
  time_t server_time_t = time(NULL);
  if (server_time_t != -1) {
    gmtime_r(&server_time_t, &serverTime);
    strftime(serverDateString, 30, "%a, %d %b %Y %H:%M:%S GMT", &serverTime);
    debug("Server Date = %s |\n", serverDateString);
    sprintf(response_header, "HTTP/1.0 200 OK\r\n"
                             "Date: %s\r\n"
                             "Content-Length: %ld\r\n"
                             "Content-Type: %s\r\n"
                             "Connection: Close\r\n\r\n",
            serverDateString, strlen(message), "text/html");
  } else {
    sprintf(response_header, "HTTP/1.0 200 OK\r\n"
                             "Content-Length: %ld\r\n"
                             "Content-Type: %s\r\n"
                             "Connection: Close\r\n\r\n",
            strlen(message), "text/html");
  }
  fprintf(stderr, "Sending 200 response\n");
  int write_count;
  int total_sent;
  int amount_to_send;
  amount_to_send = strlen(response_header);
  total_sent = 0;
  while (total_sent < amount_to_send) {
    write_count =
        write(out, response_header + total_sent, amount_to_send - total_sent);
    if (write_count < 0) {
      debug("Error sending on socket\n");
      return;
    }
    total_sent += write_count;
  }
  amount_to_send = strlen(message);
  total_sent = 0;
  while (total_sent < amount_to_send) {
    write_count = write(out, message + total_sent, amount_to_send - total_sent);
    if (write_count < 0) {
      debug("Error sending on socket\n");
      return;
    }
    total_sent += write_count;
  }
  if (message != response_data) {
    free(message);
  }
  return;
}
void headFile(int s, serverConfigType *config, char *uri,
              dictionaryType *headers) {
  char response[400];
  char contentType[100];
  char *resourcePath;
  char *decodedResourcePath;
  if (config == 0)
    return;
  if (headers == 0)
    return;
  if (uri == 0)
    return;
  if (strstr(uri, "..") != 0) {
    sendError(s, 404);
    return;
  }
  if (uri[strlen(uri) - 1] == '/') {
    resourcePath = malloc(strlen(config->docRoot) + strlen(uri) +
                          strlen(config->defaultFile) + 2);
  } else {
    resourcePath = malloc(strlen(config->docRoot) + strlen(uri) + 2);
  }
  if (!resourcePath) {
    return;
  }
  strcpy(resourcePath, config->docRoot);
  strcat(resourcePath, uri);
  if (resourcePath[strlen(resourcePath) - 1] == '/') {
    strcat(resourcePath, config->defaultFile);
  }
  debug("resource: %s|\n", resourcePath);
  decodedResourcePath = url_decode(resourcePath);
  debug("decoded resource: %s|\n", decodedResourcePath);
  free(resourcePath);
  int fileCheck = checkFile(decodedResourcePath);
  if (fileCheck != 0) {
    switch (fileCheck) {
    case -1:
      debug("Errno: %d\n", errno);
      perror(0);
      sendError(s, 500);
      break;
    case 1:
      sendError(s, 404);
      break;
    case 2:
      sendError(s, 403);
      break;
    }
    free(decodedResourcePath);
    return;
  }
  if (checkAuth(s, decodedResourcePath, headers) != 0) {
    debug("Not authorized to read that file\n");
    free(decodedResourcePath);
    return;
  }
  debug("No authorization required\n");
  int fd = open(decodedResourcePath, O_RDONLY);
  if (fd == -1) {
    debug("File was not found or could't be read\n");
    debug("Errno: %d\n", errno);
    perror(0);
    sendError(s, 404);
    free(decodedResourcePath);
    return;
  }
  struct stat sb;
  if (fstat(fd, &sb) != 0) {
    debug("File was not found or could't be read\n");
    sendError(s, 404);
    free(decodedResourcePath);
    close(fd);
    return;
  }
  char *extension;
  extension = strrchr(decodedResourcePath, '.');
  if (!extension) {
    strcpy(contentType, "application/octet-stream");
  } else {
    extension++;
    debug("extension = %s\n", extension);
    char *value = findDict(mimeDict, extension);
    debug("Found mime type = %s\n", value);
    if (value) {
      strcpy(contentType, value);
    } else {
      strcpy(contentType, "application/octet-stream");
    }
  }
  free(decodedResourcePath);
  struct tm fileTime;
  struct tm ifModTime;
  struct tm expireDate;
  struct tm serverTime;
  char fileDateString[31];
  char serverDateString[31];
  char expireString[31];
  memset(&fileTime, 0, sizeof(struct tm));
  memset(&ifModTime, 0, sizeof(struct tm));
  memset(&serverTime, 0, sizeof(struct tm));
  gmtime_r(&sb.st_mtime, &fileTime);
  strftime(fileDateString, 30, "%a, %d %b %Y %H:%M:%S GMT", &fileTime);
  debug("File Modified Date = %s |\n", fileDateString);
  sb.st_mtime += 120;
  gmtime_r(&sb.st_mtime, &expireDate);
  strftime(expireString, 30, "%a, %d %b %Y %H:%M:%S GMT", &expireDate);
  debug("File Expire Date = %s |\n", expireString);
  time_t server_time_t = time(NULL);
  if (server_time_t != -1) {
    gmtime_r(&server_time_t, &serverTime);
    strftime(serverDateString, 30, "%a, %d %b %Y %H:%M:%S GMT", &serverTime);
    debug("Server Date = %s |\n", serverDateString);
    sprintf(response, "HTTP/1.0 200 OK\r\n"
                      "Date: %s\r\n"
                      "Content-Length: %ld\r\n"
                      "Content-Type: %s\r\n"
                      "Last-Modified: %s\r\n"
                      "Expires: %s\r\n"
                      "Connection: Close\r\n\r\n",
            serverDateString, sb.st_size, contentType, fileDateString,
            expireString);
  } else {
    sprintf(response, "HTTP/1.0 200 OK\r\n"
                      "Content-Length: %ld\r\n"
                      "Content-Type: %s\r\n"
                      "Last-Modified: %s\r\n"
                      "Expires: %s\r\n"
                      "Connection: Close\r\n\r\n",
            sb.st_size, contentType, fileDateString, expireString);
  }
  fprintf(stderr, "Sending 200 response\n");
  int write_count;
  int total_sent;
  int amount_to_send;
  amount_to_send = strlen(response);
  total_sent = 0;
  while (total_sent < amount_to_send) {
    write_count = write(s, response + total_sent, amount_to_send - total_sent);
    if (write_count < 0) {
      debug("Error sending on socket\n");
      close(fd);
      return;
    }
    total_sent += write_count;
  }
  close(fd);
  debug("Finished sending file\n");
  return;
}
void showStats(int s) {
  char response[400];
  debug("showing connection stats\n");
  struct tm serverTime;
  char serverDateString[31];
  memset(&serverTime, 0, sizeof(struct tm));
  time_t server_time_t = time(NULL);
  if (server_time_t != -1) {
    gmtime_r(&server_time_t, &serverTime);
    strftime(serverDateString, 30, "%a, %d %b %Y %H:%M:%S GMT", &serverTime);
    debug("Server Date = %s |\n", serverDateString);
    sprintf(response, "HTTP/1.0 200 OK\r\n"
                      "Date: %s\r\n"
                      "Content-Type: %s\r\n"
                      "Transfer-Encoding: chunked\r\n"
                      "Connection: Close\r\n\r\n",
            serverDateString, "text/html");
  } else {
    sprintf(response, "HTTP/1.0 200 OK\r\n"
                      "Content-Type: %s\r\n"
                      "Transfer-Encoding: chunked\r\n"
                      "Connection: Close\r\n\r\n",
            "text/html");
  }
  fprintf(stderr, "Sending 200 response\n");
  int write_count;
  int total_sent;
  int amount_to_send;
  amount_to_send = strlen(response);
  total_sent = 0;
  while (total_sent < amount_to_send) {
    write_count = write(s, response + total_sent, amount_to_send - total_sent);
    if (write_count < 0) {
      debug("Error sending on socket\n");
      return;
    }
    total_sent += write_count;
  }
  hostStatsHeader(s);
  printBst(s, hoststats, hostStatsTable);
  hostStatsFooter(s);
  return;
}
void showUserAgents(int s) {
  char response[400];
  struct tm serverTime;
  char serverDateString[31];
  debug("showing useragent stats\n");
  memset(&serverTime, 0, sizeof(struct tm));
  time_t server_time_t = time(NULL);
  if (server_time_t != -1) {
    gmtime_r(&server_time_t, &serverTime);
    strftime(serverDateString, 30, "%a, %d %b %Y %H:%M:%S GMT", &serverTime);
    debug("Server Date = %s |\n", serverDateString);
    sprintf(response, "HTTP/1.0 200 OK\r\n"
                      "Date: %s\r\n"
                      "Content-Type: %s\r\n"
                      "Transfer-Encoding: chunked\r\n"
                      "Connection: Close\r\n\r\n",
            serverDateString, "text/html");
  } else {
    sprintf(response, "HTTP/1.0 200 OK\r\n"
                      "Content-Type: %s\r\n"
                      "Transfer-Encoding: chunked\r\n"
                      "Connection: Close\r\n\r\n",
            "text/html");
  }
  fprintf(stderr, "Sending 200 response\n");
  int write_count;
  int total_sent;
  int amount_to_send;
  amount_to_send = strlen(response);
  total_sent = 0;
  while (total_sent < amount_to_send) {
    write_count = write(s, response + total_sent, amount_to_send - total_sent);
    if (write_count < 0) {
      debug("Error sending on socket\n");
      return;
    }
    total_sent += write_count;
  }
  uaStatsHeader(s);
  printBst(s, ua_stats, uaStatsTable);
  uaStatsFooter(s);
  return;
}
int serveFile(int s, serverConfigType *config, char *uri,
              dictionaryType *headers) {
  char buffer[HTTP_BUFFER_SIZE];
  char response[400];
  char contentType[100];
  const char format1[] = "%a, %d %b %Y %H:%M:%S";
  const char format2[] = "%A %d-%b-%y %H:%M:%S";
  const char format3[] = "%a %b %d %H:%M:%S %Y";
  char *resourcePath;
  char *decodedResourcePath;
  char *extension;
  char *acceptLanguageHeader;
  acceptLanguageHeader = findDict(headers, "Accept-Language");
  if (acceptLanguageHeader) {
    if (strstr(acceptLanguageHeader, "en;") == 0) {
      sendError(s, 406);
      return -1;
    }
  }
  if (strcmp(uri, "/stats") == 0) {
    showStats(s);
    return 0;
  }
  if (strcmp(uri, "/useragents") == 0) {
    showUserAgents(s);
    return 0;
  }
  if (strstr(uri, "..") != 0) {
    sendError(s, 404);
    return 0;
  }
  if (uri[strlen(uri) - 1] == '/') {
    resourcePath = malloc(strlen(config->docRoot) + strlen(uri) +
                          strlen(config->defaultFile) + 2);
  } else {
    resourcePath = malloc(strlen(config->docRoot) + strlen(uri) + 2);
  }
  if (!resourcePath) {
    return -1;
  }
  strcpy(resourcePath, config->docRoot);
  strcat(resourcePath, uri);
  if (resourcePath[strlen(resourcePath) - 1] == '/') {
    strcat(resourcePath, config->defaultFile);
  }
  debug("resource: %s|\n", resourcePath);
  decodedResourcePath = url_decode(resourcePath);
  debug("decoded resource: %s|\n", decodedResourcePath);
  free(resourcePath);
  int fileCheck = checkFile(decodedResourcePath);
  if (fileCheck != 0) {
    switch (fileCheck) {
    case -1:
      debug("Errno: %d\n", errno);
      perror(0);
      sendError(s, 500);
      break;
    case 1:
      sendError(s, 404);
      break;
    case 2:
      sendError(s, 403);
      break;
    }
    free(decodedResourcePath);
    return -1;
  }
  if (checkAuth(s, decodedResourcePath, headers) != 0) {
    debug("Not authorized to read that file\n");
    free(decodedResourcePath);
    return 0;
  }
  debug("No authorization required\n");
  int fd = open(decodedResourcePath, O_RDONLY);
  if (fd == -1) {
    debug("File was not found or could't be read\n");
    debug("Errno: %d\n", errno);
    sendError(s, 404);
    free(decodedResourcePath);
    return 0;
  }
  struct stat sb;
  if (fstat(fd, &sb) != 0) {
    debug("File was not found or could't be read\n");
    sendError(s, 404);
    free(decodedResourcePath);
    return 0;
  }
  fprintf(stderr, "Sending 200 response\n");
  extension = strrchr(decodedResourcePath, '.');
  if (!extension) {
    strcpy(contentType, "application/octet-stream");
  } else {
    extension++;
    debug("extension = %s\n", extension);
    char *value = findDict(mimeDict, extension);
    debug("Found mime type = %s\n", value);
    if (value) {
      strcpy(contentType, value);
    } else {
      strcpy(contentType, "application/octet-stream");
    }
  }
  free(decodedResourcePath);
  struct tm fileTime;
  struct tm ifModTime;
  struct tm serverTime;
  char fileDateString[31];
  char serverDateString[31];
  char expireString[31];
  memset(&fileTime, 0, sizeof(struct tm));
  memset(&ifModTime, 0, sizeof(struct tm));
  memset(&serverTime, 0, sizeof(struct tm));
  time_t server_time_t = time(NULL);
  if (server_time_t != -1) {
    gmtime_r(&server_time_t, &serverTime);
    strftime(serverDateString, 30, "%a, %d %b %Y %H:%M:%S GMT", &serverTime);
    debug("Server Date = %s |\n", serverDateString);
    debug("Server Date2 = %s\n", ctime(&server_time_t));
  }
  gmtime_r(&sb.st_mtime, &fileTime);
  strftime(fileDateString, 30, "%a, %d %b %Y %H:%M:%S GMT", &fileTime);
  debug("File Modified Date = %s |\n", fileDateString);
  char *modifiedDate;
  modifiedDate = findDict(headers, "If-Modified-Since");
  int modDateFail = 0;
  if (server_time_t && modifiedDate) {
    debug("There's a If-Modified-Since header of value: %s\n", modifiedDate);
    if (strptime(modifiedDate, format1, &ifModTime) == 0) {
      if (strptime(modifiedDate, format2, &ifModTime) == 0) {
        if (strptime(modifiedDate, format3, &ifModTime) == 0) {
          modDateFail = 1;
        }
      }
    }
    if (!modDateFail) {
      extern long timezone;
      tzset();
      debug("%ld <= %ld\n", mktime(&ifModTime) - timezone, server_time_t);
      if (mktime(&ifModTime) - timezone <= server_time_t) {
        debug("%ld >= %ld\n", mktime(&ifModTime), mktime(&fileTime));
        if (mktime(&ifModTime) >= mktime(&fileTime)) {
          debug("File mod time is earlier or equal, so don't send it\n");
          sendError(s, 304);
          return 0;
        }
      }
    }
  }
  struct tm expireDate;
  sb.st_mtime += 120;
  gmtime_r(&sb.st_mtime, &expireDate);
  strftime(expireString, 30, "%a, %d %b %Y %H:%M:%S GMT", &expireDate);
  debug("File Expire Date = %s |\n", expireString);
  if (strlen(serverDateString)) {
    sprintf(response, "HTTP/1.0 200 OK\r\n"
                      "Date: %s\r\n"
                      "Content-Length: %ld\r\n"
                      "Content-Type: %s\r\n"
                      "Last-Modified: %s\r\n"
                      "Expires: %s\r\n"
                      "Connection: Close\r\n\r\n",
            serverDateString, sb.st_size, contentType, fileDateString,
            expireString);
  } else {
    sprintf(response, "HTTP/1.0 200 OK\r\n"
                      "Content-Length: %ld\r\n"
                      "Content-Type: %s\r\n"
                      "Last-Modified: %s\r\n"
                      "Expires: %s\r\n"
                      "Connection: Close\r\n\r\n",
            sb.st_size, contentType, fileDateString, expireString);
  }
  fprintf(stderr, "Sending 200 response\n");
  int write_count;
  int total_sent;
  int amount_to_send;
  amount_to_send = strlen(response);
  total_sent = 0;
  while (total_sent < amount_to_send) {
    write_count = write(s, response + total_sent, amount_to_send - total_sent);
    if (write_count < 0) {
      debug("Error sending on socket\n");
      close(fd);
      return -1;
    }
    total_sent += write_count;
  }
  int readSize;
  while ((readSize = read(fd, buffer, HTTP_BUFFER_SIZE))) {
    if (readSize <= 0)
      break;
    debug("writing %d bytes of file\n", readSize);
    amount_to_send = readSize;
    total_sent = 0;
    while (total_sent < amount_to_send) {
      write_count = write(s, buffer + total_sent, amount_to_send - total_sent);
      if (write_count < 0) {
        debug("Error sending on socket\n");
        close(fd);
        return -1;
      }
      total_sent += write_count;
    }
  }
  close(fd);
  debug("Finished sending file\n");
  return 0;
}
