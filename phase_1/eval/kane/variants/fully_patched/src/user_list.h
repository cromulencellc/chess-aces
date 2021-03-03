#ifndef USER_LIST_H
#define USER_LIST_H
#include "line_state_table.h"
typedef struct User {
  char *username;
  char *password;
  char **history_list;
  char *type;
} User;
typedef struct UserList {
  User **users;
  int count;
} UserList;
UserList *retrieve_user_list();
void clear_list();
void add_user(UserList *list, char *uname, char *pass, char *type);
void edit_user(char *uname, char *change_to_string, char *edit_field,
               LineStateTable *lst, UserList *ulist);
void delete_user(char *uname);
void delete_user_history(char *uname);
void destroy_user(User *user);
void destroy_user_list(UserList *userlist);
#endif
