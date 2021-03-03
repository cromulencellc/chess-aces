#include "email.hpp"
#include "utils.hpp"
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <regex>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
int open_message(std::string fn, char **data) {
  struct stat st;
  char *t = NULL;
  int fd = 0;
  if (!data) {
    return 0;
  }
  if (stat(fn.c_str(), &st) != 0) {
    return 0;
  }
  t = new char[st.st_size + 1];
  if (!t) {
    return 0;
  }
  fd = open(fn.c_str(), O_RDONLY);
  if (fd <= 0) {
    delete t;
    return 0;
  }
  read(fd, t, st.st_size);
  close(fd);
  t[st.st_size] = '\x00';
  *data = t;
  return st.st_size;
}
message *parse_message(char *data) {
  message *m = NULL;
  header *h = NULL;
  std::string line;
  std::string d;
  std::vector<std::string> tokens;
  int index;
  std::string field;
  std::string value;
  if (!data) {
    return NULL;
  }
  m = new message;
  if (!m) {
    return NULL;
  }
  d = std::string(data);
  while ((index = d.find("\x0a\x09")) != std::string::npos) {
    d.replace(index, 2, " ");
  }
  tokens = tokenize_line(std::string(d), '\x0a');
  for (auto it = tokens.begin(); it != tokens.end() - 2; ++it) {
    line = *it;
    index = line.find_first_of(':');
    if (index == std::string::npos) {
      free_message(m);
      return NULL;
    }
    field = line.substr(0, index);
    value = line.substr(index + 1);
    h = new header;
    if (!h) {
      free_message(m);
      return NULL;
    }
    h->field = field;
    h->field_data = value;
    m->h.push_back(h);
  }
  m->body_length = tokens[tokens.size() - 1].size();
  m->body.assign(tokens[tokens.size() - 1]);
  return m;
}
std::vector<header *> get_all_headers(message *m) {
  std::vector<header *> v;
  v.clear();
  if (!m) {
    return v;
  }
  for (auto it = m->h.begin(); it != m->h.end(); ++it) {
    v.push_back(*it);
  }
  return v;
}
std::vector<header *> get_headers_by_vector(message *m,
                                            std::vector<std::string> vs) {
  std::vector<header *> headers;
  headers.clear();
  if (!m) {
    return headers;
  }
  for (auto it = m->h.begin(); it != m->h.end(); ++it) {
    for (auto it_vs = vs.begin(); it_vs != vs.end(); ++it_vs) {
      if (cppstrncasecmp((*it)->field, *it_vs) == true) {
        headers.push_back(*it);
        break;
      }
    }
  }
  return headers;
}
std::string get_header_by_name(message *m, std::string n) {
  if (!m) {
    return "NIL";
  }
  for (auto it = m->h.begin(); it != m->h.end(); ++it) {
    if (cppstrncasecmp((*it)->field, n) == true) {
      return (*it)->field_data;
    }
  }
  return "NIL";
}
void free_message(message *m) {
  if (!m) {
    return;
  }
  for (auto it = m->h.begin(); it != m->h.end(); ++it) {
    delete *it;
  }
  m->h.clear();
  m->flags.clear();
  delete m;
  return;
}
bool hasflag(std::string fn, char flag) {
  std::string flags;
  int index = fn.find(":2,");
  if (index == std::string::npos) {
    return false;
  }
  flags = fn.substr(index + 3);
  for (auto it = flags.begin(); it != flags.end(); ++it) {
    if (*it == flag) {
      return true;
    }
  }
  return false;
}
std::string get_base_name(std::string name) {
  int index;
  index = name.find(":2,");
  if (index == std::string::npos) {
    return name + ":2,";
  }
  return name.substr(0, index + 3);
}
std::string add_flag_to_name(std::string name, char f) {
  std::string flags;
  std::string newname;
  std::vector<char> fv;
  int index;
  index = name.find(":2,");
  if (index == std::string::npos) {
    newname = name + ":2," + f;
    return newname;
  }
  flags = name.substr(index + 3);
  switch (f) {
  case 'P':
  case 'R':
  case 'S':
  case 'T':
  case 'D':
  case 'F':
    break;
  default:
    return name;
    break;
  }
  for (auto it = flags.begin(); it != flags.end(); ++it) {
    switch (*it) {
    case 'P':
    case 'R':
    case 'S':
    case 'T':
    case 'D':
    case 'F':
      if (*it == f) {
        return name;
      } else {
        fv.push_back(*it);
      }
      break;
    default:
      break;
    }
  }
  newname = name.substr(0, index + 3);
  fv.push_back(f);
  std::sort(fv.begin(), fv.end());
  for (auto it = fv.begin(); it != fv.end(); ++it) {
    newname += *it;
  }
  return newname;
}
std::string remove_flag_from_name(std::string name, char f) {
  std::string flags;
  std::string newname;
  std::vector<char> fv;
  int index;
  index = name.find(":2,");
  if (index == std::string::npos) {
    newname = name + ":2," + std::to_string(f);
    return newname;
  }
  flags = name.substr(index + 3);
  switch (f) {
  case 'P':
  case 'R':
  case 'S':
  case 'T':
  case 'D':
  case 'F':
    break;
  default:
    return name;
    break;
  }
  for (auto it = flags.begin(); it != flags.end(); ++it) {
    switch (*it) {
    case 'P':
    case 'R':
    case 'S':
    case 'T':
    case 'D':
    case 'F':
      if (*it != f) {
        fv.push_back(*it);
      }
      break;
    default:
      break;
    }
  }
  newname = name.substr(0, index + 3);
  std::sort(fv.begin(), fv.end());
  for (auto it = fv.begin(); it != fv.end(); ++it) {
    newname += *it;
  }
  return newname;
}
std::string get_flag_string(std::string fn) {
  std::string line;
  line += "FLAGS (";
  if (hasflag(fn, 'P') == true) {
    line += "\\Passed ";
  }
  if (hasflag(fn, 'R') == true) {
    line += "\\Replied ";
  }
  if (hasflag(fn, 'S') == true) {
    line += "\\Seen ";
  }
  if (hasflag(fn, 'T') == true) {
    line += "\\Trashed ";
  }
  if (hasflag(fn, 'D') == true) {
    line += "\\Draft ";
  }
  if (hasflag(fn, 'F') == true) {
    line += "\\Flagged ";
  }
  rtrim(line);
  line += ")";
  return line;
}
std::string get_internal_date_string(std::string fn) {
  std::string line = "";
  std::filesystem::path p;
  try {
    p = std::filesystem::path(fn);
  } catch (const std::exception &e) {
    return "";
  }
  auto t = std::filesystem::last_write_time(p);
  std::time_t ct = decltype(t)::clock::to_time_t(t);
  line = "INTERNALDATE \"";
  line += std::asctime(std::localtime(&ct));
  rtrim(line);
  line += "\"";
  return line;
}
std::string rfc822(std::string fn) {
  std::string line = "RFC822 {";
  char *data = NULL;
  int length;
  length = open_message(fn, &data);
  if (!length) {
    line += "0}";
    return line;
  }
  line += std::to_string(length) + "}\r\n";
  line += std::string(data);
  delete[] data;
  return line;
}
std::string rfc822_size(std::string fn) {
  std::string line = "RFC822.SIZE ";
  char *data = NULL;
  int length;
  length = open_message(fn, &data);
  if (!length) {
    line += "0";
    return line;
  }
  line += std::to_string(length);
  delete[] data;
  return line;
}
std::string rfc822_header(std::string fn) {
  std::string line = "RFC822.HEADER {";
  char *data = NULL;
  int length;
  message *m = NULL;
  std::vector<header *> h;
  std::string t = "";
  length = open_message(fn, &data);
  if (!length) {
    line += "0}";
    return line;
  }
  m = parse_message(data);
  delete[] data;
  if (!m) {
    line += "0}";
    return line;
  }
  h = get_all_headers(m);
  for (auto it = h.begin(); it != h.end(); ++it) {
    t += (*it)->field + ": " + (*it)->field_data + "\n";
  }
  t += "\n";
  line += std::to_string(t.size()) + "}\n";
  line += t;
  free_message(m);
  return line;
}
std::string rfc822_text(std::string fn) {
  std::string line = "RFC822.TEXT {";
  char *data = NULL;
  int length;
  message *m = NULL;
  std::string t = "";
  length = open_message(fn, &data);
  if (!length) {
    line += "0}";
    return line;
  }
  m = parse_message(data);
  delete[] data;
  if (!m) {
    line += "0}";
    return line;
  }
  line += std::to_string(m->body_length) + "}\n";
  line += m->body;
  free_message(m);
  return line;
}
std::string envelope(std::string fn) {
  std::string line = "ENVELOPE (";
  char *data = NULL;
  int length;
  message *m = NULL;
  std::vector<std::string> t;
  std::string fd;
  length = open_message(fn, &data);
  if (!length) {
    line += " )";
    return line;
  }
  m = parse_message(data);
  delete[] data;
  if (!m) {
    line += " )";
    return line;
  }
  line += "\"";
  line += get_header_by_name(m, "date");
  line += "\"";
  line += " \"" + get_header_by_name(m, "subject") + "\"";
  fd = get_header_by_name(m, "from");
  t = tokenize_line(fd, '@');
  line += " (\"" + t[0];
  if (t.size() < 2) {
    line += " \"NIL\"";
  } else {
    line += " \"" + t[1] + "\"";
  }
  line += ")";
  fd = get_header_by_name(m, "to");
  t = tokenize_line(fd, '@');
  line += " (\"" + t[0];
  if (t.size() < 2) {
    line += " \"NIL\"";
  } else {
    line += " \"" + t[1] + "\"";
  }
  line += ")";
  line += " \"" + get_header_by_name(m, "message-id") + "\"";
  line += ")";
  free_message(m);
  return line;
}
std::string body_headers(std::string fn, std::vector<std::string> body_tokens) {
  std::string line = "";
  message *m = NULL;
  char *data = NULL;
  int length = 0;
  std::vector<header *> h;
  std::string t = "";
  line = "BODY[";
  for (auto it = body_tokens.begin(); it != body_tokens.end(); ++it) {
    line += *it + " ";
  }
  rtrim(line);
  line += "] {";
  length = open_message(fn, &data);
  if (!length) {
    line += "0}";
    return line;
  }
  m = parse_message(data);
  delete[] data;
  if (!m) {
    line += " 0}";
    return line;
  }
  h = get_headers_by_vector(m, body_tokens);
  for (auto it = h.begin(); it != h.end(); ++it) {
    t += (*it)->field + ": " + (*it)->field_data + "\n";
  }
  line += std::to_string(t.size()) + "}\n" + t;
  return line;
}