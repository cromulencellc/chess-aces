#include "oqtopus.hpp"

#include "logger.hpp"

#include "oqtopus/cell.hpp"
#include "oqtopus/eval.hpp"
#include "oqtopus/valuevec.hpp"

using namespace oqtopus;

void Oqtopus::consume(int in_fd, int out_fd) {
  ValueVec tokens(in_fd);
  LLL.info() << tokens;
  ValuePtr expr = tokens.resolve();
  LLL.info() << expr;

  if (expr->is_keyword("data-load")) {
    want_data_load = true;
    return;
  }

  ValuePtr got = eval(expr, env);
  LLL.info() << got;
  if (nullptr == got) {
    throw EvalError("couldn't return a nullptr");
  }
  got->serialize(out_fd);
}
