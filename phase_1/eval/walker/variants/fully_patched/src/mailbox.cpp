#include "mailbox.hpp"
#include "utils.hpp"
#include <filesystem>
#include <regex>
#include <sys/stat.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
mailbox *parse_mailbox(std::string root, int depth, std::string base) {
  struct stat st;
  mailbox *parent = NULL;
  mailbox *child = NULL;
  std::filesystem::path canon;
  std::string dirline;
  if (stat(root.c_str(), &st) != 0) {
    return NULL;
  }
  canon.assign(root);
  try {
    canon = std::filesystem::canonical(canon);
  } catch (const std::exception &e) {
    return NULL;
  }
  parent = new mailbox;
  if (!parent) {
    return parent;
  }
  if (depth == 0) {
    parent->fullname = "INBOX";
    parent->name = "INBOX";
  }
  if (depth > 0) {
    parent->fullname = base + canon.filename().string();
    parent->name = canon.filename().string();
  }
  parent->fullpath = root;
  if (depth != 0) {
    if (access((root + "/cur").c_str(), F_OK | W_OK | R_OK | X_OK)) {
      delete parent;
      return NULL;
    }
    if (access((root + "/new").c_str(), F_OK | W_OK | R_OK | X_OK)) {
      delete parent;
      return NULL;
    }
    if (access((root + "/tmp").c_str(), F_OK | W_OK | R_OK | X_OK)) {
      delete parent;
      return NULL;
    }
  }
  parent->children = new mailbox *[5];
  if (!parent->children) {
    delete parent;
    return NULL;
  }
  parent->max_children = 5;
  for (int i = 0; i < parent->max_children; i++) {
    parent->children[i] = NULL;
  }
  for (const std::filesystem::directory_entry &e :
       std::filesystem::directory_iterator(canon)) {
    dirline = e.path().string();
    if (e.path().filename().string()[0] == '.') {
      child = parse_mailbox(dirline, depth + 1, parent->fullname);
      if (child != NULL && parent->total_children < parent->max_children) {
        parent->children[parent->total_children] = child;
        parent->total_children += 1;
      } else if (child) {
        delete child;
      }
    }
  }
  return parent;
}
mailbox *get_child_by_name(mailbox *parent, std::string n) {
  std::string t;
  int index;
  if (!parent) {
    return NULL;
  }
  index = n.find_first_of('.', 1);
  if (index != std::string::npos) {
    t = n.substr(0, index);
  } else {
    t = n;
  }
  for (int i = 0; i < parent->total_children; i++) {
    if (parent->children[i]->name == t) {
      return parent->children[i];
    }
  }
  return NULL;
}
mailbox *find_root_from_regex(mailbox *root, std::regex r) {
  std::smatch m;
  if (!root) {
    return NULL;
  }
  if (std::regex_search(root->fullname, m, r)) {
    return root;
  }
  for (int i = 0; i < root->total_children; i++) {
    if (std::regex_search(root->children[i]->fullname, m, r)) {
      return root->children[i];
    }
  }
  return NULL;
}
mailbox *find_root_from_string(mailbox *root, std::string s) {
  if (!root) {
    return NULL;
  }
  if (cppstrncasecmp(root->fullname, s) == true) {
    return root;
  }
  for (int i = 0; i < root->total_children; i++) {
    if (cppstrncasecmp(root->children[i]->fullname, s) == true) {
      return root->children[i];
    }
  }
  return NULL;
}
bool shared_parent(std::string a, std::string b) {
  std::vector<std::string> toks_a = tokenize_line(a, '.');
  std::vector<std::string> toks_b = tokenize_line(b, '.');
  if (toks_a.size() != toks_b.size()) {
    return false;
  }
  for (int i = 0; i < toks_a.size() - 1; i++) {
    if (toks_a[i] != toks_b[i]) {
      return false;
    }
  }
  return true;
}
mailbox *get_mailbox_by_fullname(mailbox *root, std::string n) {
  std::string current;
  std::string child;
  mailbox *walker = NULL;
  int index = 0;
  if (!root) {
    return NULL;
  }
  if (cppstrncasecmp(n, "inbox") == true) {
    return root;
  }
  current = n;
  walker = root;
  index = n.find_first_of('.');
  if (index == std::string::npos) {
    return NULL;
  }
  while (1) {
    current = current.substr(index);
    index = current.find_first_of('.', 1);
    if (index == std::string::npos) {
      for (int i = 0; i < walker->total_children; i++) {
        if (walker->children[i]->name == current) {
          return walker->children[i];
        }
      }
      return NULL;
    } else {
      child = current.substr(0, index);
      int child_found = 0;
      for (int i = 0; i < walker->total_children; i++) {
        if (walker->children[i]->name == child) {
          walker = walker->children[i];
          child_found = 1;
          break;
        }
      }
      if (!child_found) {
        return NULL;
      }
    }
  }
  return NULL;
}
void remove_child_by_name(mailbox *parent, std::string n) {
  mailbox *ch = NULL;
  int i = 0;
  int j = 0;
  if (!parent) {
    return;
  }
  ch = get_child_by_name(parent, n);
  if (!ch) {
    return;
  }
  while (i < parent->max_children) {
    if (parent->children[i] != ch) {
      parent->children[j] = parent->children[i];
      i++;
      j++;
    } else {
      parent->children[i] = NULL;
      i++;
      parent->total_children -= 1;
    }
  }
  free_mailbox(ch);
  return;
}
mailbox *deep_copy_mb(mailbox *root) {
  mailbox *mb = NULL;
  if (!root) {
    return mb;
  }
  mb = new mailbox;
  mb->name = root->name;
  mb->fullname = root->fullname;
  mb->fullpath = root->fullpath;
  mb->children = new mailbox *[5];
  if (!mb->children) {
    delete mb;
    return NULL;
  }
  mb->max_children = 5;
  mb->total_children = root->total_children;
  for (int i = 0; i < root->total_children; i++) {
    mb->children[i] = deep_copy_mb(root->children[i]);
    if (mb->children[i] == NULL) {
      free_mailbox(mb);
      return NULL;
    }
  }
  return mb;
}
void free_mailbox(mailbox *root) {
  if (!root) {
    return;
  }
  for (int i = 0; i < root->total_children; i++) {
    if (root->children[i]) {
      free_mailbox(root->children[i]);
    }
  }
  delete[] root->children;
  delete root;
  return;
}