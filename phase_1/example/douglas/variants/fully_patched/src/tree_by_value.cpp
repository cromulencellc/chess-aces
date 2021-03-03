#include <iostream>
#include <string>
#include <vector>

#include "assert.h"
#include "log.h"
#include "tree_by_value.h"

TreeByValue::TreeByValue(std::vector<ZMemberPair> list) {
  assert(0 < list.size());

  ZMemberPair e = list[0];
  list.erase(list.begin());

  entry = e.first;
  score = e.second;

  for (ZMemberPair p : list) {
    add(p.first, p.second);
  }
}

TreeByValue::~TreeByValue() {
  if (nullptr != left) {
    delete left;
    left = nullptr;
  }

  if (nullptr != right) {
    delete right;
    right = nullptr;
  }
}

void TreeByValue::add(string e, double s) {
  if (entry == e) {
    score = s;
    return;
  }

  if (entry > e) return add_left(e, s);
  if (entry < e) return add_right(e, s);

  assert(false);
}

void TreeByValue::add_left(string e, double s) {
  if (nullptr == left) {
    left = new TreeByValue(e, s);
  } else {
    left->add(e, s);
  }
}

void TreeByValue::add_right(string e, double s) {
  if (nullptr == right) {
    right = new TreeByValue(e, s);
  } else {
    right->add(e, s);
  }
}

void TreeByValue::remove(string e) {
  if (entry == e) return remove_self();

  if (entry > e) return remove_left(e);
  if (entry < e) return remove_right(e);

  assert(false);
}

void TreeByValue::remove_self() {
  if (leaf()) assert(false);

  if (nullptr != left) {
    TreeByValue replacement_node = left->maximum();
    entry = replacement_node.entry;
    score = replacement_node.score;

    if (left->leaf()) {
      delete left;
      left = nullptr;
      return;
    }

    left->remove(entry);
    return;
  }

  if (nullptr != right) {
    TreeByValue replacement_node = right->minimum();
    entry = replacement_node.entry;
    score = replacement_node.score;

    if (right->leaf()) {
      delete right;
      right = nullptr;
      return;
    }

    right->remove(entry);
    return;
  }

  assert(false);
}

void TreeByValue::remove_left(string e) {
  if (nullptr == left) return;

  if (left->entry != e) return left->remove(e);

  if (left->leaf()) {
    delete left;
    left = nullptr;
    return;
  }

  left->remove(e);
}

void TreeByValue::remove_right(string e) {
  if (nullptr == right) return;

  if (right->entry != e) return right->remove(e);

  if (right->leaf()) {
    delete right;
    right = nullptr;
    return;
  }

  right->remove(e);
}

double TreeByValue::get_score(string e) {
  if (e == entry) return score;

  if ((nullptr != left) && (e < entry)) return left->get_score(e);
  if ((nullptr != right) && (e > entry)) return right->get_score(e);

  assert(false);
}

bool TreeByValue::member(string e) {
  if (e == entry) return true;

  if ((nullptr != left) && (e < entry)) return left->member(e);
  if ((nullptr != right) && (e > entry)) return right->member(e);

  return false;
}

bool TreeByValue::leaf() {
  if (nullptr != left) return false;
  if (nullptr != right) return false;

  return true;
}

TreeByValue TreeByValue::minimum() {
  if (nullptr != left) return left->minimum();

  return *this;
}

TreeByValue TreeByValue::maximum() {
  if (nullptr != right) return right->maximum();

  return *this;
}

long int TreeByValue::count() {
  long int r = 0;
  if (nullptr != left) r += left->count();
  r += 1;
  if (nullptr != right) r += right->count();
  return r;
}
