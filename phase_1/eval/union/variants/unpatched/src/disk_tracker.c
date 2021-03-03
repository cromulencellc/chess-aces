#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define DATA "/data/"
#define SESSION "sessions/"
#define MASTERLIST_PATH "/masterlist/mlist.log"
#define MASTER_TEMP_PATH "/masterlist/.tmp.txt"
#define LIST_PATH "/lists/"
#define TMP_LIST_PATH "/lists/.tmp.txt"
#define MASTER_TYPE "master"
#define LIST_TYPE "list"
#define MAX_PATH 2048
#define MAX_ENTRY 512
#include "disk_tracker.h"
int add_to_disk(char *spath, char *list_type, char *list_name,
                char *add_field) {
  if (spath == NULL) {
    fprintf(stderr, "[ADD_TO_DISK] No session path given\n");
    return 1;
  }
  if (list_type == NULL) {
    fprintf(stderr, "[ADD_TO_DISK] List type given empty\n");
    return 1;
  }
  if (list_name == NULL) {
    fprintf(stderr, "[ADD_TO_DISK] Listname empty\n");
    return 1;
  }
  char *filepath = calloc(1, MAX_PATH + 1);
  printf("Filepath: %s\n", spath);
  if (strncmp(list_type, MASTER_TYPE, strlen(MASTER_TYPE)) == 0) {
    FILE *fptr, *lptr;
    char *mpath = calloc(1, MAX_PATH + 1);
    snprintf(mpath, MAX_PATH, "%s%s", spath, MASTERLIST_PATH);
    char *lpath = calloc(1, MAX_PATH + 1);
    snprintf(lpath, MAX_PATH, "%s%s%s.txt", spath, LIST_PATH, list_name);
    if ((fptr = fopen(mpath, "a")) != NULL) {
      if ((lptr = fopen(lpath, "w")) != NULL) {
        fwrite(list_name, strlen(list_name), 1, fptr);
        fwrite("\n", 1, 1, fptr);
        fclose(lptr);
        fclose(fptr);
      } else {
        fprintf(stderr, "[ADD_TO_DISK] Failed to create list file\n");
        fclose(fptr);
        free(filepath);
        filepath = NULL;
        return 4;
      }
    } else {
      fprintf(stderr, "[ADD_TO_DISK] Failed to open master list\n");
      free(filepath);
      filepath = NULL;
      return 4;
    }
  } else if (strncmp(list_type, LIST_TYPE, strlen(LIST_TYPE)) == 0) {
    snprintf(filepath, MAX_PATH, "%s%s%s.txt", spath, LIST_PATH, list_name);
    printf("fp: %s\n", filepath);
    if (access(filepath, F_OK) != 0) {
      fprintf(stderr, "[ADD_TO_DISK] The list you are trying to add entry to "
                      "does not exist\n");
      free(filepath);
      filepath = NULL;
      return 4;
    }
    FILE *fptr;
    if ((fptr = fopen(filepath, "a")) != NULL) {
      fwrite(add_field, strlen(add_field), 1, fptr);
      fwrite("\n", strlen("\n"), 1, fptr);
      fclose(fptr);
    } else {
      fprintf(stderr, "[ADD_TO_DISK] Failed to open list file\n");
      fclose(fptr);
      free(filepath);
      filepath = NULL;
      return 4;
    }
  } else {
    fprintf(stderr, "[ADD_TO_DISK] List type provided invalid\n");
    free(filepath);
    filepath = NULL;
    return 1;
  }
  return 0;
}
int edit_disk_entry(char *spath, char *list_type, char *list_name, char *entry,
                    char *change) {
  if (strncmp(list_type, MASTER_TYPE, strlen(MASTER_TYPE)) == 0) {
    FILE *fptr, *tmp;
    char *mpath = calloc(1, MAX_PATH + 1);
    snprintf(mpath, MAX_PATH, "%s%s", spath, MASTERLIST_PATH);
    char *cpath = calloc(1, MAX_PATH + 1);
    snprintf(cpath, MAX_PATH, "%s%s", spath, MASTER_TEMP_PATH);
    if ((fptr = fopen(mpath, "r")) != NULL) {
    } else {
      fprintf(stderr, "[EDIT_TO_DISK] Failed to open master\n");
      free(mpath);
      mpath = NULL;
      free(cpath);
      cpath = NULL;
      return 4;
    }
    if ((tmp = fopen(cpath, "w")) != NULL) {
    } else {
      fprintf(stderr,
              "[EDIT_TO_DISK] Failed to open /data/master_list/.tmp.txt\n");
      free(mpath);
      mpath = NULL;
      free(cpath);
      cpath = NULL;
      fclose(fptr);
      return 1;
    }
    char *holder = calloc(1, MAX_ENTRY + 1);
    while (fgets(holder, MAX_ENTRY, fptr) != NULL) {
      holder[strlen(holder) - 1] = '\0';
      if (strncmp(holder, list_name, MAX_ENTRY) == 0) {
        fwrite(change, strlen(change), 1, tmp);
        fwrite("\n", 1, 1, tmp);
      } else {
        fwrite(holder, strlen(holder), 1, tmp);
        fwrite("\n", 1, 1, tmp);
      }
    }
    fclose(fptr);
    fclose(tmp);
    free(holder);
    holder = NULL;
    int rem_check = -1;
    if ((rem_check = remove(mpath)) != 0) {
      fprintf(stderr,
              "[EDIT_TO_DISK] There was trouble removing the old file\n");
      return 1;
    }
    int ren_check = -1;
    if ((ren_check = rename(cpath, mpath)) != 0) {
      fprintf(stderr,
              "[EDIT_TO_DISK] There was trouble copying over the old file\n");
      return 1;
    }
    char *filepath = calloc(1, MAX_PATH + 1);
    snprintf(filepath, MAX_PATH, "%s%s%s.txt", spath, LIST_PATH, list_name);
    char *new_name = calloc(1, MAX_PATH + 1);
    snprintf(new_name, MAX_PATH, "%s%s%s.txt", spath, LIST_PATH, change);
    FILE *fptr2, *new_list;
    if ((fptr2 = fopen(filepath, "r")) != NULL) {
    } else {
      fprintf(stderr, "[EDIT_TO_DISK] Failed to open list file\n");
      free(filepath);
      filepath = NULL;
      free(new_name);
      new_name = NULL;
      return 4;
    }
    if ((new_list = fopen(new_name, "w")) != NULL) {
    } else {
      fprintf(stderr, "[EDIT_TO_DISK] Failed to open change");
      free(filepath);
      filepath = NULL;
      free(new_name);
      new_name = NULL;
      fclose(fptr2);
      return 4;
    }
    char *holder2 = calloc(1, MAX_ENTRY + 1);
    while (fgets(holder2, MAX_ENTRY, fptr2) != NULL) {
      holder2[strlen(holder2) - 1] = '\0';
      fwrite(holder2, strlen(holder2), 1, new_list);
      fwrite("\n", 1, 1, new_list);
    }
    free(holder2);
    holder2 = NULL;
    remove(filepath);
    fclose(fptr2);
    fclose(new_list);
    return 0;
  } else if (strncmp(list_type, LIST_TYPE, strlen(LIST_TYPE)) == 0) {
    FILE *fptr, *tmp;
    char *filepath = calloc(1, MAX_PATH + 1);
    snprintf(filepath, MAX_PATH, "%s%s%s.txt", spath, LIST_PATH, list_name);
    char *tmp_path = calloc(1, MAX_PATH + 1);
    snprintf(tmp_path, MAX_PATH, "%s%s", spath, TMP_LIST_PATH);
    if ((fptr = fopen(filepath, "r")) != NULL) {
    } else {
      fprintf(stderr, "[EDIT_TO_DISK] Failed to open %s.txt\n", list_name);
      free(filepath);
      filepath = NULL;
      free(tmp_path);
      tmp_path = NULL;
      return 4;
    }
    if ((tmp = fopen(tmp_path, "w")) != NULL) {
    } else {
      fprintf(stderr, "[EDIT_TO_DISK] Failed to open /data/list/tmp.txt\n");
      free(filepath);
      filepath = NULL;
      free(tmp_path);
      tmp_path = NULL;
      fclose(fptr);
      return 4;
    }
    char *holder = calloc(1, MAX_ENTRY + 1);
    while (fgets(holder, MAX_ENTRY, fptr) != NULL) {
      holder[strlen(holder) - 1] = '\0';
      printf("hOlDeR: %s\n", holder);
      if (strncmp(holder, entry, MAX_ENTRY) == 0) {
        fwrite(change, strlen(change), 1, tmp);
        fwrite("\n", 1, 1, tmp);
      } else {
        fwrite(holder, strlen(holder), 1, tmp);
        fwrite("\n", 1, 1, tmp);
      }
    }
    fclose(fptr);
    fclose(tmp);
    remove(filepath);
    rename(tmp_path, filepath);
    free(filepath);
    filepath = NULL;
    free(tmp_path);
    tmp_path = NULL;
  } else {
    fprintf(stderr, "[EDIT_TO_DISK] Failed to open list file\n");
    return 1;
  }
  return 0;
}
int delete_disk_entry(char *spath, char *list_type, char *list_name,
                      char *entry) {
  if (strncmp(list_type, MASTER_TYPE, strlen(MASTER_TYPE)) == 0) {
    FILE *mptr, *mtmp;
    char *mpath = calloc(1, MAX_PATH + 1);
    snprintf(mpath, MAX_PATH, "%s%s", spath, MASTERLIST_PATH);
    char *cpath = calloc(1, MAX_PATH + 1);
    snprintf(cpath, MAX_PATH, "%s%s", spath, MASTER_TEMP_PATH);
    if ((mptr = fopen(mpath, "r")) != NULL) {
    } else {
      fprintf(stderr, "[DELETE_DISK] Failed to open master\n");
      return 4;
    }
    if ((mtmp = fopen(cpath, "w")) != NULL) {
    } else {
      fprintf(stderr,
              "[DELETE_DISK] Failed to open /data/master_list/.tmp.txt\n");
      free(mpath);
      mpath = NULL;
      free(cpath);
      cpath = NULL;
      fclose(mptr);
      return 1;
    }
    char *holder = calloc(1, MAX_ENTRY + 1);
    while (fgets(holder, MAX_ENTRY, mptr) != NULL) {
      holder[strlen(holder) - 1] = '\0';
      if (strncmp(holder, list_name, MAX_ENTRY) == 0) {
        continue;
      } else {
        fwrite(holder, strlen(holder), 1, mtmp);
        fwrite("\n", 1, 1, mtmp);
      }
    }
    fclose(mptr);
    fclose(mtmp);
    free(holder);
    holder = NULL;
    int rem_check = -1;
    if ((rem_check = remove(mpath)) != 0) {
      fprintf(stderr,
              "[DELETE_TO_DISK] There was trouble removing the old file\n");
      return 4;
    }
    int ren_check = -1;
    if ((ren_check = rename(cpath, mpath)) != 0) {
      fprintf(stderr,
              "[DELETE_TO_DISK] There was trouble copying over the old file\n");
      return 4;
    }
    char *filepath = calloc(1, MAX_PATH + 1);
    snprintf(filepath, MAX_PATH, "%s%s%s.txt", spath, LIST_PATH, list_name);
    int r_check = -1;
    if ((r_check = remove(filepath)) != 0) {
      fprintf(stderr,
              "[DELETE_TO_DISK] There was trouble removing the list file\n");
      free(filepath);
      filepath = NULL;
      return 4;
    }
    free(filepath);
    filepath = NULL;
    return 0;
  } else if (strncmp(list_type, LIST_TYPE, strlen(LIST_TYPE)) == 0) {
    FILE *lptr, *ltmp;
    char *filepath = calloc(1, MAX_PATH + 1);
    snprintf(filepath, MAX_PATH, "%s%s%s.txt", spath, LIST_PATH, list_name);
    char *tmp_path = calloc(1, MAX_PATH + 1);
    snprintf(tmp_path, MAX_PATH, "%s%s", spath, TMP_LIST_PATH);
    if ((lptr = fopen(filepath, "r")) != NULL) {
    } else {
      fprintf(stderr, "[DELETE_DISK] Failed to open master\n");
      free(filepath);
      filepath = NULL;
      return 4;
    }
    if ((ltmp = fopen(tmp_path, "w")) != NULL) {
    } else {
      fprintf(stderr,
              "[DELETE_DISK] Failed to open /data/master_list/.tmp.txt\n");
      free(filepath);
      filepath = NULL;
      free(tmp_path);
      filepath = NULL;
      fclose(lptr);
      return 4;
    }
    char *holder = calloc(1, MAX_ENTRY + 1);
    while (fgets(holder, MAX_ENTRY, lptr) != NULL) {
      holder[strlen(holder) - 1] = '\0';
      if (strncmp(holder, entry, MAX_ENTRY) == 0) {
        printf("Skipping deleted entry\n");
        continue;
      } else {
        fwrite(holder, strlen(holder), 1, ltmp);
        fwrite("\n", 1, 1, ltmp);
      }
    }
    fclose(lptr);
    lptr = NULL;
    fclose(ltmp);
    ltmp = NULL;
    free(holder);
    holder = NULL;
    int rem_check = -1;
    if ((rem_check = remove(filepath)) != 0) {
      fprintf(stderr,
              "[DELETE_DISK] There was trouble removing the old file\n");
      return 4;
    }
    int ren_check = -1;
    if ((ren_check = rename(tmp_path, filepath)) != 0) {
      fprintf(stderr,
              "[DELETE_DISK] There was trouble copying over the old file\n");
      return 4;
    }
    free(filepath);
    filepath = NULL;
    free(tmp_path);
    tmp_path = NULL;
  }
  return 0;
}
