#define _GNU_SOURCE
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "user_list.h"
#define PATH "/data/user_list/"
#define FILENAME "ulist.log"
#define MAX_CRED_LEN 128
#define MAX_LIST_SIZE 4096
UserList *retrieve_user_list() {
  UserList *ulist = malloc(sizeof(UserList));
  ulist->users = NULL;
  ulist->count = 0;
  char *canon_path = canonicalize_file_name(PATH);
  if (canon_path == NULL) {
    fprintf(stderr, "There was an error canonicalizing the path\n");
    free(canon_path);
    canon_path = NULL;
    return ulist;
  } else {
    fprintf(stderr, "RETRIEVE_USER_LIST: Canon path exists\n");
  }
  char *file_list = calloc(1, strlen(canon_path) + strlen(FILENAME) + 2);
  snprintf(file_list, strlen(canon_path) + strlen(FILENAME) + 2, "%s/%s",
           canon_path, FILENAME);
  if (access(file_list, F_OK)) {
    FILE *fptr = fopen(file_list, "w");
    if (fptr == NULL) {
      fprintf(stderr, " There was a problem creating the file\n");
    }
    free(canon_path);
    canon_path = NULL;
    free(file_list);
    file_list = NULL;
    fclose(fptr);
    return ulist;
  } else {
    fprintf(stderr, "Retrieving names\n\n");
    FILE *rfptr = fopen(file_list, "r");
    if (rfptr == NULL) {
      fprintf(stderr, "There was a problem opening the file");
      free(file_list);
      file_list = NULL;
      free(canon_path);
      canon_path = NULL;
      return ulist;
    }
    char *buffer = calloc(1, MAX_CRED_LEN);
    int newline_count = 0;
    while (fgets(buffer, MAX_CRED_LEN, rfptr) != NULL) {
      newline_count++;
    }
    ulist->count = newline_count;
    free(buffer);
    buffer = NULL;
    rewind(rfptr);
    printf("Newline count: %d\n", newline_count);
    if (newline_count > 0) {
      User **guard = NULL;
      guard = realloc(ulist->users, newline_count * sizeof(User *));
      if (guard == NULL) {
        fprintf(stderr, "RETRIEVE_USER_LIST: Realloc error\n");
      } else {
        ulist->users = guard;
      }
      int i;
      for (i = 0; i < newline_count; ++i) {
        ulist->users[i] = malloc(sizeof(User));
        ulist->users[i]->history_list = NULL;
      }
      char *hold;
      char *buffer2 = calloc(1, MAX_CRED_LEN);
      int count = 0;
      while (fgets(buffer2, MAX_CRED_LEN, rfptr) != NULL &&
             count < newline_count) {
        hold = strtok(buffer2, "\t");
        ulist->users[count]->username = calloc(1, MAX_CRED_LEN);
        strncpy(ulist->users[count]->username, hold, MAX_CRED_LEN - 1);
        ulist->users[count]->username[MAX_CRED_LEN - 1] = '\0';
        hold = strtok(NULL, "\t");
        ulist->users[count]->password = calloc(1, MAX_CRED_LEN);
        strncpy(ulist->users[count]->password, hold, MAX_CRED_LEN - 1);
        ulist->users[count]->password[MAX_CRED_LEN - 1] = '\0';
        hold = strtok(NULL, "\n");
        ulist->users[count]->type = calloc(1, MAX_CRED_LEN);
        strncpy(ulist->users[count]->type, hold, MAX_CRED_LEN - 1);
        ulist->users[count]->type[MAX_CRED_LEN - 1] = '\0';
        count++;
      }
      free(buffer2);
      buffer2 = NULL;
    }
    fclose(rfptr);
    free(file_list);
    file_list = NULL;
    free(canon_path);
    canon_path = NULL;
    return ulist;
  }
}
void clear_list() {
  char *canon_path = canonicalize_file_name(PATH);
  char *file_list = calloc(1, strlen(canon_path) + strlen(FILENAME) + 2);
  snprintf(file_list, strlen(canon_path) + strlen(FILENAME) + 2, "%s/%s",
           canon_path, FILENAME);
  if (canon_path == NULL) {
    fprintf(stderr, "There was an error canonicalizing the path\n");
    free(file_list);
    file_list = NULL;
    free(canon_path);
    canon_path = NULL;
    return;
  } else {
    fprintf(stderr, "CLEAR_LIST: Canon Path exists\n");
  }
  if (!access(file_list, F_OK)) {
    remove(file_list);
    fprintf(stderr, "File removed\n");
  } else {
    fprintf(stderr, "The file does not exist\n");
  }
  free(canon_path);
  canon_path = NULL;
  free(file_list);
  file_list = NULL;
}
void add_user(UserList *list, char *uname, char *pass, char *type) {
  char *canon_path = canonicalize_file_name(PATH);
  if (canon_path == NULL) {
    fprintf(stderr, "There was an error canonicalizing the path\n");
    free(canon_path);
    canon_path = NULL;
    return;
  } else {
    fprintf(stderr, "ADD_USER: Path canonicalized\n");
  }
  char *file_list = calloc(1, strlen(canon_path) + strlen(FILENAME) + 2);
  snprintf(file_list, strlen(canon_path) + strlen(FILENAME) + 2, "%s/%s",
           canon_path, FILENAME);
  int i;
  for (i = 0; i < list->count; ++i) {
    if (strcmp(list->users[i]->username, uname) == 0 &&
        strcmp(list->users[i]->password, pass) == 0) {
      fprintf(stderr, "This username password combination already is taken\n");
      free(canon_path);
      canon_path = NULL;
      free(file_list);
      file_list = NULL;
      return;
    }
  }
  User **guard = NULL;
  guard = realloc(list->users, (list->count + 1) * sizeof(User *));
  if (guard == NULL) {
    fprintf(stderr, "USER_RETRIEVE_LIST: There was a realloc error\n");
  } else {
    list->users = guard;
  }
  list->users[list->count] = calloc(1, sizeof(User));
  list->users[list->count]->username = calloc(1, (MAX_CRED_LEN / 2) + 1);
  strncpy(list->users[list->count]->username, uname, (MAX_CRED_LEN / 2));
  list->users[list->count]->username[(MAX_CRED_LEN / 2)] = '\0';
  list->users[list->count]->password = calloc(1, (MAX_CRED_LEN / 2) + 1);
  strncpy(list->users[list->count]->password, pass, (MAX_CRED_LEN / 2));
  list->users[list->count]->password[(MAX_CRED_LEN / 2)] = '\0';
  list->users[list->count]->type = calloc(1, (MAX_CRED_LEN / 2) + 1);
  strncpy(list->users[list->count]->type, type, (MAX_CRED_LEN / 2));
  list->users[list->count]->type[(MAX_CRED_LEN / 2)] = '\0';
  list->count++;
  FILE *append_ptr = fopen(file_list, "a");
  if (append_ptr == NULL) {
    fprintf(stderr, "There was a problem opening the file for appending\n");
  } else {
    fwrite(list->users[list->count - 1]->username,
           strlen(list->users[list->count - 1]->username), 1, append_ptr);
    fwrite("\t", 1, 1, append_ptr);
    fwrite(list->users[list->count - 1]->password,
           strlen(list->users[list->count - 1]->password), 1, append_ptr);
    fwrite("\t", 1, 1, append_ptr);
    fwrite(list->users[list->count - 1]->type,
           strlen(list->users[list->count - 1]->type), 1, append_ptr);
    fwrite("\n", 1, 1, append_ptr);
  }
  fclose(append_ptr);
  free(canon_path);
  canon_path = NULL;
  free(file_list);
  file_list = NULL;
}
void edit_user(char *uname, char *change_to_string, char *edit_field,
               LineStateTable *lst, UserList *ulist) {
  char *canon_path = canonicalize_file_name(PATH);
  char *file_list = calloc(1, strlen(canon_path) + strlen(FILENAME) + 2);
  snprintf(file_list, strlen(canon_path) + strlen(FILENAME) + 2, "%s/%s",
           canon_path, FILENAME);
  if (canon_path == NULL) {
    fprintf(stderr, "There was an error canonicalizing the path\n");
    free(file_list);
    file_list = NULL;
    free(canon_path);
    canon_path = NULL;
    return;
  } else {
    FILE *fptr = fopen(file_list, "r");
    if (fptr == NULL) {
      fprintf(stderr, "EDIT_USER: There was an error opening the file\n");
      free(file_list);
      file_list = NULL;
      free(canon_path);
      canon_path = NULL;
      return;
    }
    char *buffer = calloc(1, MAX_CRED_LEN);
    int newline_count = 0;
    while (fgets(buffer, MAX_CRED_LEN - 1, fptr) != NULL) {
      newline_count++;
    }
    char **file_contents = calloc(newline_count, sizeof(char *));
    rewind(fptr);
    int i;
    for (i = 0; i < newline_count; ++i, fgets(buffer, MAX_CRED_LEN, fptr)) {
      file_contents[i] = calloc(1, MAX_CRED_LEN);
      strncpy(file_contents[i], buffer, MAX_CRED_LEN - 1);
      file_contents[i][MAX_CRED_LEN - 1] = '\0';
    }
    free(buffer);
    buffer = NULL;
    fclose(fptr);
    FILE *wfptr = fopen(file_list, "w");
    char *holder;
    for (i = 0; i < newline_count; ++i) {
      holder = strtok(file_contents[i], "\t");
      if (strcmp(uname, holder) == 0) {
        if (edit_field[0] == '1') {
          fwrite(change_to_string, strlen(change_to_string), 1, wfptr);
          fwrite("\t", 1, 1, wfptr);
          holder = strtok(NULL, "\t");
          fwrite(holder, strlen(holder), 1, wfptr);
          fwrite("\t", 1, 1, wfptr);
          holder = strtok(NULL, "\n");
          fwrite(holder, strlen(holder), 1, wfptr);
          fwrite("\n", 1, 1, wfptr);
        } else if (edit_field[0] == '2') {
          fwrite(holder, strlen(holder), 1, wfptr);
          fwrite("\t", 1, 1, wfptr);
          holder = strtok(NULL, "\t");
          fwrite(change_to_string, strlen(change_to_string), 1, wfptr);
          fwrite("\t", 1, 1, wfptr);
          holder = strtok(NULL, "\n");
          fwrite(holder, strlen(holder), 1, wfptr);
          fwrite("\n", 1, 1, wfptr);
        } else if (edit_field[0] == '3') {
          fwrite(holder, strlen(holder), 1, wfptr);
          fwrite("\t", 1, 1, wfptr);
          holder = strtok(NULL, "\t");
          fwrite(holder, strlen(holder), 1, wfptr);
          fwrite("\t", 1, 1, wfptr);
          holder = strtok(NULL, "\n");
          fwrite(change_to_string, strlen(change_to_string), 1, wfptr);
          fwrite("\n", 1, 1, wfptr);
        }
      } else {
        fwrite(holder, strlen(holder), 1, wfptr);
        fwrite("\t", 1, 1, wfptr);
        holder = strtok(NULL, "\n");
        fwrite(holder, strlen(holder), 1, wfptr);
        fwrite("\n", 1, 1, wfptr);
      }
    }
    for (i = 0; i < newline_count; ++i) {
      free(file_contents[i]);
      file_contents[i] = NULL;
    }
    free(file_contents);
    file_contents = NULL;
    free(canon_path);
    canon_path = NULL;
    free(file_list);
    file_list = NULL;
    fclose(wfptr);
  }
}
void delete_user(char *uname) {
  char *canon_path = canonicalize_file_name(PATH);
  char *file_list = calloc(1, strlen(canon_path) + strlen(FILENAME) + 2);
  snprintf(file_list, strlen(canon_path) + strlen(FILENAME) + 2, "%s/%s",
           canon_path, FILENAME);
  if (canon_path == NULL) {
    fprintf(stderr, "There was an error canonicalizing the path\n");
    free(file_list);
    file_list = NULL;
    free(canon_path);
    canon_path = NULL;
    return;
  } else {
    FILE *fptr = fopen(file_list, "r");
    if (fptr == NULL) {
      free(canon_path);
      canon_path = NULL;
      free(file_list);
      file_list = NULL;
      return;
    }
    char *buffer = calloc(1, MAX_CRED_LEN);
    int newline_count = 0;
    while (fgets(buffer, MAX_CRED_LEN - 1, fptr) != NULL) {
      newline_count++;
    }
    char **file_contents = calloc(newline_count, sizeof(char *));
    rewind(fptr);
    int i;
    for (i = 0; i < newline_count; ++i, fgets(buffer, MAX_CRED_LEN, fptr)) {
      file_contents[i] = calloc(1, MAX_CRED_LEN);
      strncpy(file_contents[i], buffer, MAX_CRED_LEN - 1);
      file_contents[i][MAX_CRED_LEN - 1] = '\0';
    }
    free(buffer);
    buffer = NULL;
    fclose(fptr);
    FILE *wfptr = fopen(file_list, "w");
    char *holder;
    for (i = 0; i < newline_count; ++i) {
      holder = strtok(file_contents[i], "\t");
      if (strcmp(uname, holder) == 0) {
        continue;
      } else {
        fwrite(holder, strlen(holder), 1, wfptr);
        fwrite("\t", 1, 1, wfptr);
        holder = strtok(NULL, "\n");
        fwrite(holder, strlen(holder), 1, wfptr);
        fwrite("\n", 1, 1, wfptr);
      }
    }
    for (i = 0; i < newline_count; ++i) {
      free(file_contents[i]);
      file_contents[i] = NULL;
    }
    free(file_contents);
    file_contents = NULL;
    free(canon_path);
    canon_path = NULL;
    free(file_list);
    file_list = NULL;
    fclose(wfptr);
  }
}
void delete_user_history(char *uname) {}
void destroy_user(User *user) {
  free(user->username);
  user->username = NULL;
  free(user->password);
  user->password = NULL;
  free(user->history_list);
  user->history_list = NULL;
  free(user->type);
  user->type = NULL;
  free(user);
  user = NULL;
}
void destroy_user_list(UserList *userlist) {
  int i;
  for (i = 0; i < userlist->count; ++i) {
    if (userlist->users[i] != NULL) {
      destroy_user(userlist->users[i]);
    }
  }
  free(userlist->users);
  userlist->users = NULL;
  free(userlist);
  userlist = NULL;
}
