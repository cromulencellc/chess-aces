#include "valuevec.hpp"

#include "cell.hpp"
#include "value.hpp"

#include "../logger.hpp"

#include <cassert>
#include <unistd.h>

using namespace oqtopus;

ValueVec::ValueVec(int fd) {
  size_t open_lists = 0;
  do {
    char buf = 0;
    ssize_t got = read(fd, &buf, sizeof(buf));
    if (0 >= got) {
      return;
    }

    value::Type t = (value::Type) buf;

    switch(t) {
    case value::Type::sint64:
      eat(value::Sint64(fd));
      break;
    case value::Type::float64:
      eat(value::Float64(fd));
      break;
    case value::Type::stringz:
      eat(value::Stringz(fd));
      break;
    case value::Type::keyword:
      eat(value::Keyword(fd));
      break;
    case value::Type::openlist:
      eat(value::Openlist());
      open_lists += 1;
      break;
    case value::Type::closelist:
      eat(value::Closelist());
      open_lists -= 1;
      break;
    default:
      throw UnexpectedValueError(buf);
    }
  } while (open_lists > 0);
}

ValuePtr ValueVec::inner_resolve(size_t& cur,
                                 size_t end) const {
  if (end <= cur) throw UnterminatedListError();

  ValuePtr token = inner[cur];
  cur++;

  if (token->is_closelist()) throw UnexpectedCloselistError();

  if (token->is_openlist()) {
    std::shared_ptr<Cell> root = mk_cell();
    std::shared_ptr<Cell> run = root;
    std::shared_ptr<Cell> prev_run = nullptr;
    while ((end > cur) && (! (inner[cur]->is_closelist()))) {
      run->axr = inner_resolve(cur, end);
      std::shared_ptr<Cell> next_cell = mk_cell();
      run->dxr = next_cell;
      prev_run = run;
      run = next_cell;

      //LLL.debug() << *root;

      if (end <= cur) throw UnterminatedListError();
    }

    if (end <= cur) {
      throw OvingtonRuntimeError("weird list");
    }

    if (prev_run != nullptr) prev_run->dxr = nullptr;

    cur++;

    return root;
  }

  return token;
}

ValuePtr ValueVec::resolve() const {
  size_t cur = 0;
  return inner_resolve(cur, inner.size());
}

std::ostream& operator<<(std::ostream& os, oqtopus::ValueVec vv) {
  os << "ValueVec[";
  for (ValuePtr vp : vv.inner) {
    os << vp << std::string(" ");
  }
  os << "]";

  return os;
}
