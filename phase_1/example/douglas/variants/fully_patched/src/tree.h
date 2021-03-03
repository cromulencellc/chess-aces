#pragma once

#include <iostream>
#include <string>
#include <vector>

using std::string;

class Tree {
 public:
  Tree(string e) : entry(e){};
  Tree(const Tree& o) : entry(o.entry), left(o.left), right(o.right){};
  Tree(std::vector<string>);

  ~Tree();

  void add(string candidate);
  void remove(string candidate);

  bool member(string candidate);

  std::vector<string> to_vec();
  void dump(std::ostream& o);

  long int count();

  bool leaf();

 private:
  string entry;
  Tree* left = nullptr;
  Tree* right = nullptr;

  string minimum();
  string maximum();

  void add_left(string candidate);
  void add_right(string candidate);

  void remove_self();
  void remove_left(string candidate);
  void remove_right(string candidate);

  bool member_left(string candidate);
  bool member_right(string candidate);

  void add_to_vec(std::vector<string>* running);
};
