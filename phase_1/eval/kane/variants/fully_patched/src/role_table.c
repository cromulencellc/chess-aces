#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "cookie_generation.h"
#include "role_table.h"
#define NUMBER_OF_ROLES 3
#define ADMIN " ADMIN"
#define USER " USER"
#define ANON " ANONYMOUS"
RoleTable *generate_rtable(RoleTable *rtable, Header header) {
  if (rtable == NULL) {
    fprintf(stderr, "GENERATE_RTABLE: You must allocated rtable outside of "
                    "this function\n");
    return rtable;
  }
  if (strncmp(header.value, ANON, strlen(ANON)) == 0) {
    rtable->entries[0].number[0] = '0';
    rtable->entries[0].number[1] = '\0';
    int i = 0;
    for (i = 0; i < 10; ++i) {
      rtable->entries[0].name[i] = header.value[i];
    }
    rtable->entries[0].name[i] = '\0';
  } else if (strncmp(header.value, USER, strlen(USER)) == 0) {
    rtable->entries[1].number[0] = '1';
    rtable->entries[1].number[1] = '\0';
    int i = 0;
    for (i = 0; i <= strlen(USER); ++i) {
      rtable->entries[1].name[i] = header.value[i];
    }
    rtable->entries[1].name[i] = '\0';
  } else if (strncmp(header.value, ADMIN, strlen(ADMIN)) == 0) {
    rtable->entries[2].number[0] = '2';
    rtable->entries[2].number[1] = '\0';
    int i = 0;
    for (i = 0; i <= strlen(ADMIN); ++i) {
      rtable->entries[2].name[i] = header.value[i];
    }
    rtable->entries[2].name[i] = '\0';
  }
  if (!rtable->entries[0].gen && !rtable->entries[0].gen &&
      !rtable->entries[0].gen) {
    int i = 0;
    for (i = 0; i < NUMBER_OF_ROLES; ++i) {
      sleep(1);
      strncpy(rtable->entries[i].cookie, generate_cookie(time(NULL)), 32);
      rtable->entries[i].cookie[32] = '\0';
      rtable->entries[i].gen = true;
    }
  }
  return rtable;
}
Header find_role_tag(Header *headers) {
  Header header;
  header.name = NULL;
  header.value = NULL;
  int i = 0;
  int max_header_count = 32;
  for (i = 0; i < max_header_count; ++i) {
    if (headers[i].name) {
      if (strcmp(headers[i].name, "Role") == 0) {
        header = headers[i];
        return header;
      }
    }
  }
  return header;
}
