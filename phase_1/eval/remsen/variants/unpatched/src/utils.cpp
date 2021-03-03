#include "utils.hpp"
std::vector<std::string> cpptok(std::string base, char c) {
  std::vector<std::string> result;
  std::string newcut;
  std::size_t end = 0;
  std::size_t start = 0;
  while (end != std::string::npos) {
    end = base.find(c, start);
    if (end == std::string::npos) {
      newcut = base.substr(start, base.size() - start);
    } else {
      newcut = base.substr(start, end - start);
    }
    result.push_back(newcut);
    start = end + 1;
  }
  return result;
}
bool endswith(std::string base, std::string needle) {
  bool result = false;
  if (base.size() >= needle.size() &&
      base.compare(base.size() - needle.size(), base.size(), needle) == 0) {
    result = true;
  }
  return result;
}
bool startswith(std::string base, std::string needle) {
  bool result = base.find(needle);
  return (result == 0);
}
bool onlydigits(std::string base) {
  return base.find_first_not_of("0123456789") == std::string::npos;
}
std::string getcwd(void) {
  char cwd[PATH_MAX];
  memset(cwd, 0, PATH_MAX);
  getcwd(cwd, PATH_MAX - 1);
  return std::string(cwd);
}
int connect_socket(std::string ip, int port) {
  int sockfd;
  struct sockaddr_in clnt;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "socket() failed: %s\n", strerror(errno));
    return -1;
  }
  memset(&clnt, 0, sizeof(struct sockaddr_in));
  clnt.sin_family = AF_INET;
  clnt.sin_port = htons(port);
  if (inet_pton(AF_INET, ip.c_str(), &clnt.sin_addr) <= 0) {
    return -1;
  }
  if (connect(sockfd, (struct sockaddr *)&clnt, sizeof(clnt)) < 0) {
    return -1;
  }
  return sockfd;
}
int read_data(int fd, char **buffer) {
  int max = 0;
  int index = 0;
  int bytes = 0;
  char tbuff[1024];
  void * final = NULL;
  if (!buffer) {
    return 0;
  }
  memset(tbuff, 0, 1024);
  final = calloc(1, 1024);
  if (!final) {
    return 0;
  }
  max = 1024;
  while ((bytes = read(fd, tbuff, 1024)) > 0) {
    if (index + bytes > max) {
      final = realloc(final, max + 1024);
      if (!final) {
        return 0;
      }
      max += 1024;
    }
    memcpy(((char *)final) + index, tbuff, bytes);
    index += bytes;
    memset(tbuff, 0, 1024);
  }
  *buffer = (char *)final;
  return index;
}
std::string get_permission_string(const std::filesystem::directory_entry &e) {
  char entry_type;
  std::string str_perms;
  if (!e.exists()) {
    return "";
  }
  if (e.is_regular_file()) {
    entry_type = '-';
  } else if (e.is_block_file()) {
    entry_type = 'b';
  } else if (e.is_character_file()) {
    entry_type = 'c';
  } else if (e.is_directory()) {
    entry_type = 'd';
  } else if (e.is_symlink()) {
    entry_type = 'l';
  } else if (e.is_fifo()) {
    entry_type = 'p';
  } else if (e.is_socket()) {
    entry_type = 's';
  } else {
    entry_type = '?';
  }
  std::filesystem::perms p =
      std::filesystem::status(e.path().string()).permissions();
  str_perms =
      entry_type +
      ((p & std::filesystem::perms::owner_read) != std::filesystem::perms::none
           ? std::string("r")
           : std::string("-")) +
      ((p & std::filesystem::perms::owner_write) != std::filesystem::perms::none
           ? "w"
           : "-") +
      ((p & std::filesystem::perms::owner_exec) != std::filesystem::perms::none
           ? "x"
           : "-") +
      ((p & std::filesystem::perms::group_read) != std::filesystem::perms::none
           ? "r"
           : "-") +
      ((p & std::filesystem::perms::group_write) != std::filesystem::perms::none
           ? "w"
           : "-") +
      ((p & std::filesystem::perms::group_exec) != std::filesystem::perms::none
           ? "x"
           : "-") +
      ((p & std::filesystem::perms::others_read) != std::filesystem::perms::none
           ? "r"
           : "-") +
      ((p & std::filesystem::perms::others_write) !=
               std::filesystem::perms::none
           ? "w"
           : "-") +
      ((p & std::filesystem::perms::others_exec) != std::filesystem::perms::none
           ? "x"
           : "-");
  return str_perms;
}
int getownerid(const std::filesystem::directory_entry &directory) {
  struct stat st;
  stat(directory.path().c_str(), &st);
  return st.st_uid;
}
int getgroupid(const std::filesystem::directory_entry &directory) {
  struct stat st;
  stat(directory.path().c_str(), &st);
  return st.st_gid;
}
int checkperms(unsigned long perms, unsigned long tocheck) {
  if (perms & tocheck) {
    return 1;
  }
  return 0;
}
std::vector<std::string> get_detailed_listing(std::string directory,
                                              std::string cwd) {
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::vector<std::string> listing;
  std::filesystem::file_time_type lastmod;
  std::string dirline;
  fullpath = std::filesystem::relative(directory, cwd);
  canon.assign(cwd + "/" + fullpath.string());
  try {
    canon = std::filesystem::canonical(canon);
  } catch (const std::exception &e) {
    listing.clear();
    return listing;
  }
  for (const std::filesystem::directory_entry &e :
       std::filesystem::directory_iterator(canon)) {
    dirline = get_permission_string(e);
    dirline += " " + std::to_string(e.hard_link_count());
    dirline += " " + std::to_string(getownerid(e));
    dirline += " " + std::to_string(getownerid(e));
    if (e.is_directory()) {
      dirline += " 4096";
    } else {
      try {
        dirline += " " + std::to_string(std::filesystem::file_size(e.path()));
      } catch (const std::exception &e) {
        dirline += " 0";
      }
    }
    lastmod = std::filesystem::last_write_time(e.path());
    std::time_t cftime = decltype(lastmod)::clock::to_time_t(lastmod);
    dirline += " " + std::to_string(cftime);
    dirline += std::string("\t") + e.path().filename().string();
    listing.push_back(dirline);
  }
  return listing;
}
std::vector<std::string> get_listing(std::string directory, std::string cwd) {
  std::filesystem::path fullpath;
  std::filesystem::path canon;
  std::vector<std::string> listing;
  std::string dirline;
  fullpath = std::filesystem::relative(directory, cwd);
  canon.assign(cwd + "/" + fullpath.string());
  try {
    canon = std::filesystem::canonical(canon);
  } catch (const std::exception &e) {
    listing.clear();
    return listing;
  }
  for (const std::filesystem::directory_entry &e :
       std::filesystem::directory_iterator(canon)) {
    dirline += e.path().filename().string();
    listing.push_back(dirline);
  }
  return listing;
}