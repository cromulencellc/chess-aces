#include <iostream>
#include <string>
#include <vector>

#include "assert.h"
#include "log.h"
#include "tree_by_score.h"

using std::string;

TreeByScore::TreeByScore(std::vector<ZScorePair> list) {
  assert(0 < list.size());

  ZScorePair e = list[0];
  list.erase(list.begin());

  score = e.first;
  entry = e.second;

  for (ZScorePair p : list) {
    add(p.first, p.second);
  }
}

TreeByScore::~TreeByScore() {
  if (nullptr != left) {
    TreeByScore* tmp = left;
    left = nullptr;
    delete tmp;
  }

  if (nullptr != right) {
    TreeByScore* tmp = right;
    right = nullptr;
    delete tmp;
  }
}

void TreeByScore::add(double s, string e) {
  if ((score == s) && (entry == e)) return;

  if (score > s) return add_left(s, e);
  if (score < s) return add_right(s, e);

  if (entry > e) return add_left(s, e);
  if (entry < e) return add_right(s, e);

  assert(false);
}

void TreeByScore::add_left(double s, string e) {
  if (nullptr == left) {
    left = new TreeByScore(s, e);
  } else {
    left->add(s, e);
  }
}

void TreeByScore::add_right(double s, string e) {
  if (nullptr == right) {
    right = new TreeByScore(s, e);
  } else {
    right->add(s, e);
  }
}

void TreeByScore::remove(double s, string e) {
  if ((score == s) && (entry == e)) return remove_self();

  if (score > s) return remove_left(s, e);
  if (score < s) return remove_right(s, e);

  if (entry > e) return remove_left(s, e);
  if (entry < e) return remove_right(s, e);

  assert(false);
}

void TreeByScore::remove_self() {
  if (leaf()) assert(false);

  if (nullptr != left) {
    TreeByScore replacement_node = left->maximum();
    score = replacement_node.score;
    entry = replacement_node.entry;

    if (left->leaf()) {
      TreeByScore* tmp = left;
      left = nullptr;
      delete tmp;
      return;
    }

    left->remove(score, entry);
    return;
  }

  if (nullptr != right) {
    TreeByScore replacement_node = right->minimum();
    score = replacement_node.score;
    entry = replacement_node.entry;

    if (right->leaf()) {
      TreeByScore* tmp = right;
      right = nullptr;
      delete tmp;
      return;
    }

    right->remove(score, entry);
    return;
  }

  assert(false);
}

void TreeByScore::remove_left(double s, string e) {
  if (nullptr == left) return;

  if (left->entry != e) return left->remove(s, e);

  if (left->leaf()) {
    TreeByScore* tmp = left;
    left = nullptr;
    delete tmp;
    return;
  }

  left->remove(s, e);
}

void TreeByScore::remove_right(double s, string e) {
  if (nullptr == right) return;
  if (right->entry != e) return right->remove(s, e);

  if (right->leaf()) {
    TreeByScore* tmp = right;
    right = nullptr;
    delete tmp;
    return;
  }

  right->remove(s, e);
}

bool TreeByScore::member(double s, string e) {
  if ((s == score) && (e == entry)) return true;

  if (s < score) return member_left(s, e);
  if (s > score) return member_right(s, e);

  if (e < entry) return member_left(s, e);
  if (e > entry) return member_right(s, e);

  assert(false);
}

bool TreeByScore::member_left(double s, string e) {
  if (nullptr == left) return false;
  return left->member(s, e);
}

bool TreeByScore::member_right(double s, string e) {
  if (nullptr == right) return false;
  return right->member(s, e);
}

std::vector<ZScorePair> TreeByScore::to_vec() {
  std::vector<ZScorePair> running;
  add_to_vec(running);
  return running;
}

void TreeByScore::add_to_vec(std::vector<ZScorePair>& running) {
  if (nullptr != left) left->add_to_vec(running);
  running.push_back({score, entry});
  if (nullptr != right) right->add_to_vec(running);
}

void TreeByScore::dump(std::ostream& o) {
  if (nullptr != left) left->dump(o);
  o << entry << " (" << score << "), ";
  if (nullptr != right) right->dump(o);
}

void TreeByScore::dump_graphviz(std::ostream& o) {
  o << "x" << std::hex << (long int)(this) << " [label=\"" << (long int)(this)
    << "\\n"
    << score << "\\n"
    << entry << "\"];" << std::endl;
  if (nullptr != left) {
    left->dump_graphviz(o);
    o << "x" << (long int)(this) << " -> "
      << "x" << (long int)(left) << "[label=\"left\"];" << std::endl;
  }
  if (nullptr != right) {
    right->dump_graphviz(o);
    o << "x" << (long int)(this) << " -> "
      << "x" << (long int)(right) << "[label=\"right\"];" << std::endl;
  }
}

long int TreeByScore::count() {
  long int r = 0;
  if (nullptr != left) r += left->count();
  r += 1;
  if (nullptr != right) r += right->count();
  return r;
}

bool TreeByScore::leaf() {
  if (nullptr != left) return false;
  if (nullptr != right) return false;

  return true;
}

TreeByScore TreeByScore::minimum() {
  if (nullptr != left) return left->minimum();

  return *this;
}

TreeByScore TreeByScore::maximum() {
  if (nullptr != right) return right->maximum();

  return *this;
}
