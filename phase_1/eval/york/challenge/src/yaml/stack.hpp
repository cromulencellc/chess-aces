#pragma once

#include <stack>
#include <string>
#include <variant>

namespace yaml {
  using _Stack_Cell = std::variant<std::string, bool>;

  using _Stack_Base = std::stack<_Stack_Cell>;

  class Stack : private _Stack_Base {
  public:
    Stack() : _Stack_Base() {};

    void push(bool b);
    void push(std::string c);

    bool top_is_string();
    bool top_is_bool();

    std::string pop_string();
    bool pop_bool();

    void assert_not_empty();

    size_t size() const { return _Stack_Base::size(); }
  };
}
