#pragma once

#include "value.hpp"

namespace oqtopus {
  class Cell : public value::Base {
  public:
    Cell() : axr(nullptr), dxr(nullptr) {};
    Cell(ValuePtr ar, ValuePtr dr) : axr(ar), dxr(dr) {};
    virtual ~Cell() {};

    virtual bool is_truthy() const;

    virtual bool is_cell() const;
    virtual ValuePtr car() const;
    virtual ValuePtr cdr() const;

    virtual std::string inspect() const;

    virtual void serialize(int fd) const;

    ValuePtr axr;
    ValuePtr dxr;

  protected:
    void buddy_serialize(int fd) const;
  };


  using CellPtr = std::shared_ptr<Cell>;

  inline CellPtr mk_cell(ValuePtr l, ValuePtr r) {
    return std::make_shared<Cell>(l, r);
  }

  inline CellPtr mk_cell() {
    return mk_cell(nullptr, nullptr);
  }

}

std::ostream& operator<<(std::ostream& os, const oqtopus::Cell c);
