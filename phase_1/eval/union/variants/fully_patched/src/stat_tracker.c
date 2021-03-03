#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stat_tracker.h"
#define STAT_PATH "/data/stats/stat.log"
#define TMP_PATH "/data/stats/.tmp.log"
#define MAX_LINE 32
void update_tracker(char *event_string) {
  if (access(STAT_PATH, F_OK) != 0) {
    FILE *sfptr;
    if ((sfptr = fopen(STAT_PATH, "w")) == NULL) {
      fprintf(stderr, "[UPDATE_TRACKER] Problem creating stat.log file\n");
      return;
    }
    fwrite("\n\nACHIEVEMENT LOG\n\n", strlen("\n\nACHIEVEMENT LOG\n\n"), 1,
           sfptr);
    fwrite("TASKS CREATED:\t0\n", strlen("TASKS CREATED:\t0\n"), 1, sfptr);
    fwrite("TASKS COMPLETED:\t0\n", strlen("TASKS COMPLETED: 0\n"), 1, sfptr);
    fwrite("SESSIONS CREATED:\t0\n", strlen("SESSIONS CREATED: 0\n"), 1, sfptr);
    fwrite("SESSIONS COMPLETED:\t0\n", strlen("SESSIONS COMPLETED: 0\n"), 1,
           sfptr);
    fclose(sfptr);
  }
  FILE *rfptr, *tfptr;
  if ((rfptr = fopen(STAT_PATH, "r")) == NULL) {
    fprintf(stderr, "[UPDATE_TRACKER] Problem creating stat.log file\n");
    return;
  }
  char *buffer = calloc(1, MAX_LINE + 1);
  if (strncmp("Task_Created", event_string, strlen("Task_Created")) == 0) {
    if ((tfptr = fopen(TMP_PATH, "w")) == NULL) {
      fprintf(stderr, "[UPDATE_TRACKER] Problem creating .tmp.log file\n");
      return;
    }
    while (fgets(buffer, MAX_LINE, rfptr) != NULL) {
      if (strncmp(buffer, "TASKS CREATED", 13) == 0) {
        char *cpy = strdup(buffer);
        char hold_buffer[MAX_LINE + 1];
        cpy[15] = '\0';
        char *holder = strchr(buffer, '\t');
        holder[strlen(holder) - 1] = '\0';
        holder += 1;
        int stat = atoi(holder);
        stat++;
        snprintf(hold_buffer, MAX_LINE, "%s%d\n", cpy, stat);
        free(cpy);
        cpy = NULL;
        fwrite(hold_buffer, strlen(hold_buffer), 1, tfptr);
      } else {
        fwrite(buffer, strlen(buffer), 1, tfptr);
      }
    }
    free(buffer);
    buffer = NULL;
    fclose(rfptr);
    rfptr = NULL;
    fclose(tfptr);
    tfptr = NULL;
    int rem_check = -1;
    if ((rem_check = remove(STAT_PATH)) != 0) {
      fprintf(stderr,
              "[UPDATE_TRACKER] There was trouble removing old stat.log\n");
      return;
    }
    int ren_check = -1;
    if ((ren_check = rename(TMP_PATH, STAT_PATH)) != 0) {
      fprintf(stderr,
              "[UPDATE_TRACKER] There was trouble removing old stat.log\n");
      return;
    }
  } else if (strncmp("Task_Completed", event_string,
                     strlen("Task_Completed")) == 0) {
    if ((tfptr = fopen(TMP_PATH, "w")) == NULL) {
      fprintf(stderr, "[UPDATE_TRACKER] Problem creating .tmp.log file\n");
      return;
    }
    while (fgets(buffer, MAX_LINE, rfptr) != NULL) {
      if (strncmp(buffer, "TASKS COMPLETED", 15) == 0) {
        char *cpy = strdup(buffer);
        char hold_buffer[MAX_LINE + 1];
        cpy[17] = '\0';
        char *holder = strchr(buffer, '\t');
        holder[strlen(holder) - 1] = '\0';
        holder += 1;
        int stat = atoi(holder);
        stat++;
        snprintf(hold_buffer, MAX_LINE, "%s%d\n", cpy, stat);
        free(cpy);
        cpy = NULL;
        fwrite(hold_buffer, strlen(hold_buffer), 1, tfptr);
      } else {
        fwrite(buffer, strlen(buffer), 1, tfptr);
      }
    }
    free(buffer);
    buffer = NULL;
    fclose(rfptr);
    rfptr = NULL;
    fclose(tfptr);
    tfptr = NULL;
    int rem_check = -1;
    if ((rem_check = remove(STAT_PATH)) != 0) {
      fprintf(stderr,
              "[UPDATE_TRACKER] There was trouble removing old stat.log\n");
      return;
    }
    int ren_check = -1;
    if ((ren_check = rename(TMP_PATH, STAT_PATH)) != 0) {
      fprintf(stderr,
              "[UPDATE_TRACKER] There was trouble removing old stat.log\n");
      return;
    }
  } else if (strncmp("Session_Created", event_string,
                     strlen("Session_Created")) == 0) {
    if ((tfptr = fopen(TMP_PATH, "w")) == NULL) {
      fprintf(stderr, "[UPDATE_TRACKER] Problem creating .tmp.log file\n");
      return;
    }
    while (fgets(buffer, MAX_LINE, rfptr) != NULL) {
      if (strncmp(buffer, "SESSIONS CREATED", 16) == 0) {
        char *cpy = strdup(buffer);
        char hold_buffer[MAX_LINE + 1];
        cpy[18] = '\0';
        char *holder = strchr(buffer, '\t');
        holder[strlen(holder) - 1] = '\0';
        holder += 1;
        int stat = atoi(holder);
        stat++;
        snprintf(hold_buffer, MAX_LINE, "%s%d\n", cpy, stat);
        free(cpy);
        cpy = NULL;
        fwrite(hold_buffer, strlen(hold_buffer), 1, tfptr);
      } else {
        fwrite(buffer, strlen(buffer), 1, tfptr);
      }
    }
    free(buffer);
    buffer = NULL;
    fclose(rfptr);
    rfptr = NULL;
    fclose(tfptr);
    tfptr = NULL;
    int rem_check = -1;
    if ((rem_check = remove(STAT_PATH)) != 0) {
      fprintf(stderr,
              "[UPDATE_TRACKER] There was trouble removing old stat.log\n");
      return;
    }
    int ren_check = -1;
    if ((ren_check = rename(TMP_PATH, STAT_PATH)) != 0) {
      fprintf(stderr,
              "[UPDATE_TRACKER] There was trouble removing old stat.log\n");
      return;
    }
  } else if (strncmp("Session_Completed", event_string,
                     strlen("Session_Completed")) == 0) {
    if ((tfptr = fopen(TMP_PATH, "w")) == NULL) {
      fprintf(stderr, "[UPDATE_TRACKER] Problem creating .tmp.log file\n");
      return;
    }
    while (fgets(buffer, MAX_LINE, rfptr) != NULL) {
      if (strncmp(buffer, "SESSIONS COMPLETED", 18) == 0) {
        char *cpy = strdup(buffer);
        char hold_buffer[MAX_LINE + 1];
        cpy[20] = '\0';
        char *holder = strchr(buffer, '\t');
        holder[strlen(holder) - 1] = '\0';
        holder += 1;
        int stat = atoi(holder);
        stat++;
        snprintf(hold_buffer, MAX_LINE, "%s%d\n", cpy, stat);
        free(cpy);
        cpy = NULL;
        fwrite(hold_buffer, strlen(hold_buffer), 1, tfptr);
      } else {
        fwrite(buffer, strlen(buffer), 1, tfptr);
      }
    }
    free(buffer);
    buffer = NULL;
    fclose(rfptr);
    rfptr = NULL;
    fclose(tfptr);
    tfptr = NULL;
    int rem_check = -1;
    if ((rem_check = remove(STAT_PATH)) != 0) {
      fprintf(stderr,
              "[UPDATE_TRACKER] There was trouble removing old stat.log\n");
      return;
    }
    int ren_check = -1;
    if ((ren_check = rename(TMP_PATH, STAT_PATH)) != 0) {
      fprintf(stderr,
              "[UPDATE_TRACKER] There was trouble removing old stat.log\n");
      return;
    }
  } else {
    fprintf(stderr, "[UPDATE_TRACKER] The event string provided is invalid\n");
    fclose(rfptr);
    return;
  }
}
void print_tracker() {
  char buffer[MAX_LINE + 1];
  FILE *rfptr;
  if ((rfptr = fopen(STAT_PATH, "r")) == NULL) {
    fprintf(stderr, "[PRINT_TRACKER] No stat file to read\n");
    return;
  }
  while (fgets(buffer, MAX_LINE, rfptr) != NULL) {
    printf("%s", buffer);
  }
  fclose(rfptr);
}
