#include <capstone/capstone.h>
#include <iostream>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include "socket_code.hpp"
#include "system.hpp"
std::string stdinfile;
std::string elffile;
std::string vdsofile;
void takeover(int fd) {
  close(fileno(stdin));
  close(fileno(stdout));
  dup2(fd, 0);
  dup2(fd, 1);
  return;
}
char *readtoken(char *data) {
  struct stat st;
  int fd;
  if (stat("/token", &st) != 0) {
    printf("[ERROR} failed to stat token\n");
    exit(1);
  }
  assert(data != NULL);
  fd = open("/token", O_RDONLY);
  if (fd <= 0) {
    printf("[ERROR] failed to open token: %s\n", strerror(errno));
    exit(1);
  }
  strcpy(data, "token:  ");
  read(fd, data + 8, st.st_size);
  close(fd);
  return data;
}
void dropprivs() {
  struct passwd *pw = NULL;
  pw = getpwnam("chess");
  if (pw == NULL) {
    std::cerr << "[FAIL] could not get \"chess\" user info" << std::endl;
    exit(1);
  }
  if (seteuid(pw->pw_uid) != 0) {
    std::cerr << "[FAIL] failed to seteuid" << std::endl;
    exit(1);
  }
  if (setegid(pw->pw_gid) != 0) {
    std::cerr << "[FAIL] failed to setegid" << std::endl;
    exit(1);
  }
  return;
}
void menu(int fd) {
  uint64_t vdso_set = 0;
  uint64_t elf_set = 0;
  uint64_t choice = 0;
  System *sys = NULL;
  int outfile_fd;
  char *tempName = NULL;
  char *data = NULL;
  uint64_t length = 0;
  while (!vdso_set | !elf_set) {
    std::cout << "1) Send ELF" << std::endl;
    std::cout << "2) Send vdso" << std::endl;
    std::cout << "3) Execute" << std::endl;
    std::cout << "4) Exec SC" << std::endl;
    std::cout << "5) Quit" << std::endl;
    std::cout << "# ";
    std::cin >> choice;
    switch (choice) {
    case 1:
      if (elf_set) {
        std::cout << "No" << std::endl;
        continue;
      }
      length = readfile(fd, &data);
      if (length == 0) {
        std::cerr << "[ERROR] program error: bybenmr" << std::endl;
        close(fd);
        exit(1);
      }
      tempName = tmpnam(NULL);
      if (tempName == NULL) {
        std::cerr << "[ERROR] program error: 34hlj5" << std::endl;
        close(fd);
        exit(1);
      }
      outfile_fd = open(tempName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
      if (outfile_fd <= 0) {
        std::cerr << "[ERROR] program error: asf9 -- " << strerror(errno)
                  << std::endl;
        close(fd);
        exit(1);
      }
      write(outfile_fd, data, length);
      close(outfile_fd);
      free(data);
      elffile = std::string(tempName);
      std::cerr << "[INFO] Temporary elf file: " << elffile << std::endl;
      elf_set = 1;
      break;
    case 2:
      if (vdso_set) {
        std::cout << "No" << std::endl;
        continue;
      }
      length = readfile(fd, &data);
      if (length == 0) {
        close(fd);
        exit(1);
      }
      tempName = tmpnam(NULL);
      if (tempName == NULL) {
        std::cerr << "[ERROR] program error: j2lk34" << std::endl;
        close(fd);
        exit(1);
      }
      outfile_fd = open(tempName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
      if (outfile_fd <= 0) {
        std::cerr << "[ERROR] program error: asd80asd" << std::endl;
        close(fd);
        exit(1);
      }
      write(outfile_fd, data, length);
      close(outfile_fd);
      free(data);
      vdsofile = std::string(tempName);
      std::cerr << "[INFO] Temporary vdso file: " << vdsofile << std::endl;
      vdso_set = 1;
      break;
    case 3:
      if (!vdso_set | !elf_set) {
        std::cout << "vdso and elf are required first" << std::endl;
      } else {
        return;
      }
      break;
    case 4:
      length = readfile(fd, &data);
      if (length == 0) {
        close(fd);
        exit(1);
      }
      sys = new System(data, length);
      delete sys;
      free(data);
      close(fd);
      exit(0);
      break;
    case 5:
      std::cout << "Quitting" << std::endl;
      close(fd);
      exit(0);
      break;
    default:
      std::cout << "Invalid..." << std::endl;
      close(fd);
      exit(1);
      break;
    }
  }
  return;
}
void usage(char *nm) {
  if (!nm) {
    exit(1);
  }
  std::cout << "USAGE: " << nm << " -s -v <vdso> -b <elf>" << std::endl;
  std::cout << "\tIf no command line options are specified then arborway will "
               "listen on the port specified by "
            << std::endl;
  std::cout << "\tthe environment variable PORT. However if the -s option is "
               "used then -v and -b are also required."
            << std::endl;
  std::cout << "\n\t-s -- Use stdin in place of opening a port." << std::endl;
  std::cout << "-v -- Location of the vdso file to load" << std::endl;
  std::cout << "-b -- Location of the elf to emulate" << std::endl;
  exit(1);
}
int main(int argc, char **argv, char **envp) {
  uint64_t stdin_flag = 0;
  uint64_t c;
  std::string vdso;
  std::string elf;
  std::vector<std::string> venvp = vectorize_array(envp);
  uint64_t port = 0;
  uint64_t fd = 0;
  uint64_t s_fd = 0;
  char token[128] = {0};
#ifndef NO_TESTBED
  char *b = getenv("CHESS");
  if (b == NULL) {
    std::cout << "[TESTBED] ENV variable check failed" << std::endl;
    exit(0);
  }
#endif
  while ((c = getopt(argc, argv, "v:b:s")) != -1) {
    switch (c) {
    case 's':
      stdin_flag = 1;
      break;
    case 'b':
      elf = std::string(optarg);
      break;
    case 'v':
      vdso = std::string(optarg);
      break;
    case '?':
      if (optopt == 'v' || optopt == 'b') {
        std::cout << "[ERROR] -" << optopt << " argument required" << std::endl;
        usage(argv[0]);
      } else {
        std::cout << "[ERROR] Unknown option" << std::endl;
        usage(argv[0]);
      }
    default:
      exit(1);
    }
  }
  if (stdin_flag == 0) {
    if (getenv("PORT") == NULL) {
      usage(argv[0]);
    }
    port = atoi(getenv("PORT"));
    s_fd = setup_socket(port);
    if (s_fd == 0) {
      exit(0);
    }
    fd = accept_socket(s_fd);
    if (fd == 0) {
      exit(0);
    }
    close(s_fd);
    takeover(fd);
    menu(fd);
    elf = elffile;
    vdso = vdsofile;
  }
  readtoken(token);
  System sys(elf, venvp, vdso);
  sys.Run();
  close(fd);
  close(0);
  close(1);
  fclose(sys.elf.fp);
  return 0;
}
