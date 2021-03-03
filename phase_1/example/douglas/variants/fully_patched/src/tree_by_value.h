#pragma once

#include <string>
#include <vector>

#include "zpair.h"

using std::string;

class TreeByValue {
 public:
  TreeByValue(string e, double s) : entry(e), score(s){};
  TreeByValue(ZMemberPair p) : entry(p.first), score(p.second){};
  TreeByValue(const TreeByValue& o)
      : entry(o.entry), score(o.score), left(nullptr), right(nullptr){};

  TreeByValue(std::vector<ZMemberPair> l);

  ~TreeByValue();

  void add(string e, double s);
  void remove(string e);

  double get_score(string e);

  bool member(string e);

  std::vector<ZMemberPair> to_vec();
  void dump(std::ostream& o);

  long int count();

  bool leaf();

 private:
  string entry;
  double score;
  TreeByValue* left = nullptr;
  TreeByValue* right = nullptr;

  TreeByValue minimum();
  TreeByValue maximum();

  void add_left(string e, double s);
  void add_right(string e, double s);

  void remove_self();
  void remove_left(string e);
  void remove_right(string e);
};
