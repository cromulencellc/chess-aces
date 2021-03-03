#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "html_generation.h"
#define SESSION_PATH "/data/sessions/"
#define SESSION_FORMAT "sid_"
#define MAX_PATH 2048
void generate_html(Session *session) {
  if (session == NULL) {
    fprintf(stderr, "[GEN_HTML] Session is empty\n");
    return;
  }
  if (session->path == NULL) {
    fprintf(stderr, "[GEN_HTML] No path in session struct");
    return;
  }
  if (session->mlist == NULL) {
    fprintf(stderr, "[GEN_HTML] Session mlist is empty");
    return;
  }
  if (session->mlist->lists == NULL) {
    fprintf(stderr, "[GEN_HTML] Session mlist's lists linked list is empty");
    return;
  }
  char *path = calloc(1, MAX_PATH + 1);
  snprintf(path, MAX_PATH, "%s%s%d.html", SESSION_PATH, SESSION_FORMAT,
           session->id);
  FILE *hptr;
  if ((hptr = fopen(path, "w")) == NULL) {
    fprintf(stderr, "[GEN_HTML] Unable to create html file\n");
    free(path);
    path = NULL;
    return;
  }
  char *sid = calloc(1, MAX_PATH + 1);
  snprintf(sid, MAX_PATH, "%s%d", SESSION_FORMAT, session->id);
  fwrite("<!DOCTYPE html>\n", strlen("<!DOCTYPE html>\n"), 1, hptr);
  fwrite("<html>\n", strlen("<html>\n"), 1, hptr);
  fwrite("\t<title>Session</title>\n", strlen("\t<title>Session</title>\n"), 1,
         hptr);
  fwrite("\t<h1>", strlen("\t<h1>"), 1, hptr);
  fwrite(sid, strlen(sid), 1, hptr);
  fwrite("</h1>\n", strlen("</h1>\n"), 1, hptr);
  fwrite("\t<ul>\n", strlen("\t<ul>\n"), 1, hptr);
  Node *traveler = session->mlist->lists->head;
  if (traveler == NULL) {
    fprintf(stderr, "[GEN_HTML] There is nothing to create\n");
    free(path);
    path = NULL;
    fclose(hptr);
    return;
  }
  List *lhold = NULL;
  Entry *ehold = NULL;
  while (traveler != NULL) {
    if (traveler->data != NULL) {
      lhold = traveler->data;
      printf("\n----LISTNAME: %s----\n", lhold->name);
      fwrite("\t\t<li>", strlen("\t\t<li>"), 1, hptr);
      fwrite(lhold->name, strlen(lhold->name), 1, hptr);
      fwrite("</li>\n", strlen("</li>\n"), 1, hptr);
      fwrite("\t\t\t\t<ul>\n", strlen("\t\t\t\t<ul>\n"), 1, hptr);
      Node *subnode = lhold->entries->head;
      if (subnode == NULL) {
        fprintf(stderr, "[GEN_HTML] This list contains no tasks\n");
        traveler = traveler->next;
        continue;
      } else {
        while (subnode != NULL) {
          if (subnode->data != NULL) {
            ehold = subnode->data;
            printf("Entry: %s\n", ehold->task);
            fwrite("\t\t\t\t\t<li>", strlen("\t\t\t\t\t<li>"), 1, hptr);
            fwrite(ehold->task, strlen(ehold->task), 1, hptr);
            fwrite("</li>\n", strlen("</li>\n"), 1, hptr);
          } else {
            fprintf(stderr,
                    "[GEN_HTML] Something go must up in the data field\n");
            break;
          }
          subnode = subnode->next;
        }
      }
      traveler = traveler->next;
    } else {
      fprintf(stderr, "[GEN_HTML] There was a problem with traveler->data\n");
      free(path);
      path = NULL;
      fclose(hptr);
      return;
    }
  }
  fwrite("\t\t\t\t</ul>\n", strlen("\t\t\t\t</ul>\n"), 1, hptr);
  fwrite("\t</ul>\n", strlen("\t</ul>\n"), 1, hptr);
  fwrite("</html>", strlen("</html>"), 1, hptr);
  fclose(hptr);
}
