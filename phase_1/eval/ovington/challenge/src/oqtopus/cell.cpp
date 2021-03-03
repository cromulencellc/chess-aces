#include "cell.hpp"

#include "../checked_io.hpp"

using namespace oqtopus;

bool Cell::is_truthy() const {
  if (nullptr != axr) return true;
  if (nullptr != dxr) return true;
  return false;
}

bool Cell::is_cell() const {
  return true;
}

ValuePtr Cell::car() const {
  return axr;
}

ValuePtr Cell::cdr() const {
  return dxr;
}

std::string check_inspect(ValuePtr xr) {
  if (nullptr == xr) return "";
  return xr->inspect();
}

std::string Cell::inspect() const {
  std::string joiner = " ";
  std::string wal = "";
  std::string war = "";
  if ((nullptr == axr) && (nullptr == dxr)) return "()";

  if ((nullptr != dxr) && (!dxr->is_cell())) {
    joiner = " . ";
  }

  if ((nullptr != axr) && (axr->is_cell())) {
    wal = "( ";
    war = ")";
  }

  return wal + check_inspect(axr) + war + joiner + check_inspect(dxr);
}

void Cell::serialize(int fd) const {
  checked_write(fd, (char) value::Type::openlist);

  // if we have no cdr, escape quick
  if (nullptr == cdr()) {
    if (nullptr != car()) car()->serialize(fd);

    checked_write(fd, (char) value::Type::closelist);
    return;
  }

  buddy_serialize(fd);

  checked_write(fd, (char) value::Type::closelist);
}

void Cell::buddy_serialize(int fd) const {
  if (nullptr != car()) car()->serialize(fd);

  std::shared_ptr<Cell> maybe_cell =
    std::dynamic_pointer_cast<Cell>(cdr());

  if (nullptr != maybe_cell) {

    maybe_cell->buddy_serialize(fd);
  } else if (nullptr != cdr()) {
    cdr()->serialize(fd);
  }
}

std::ostream& operator<<(std::ostream& os, const Cell c) {
  os << c.inspect();
  return os;
}
