#define _GNU_SOURCE
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "action_stack.h"
#include "admin_console.h"
#include "html_writer.h"
#include "line_state_table.h"
#include "user_list.h"
#define MAX_NAME_SIZE 64
#define MAX_PATH_SIZE 128
#define MAX_READ_SIZE 512
#define SESSION_ACTIONS_LIMIT 10
#define LAND_WELCOME_MESSAGE                                                   \
  "\n\n\t\t---------Welcome to the Admin Console---------\t\t\n"
#define FILE_CONSOLE "\t\t\t1) FILE EDITOR\t\t\t\n"
#define USER_LIST_CONSOLE "\t\t\t2) USER LIST EDITOR\t\t\t\n"
#define FILE_WELCOME_MESSAGE                                                   \
  "\n\n\t\t---------Welcome to the File Editor---------\t\t\n"
#define FILE_EDIT_MESSAGE "\t\t\t1) EDIT DOCUMENT\t\t\t\n"
#define FILE_CREATE_MESSAGE "\t\t\t2) CREATE DOCUMENT\t\t\t\n"
#define FILE_DELETE_MESSAGE "\t\t\t3) DELETE DOCUMENT\t\t\t\n"
#define FILE_EXIT_MESSAGE "\t\t\t4) EXIT\t\t\t\n"
#define USER_WELCOME_MESSAGE                                                   \
  "\n\n\t\t---------Welcome to the User Editor---------\t\t\n"
#define USER_EDIT_MESSAGE "\t\t\t1) EDIT USER\t\t\t\n"
#define USER_CREATE_MESSAGE "\t\t\t2) CREATE USER\t\t\t\n"
#define USER_DELETE_MESSAGE "\t\t\t3) DELETE USER\t\t\t\n"
#define USER_EXIT_MESSAGE "\t\t\t4) EXIT\t\t\t\n"
#define KILL_CONNECTION "4"
#define KEEP_ALIVE "2"
void edit_document(int admin_accept);
void create_document(int admin_accept);
void delete_document();
void print_admin_choices(int admin_accept);
void start_admin_console(int admin_accept) {
  int land_page_size = strlen(LAND_WELCOME_MESSAGE) + strlen(FILE_CONSOLE) +
                       strlen(USER_LIST_CONSOLE) + 1;
  int file_prompt_size =
      strlen(FILE_WELCOME_MESSAGE) + strlen(FILE_EDIT_MESSAGE) +
      strlen(FILE_CREATE_MESSAGE) + strlen(FILE_DELETE_MESSAGE) +
      strlen(FILE_EXIT_MESSAGE) + 1;
  int user_prompt_size =
      strlen(USER_WELCOME_MESSAGE) + strlen(USER_EDIT_MESSAGE) +
      strlen(USER_CREATE_MESSAGE) + strlen(USER_DELETE_MESSAGE) +
      strlen(USER_EXIT_MESSAGE) + 1;
  int send_check = 0, recv_check = 0;
  char file_prompt[file_prompt_size];
  char land_page[land_page_size];
  char user_prompt[user_prompt_size];
  snprintf(land_page, land_page_size, "%s%s%s", LAND_WELCOME_MESSAGE,
           FILE_CONSOLE, USER_LIST_CONSOLE);
  snprintf(file_prompt, file_prompt_size, "%s%s%s%s%s", FILE_WELCOME_MESSAGE,
           FILE_EDIT_MESSAGE, FILE_CREATE_MESSAGE, FILE_DELETE_MESSAGE,
           FILE_EXIT_MESSAGE);
  snprintf(user_prompt, user_prompt_size, "%s%s%s%s%s", USER_WELCOME_MESSAGE,
           USER_EDIT_MESSAGE, USER_CREATE_MESSAGE, USER_DELETE_MESSAGE,
           USER_EXIT_MESSAGE);
  if ((send_check =
           send(admin_accept, land_page, land_page_size, MSG_NOSIGNAL) > 0)) {
    printf("Prompt sent\n\n");
  } else if (send_check == -1) {
    fprintf(stderr, "PROMPT FAILED\n");
    fprintf(stderr, "ERRNO: %s\n", strerror(errno));
    return;
  }
  char response[2];
  if ((recv_check = recv(admin_accept, response, 1, MSG_WAITALL)) > 0) {
    printf("Bytes read in: %d\n", recv_check);
    response[1] = '\0';
    printf("RESPONSE: %s\n\n", response);
  } else if (recv_check == -1) {
    fprintf(stderr, "FAILED TO CATCH LAND_PAGE RESPONSE\n");
    fprintf(stderr, "ERRNO: %s\n", strerror(errno));
    return;
  }
  if (response[0] == '1') {
    if ((send_check = send(admin_accept, file_prompt, file_prompt_size,
                           MSG_NOSIGNAL)) > 0) {
      printf("Prompt sent\n\n");
    } else if (send_check == -1) {
      fprintf(stderr, "FILE PROMPT FAILED\n");
      fprintf(stderr, "ERRNO: %s\n", strerror(errno));
      return;
    }
    char file_response[2];
    if ((recv_check = recv(admin_accept, file_response, 1, MSG_WAITALL)) > 0) {
      printf("Bytes read in: %d\n", recv_check);
      file_response[1] = '\0';
      printf("File Response: %s\n\n", file_response);
    } else if (recv_check == -1) {
      fprintf(stderr, "FAILED TO CATCH RESPONSE\n");
      fprintf(stderr, "ERRNO: %s\n", strerror(errno));
      return;
    }
    switch (file_response[0]) {
    case '1':
      printf("Editing...\n\n");
      edit_document(admin_accept);
      break;
    case '2':
      printf("Creating...\n\n");
      create_document(admin_accept);
      break;
    case '3':
      printf("Deleting...\n\n");
      delete_document(admin_accept);
      break;
    case '4':
      printf("Exiting...\n\n");
      return;
    default:
      printf("File response was: %c\n", file_response[0]);
      printf("An invalid command was entered..\nTry Again\n\n");
      break;
    }
  } else if (response[0] == '2') {
    if ((send_check = send(admin_accept, user_prompt, user_prompt_size,
                           MSG_NOSIGNAL)) > 0) {
      printf("User Prompt sent\n\n");
    } else if (send_check == -1) {
      fprintf(stderr, "USER PROMPT FAILED\n");
      fprintf(stderr, "ERRNO: %s\n", strerror(errno));
      return;
    }
    char user_response[2];
    if ((recv_check = recv(admin_accept, user_response, 1, MSG_WAITALL)) > 0) {
      printf("Bytes read in: %d\n", recv_check);
      user_response[1] = '\0';
      printf("User Response: %s\n\n", user_response);
    } else if (recv_check == -1) {
      fprintf(stderr, "FAILED TO CATCH RESPONSE\n");
      fprintf(stderr, "ERRNO: %s\n", strerror(errno));
      return;
    }
    UserList *ulist = retrieve_user_list();
    LineStateTable *lst = initalize_ls_table();
    struct pollfd fds[1];
    switch (user_response[0]) {
    case '1':
      fds[0].fd = admin_accept;
      fds[0].events = POLLIN;
      fds[0].revents = 0;
      char curr_user[64];
      char change_to[64];
      char edit_choice[2];
      unsigned int cont_len[1];
      int ret = poll(fds, 1, 7500);
      if (fds[0].revents & POLLIN) {
        if ((recv_check = recv(admin_accept, cont_len, 4, MSG_DONTWAIT)) > 0) {
          if (cont_len[0] < 0x44) {
            printf("The cont_len:  %d\n\n", cont_len[0]);
          } else {
            fprintf(stderr, "There was an error with the cont_len\n");
            return;
          }
        } else if (recv_check == -1) {
          fprintf(stderr, "There was an error with the cont_len\n");
          return;
        }
        fds[0].revents = 0;
        ret = poll(fds, 1, 7500);
        if (fds[0].revents & POLLIN) {
          if ((recv_check = recv(admin_accept, curr_user, cont_len[0],
                                 MSG_DONTWAIT)) > 0) {
            curr_user[recv_check] = '\0';
            printf("Editing: %s\n\n", curr_user);
          } else if (recv_check == -1) {
            fprintf(stderr, "FAILED TO CATCH EDIT RESPONSE\n");
            fprintf(stderr, "ERRNO: %s\n", strerror(errno));
            return;
          }
        }
        fds[0].revents = 0;
        int ret2 = poll(fds, 1, 7500);
        if (fds[0].events & POLLIN) {
          if ((recv_check = recv(admin_accept, cont_len, 4, MSG_DONTWAIT)) >
              0) {
            if (cont_len[0] < 0x44) {
              printf("The cont_len:  %d\n\n", cont_len[0]);
            } else {
              fprintf(stderr, "There was an error with the cont_len\n");
              return;
            }
          } else if (recv_check == -1) {
            fprintf(stderr, "There was an error with the cont_len\n");
            fprintf(stderr, "ERRNO: %s\n", strerror(errno));
            return;
          }
          fds[0].revents = 0;
          ret = poll(fds, 1, 7500);
          if (fds[0].revents & POLLIN) {
            if ((recv_check = recv(admin_accept, change_to, cont_len[0],
                                   MSG_DONTWAIT)) > 0) {
              change_to[recv_check] = '\0';
            } else if (recv_check == -1) {
              fprintf(stderr, "FAILED TO CATCH CHANGE RESPONSE\n");
              fprintf(stderr, "ERRNO: %s\n", strerror(errno));
              return;
            }
          }
          fds[0].revents = 0;
          ret = poll(fds, 1, 7500);
          if (fds[0].revents & POLLIN) {
            if ((recv_check =
                     recv(admin_accept, edit_choice, 1, MSG_DONTWAIT)) > 0) {
              edit_choice[1] = '\0';
              printf("Edit_field: %s\n\n", edit_choice);
              edit_user(curr_user, change_to, edit_choice, lst, ulist);
            } else if (recv_check == -1) {
              fprintf(stderr, "FAILED TO CATCH CHANGE RESPONSE\n");
              fprintf(stderr, "ERRNO: %s\n", strerror(errno));
              return;
            }
          }
        } else if (ret2 == 0) {
          fprintf(stderr, "The connection timedout due to no response\n");
          return;
        } else if (ret2 == -1) {
          fprintf(stderr, "There was an error with poll\n");
          return;
        }
      } else if (ret == 0) {
        fprintf(stderr, "The connection timedout due to no response\n");
        return;
      } else if (ret == -1) {
        fprintf(stderr, "There was an error with poll\n");
        return;
      }
      printf("Finished editing\n");
      break;
    case '2':
      printf("Creating...\n\n");
      fds[0].fd = admin_accept;
      fds[0].events = POLLIN;
      fds[0].revents = 0;
      ret = poll(fds, 1, 7500);
      char username[MAX_NAME_SIZE];
      char password[MAX_NAME_SIZE];
      char type[MAX_NAME_SIZE];
      if (fds[0].revents & POLLIN) {
        if ((recv_check = recv(admin_accept, cont_len, 4, MSG_DONTWAIT)) > 0) {
          if (cont_len[0] < 0x44) {
            printf("The cont_len:  %d\n\n", cont_len[0]);
          } else {
            printf("There was an error with cont_len");
            return;
          }
        } else if (recv_check == -1 || cont_len[0] > 63) {
          fprintf(stderr, "There was an error with the cont_len\n");
          fprintf(stderr, "ERRNO: %s\n", strerror(errno));
          return;
        }
        fds[0].revents = 0;
        ret = poll(fds, 1, 7500);
        if (fds[0].revents & POLLIN) {
          if ((recv_check = recv(admin_accept, username, cont_len[0],
                                 MSG_DONTWAIT)) > 0) {
            username[recv_check] = '\0';
            printf("The number of bytes read in is: %d\n", recv_check);
            printf("Username: %s\n", username);
          } else if (recv_check == -1) {
            fprintf(stderr, "There was a problem with a recv\n");
            return;
          }
        }
      } else if (ret == 0) {
        fprintf(stderr, "TIMEOUT\n");
        return;
      }
      fds[0].revents = 0;
      ret = poll(fds, 1, 7500);
      if (fds[0].revents & POLLIN) {
        if ((recv_check = recv(admin_accept, cont_len, 4, MSG_DONTWAIT)) > 0) {
          if (cont_len[0] < 0x44) {
            printf("The cont_len:  %d\n\n", cont_len[0]);
          } else {
            printf("There was an error with cont_len");
            return;
          }
        } else if (recv_check == -1 || cont_len[0] > 63) {
          fprintf(stderr, "There was an error with the cont_len\n");
          fprintf(stderr, "ERRNO: %s\n", strerror(errno));
          return;
        }
        fds[0].revents = 0;
        ret = poll(fds, 1, 7500);
        if (fds[0].revents & POLLIN) {
          if ((recv_check = recv(admin_accept, password, cont_len[0],
                                 MSG_DONTWAIT)) > 0) {
            password[recv_check] = '\0';
          } else if (recv_check == -1) {
            fprintf(stderr, "There was a problem with a recv\n");
            return;
          }
        } else if (ret == 0) {
          fprintf(stderr, "TIMEOUT\n");
          return;
        }
      }
      fds[0].revents = 0;
      ret = poll(fds, 1, 7500);
      if (fds[0].revents & POLLIN) {
        if ((recv_check = recv(admin_accept, cont_len, 4, MSG_DONTWAIT)) > 0) {
          if (cont_len[0] < 0x44) {
            printf("The cont_len:  %d\n\n", cont_len[0]);
          } else {
            printf("There was an error in cont_len\n");
            return;
          }
        } else if (recv_check == -1 || cont_len[0] > 63) {
          fprintf(stderr, "There was an error with the cont_len\n");
          fprintf(stderr, "ERRNO: %s\n", strerror(errno));
          return;
        }
        fds[0].revents = 0;
        ret = poll(fds, 1, 7500);
        if (fds[0].revents & POLLIN) {
          if ((recv_check =
                   recv(admin_accept, type, cont_len[0], MSG_DONTWAIT)) > 0) {
            type[recv_check] = '\0';
          } else if (recv_check == -1) {
            fprintf(stderr, "There was a problem with a recv\n");
            return;
          }
        }
      } else if (ret == 0) {
        fprintf(stderr, "TIMEOUT\n");
        return;
      }
      add_user(ulist, username, password, type);
      printf("Created user!\n");
      break;
    case '3':
      printf("Deleting...\n\n");
      char del_uname[MAX_NAME_SIZE];
      struct pollfd fds[1];
      fds[0].fd = admin_accept;
      fds[0].events = POLLIN;
      fds[0].revents = 0;
      ret = poll(fds, 1, 7500);
      if (fds[0].revents & POLLIN) {
        if ((recv_check = recv(admin_accept, cont_len, 4, MSG_DONTWAIT)) > 0) {
          if (cont_len[0] < 0x44) {
            printf("Cont_len: %d\n", cont_len[0]);
          } else {
            printf("There was an error in cont_len\n");
            return;
          }
        } else if (recv_check == -1) {
          fprintf(stderr, "There was a problem with a recv\n");
          return;
        }
      } else if (ret == 0) {
        fprintf(stderr, "TIMEOUT\n");
        return;
      }
      fds[0].revents = 0;
      ret = poll(fds, 1, 7500);
      if (fds[0].revents & POLLIN) {
        if ((recv_check = recv(admin_accept, del_uname, cont_len[0],
                               MSG_DONTWAIT)) > 0) {
          del_uname[recv_check] = '\0';
          printf("Username to delete recieved: %s\n", del_uname);
        } else if (recv_check == -1) {
          fprintf(stderr, "There was a problem with a recv\n");
          return;
        }
      } else if (ret == 0) {
        fprintf(stderr, "TIMEOUT\n");
        return;
      }
      delete_user(del_uname);
      printf("User deleted!\n");
      break;
    case '4':
      printf("Exiting...\n\n");
      return;
    default:
      printf("An invalid command was entered..\nTry Again\n\n");
      printf("User Response was: %s\n", user_response);
      break;
    }
  } else {
    fprintf(stderr, "ADMIN_CONSOLE: INVALID CHOICE LANDPAGE\n");
    return;
  }
}
void edit_document(int admin_accept) {
  int r_check = 0;
  int ret = 0;
  struct pollfd fds[1];
  fds[0].fd = admin_accept;
  fds[0].events = POLLIN;
  fds[0].revents = 0;
  ret = poll(fds, 1, 7500);
  char filename[MAX_NAME_SIZE];
  unsigned int cont_len[1];
  if (ret > 0) {
    if (fds[0].revents & POLLIN) {
      if ((r_check = recv(admin_accept, cont_len, 4, MSG_DONTWAIT)) > 0) {
        if (cont_len[0] < 0x44) {
          printf("Cont_len: %x\n", cont_len[0]);
        } else {
          fprintf(stderr, "FAILED TO CONT_LEN CHECK\n");
          return;
        }
        printf("Cont_len: %d\n", cont_len[0]);
      } else if (r_check == -1) {
        fprintf(stderr, "FAILED TO CATCH FILE NAME\n");
        fprintf(stderr, "ERRNO: %s\n", strerror(errno));
        return;
      }
    }
    fds[0].revents = 0;
    ret = poll(fds, 1, 7500);
    if (fds[0].revents & POLLIN) {
      if ((r_check = recv(admin_accept, filename, cont_len[0], MSG_DONTWAIT)) >
          0) {
        filename[r_check] = '\0';
        printf("The file name caught was: %s\n\n", filename);
      } else if (r_check == -1) {
        fprintf(stderr, "FAILED TO CATCH FILE NAME\n");
        fprintf(stderr, "ERRNO: %s\n", strerror(errno));
        return;
      }
    }
  } else if (ret <= 0) {
    fprintf(stderr, "TIMEOUT: Connection Terminated\n");
    fprintf(stderr, "ERRNO: %s\n", strerror(errno));
    close(admin_accept);
    return;
  }
  printf("FILE EDITING BEGINNING\n");
  FILE *fptr;
  char *root_path = "/data/wiki/origins/";
  char *path_holder = calloc(1, MAX_PATH_SIZE);
  snprintf(path_holder, strlen(root_path) + strlen(filename) + 1, "%s%s",
           root_path, filename);
  printf("PATH OF FILE: %s\n", path_holder);
  int f_check = 0;
  if ((f_check = access(path_holder, F_OK)) != 0) {
    fprintf(stderr, "The file does not exist\n");
    return;
  }
  if ((fptr = fopen(path_holder, "r")) == NULL) {
    fprintf(stderr, "EDIT: The file was unable to be opened for readingn!\n");
    return;
  }
  fseek(fptr, 0L, SEEK_END);
  int size = ftell(fptr);
  rewind(fptr);
  char *buffer = calloc(1, size + 1);
  fread(buffer, size, 1, fptr);
  printf("Size of file is: %d\n", size);
  printf("Content of files: %s\n", buffer);
}
void create_document(int admin_accept) {
  int r_check = 0;
  int ret = 0;
  struct pollfd fds[1];
  fds[0].fd = admin_accept;
  fds[0].events = POLLIN;
  fds[0].revents = 0;
  ret = poll(fds, 1, 7500);
  char filename[MAX_NAME_SIZE];
  unsigned int cont_len[1];
  if (ret > 0) {
    if (fds[0].revents & POLLIN) {
      if ((r_check = recv(admin_accept, cont_len, 4, MSG_DONTWAIT)) > 0) {
        if (cont_len[0] < 0x44) {
          printf("Cont_len: %d\n", cont_len[0]);
        } else {
          fprintf(stderr, "Cont_len ERROR\n");
          fprintf(stderr, "ERRNO: %s\n", strerror(errno));
          return;
        }
      } else if (r_check == -1 || cont_len[0] > 63) {
        fprintf(stderr, "FILENAME ERROR\n");
        fprintf(stderr, "ERRNO: %s\n", strerror(errno));
        return;
      }
    }
    fds[0].revents = 0;
    ret = poll(fds, 1, 7500);
    if (fds[0].revents & POLLIN) {
      if ((r_check =
               recv(admin_accept, filename, MAX_NAME_SIZE, MSG_DONTWAIT)) > 0) {
        filename[r_check] = '\0';
        printf("The file name caught was: %s\n\n", filename);
      } else if (r_check == -1) {
        fprintf(stderr, "FAILED TO CATCH FILE NAME\n");
        fprintf(stderr, "ERRNO: %s\n", strerror(errno));
        return;
      }
    }
  } else if (ret == 0) {
    fprintf(stderr, "TIMEOUT: Connection Terminated\n");
    fprintf(stderr, "ERRNO: %s\n", strerror(errno));
    close(admin_accept);
    return;
  }
  FILE *fptr;
  char *root_path = "/data/wiki/origins/";
  char *path_holder = calloc(1, MAX_PATH_SIZE);
  snprintf(path_holder, strlen(root_path) + strlen(filename) + 1, "%s%s",
           root_path, filename);
  int f_check = 0;
  if ((f_check = access(path_holder, F_OK)) == 0) {
    fprintf(stderr,
            "The file that you are attempting to create already exists\n");
    return;
  }
  if ((fptr = fopen(path_holder, "w")) == NULL) {
    fprintf(stderr, "CREATE: The file was unable to be opened for creation!\n");
    return;
  } else {
    printf("The file was successfully created\n");
    printf("Attempting to create corresponding html file\n");
  }
}
void delete_document(int admin_accept) {
  int r_check = 0;
  int ret = 0;
  struct pollfd fds[1];
  fds[0].fd = admin_accept;
  fds[0].events = POLLIN;
  ret = poll(fds, 1, 7500);
  char filename[MAX_NAME_SIZE];
  unsigned int cont_len[1];
  if (ret > 0) {
    if (fds[0].revents & POLLIN) {
      if ((r_check = recv(admin_accept, cont_len, 4, MSG_DONTWAIT)) > 0) {
        if (cont_len[0] < 0x44) {
          printf("Cont_len: %d\n", cont_len[0]);
        } else {
          fprintf(stderr, "Failed cont_len check\n");
          return;
        }
      } else if (r_check == -1 || cont_len[0] > 63) {
        fprintf(stderr, "FILENAME ERROR\n");
        fprintf(stderr, "ERRNO: %s\n", strerror(errno));
        return;
      }
    }
    fds[0].revents = 0;
    ret = poll(fds, 1, 7500);
    if (fds[0].revents & POLLIN) {
      if ((r_check = recv(admin_accept, filename, cont_len[0], MSG_DONTWAIT)) >
          0) {
        filename[r_check] = '\0';
        printf("The file name caught was: %s\n\n", filename);
      } else if (r_check == -1) {
        fprintf(stderr, "FAILED TO CATCH FILE NAME\n");
        fprintf(stderr, "ERRNO: %s\n", strerror(errno));
        return;
      }
    }
  } else if (ret == 0) {
    fprintf(stderr, "TIMEOUT: Connection Terminated\n");
    fprintf(stderr, "ERRNO: %s\n", strerror(errno));
    close(admin_accept);
    return;
  }
  char *root_path = "/data/wiki/origins/";
  char *path_holder = calloc(1, MAX_PATH_SIZE);
  snprintf(path_holder, strlen(root_path) + strlen(filename) + 1, "%s%s",
           root_path, filename);
  int f_check = 0;
  if ((f_check = access(path_holder, F_OK)) == 0) {
    printf("Found the file in question\n");
  } else {
    fprintf(stderr, "f_check value: %d\n", f_check);
    fprintf(stderr, "The file does not exist\n");
    return;
  }
  r_check = 0;
  if ((r_check = remove(path_holder)) == 0) {
    printf("File removed!\n");
  } else {
    fprintf(stderr, "FAILED TO REMOVE FILE\n");
    fprintf(stderr, "ERRNO: %s\n", strerror(errno));
  }
}
