#ifndef SESSION_H
#define SESSION_H
#include <stdbool.h>
#include "masterlist.h"
typedef struct Session {
  int id;
  char *path;
  MasterList *mlist;
  bool valid;
  int error_number;
} Session;
Session *create_session();
char *create_no_mem_session();
void write_session(Session *session);
Session *load_session(char *session_name);
int get_session_amount();
void delete_session();
void destroy_session();
#endif
