#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CREATE_LIST "create_list"
#define EDIT_LIST "edit_list"
#define DELETE_LIST "delete_list"
#define CREATE_ENTRY "create_entry"
#define EDIT_ENTRY "edit_entry"
#define DELETE_ENTRY "delete_entry"
#define ENTRY_COMPLETE "entry_completed"
#define ENTRY_INCOMPLETE "entry_incomplete"
#define MAX_LIST_SIZE 256
#define MAX_PATH 2048
#define MAX_ENTRY_SIZE 512
#define MAX_COMMANDS 10
#include "disk_tracker.h"
#include "list.h"
#include "query_director.h"
#include "session.h"
#include "stat_tracker.h"
bool direct_single_query(Request *req, char *query) {
  char *holder = strtok(query, "=");
  if (holder == NULL) {
    fprintf(stderr, "Not able to tok\n");
    req->error_number = 1;
    return false;
  }
  holder = strtok(NULL, "&");
  if (holder == NULL) {
    fprintf(stderr, "Not able to tok\n");
    req->error_number = 1;
    return false;
  }
  if (strncmp(holder, CREATE_LIST, strlen(CREATE_LIST)) == 0) {
    holder = strtok(NULL, "&");
    if (holder == NULL) {
      fprintf(stderr, "[DQ_CREATE_LIST] There was aligning to listname\n");
      req->error_number = 1;
      return false;
    }
    if (strncmp(holder, "list_name", strlen("list_name")) == 0) {
      if ((holder = strchr(holder, '=')) == NULL) {
        fprintf(stderr, "[DQ_CREATE_LIST] Unable to align for list_name\n");
        req->error_number = 1;
        return false;
      }
      holder += 1;
      char *sid = create_no_mem_session();
      int check = add_to_disk(sid, "master", holder, NULL);
      req->error_number = check;
    } else {
      fprintf(stderr,
              "[DQ_CREATE_LIST] There was a problem with the list_name field");
      req->error_number = 1;
      return false;
    }
  } else if (strncmp(holder, EDIT_LIST, strlen(EDIT_LIST)) == 0) {
    holder = strtok(NULL, "&");
    if (holder == NULL) {
      fprintf(stderr, "[DQ_EDIT_LIST] Unable to align for sid\n");
      req->error_number = 1;
      return false;
    }
    if (strncmp(holder, "sid", strlen("sid")) == 0) {
      if ((holder = strchr(holder, '=')) == NULL) {
        fprintf(stderr, "[DQ_EDIT_LIST] Unable to get sid\n");
        req->error_number = 1;
        return false;
      }
      holder += 1;
      char *holder2 = strtok(NULL, "&");
      if (holder2 == NULL) {
        fprintf(stderr, "[DQ_EDIT_LIST] Unable to align for list_name\n");
        req->error_number = 1;
        return false;
      }
      if (strncmp(holder2, "list_name", strlen("list_name")) == 0) {
        if ((holder2 = strchr(holder2, '=')) == NULL) {
          fprintf(stderr, "[DQ_EDIT_LIST] Unable to get list_name\n");
          req->error_number = 1;
          return false;
        }
        holder2 += 1;
        char *holder3 = strtok(NULL, "&");
        if (holder3 == NULL) {
          fprintf(stderr, "[DQ_EDIT_LIST] Unable to align for change_name\n");
          req->error_number = 1;
          return false;
        }
        if ((holder3 = strchr(holder3, '=')) == NULL) {
          fprintf(stderr, "[DQ_EDIT_LIST] Unable to get change name\n");
          req->error_number = 1;
          return false;
        }
        holder3 += 1;
        char *datapath = calloc(1, MAX_PATH + 1);
        snprintf(datapath, MAX_PATH, "%s%s/", "/data/sessions/", holder);
        printf("SID: %s\n", holder);
        printf("ListName: %s\n", holder2);
        printf("Change: %s\n", holder3);
        int check = edit_disk_entry(datapath, "master", holder2, NULL, holder3);
        req->error_number = check;
      }
    }
  } else if (strncmp(holder, DELETE_LIST, strlen(DELETE_LIST)) == 0) {
    holder = strtok(NULL, "&");
    if (holder == NULL) {
      fprintf(stderr, "[DQ_EDIT_LIST] Unable to align for sid\n");
      req->error_number = 1;
      return false;
    }
    if (strncmp(holder, "sid", strlen("sid")) == 0) {
      if ((holder = strchr(holder, '=')) == NULL) {
        fprintf(stderr, "[DQ_EDIT_LIST] Unable to get sid\n");
        req->error_number = 1;
        return false;
      }
      holder += 1;
      char *holder2 = strtok(NULL, "&");
      if (holder2 == NULL) {
        fprintf(stderr, "[DQ_EDIT_LIST] Unable to align for list_name\n");
        req->error_number = 1;
        return false;
      }
      if (strncmp(holder2, "list_name", strlen("list_name")) == 0) {
        if ((holder2 = strchr(holder2, '=')) == NULL) {
          fprintf(stderr, "[DQ_EDIT_LIST] Unable to get list_name\n");
          req->error_number = 1;
          return false;
        }
        holder2 += 1;
        char *datapath = calloc(1, MAX_PATH + 1);
        snprintf(datapath, MAX_PATH, "%s%s/", "/data/sessions/", holder);
        printf("SID: %s\n", holder);
        printf("ListName: %s\n", holder2);
        int check = delete_disk_entry(datapath, "master", holder2, NULL);
        req->error_number = check;
      }
    }
  } else if (strncmp(holder, CREATE_ENTRY, strlen(CREATE_ENTRY)) == 0) {
    holder = strtok(NULL, "&");
    if (holder == NULL) {
      fprintf(stderr, "[DQ_CREATE_ENTRY] There was a problem alinging sid\n");
      req->error_number = 1;
      return false;
    }
    if (strncmp(holder, "sid", strlen("sid")) == 0) {
      if ((holder = strchr(holder, '=')) == NULL) {
        fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get sid\n");
        req->error_number = 1;
        return false;
      }
      holder += 1;
      char *holder2 = strtok(NULL, "&");
      if (holder2 == NULL) {
        fprintf(stderr,
                "[DQ_CREATE_ENTRY] There was a problem alinging list_name\n");
        req->error_number = 1;
        return false;
      }
      if (strncmp(holder2, "list_name", strlen("list_name")) == 0) {
        if ((holder2 = strchr(holder2, '=')) == NULL) {
          fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get list_name\n");
          req->error_number = 1;
          return false;
        }
        holder2 += 1;
        char *holder3 = strtok(NULL, "&");
        if (strncmp(holder3, "entry", strlen("entry")) == 0) {
          if ((holder3 = strchr(holder3, '=')) == NULL) {
            fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get change name\n");
            req->error_number = 1;
            return false;
          }
          holder3 += 1;
          char *datapath = calloc(1, MAX_PATH + 1);
          snprintf(datapath, MAX_PATH, "%s%s/", "/data/sessions/", holder);
          printf("Datapath: %s\n", datapath);
          printf("SID: %s\n", holder);
          printf("List name: %s\n", holder2);
          printf("Entry: %s\n", holder3);
          add_to_disk(datapath, "list", holder2, holder3);
          update_tracker("Task_Created");
          print_tracker();
        }
      }
    }
  } else if (strncmp(holder, EDIT_ENTRY, strlen(EDIT_ENTRY)) == 0) {
    holder = strtok(NULL, "&");
    if (holder == NULL) {
      fprintf(stderr, "[DQ_CREATE_ENTRY] There was a problem alinging sid\n");
      req->error_number = 1;
      return false;
    }
    if (strncmp(holder, "sid", strlen("sid")) == 0) {
      if ((holder = strchr(holder, '=')) == NULL) {
        fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get sid\n");
        req->error_number = 1;
        return false;
      }
      holder += 1;
      char *holder2 = strtok(NULL, "&");
      if (holder2 == NULL) {
        fprintf(stderr,
                "[DQ_CREATE_ENTRY] There was a problem alinging list_name\n");
        req->error_number = 1;
        return false;
      }
      if (strncmp(holder2, "list_name", strlen("list_name")) == 0) {
        if ((holder2 = strchr(holder2, '=')) == NULL) {
          fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get list_name\n");
          req->error_number = 1;
          return false;
        }
        holder2 += 1;
        char *holder3 = strtok(NULL, "&");
        if (strncmp(holder3, "entry", strlen("entry")) == 0) {
          if ((holder3 = strchr(holder3, '=')) == NULL) {
            fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get change name\n");
            req->error_number = 1;
            return false;
          }
          holder3 += 1;
          char *holder4 = strtok(NULL, "&");
          if (strncmp(holder4, "change", strlen("change")) == 0) {
            if ((holder4 = strchr(holder4, '=')) == NULL) {
              fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get change name\n");
              req->error_number = 1;
              return false;
            }
            holder4 += 1;
            char *datapath = calloc(1, MAX_PATH + 1);
            snprintf(datapath, MAX_PATH, "%s%s/", "/data/sessions/", holder);
            printf("Datapath: %s\n", datapath);
            printf("SID: %s\n", holder);
            printf("List name: %s\n", holder2);
            printf("Entry: %s\n", holder3);
            printf("Change: %s\n", holder4);
            edit_disk_entry(datapath, "list", holder2, holder3, holder4);
          }
        }
      }
    }
  } else if (strncmp(holder, DELETE_ENTRY, strlen(DELETE_ENTRY)) == 0) {
    holder = strtok(NULL, "&");
    if (holder == NULL) {
      fprintf(stderr, "[DQ_CREATE_ENTRY] There was a problem alinging sid\n");
      req->error_number = 1;
      return false;
    }
    if (strncmp(holder, "sid", strlen("sid")) == 0) {
      if ((holder = strchr(holder, '=')) == NULL) {
        fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get sid\n");
        req->error_number = 1;
        return false;
      }
      holder += 1;
      char *holder2 = strtok(NULL, "&");
      if (holder2 == NULL) {
        fprintf(stderr,
                "[DQ_CREATE_ENTRY] There was a problem alinging list_name\n");
        req->error_number = 1;
        return false;
      }
      if (strncmp(holder2, "list_name", strlen("list_name")) == 0) {
        if ((holder2 = strchr(holder2, '=')) == NULL) {
          fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get list_name\n");
          req->error_number = 1;
          return false;
        }
        holder2 += 1;
        char *holder3 = strtok(NULL, "&");
        if (strncmp(holder3, "entry", strlen("entry")) == 0) {
          if ((holder3 = strchr(holder3, '=')) == NULL) {
            fprintf(stderr, "[DQ_CREATE_ENTRY] Unable to get change name\n");
            req->error_number = 1;
            return false;
          }
          holder3 += 1;
          char *datapath = calloc(1, MAX_PATH + 1);
          snprintf(datapath, MAX_PATH, "%s%s/", "/data/sessions/", holder);
          printf("Datapath: %s\n", datapath);
          printf("SID: %s\n", holder);
          printf("List name: %s\n", holder2);
          printf("Entry: %s\n", holder3);
          delete_disk_entry(datapath, "list", holder2, holder3);
        }
      }
    }
  } else {
    fprintf(stderr, "[DIRECT_QUERY] Not correct query format\n");
    return false;
  }
  return true;
}
Session *direct_multi_query(Session *session, char *request) {
  printf("request[0]: %c\n", request[0]);
  if (request[0] != 'c' && request[0] != 'e' && request[0] != 'd') {
    printf("Invalid request\n");
    Session *session = NULL;
    return session;
  }
  if (session == NULL) {
    session = create_session();
  }
  char *req_cpy = strdup(request);
  char *holder = NULL;
  if ((holder = strtok(req_cpy, "\n")) == NULL) {
    session->valid = false;
    return session;
  }
  printf("Action: %s\n", holder);
  int i = 0;
  for (i = 0; i < MAX_COMMANDS && holder != NULL; ++i) {
    if (strncmp(holder, CREATE_LIST, strlen(CREATE_LIST)) == 0) {
      if ((holder = strtok(NULL, ":\n")) != NULL) {
        if (strncmp(holder, "list_name", strlen("list_name")) == 0) {
          if ((holder = strtok(NULL, "\n")) != NULL) {
          } else {
            fprintf(
                stderr,
                "[MULTI_QUERY] Create_list failed due to list_name field\n");
            session->valid = false;
            free(req_cpy);
            req_cpy = NULL;
            return session;
          }
          char *list_name;
          if ((list_name = strchr(holder, ' ')) != NULL) {
            list_name += 1;
            printf("List_Name: %s\n", list_name);
            List *list = create_list(session->path, list_name);
            add_to_master(session->mlist, list);
            print_master(session->mlist);
            if ((holder = strtok(NULL, "\n")) == NULL) {
              fprintf(stderr, "[MQ_CREATE_LIST] End of query\n");
              session->valid = false;
              free(req_cpy);
              req_cpy = NULL;
              return session;
            }
          } else {
            fprintf(
                stderr,
                "[MULTI_QUERY] Create_list failed due to list_name field\n");
            session->valid = false;
            free(req_cpy);
            req_cpy = NULL;
            return session;
          }
        } else {
          fprintf(stderr, "[MULTI_QUERY] Create_list failed due to inaccurate "
                          "formating provided\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
      } else {
        fprintf(stderr, "[MULTI_QUERY] Incorrect format\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
    } else if (strncmp(holder, EDIT_LIST, strlen(EDIT_LIST)) == 0) {
      char *list_name;
      if ((holder = strtok(NULL, ":\n")) != NULL) {
        if (strncmp(holder, "list_name", strlen("list_name")) == 0) {
          if ((holder = strtok(NULL, "\n")) != NULL) {
          } else {
            fprintf(
                stderr,
                "[MULTI_QUERY] Create_list failed due to list_name field\n");
            session->valid = false;
            free(req_cpy);
            req_cpy = NULL;
            return session;
          }
          if ((list_name = strchr(holder, ' ')) != NULL) {
            list_name += 1;
            printf("List_Name: %s\n", list_name);
          } else {
            fprintf(
                stderr,
                "[MULTI_QUERY] Create_list failed due to list_name field\n");
            session->valid = false;
            free(req_cpy);
            req_cpy = NULL;
            return session;
          }
          char *change = NULL;
          char *holder2 = strtok(NULL, "\n");
          if ((change = strchr(holder2, ' ')) == NULL) {
            fprintf(stderr, "[MQ_EDIT_LIST] Unable to get change name\n");
            session->valid = false;
            free(req_cpy);
            req_cpy = NULL;
            return session;
          }
          change += 1;
          printf("Change: %s\n", change);
          List *list;
          if ((list = find_list(session->mlist, list_name)) == NULL) {
            fprintf(stderr, "[MQ_EDIT_LIST] Unable to find list\n");
            session->valid = false;
            free(req_cpy);
            req_cpy = NULL;
            return session;
          }
          edit_list(list, change);
          print_master(session->mlist);
          if ((holder = strtok(NULL, "\n")) == NULL) {
            fprintf(stderr, "[MQ_EDIT_LIST] End of query\n");
            session->valid = false;
            free(req_cpy);
            req_cpy = NULL;
            return session;
          }
        }
      }
    } else if (strncmp(holder, DELETE_LIST, strlen(DELETE_LIST)) == 0) {
      if ((holder = strtok(NULL, ":\n")) != NULL) {
        if (strncmp(holder, "list_name", strlen("list_name")) == 0) {
          if ((holder = strtok(NULL, "\n")) != NULL) {
          } else {
            fprintf(
                stderr,
                "[MULTI_QUERY] Delete_list failed due to list_name field\n");
            session->valid = false;
            free(req_cpy);
            req_cpy = NULL;
            return session;
          }
          char *list_name;
          if ((list_name = strchr(holder, ' ')) != NULL) {
            list_name += 1;
            printf("List_Name: %s\n", list_name);
            delete_from_master(session->mlist, list_name);
            print_master(session->mlist);
            if ((holder = strtok(list_name, "\n")) == NULL) {
              fprintf(stderr, "[MQ_DELETE_LIST] End of query\n");
              session->valid = false;
              free(req_cpy);
              req_cpy = NULL;
              return session;
            }
          }
        }
      }
    } else if (strncmp(holder, CREATE_ENTRY, strlen(CREATE_ENTRY)) == 0) {
      holder = strtok(NULL, "\n");
      if (holder == NULL) {
        fprintf(stderr, "[MQ_CREATE_ENTRY] Failure to align to listname\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      if (strncmp(holder, "list_name", strlen("list_name")) == 0) {
        if ((holder = strchr(holder, ' ')) == NULL) {
          fprintf(stderr, "[MQ_CREATE_ENTRY] Unable to get list_name\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        holder += 1;
        printf("List name: %s\n", holder);
        char *holder2 = strtok(NULL, "\n");
        if (holder2 == NULL) {
          fprintf(stderr, "Failure to align entry\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        if ((holder2 = strchr(holder2, ' ')) == NULL) {
          fprintf(stderr, "[MQ_CREATE_ENTRY] Unable to get change name\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        holder2 += 1;
        printf("Entry: %s\n", holder2);
        List *list;
        if ((list = find_list(session->mlist, holder)) == NULL) {
          fprintf(stderr,
                  "[MQ_CREATE_ENTRY] Could not find list in masterlist\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        add_entry(list, holder2);
        print_list(list);
        update_tracker("Task_Created");
        print_tracker();
        if ((holder = strtok(NULL, "\n")) == NULL) {
          fprintf(stderr, "[MQ_CREATE_ENTRY] END OF FUNCTION ERROR\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
      }
    } else if (strncmp(holder, EDIT_ENTRY, strlen(EDIT_ENTRY)) == 0) {
      holder = strtok(NULL, "\n");
      if (holder == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Failure to align for listname\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      if (strncmp(holder, "list_name", strlen("list_name")) == 0) {
        if ((holder = strchr(holder, ' ')) == NULL) {
          fprintf(stderr, "[MQ_EDIT_LIST] Unable to get list_name\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        holder += 1;
        printf("List Name: %s\n", holder);
        char *holder2 = strtok(NULL, "\n");
        if (holder2 == NULL) {
          fprintf(stderr, "[MQ_EDIT_ENTRY] Failure to align for entry\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        if ((holder2 = strchr(holder2, ' ')) == NULL) {
          fprintf(stderr, "[MQ_EDIT_ENTRY] Unable to get change name\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        holder2 += 1;
        printf("Entry: %s\n", holder2);
        char *holder3 = strtok(NULL, "\n");
        if (holder3 == NULL) {
          fprintf(stderr, "[MQ_EDIT_ENTRY] Failure to align for change\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        if ((holder3 = strchr(holder3, ' ')) == NULL) {
          fprintf(stderr, "[MQ_EDIT_LIST] Unable to get change name\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        holder3 += 1;
        printf("Changing to: %s\n", holder3);
        List *list;
        if ((list = find_list(session->mlist, holder)) == NULL) {
          fprintf(stderr,
                  "[MQ_EDIT_ENTRY] Could not find list in masterlist\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        Entry *entry;
        if ((entry = find_entry(list, holder2)) == NULL) {
          fprintf(stderr, "[MQ_EDIT_ENTRY] Could not find entry in list\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        edit_entry_task(entry, holder3);
        print_list(list);
        if ((holder = strtok(NULL, "\n")) == NULL) {
          fprintf(stderr, "[MQ_EDIT_ENTRY] End of query\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
      }
    } else if (strncmp(holder, DELETE_ENTRY, strlen(DELETE_ENTRY)) == 0) {
      holder = strtok(NULL, "\n");
      if (holder == NULL) {
        fprintf(stderr, "[MQ_DELETE_ENTRY] Failure to align for listname\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      if (strncmp(holder, "list_name", strlen("list_name")) == 0) {
        if ((holder = strchr(holder, ' ')) == NULL) {
          fprintf(stderr, "[MQ_DELETE_ENTRY] Unable to get list_name\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        holder += 1;
        printf("List Name: %s\n", holder);
        char *holder2 = strtok(NULL, "\n");
        if (holder2 == NULL) {
          fprintf(stderr, "[MQ_DELETE_ENTRY] Failure to align for entry\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        if ((holder2 = strchr(holder2, ' ')) == NULL) {
          fprintf(stderr, "[MQ_DELETE_ENTRY] Unable to get entry to delete\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
        holder2 += 1;
        printf("Entry Delete: %s\n", holder2);
        List *list;
        if ((list = find_list(session->mlist, holder)) == NULL) {
          fprintf(stderr,
                  "[MQ_DELETE_ENTRY] Could not find list in masterlist\n");
          session->valid = false;
          return session;
        }
        delete_list_entry(list, holder2);
        print_list(list);
      }
    } else if (strncmp(holder, ENTRY_COMPLETE, strlen(ENTRY_COMPLETE)) == 0) {
      holder = strtok(NULL, "\n");
      if (holder == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Failure to align for listname\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      if (strncmp(holder, "list_name", strlen("list_name")) == 0) {
        if ((holder = strchr(holder, ' ')) == NULL) {
          fprintf(stderr, "[MQ_EDIT_LIST] Unable to get list_name\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
      }
      holder += 1;
      printf("List Name: %s\n", holder);
      char *holder2 = strtok(NULL, "\n");
      if (holder2 == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Failure to align for entry\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      if ((holder2 = strchr(holder2, ' ')) == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Unable to get change name\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      holder2 += 1;
      printf("Entry: %s\n", holder2);
      List *list;
      if ((list = find_list(session->mlist, holder)) == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Could not find list in masterlist\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      Entry *entry;
      if ((entry = find_entry(list, holder2)) == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Could not find entry in list\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      set_to_complete(entry);
      update_tracker("Task_Completed");
      print_tracker();
    } else if (strncmp(holder, ENTRY_INCOMPLETE, strlen(ENTRY_INCOMPLETE)) ==
               0) {
      holder = strtok(NULL, "\n");
      if (holder == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Failure to align for listname\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      if (strncmp(holder, "list_name", strlen("list_name")) == 0) {
        if ((holder = strchr(holder, ' ')) == NULL) {
          fprintf(stderr, "[MQ_EDIT_LIST] Unable to get list_name\n");
          session->valid = false;
          free(req_cpy);
          req_cpy = NULL;
          return session;
        }
      }
      holder += 1;
      printf("List Name: %s\n", holder);
      char *holder2 = strtok(NULL, "\n");
      if (holder2 == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Failure to align for entry\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      if ((holder2 = strchr(holder2, ' ')) == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Unable to get change name\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      holder2 += 1;
      printf("Entry: %s\n", holder2);
      List *list;
      if ((list = find_list(session->mlist, holder)) == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Could not find list in masterlist\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      Entry *entry;
      if ((entry = find_entry(list, holder2)) == NULL) {
        fprintf(stderr, "[MQ_EDIT_ENTRY] Could not find entry in list\n");
        session->valid = false;
        free(req_cpy);
        req_cpy = NULL;
        return session;
      }
      set_to_complete(entry);
      update_tracker("Task_Completed");
      print_tracker();
    } else {
      fprintf(stderr,
              "[MQ_DIRECT] Either Invalid Query or query is finished...\n");
      free(req_cpy);
      req_cpy = NULL;
      return session;
    }
  }
  free(req_cpy);
  req_cpy = NULL;
  return session;
}
