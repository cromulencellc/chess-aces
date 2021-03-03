#include "native_lambda.hpp"

#include "../logger.hpp"

#include <limits>
#include <sstream>

using namespace oqtopus;
using namespace oqtopus::value::literals;

using ArgE = ArgumentError;
using FuncE = FunctionError;

std::string NativeLambda::inspect() const {
  std::stringstream dest;
#ifndef PATCH_MASK_NL_INSPECT
  dest << "NativeLambda(" << std::hex << (void*)target<ValuePtr(*)(ValuePtr)>();
#else
  dest << "NativeLambda(" << std::hex <<
    (void*)(0xFFFF & (uint64_t)(void*)target<ValuePtr(*)(ValuePtr)>());
#endif
  dest << ")";
  return dest.str();
}

#ifndef PATCH_DISABLE_NL_CAST_TO_FLOAT
float64_t NativeLambda::cast_float() const {
  return *(float64_t*)(void*)target<ValuePtr(*)(ValuePtr)>();
}
#endif

#define twotwo(name, op) \
  template <typename T> \
  T name(T l, T r) { return l op r; }

twotwo(add_two, +);
twotwo(sub_two, -);
twotwo(mul_two, *);
int64_t div_two_i(int64_t l, int64_t r) {
  if (0 == r) throw FuncE("integer divide by zero");
  return l / r;
}
float64_t div_two_f(float64_t l, float64_t r) {
  if (std::abs(0.0 - r) <= std::numeric_limits<float64_t>::epsilon()) {
    throw FuncE("float divide by zero");
  }
  return l / r;
}

ValuePtr rollup_float(ValuePtr i,
                      std::function<float64_t(float64_t, float64_t)> op) {
  float64_t base = i->car()->to_float();
  ValuePtr tail = i->cdr();

  while (nullptr != tail) {
    if (nullptr == tail->car()) {
      throw ArgE("tried to roll up something with a malformed cell");
    }
    base = op(base, tail->car()->cast_float());
    tail = tail->cdr();
  }

  return std::make_shared<value::Float64>(base);
}

ValuePtr rollup_int(ValuePtr i,
                    std::function<int64_t(int64_t, int64_t)> op) {
  int64_t base = i->car()->to_int();
  ValuePtr tail = i->cdr();

  while (nullptr != tail) {
    if (nullptr == tail->car()) {
      throw ArgE("tried to roll up something with a malformed cell");
    }
    base = op(base, tail->car()->cast_int());
    tail = tail->cdr();
  }

  return std::make_shared<value::Sint64>(base);
}

ValuePtr oqtopus::builtin::add(ValuePtr i) {
  if ((nullptr == i) ||
      (nullptr == i->car())) {
    throw ArgE("`add` needs numeric arguments");
  }

  if (i->car()->is_float()) return rollup_float(i, add_two<float64_t>);
  return rollup_int(i, add_two<int64_t>);
}

ValuePtr oqtopus::builtin::subtract(ValuePtr i) {
  if ((nullptr == i) ||
      (nullptr == i->car())) {
    throw ArgE("`subtract` needs numeric arguments");
  }

  if (i->car()->is_float()) return rollup_float(i, sub_two<float64_t>);
  return rollup_int(i, sub_two<int64_t>);
}

ValuePtr oqtopus::builtin::multiply(ValuePtr i) {
  if ((nullptr == i) ||
      (nullptr == i->car())) {
    throw ArgE("`multiply` needs numeric arguments");
  }

  if (i->car()->is_float()) return rollup_float(i, mul_two<float64_t>);
  return rollup_int(i, mul_two<int64_t>);
}

ValuePtr oqtopus::builtin::divide(ValuePtr i) {
  if ((nullptr == i) ||
      (nullptr == i->car())) {
    throw ArgE("`divide` needs numeric arguments");
  }

  if (i->car()->is_float()) {
    return rollup_float(i, div_two_f);
  }
  return rollup_int(i, div_two_i);
}

ValuePtr oqtopus::builtin::car(ValuePtr i) {
  if (nullptr == i) throw ArgE("`car` operates on lists");
  return i->car();
}

ValuePtr oqtopus::builtin::cdr(ValuePtr i) {
  if (nullptr == i) throw ArgE("`cdr` operates on lists");
  return i->cdr();
}

ValuePtr boolify(bool b) {
  if (b) return 1_o;
  return std::make_shared<Cell>();
}

#define int_spaceship(i, j, op) (i->cast_int() op j->cast_int())
#define float_spaceship(i, j, op) (i->cast_float() op j->cast_float())

#define spaceship(i, j, op)             \
  (i->is_float() || j->is_float()) ?    \
  float_spaceship(i, j, op) :           \
  int_spaceship(i, j, op)

#define hyperdrive(op) {                                        \
    if ((nullptr == i) ||                                       \
        (nullptr == i->car()) ||                                \
        (nullptr == i->cdr()) ||                                \
        (nullptr == i->cdr()->car())) {                         \
      throw ArgE("couldn't " #op ", bad arguments");            \
    }                                                           \
    return boolify(spaceship(i->car(), i->cdr()->car(), op));   \
      }

ValuePtr oqtopus::builtin::lt(ValuePtr i) {
  hyperdrive(<);
}

ValuePtr oqtopus::builtin::lteq(ValuePtr i) {
  hyperdrive(<=);
}

ValuePtr oqtopus::builtin::eq(ValuePtr i) {
  hyperdrive(==);
}
ValuePtr oqtopus::builtin::gteq(ValuePtr i) {
  hyperdrive(>=);
}
ValuePtr oqtopus::builtin::gt(ValuePtr i) {
  hyperdrive(>);
}
ValuePtr oqtopus::builtin::neq(ValuePtr i) {
  hyperdrive(!=);
}

ValuePtr oqtopus::builtin::inspect(ValuePtr i) {
  return std::make_shared<value::Stringz>(i->inspect());
}
