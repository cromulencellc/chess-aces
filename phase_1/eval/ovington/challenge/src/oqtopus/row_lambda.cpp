#include "row_lambda.hpp"

#include "eval.hpp"

using namespace oqtopus;

ValuePtr RowLambda::call(Environment overlay) {
  overlay.reparent(base_env);

  return oqtopus::eval(body, overlay);
}

std::string RowLambda::inspect() const {
  return "RowLambda(" + body->inspect() + ")";
}
