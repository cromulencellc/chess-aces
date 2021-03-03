#include <sstream>
#include <string>
#include <vector>

#include "assert.h"
#include "db.h"
#include "resp.h"
#include "set.h"
#include "tree.h"

using std::string;

Set::Set(string k) {
  key = k;
  root = nullptr;

  RespEntry content = Db::get_instance()->deserialize(key);
  if (RespEntry::null == content.kind) {
    return;
  }

  std::vector<RespEntry> content_vec = content.get_array();

  for (RespEntry e : content_vec) {
    add(e.get_string());
  }
}

void Set::add(string candidate) {
  if (root == nullptr) {
    root = new Tree(candidate);
    return;
  }

  root->add(candidate);
}

void Set::remove(string candidate) {
  if (nullptr == root) {
    return;
  }

  if ((1 == root->count()) && root->member(candidate)) {
    delete root;
    root = nullptr;
    return;
  }

  root->remove(candidate);
}

bool Set::member(string candidate) {
  if (root == nullptr) return false;

  return root->member(candidate);
}

long int Set::count() {
  if (root == nullptr) return 0;

  return root->count();
}

std::vector<string> Set::to_vec() {
  if (nullptr == root) return {};

  return root->to_vec();
}

std::vector<RespEntry> Set::to_resp() {
  std::vector<RespEntry> entries = {};
  entries.reserve(count());
  for (string e : to_vec()) {
    entries.push_back(e);
  }
  return entries;
}

void Set::write() { write(key); }

void Set::write(string dest) {
  Db::get_instance()->serialize(dest, RespEntry{to_resp()});
}
