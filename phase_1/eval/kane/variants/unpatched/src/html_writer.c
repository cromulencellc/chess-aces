#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "html_writer.h"
#define MAX_FILE_NAME_SIZE 128
#define MAX_CONTENT_LENGTH 1024
#define MAX_HTML_OBJECTS 64
#define MAX_FILE_SIZE 4096
#define MAX_PATH_SIZE 128
#define MAX_DIR_SIZE 32
#define ORIGIN_PATH "/wiki/origins/"
#define CACHE_PATH "/wiki/cache/"
void parseTextFile(char *contentsOfFile, FILE *file);
void parseHtmlFile(char *contentsOfFile, FILE *file);
void textHtmlMatchPair();
int compare(const void *a, const void *b);
void textFileToHtml(char *nameOfFile) {
  char *read_path = calloc(1, MAX_FILE_NAME_SIZE);
  char *root_dir = "/data";
  char *file_name = strdup(nameOfFile);
  snprintf(read_path,
           strlen(root_dir) + strlen(ORIGIN_PATH) + strlen(file_name) + 2,
           "%s%s/%s", root_dir, ORIGIN_PATH, file_name);
  FILE *read_fptr = fopen(read_path, "r");
  fseek(read_fptr, 0L, SEEK_END);
  int size = ftell(read_fptr);
  rewind(read_fptr);
  char *file_content = calloc(1, MAX_FILE_SIZE);
  fread(file_content, size + 1, 1, read_fptr);
  free(read_path);
  read_path = NULL;
  char *write_path = calloc(1, MAX_FILE_NAME_SIZE);
  file_name[strlen(file_name) - 4] = '\0';
  strcat(file_name, ".html");
  snprintf(write_path,
           strlen(root_dir) + strlen(CACHE_PATH) + strlen(file_name) + 2,
           "%s%s/%s", root_dir, CACHE_PATH, file_name);
  FILE *write_fptr = fopen(write_path, "w");
  free(write_path);
  write_path = NULL;
  fclose(read_fptr);
  fwrite("<html>\n", 7, 1, write_fptr);
  fwrite("\t<head>\n", 8, 1, write_fptr);
  fwrite("\t\t<title>Internal Wiki</title>\n",
         strlen("\t\t<title>Internal Wiki</title>\n"), 1, write_fptr);
  fwrite("\t\t<h1>", 6, 1, write_fptr);
  fwrite(nameOfFile, strlen(nameOfFile), 1, write_fptr);
  fwrite("</h1>\n", 6, 1, write_fptr);
  fwrite("\t</head>\n", 9, 1, write_fptr);
  fwrite("\t<body>\n", 8, 1, write_fptr);
  parseTextFile(file_content, write_fptr);
  free(file_content);
  file_content = NULL;
  fwrite("\t</body>\n", 9, 1, write_fptr);
  fwrite("</html>", 7, 1, write_fptr);
  fclose(write_fptr);
}
void parseTextFile(char *contentsOfFile, FILE *file) {
  char **holder = calloc(64, sizeof(char *));
  int k;
  for (k = 0; k < 64; ++k)
    holder[k] = calloc(1, MAX_CONTENT_LENGTH * sizeof(char));
  int i, j = 0, count = 0;
  for (i = 0; i < 700; ++i) {
    holder[count][j] = contentsOfFile[i];
    j++;
    if (contentsOfFile[i] == '\n' && contentsOfFile[i + 1] == '\n') {
      fwrite("\t\t<h2>", 6, 1, file);
      fwrite(holder[count], strlen(holder[count]) - 1, 1, file);
      fwrite("</h2>\n", 6, 1, file);
      i += 1;
      count++;
      j = 0;
    }
    if (contentsOfFile[i] == '.' && contentsOfFile[i + 1] == '.' &&
        contentsOfFile[i + 2] == '.') {
      fwrite("\t\t<p>", 5, 1, file);
      fwrite(holder[count], strlen(holder[count]) - 1, 1, file);
      fwrite("</p>\n", 5, 1, file);
      count++;
      i += 3;
      j = 0;
    }
  }
  free(holder);
}
void htmlToTextFile(char *nameOfFile) {}
void parseHtmlFile(char *contentsOfFile, FILE *file) {}
int compare(const void *a, const void *b) {
  return (strcmp(*(char **)a, *(char **)b));
}
void textHtmlMatchPair() {
  DIR *originsDir, *cacheDir;
  struct dirent *oriDir;
  struct dirent *cacDir;
  char *env = "/data";
  char *origin = NULL;
  char *cache = NULL;
  origin = (char *)malloc(strlen(env) + strlen(ORIGIN_PATH) + 1);
  cache = (char *)malloc(strlen(env) + strlen(CACHE_PATH) + 1);
  snprintf(origin, strlen(env) + strlen(ORIGIN_PATH) + 1, "%s%s", env,
           ORIGIN_PATH);
  snprintf(cache, strlen(env) + strlen(CACHE_PATH) + 1, "%s%s", env,
           CACHE_PATH);
  struct stat st = {0};
  if (stat(origin, &st) == 0) {
    originsDir = opendir(origin);
  } else {
    mkdir(origin, 0777);
    originsDir = opendir(origin);
  }
  if (stat(cache, &st) == 0) {
    cacheDir = opendir(cache);
  } else {
    mkdir(cache, 0777);
    cacheDir = opendir(cache);
  }
  char **originsList = NULL;
  char **cacheList = NULL;
  int txt_countOrigins = 0;
  int txt_countCache = 0;
  while ((oriDir = readdir(originsDir)) != NULL) {
    if (strcmp(oriDir->d_name + (strlen(oriDir->d_name) - 4), ".txt") != 0) {
      continue;
    } else {
      originsList =
          realloc(originsList, (txt_countOrigins + 1) * sizeof(char *));
      originsList[txt_countOrigins] = calloc(1, strlen(oriDir->d_name) + 1);
      strncpy(originsList[txt_countOrigins], oriDir->d_name,
              strlen(oriDir->d_name));
      txt_countOrigins++;
    }
  }
  while ((cacDir = readdir(cacheDir)) != NULL) {
    if (strcmp(cacDir->d_name + (strlen(cacDir->d_name) - 5), ".html") != 0) {
      continue;
    } else {
      cacheList = realloc(cacheList, (txt_countCache + 1) * sizeof(char *));
      cacheList[txt_countCache] = calloc(1, strlen(cacDir->d_name) + 1);
      strncpy(cacheList[txt_countCache], cacDir->d_name,
              strlen(cacDir->d_name));
      txt_countCache++;
    }
  }
  closedir(originsDir);
  closedir(cacheDir);
  qsort(originsList, txt_countOrigins, sizeof(originsList[0]), compare);
  qsort(cacheList, txt_countCache, sizeof(cacheList[0]), compare);
  int i = 0;
  for (i = 0; i < txt_countOrigins && i < MAX_DIR_SIZE; ++i) {
    textFileToHtml(originsList[i]);
  }
  free(originsList);
  originsList = NULL;
  free(cacheList);
  cacheList = NULL;
}
