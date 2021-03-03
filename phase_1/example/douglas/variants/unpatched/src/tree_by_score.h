#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "zpair.h"

using std::string;

class TreeByScore {
 public:
  TreeByScore(double s, string e) : score(s), entry(e){};

  TreeByScore(ZScorePair p) : score(p.first), entry(p.second){};
  TreeByScore(ZMemberPair p) : score(p.second), entry(p.first){};

  TreeByScore(const TreeByScore& o)
      : score(o.score), entry(o.entry), left(nullptr), right(nullptr){};

  TreeByScore(std::vector<ZScorePair> l);

  ~TreeByScore();

  void add(double s, string e);
  void remove(double s, string e);

  bool member(double s, string e);

  std::vector<ZScorePair> to_vec();
  void dump(std::ostream& o);

  void dump_graphviz(std::ostream& o);

  long int count();

  bool leaf();

 private:
  double score;
  string entry;
  TreeByScore* left = nullptr;
  TreeByScore* right = nullptr;

  TreeByScore minimum();
  TreeByScore maximum();

  void add_left(double s, string e);
  void add_right(double s, string e);

  void remove_self();
  void remove_left(double s, string e);
  void remove_right(double s, string e);

  bool member_left(double s, string e);
  bool member_right(double s, string e);

  void add_to_vec(std::vector<ZScorePair>& running);
};
