#include <iostream>
#include <string>
#include <vector>

#include "assert.h"
#include "log.h"
#include "tree.h"

using std::string;

Tree::Tree(std::vector<string> list) {
  assert(0 < list.size());

  entry = list[0];
  list.erase(list.begin());

  for (string e : list) {
    add(e);
  }
}

Tree::~Tree() {
  if (nullptr != left) {
    delete left;
    left = nullptr;
  }

  if (nullptr != right) {
    delete right;
    right = nullptr;
  }
}

void Tree::add(string candidate) {
  if (entry == candidate) {
    return;
  } else if (candidate < entry) {
    add_left(candidate);
  } else {
    add_right(candidate);
  }
}

void Tree::add_left(string candidate) {
  if (nullptr == left) {
    left = new Tree(candidate);
  } else {
    left->add(candidate);
  }
}

void Tree::add_right(string candidate) {
  if (nullptr == right) {
    right = new Tree(candidate);
  } else {
    right->add(candidate);
  }
}

void Tree::remove(string candidate) {
  if (entry == candidate) {
    return remove_self();
  } else if (candidate < entry) {
    remove_left(candidate);
  } else {
    remove_right(candidate);
  }
}

void Tree::remove_self() {
  if (leaf()) assert(false);

  if (nullptr != left) {
    string replacement = left->maximum();
    entry = replacement;
    if (left->leaf()) {
      delete left;
      left = nullptr;
      return;
    }
    left->remove(replacement);
    return;
  }

  if (nullptr != right) {
    string replacement = right->minimum();
    entry = replacement;
    if (right->leaf()) {
      delete right;
      right = nullptr;
      return;
    }
    right->remove(replacement);
    return;
  }

  assert(false);
}

void Tree::remove_left(string candidate) {
  if (nullptr == left) return;

  if (left->entry != candidate) {
    left->remove(candidate);
    return;
  }

  if (left->leaf()) {
    delete left;
    left = nullptr;
    return;
  }

  left->remove(candidate);
}

void Tree::remove_right(string candidate) {
  if (nullptr == right) return;

  if (right->entry != candidate) {
    right->remove(candidate);
    return;
  }

  if (right->leaf()) {
    delete right;
    right = nullptr;
    return;
  }

  right->remove(candidate);
}

bool Tree::member(string candidate) {
  if (entry == candidate) return true;

  if (entry > candidate) return member_left(candidate);
  if (entry < candidate) return member_right(candidate);

  assert(false);
}

bool Tree::member_left(string candidate) {
  if (nullptr == left) return false;
  return left->member(candidate);
}

bool Tree::member_right(string candidate) {
  if (nullptr == right) return false;
  return right->member(candidate);
}

bool Tree::leaf() {
  if (nullptr != left) return false;
  if (nullptr != right) return false;

  return true;
}

string Tree::minimum() {
  if (nullptr != left) return left->minimum();
  return entry;
}

string Tree::maximum() {
  if (nullptr != right) return right->maximum();
  return entry;
}

std::vector<string> Tree::to_vec() {
  std::vector<string> running;
  add_to_vec(&running);
  return running;
}

void Tree::add_to_vec(std::vector<string>* running) {
  if (nullptr != left) left->add_to_vec(running);
  running->push_back(entry);
  if (nullptr != right) right->add_to_vec(running);
}

void Tree::dump(std::ostream& o) {
  if (nullptr != left) left->dump(o);
  o << entry << ", ";
  if (nullptr != right) right->dump(o);
}

long int Tree::count() {
  long int r = 0;
  if (nullptr != left) r += left->count();
  r += 1;
  if (nullptr != right) r += right->count();
  return r;
}
