#ifndef ROLE_TABLE_H
#define ROLE_TABLE_H
#include "header.h"
#include <stdbool.h>
typedef struct RoleEntry {
  char name[10];
  char number[2];
  char cookie[33];
  bool gen;
} RoleEntry;
typedef struct RoleTable { RoleEntry entries[3]; } RoleTable;
RoleTable *generate_rtable(RoleTable *rtable, Header header);
Header find_role_tag(Header *headers);
#endif
