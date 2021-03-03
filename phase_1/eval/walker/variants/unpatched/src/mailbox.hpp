#ifndef __MAILBOX_HPP__
#define __MAILBOX_HPP__
#include <iostream>
#include <regex>
typedef struct mailbox {
  std::string fullname = "";
  std::string name = "";
  std::string fullpath = "";
  struct mailbox **children = NULL;
  int max_children = 0;
  int total_children = 0;
} mailbox;
mailbox *parse_mailbox(std::string root, int depth, std::string base);
mailbox *find_root_from_regex(mailbox *root, std::regex r);
mailbox *find_root_from_string(mailbox *root, std::string s);
mailbox *get_child_by_name(mailbox *parent, std::string n);
void remove_child_by_name(mailbox *parent, std::string n);
mailbox *get_mailbox_by_fullname(mailbox *parent, std::string n);
mailbox *deep_copy_mb(mailbox *root);
bool shared_parent(std::string a, std::string b);
void free_mailbox(mailbox *root);
#endif