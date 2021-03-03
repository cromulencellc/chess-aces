#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "masterlist.h"
#include "session.h"
#include "stat_tracker.h"
#define SESSION_PATH "/data/sessions/"
#define MASTERLIST "masterlist"
#define MASTERLIST_PATH "/masterlist/mlist.log"
#define LIST_PATH "/lists/"
#define LISTS "lists"
#define SESSION_FORMAT "sid_"
#define MAX_PATH 512
#define MAX_LIST_NAME 64
#define MAX_ENTRY 512
int static session_amount = 0;
Session *create_session() {
  Session *session = calloc(1, sizeof(Session));
  session->id = session_amount;
  session->path = calloc(1, MAX_PATH + 1);
  snprintf(session->path, MAX_PATH, "%s%s%d", SESSION_PATH, SESSION_FORMAT,
           session->id);
  session->valid = true;
  int check = 1;
  if ((check = mkdir(session->path, 0110)) == -1) {
    fprintf(stderr, "[CREATE_SESSION]Unable to create session on disk\n");
    free(session->path);
    session->path = NULL;
    free(session);
    session = NULL;
    return session;
  }
  int mcheck = 1, lcheck = 1;
  char *mpath, *lpath;
  mpath = calloc(1, MAX_PATH + 1);
  snprintf(mpath, MAX_PATH, "%s/%s", session->path, MASTERLIST);
  lpath = calloc(1, MAX_PATH + 1);
  snprintf(lpath, MAX_PATH, "%s/%s", session->path, LISTS);
  if ((mcheck = mkdir(mpath, 0110)) == -1) {
    fprintf(stderr,
            "[CREATE_SESSION] Unable to create masterlist path in session ");
    free(mpath);
    mpath = NULL;
    free(lpath);
    lpath = NULL;
    free(session->path);
    session->path = NULL;
    free(session);
    session = NULL;
    return session;
  }
  if ((lcheck = mkdir(lpath, 0110)) == -1) {
    fprintf(stderr, "[CREATE_SESSION] Unable to create list path in session");
    free(mpath);
    mpath = NULL;
    free(lpath);
    lpath = NULL;
    free(session->path);
    session->path = NULL;
    free(session);
    session = NULL;
    return session;
  }
  session->mlist = create_mlist(session->path);
  session_amount++;
  return session;
}
char *create_no_mem_session() {
  char *path = calloc(1, MAX_PATH + 1);
  snprintf(path, MAX_PATH, "%s%s%d", SESSION_PATH, SESSION_FORMAT,
           session_amount);
  int check = 1;
  if ((check = mkdir(path, 0110)) == -1) {
    fprintf(stderr, "[CREATE_SESSION]Unable to create session on disk\n");
    free(path);
    path = NULL;
    return NULL;
  }
  int mcheck = 1, lcheck = 1;
  char *mpath, *lpath;
  mpath = calloc(1, MAX_PATH + 1);
  snprintf(mpath, MAX_PATH, "%s/%s", path, MASTERLIST);
  lpath = calloc(1, MAX_PATH + 1);
  snprintf(lpath, MAX_PATH, "%s/%s", path, LISTS);
  if ((mcheck = mkdir(mpath, 0110)) == -1) {
    fprintf(stderr,
            "[CREATE_SESSION] Unable to create masterlist path in session ");
    free(mpath);
    mpath = NULL;
    free(lpath);
    lpath = NULL;
    free(path);
    path = NULL;
    return NULL;
  }
  if ((lcheck = mkdir(lpath, 0110)) == -1) {
    fprintf(stderr, "[CREATE_SESSION] Unable to create list path in session");
    free(mpath);
    mpath = NULL;
    free(lpath);
    lpath = NULL;
    free(path);
    path = NULL;
    return NULL;
  }
  session_amount++;
  MasterList *mlist = create_mlist(path);
  destroy_mlist(mlist);
  update_tracker("Session_Created");
  print_tracker();
  if (mpath != NULL) {
    free(mpath);
    mpath = NULL;
  }
  if (lpath != NULL) {
    free(lpath);
    lpath = NULL;
  }
  return path;
}
void write_session(Session *session) {
  if (session == NULL) {
    fprintf(stderr, "[WRITE_SESSION] Session passed is empty\n");
    return;
  }
  if (session->path == NULL) {
    fprintf(stderr, "[WRITE_SESSION] Session path is empty\n");
    return;
  }
  if (session->mlist == NULL) {
    fprintf(stderr, "[WRITE_SESSION] Session mlist is not allocated\n");
    return;
  }
  FILE *mptr, *lptr;
  char *path = calloc(1, MAX_PATH + 1);
  char *lpath = calloc(1, MAX_PATH + 1);
  snprintf(path, MAX_PATH, "%s%s", session->path, MASTERLIST_PATH);
  if ((mptr = fopen(path, "w")) == NULL) {
    fprintf(stderr, "[WRITE_SESSION] Unable to open session dir\n");
    free(path);
    path = NULL;
    return;
  }
  Node *traveler = session->mlist->lists->head;
  List *lhold = NULL;
  Entry *ehold = NULL;
  if (traveler == NULL) {
    fprintf(stderr, "[WRITE_SESSION] Session mlist is empty\n");
    free(path);
    path = NULL;
    return;
  }
  while (traveler != NULL) {
    if (traveler->data != NULL) {
      lhold = traveler->data;
      snprintf(lpath, MAX_PATH, "%s%s%s.txt", session->path, LIST_PATH,
               lhold->name);
      fwrite(lhold->name, strlen(lhold->name), 1, mptr);
      fwrite("\n", 1, 1, mptr);
      Node *subnode = lhold->entries->head;
      if ((lptr = fopen(lpath, "w")) == NULL) {
        fprintf(stderr, "[WRITE_SESSION] Unable to write %s\n", lhold->name);
      }
      if (subnode == NULL) {
        fprintf(stderr, "[WRITE_SESSION] There was nothing in this list\n");
        traveler = traveler->next;
        continue;
      } else {
        while (subnode != NULL) {
          if (subnode->data != NULL) {
            ehold = subnode->data;
            printf("Entry: %s\n", ehold->task);
            fwrite(ehold->task, strlen(ehold->task), 1, lptr);
            fwrite("\n", 1, 1, lptr);
          } else {
            fprintf(stderr,
                    "[WRITE_SESSION] There were not enteries in data field\n");
            break;
          }
          subnode = subnode->next;
        }
        fclose(lptr);
        printf("----------------\n\n");
      }
    }
    traveler = traveler->next;
  }
  free(path);
  path = NULL;
  fclose(mptr);
}
Session *load_session(char *session_name) {
  if (session_name == NULL) {
    fprintf(stderr, "[LOAD_SESSION] The session name provided was empty\n");
    return NULL;
  }
  char *path = calloc(1, MAX_PATH + 1);
  snprintf(path, MAX_PATH, "%s%s", SESSION_PATH, session_name);
  if (access(path, F_OK) != 0) {
    fprintf(stderr, "[LOAD_SESSION] The session that you are trying to load "
                    "was not found\n");
    return NULL;
  }
  char *mpath = calloc(1, MAX_PATH + 1);
  snprintf(mpath, MAX_PATH, "%s%s", path, MASTERLIST_PATH);
  printf("MPATH: %s\n", mpath);
  FILE *mptr;
  if ((mptr = fopen(mpath, "r")) == NULL) {
    fprintf(stderr, "[LOAD_SESSION] Was unable to load session's masterlist\n");
    free(path);
    path = NULL;
    free(mpath);
    mpath = NULL;
    return NULL;
  }
  Session *session = calloc(1, sizeof(Session));
  session->path = calloc(1, MAX_PATH + 1);
  strncpy(session->path, path, MAX_PATH);
  free(path);
  path = NULL;
  session->mlist = create_mlist(session->path);
  char *holder = calloc(1, MAX_LIST_NAME + 1);
  while (fgets(holder, MAX_LIST_NAME, mptr) != NULL) {
    holder[strlen(holder) - 1] = '\0';
    List *l = create_list(session->path, holder);
    add_to_master(session->mlist, l);
  }
  free(holder);
  holder = NULL;
  int i = 0;
  Node *traveler = session->mlist->lists->head;
  if (traveler == NULL) {
    fprintf(stderr, "[LOAD_SESSION] There are no lists in master\n");
    fclose(mptr);
    return NULL;
  }
  char *lpath = calloc(1, MAX_PATH + 1);
  FILE *lptr;
  for (i = 0; i < session->mlist->lists->node_count; ++i) {
    if (traveler->data != NULL) {
      List *tmp = traveler->data;
      snprintf(lpath, MAX_PATH, "%s%s%s.txt", session->path, LIST_PATH,
               tmp->name);
      printf("LPATH: %s\n", lpath);
      if ((lptr = fopen(lpath, "r")) == NULL) {
        fprintf(stderr, "[LOAD_SESSION] Unable to open list file\n");
        return session;
      }
      char *holder = calloc(1, MAX_ENTRY + 1);
      while (fgets(holder, MAX_ENTRY, lptr) != NULL) {
        add_entry(tmp, holder);
      }
    }
  }
  return session;
}
void delete_session(Session *session, int id) {
  destroy_session(session);
  int check = 2;
  if ((check = remove(session->path)) != 0) {
    fprintf(stderr, "[DELETE_SESSION] Cannot find session in directory\n");
    return;
  }
}
void destroy_session(Session *session) {
  if (session == NULL) {
    fprintf(stderr, "[DESTROY_SESSION] Session already null\n");
    return;
  }
  if (session->path != NULL) {
    free(session->path);
    session->path = NULL;
  }
  if (session->mlist != NULL) {
    destroy_mlist(session->mlist);
  }
  if (session != NULL) {
    free(session);
    session = NULL;
  }
}
