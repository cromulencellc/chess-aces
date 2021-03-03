#pragma once

#include <string>
#include <vector>

#include "resp.h"
#include "tree.h"

using std::string;

class Set {
 public:
  Set(string k);

  bool member(string candidate);
  void add(string candidate);
  void remove(string candidate);
  long int count();

  std::vector<string> to_vec();
  std::vector<RespEntry> to_resp();

  void write();
  void write(string dest);

 private:
  string key;
  Tree* root = nullptr;
};
