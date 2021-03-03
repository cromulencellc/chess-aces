#pragma once

#include "cell.hpp"
#include "value.hpp"

#include "../logger.hpp"

#include <typeinfo>
#include <vector>

namespace oqtopus {
  using _ValueVecInner = std::vector<ValuePtr>;
  class ValueVec {
  public:
    ValueVec(int fd);

    ValuePtr resolve() const;

    _ValueVecInner inner;
  private:
    template <typename T>
    void eat(T v) {
      std::shared_ptr<T> got = std::make_shared<T>(v);
      LLL.info() << (ValuePtr) got;
      inner.push_back(got);
    }

    ValuePtr inner_resolve(size_t& cur, size_t end) const;
  };

  class UnexpectedValueError : public OvingtonRuntimeError {
  public:
    UnexpectedValueError(char sigil) :
      OvingtonRuntimeError("got unexpected oqtopus value sigil " +
                           std::to_string((int)sigil))
    {}
  };

  class UnexpectedCloselistError : public OvingtonRuntimeError {
  public:
    UnexpectedCloselistError() :
      OvingtonRuntimeError("unexpected ')'")
    {}
  };

  class UnterminatedListError : public OvingtonRuntimeError {
  public:
    UnterminatedListError() :
      OvingtonRuntimeError("list didn't terminate correctly, expected ')'")
    {}
  };
}

std::ostream& operator<<(std::ostream& os, oqtopus::ValueVec vv);
