#include "stack.hpp"

#include "error.hpp"

using namespace yaml;

void Stack::push(bool b) { return _Stack_Base::push(b); }
void Stack::push(std::string c) { return _Stack_Base::push(c); }

bool Stack::top_is_string() {
  if (empty()) return false;
  if (std::holds_alternative<std::string>(top())) return true;
  return false;
}

bool Stack::top_is_bool() {
  if (empty()) return false;
  if (std::holds_alternative<bool>(top())) return true;
  return false;
}

std::string Stack::pop_string() {
  assert_not_empty();
  if (!std::holds_alternative<std::string>(top())) {
    throw TypeError("expected string, but it wasn't");
  }

  std::string got = std::get<std::string>(top());
  pop();

  return got;
}

bool Stack::pop_bool() {
  assert_not_empty();
  if (!std::holds_alternative<bool>(top())) {
    throw TypeError("expected bool, but it wasn't");
  }

  bool got = std::get<bool>(top());
  pop();

  return got;
}

void Stack::assert_not_empty() {
  if (! empty()) return;

  throw EmptyStackError();
}
