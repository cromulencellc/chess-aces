
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "lru_queue.h"
#define MAX_FILENAME 128
#define MAX_FILE_SIZE 4096
#define CACHE_SIZE 49152
#define NUMBER_OF_ENTRIES 12
#define PATH_TO_DATA "/data/wiki/origins/"
LRU_Queue *create_queue() {
  LRU_Queue *queue = malloc(sizeof(LRU_Queue));
  queue->size = 0;
  queue->entry = NULL;
  return queue;
}
void enqueue(LRU_Queue *queue, char *fn) {
  if (queue->size > 0) {
    int index;
    if ((index = search_queue(queue, fn)) != -1) {
      printf("ENTRY: %s EXISTS IN CACHE: HIT\n", fn);
      move_back(queue, index);
      printf("ENTRY MOVED TO BACK OF QUEUE\n");
      return;
    }
    if (CACHE_SIZE / queue->size == NUMBER_OF_ENTRIES) {
      dequeue(queue);
    }
  }
  if (access(fn, F_OK) != 0) {
    printf("The file does not exist\n");
    return;
  }
  queue->entry = realloc(queue->entry, (queue->size + 1) * sizeof(Cache_Entry));
  queue->entry[queue->size].file_name = calloc(1, MAX_FILENAME);
  strncpy(queue->entry[queue->size].file_name, fn, MAX_FILENAME);
  time_t t;
  struct tm *clock;
  time(&t);
  clock = gmtime(&t);
  queue->entry[queue->size].time[0] = clock->tm_hour;
  queue->entry[queue->size].time[1] = clock->tm_min;
  queue->entry[queue->size].time[2] = clock->tm_sec;
  FILE *fp;
  fp = fopen(fn, "r");
  fseek(fp, 0L, SEEK_END);
  int size = ftell(fp);
  rewind(fp);
  queue->entry[queue->size].contents = calloc(1, MAX_FILE_SIZE);
  fread(queue->entry[queue->size].contents, size + 1, 1, fp);
  fclose(fp);
  queue->size++;
}
void intialize_queue(LRU_Queue *queue) {
  DIR *d;
  struct dirent *dir;
  char **htmlFileList;
  if ((d = opendir(PATH_TO_DATA)) == NULL) {
    printf("Unable to find directory for cache intialization\n");
    closedir(d);
    return;
  }
  int html_count = 0;
  htmlFileList = malloc(sizeof(char *));
  while ((dir = readdir(d)) != NULL) {
    if (strcasecmp(dir->d_name + strlen(dir->d_name) - 5, ".html") != 0) {
      continue;
    } else {
      htmlFileList = realloc(htmlFileList, (html_count + 1) * sizeof(char *));
      htmlFileList[html_count] =
          calloc(1, strlen(PATH_TO_DATA) + strlen(dir->d_name) + 1);
      memcpy(htmlFileList[html_count], PATH_TO_DATA,
             strlen(PATH_TO_DATA) + strlen(dir->d_name));
      strcat(htmlFileList[html_count], dir->d_name);
      html_count++;
    }
  }
  closedir(d);
  int i = 0;
  for (i = 0; i < html_count; ++i) {
    enqueue(queue, htmlFileList[i]);
  }
  for (i = 0; i < html_count; ++i) {
    free(htmlFileList[i]);
    htmlFileList[i] = NULL;
  }
  free(htmlFileList);
  htmlFileList = NULL;
}
void dequeue(LRU_Queue *queue) {
  if (queue->size == 0) {
    printf("There is nothing to do dequeue at this time\n");
    return;
  }
  if (queue->size == 1) {
    printf("Dequeuing: %s\n", queue->entry[0].file_name);
    free(queue->entry[0].file_name);
    queue->entry[0].file_name = NULL;
    free(queue->entry[0].contents);
    queue->entry[0].contents = NULL;
    free(queue->entry);
    queue->entry = NULL;
    queue->size--;
    printf("\nDEQUEUED!\n\n");
    return;
  }
  int i;
  char *tmp = calloc(1, MAX_FILENAME);
  for (i = 0; i < queue->size - 1; ++i) {
    memset(queue->entry[i].file_name, 0, MAX_FILENAME);
    strncpy(tmp, queue->entry[i].file_name, MAX_FILENAME);
    strncpy(queue->entry[i].file_name, queue->entry[i + 1].file_name,
            MAX_FILENAME);
    strncpy(queue->entry[i + 1].file_name, tmp, MAX_FILENAME);
  }
  printf("Dequeuing: %s\n", queue->entry[queue->size - 1].file_name);
  free(queue->entry[queue->size - 1].file_name);
  queue->entry[queue->size - 1].file_name = NULL;
  free(queue->entry[queue->size - 1].contents);
  queue->entry[queue->size - 1].contents = NULL;
  free(tmp);
  tmp = NULL;
  queue->size--;
  printf("\nDEQUEUED!\n\n");
}
void move_back(LRU_Queue *queue, int index) {
  printf("Move back was called on %s at the %dth position\n",
         queue->entry[index].file_name, index);
  int i;
  char *tmp = calloc(1, MAX_FILENAME);
  char *tmp2 = calloc(1, MAX_FILENAME);
  strncpy(tmp2, queue->entry[index].file_name, MAX_FILENAME);
  for (i = index; i < queue->size - 1; ++i) {
    memset(queue->entry[i].file_name, 0, MAX_FILENAME);
    strncpy(tmp, queue->entry[i].file_name, MAX_FILENAME);
    strncpy(queue->entry[i].file_name, queue->entry[i + 1].file_name,
            MAX_FILENAME);
    strncpy(queue->entry[i + 1].file_name, tmp, MAX_FILENAME);
  }
  strncpy(queue->entry[queue->size - 1].file_name, tmp2, MAX_FILENAME);
  free(tmp);
  tmp = NULL;
  free(tmp2);
  tmp2 = NULL;
}
int search_queue(LRU_Queue *queue, char *name_of_file) {
  int i;
  for (i = 0; i < queue->size; ++i) {
    if (strcasecmp(name_of_file, queue->entry[i].file_name) == 0) {
      printf("FILE WAS FOUND IN QUEUE\n");
      return i;
    }
  }
  return -1;
}
void destroy_queue(LRU_Queue *target) {
  int i;
  for (i = 0; i < target->size; ++i) {
    free(target->entry[i].file_name);
    target->entry[i].file_name = NULL;
    free(target->entry[i].contents);
    target->entry[i].contents = NULL;
  }
  free(target->entry);
  target->entry = NULL;
  free(target);
  target = NULL;
}
void print_queue(LRU_Queue *queue) {
  if (queue->size < 1) {
    printf("Nothing to print\n");
    return;
  }
  int i;
  printf("---------------------------------------------------------------------"
         "--\n");
  for (i = 0; i < queue->size; ++i) {
    printf("\t%d) %s\n", i, queue->entry[i].file_name);
    printf("\t%dh %dmin %dsec\n", queue->entry[i].time[0],
           queue->entry[i].time[1], queue->entry[i].time[2]);
    printf("\t%s\n", queue->entry[i].contents);
  }
  printf("---------------------------------------------------------------------"
         "--\n");
}
