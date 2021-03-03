#include "eval.hpp"

#include "environment.hpp"
#include "row_lambda.hpp"
#include "../logger.hpp"

using namespace oqtopus;

ValuePtr reverse(ValuePtr to_reverse, ValuePtr reversed) {
  if (nullptr == to_reverse) return reversed;

  return reverse(to_reverse->cdr(),
                 std::make_shared<Cell>(to_reverse->car(), reversed));
}

ValuePtr eval_args_inner(ValuePtr remain_args, ValuePtr done_args,
                         Environment& env) {
  if (nullptr == remain_args) return reverse(done_args, nullptr);

  ValuePtr evald = eval(remain_args->car(), env);

  // LLL.info() << remain_args->car() << " becomes " << evald;

  ValuePtr new_done = std::make_shared<Cell>(evald, done_args);
  return eval_args_inner(remain_args->cdr(), new_done, env);

}

ValuePtr eval_args(ValuePtr args, Environment& env) {
  return eval_args_inner(args, nullptr, env);
}

ValuePtr eval_proc(ValuePtr proc, ValuePtr args, Environment& _env) {
  if (std::shared_ptr<NativeLambda> nl =
      std::dynamic_pointer_cast<NativeLambda>(proc)) {
    return (*nl)(args);
  }

  LLL.error() << "failing to eval " << proc << " with args " << args;
  throw EvalError("couldn't find proc " + proc->inspect());
}

bool wants_builtin(std::string keyword,
                ValuePtr args) {
  if (!args->is_cell()) return false;
  std::shared_ptr<value::Keyword> possible_keyword;
  if (nullptr ==
      (possible_keyword =
       std::dynamic_pointer_cast<value::Keyword>(args->car())))
    return false;

  if (keyword != possible_keyword->to_keyword()) return false;

  return true;
}

bool is_truthy(ValuePtr thing) {
  if (nullptr == thing) return false; // null
  std::shared_ptr<Cell> maybe_cell =
    std::dynamic_pointer_cast<Cell>(thing);

  if (nullptr != maybe_cell) {
    if (nullptr != maybe_cell->car()) return true;
    if (nullptr != maybe_cell->cdr()) return true;
    return false; // empty list/cons cell
  }

  return true;
}

ValuePtr do_if(ValuePtr args, Environment& env) {
  if ((nullptr == args) ||
      (nullptr == args->car()) ||
      (nullptr == args->cdr()) ||
      (nullptr == args->cdr()->car())) {
    throw ArgumentError("`if` needs predicate & true-leg, maybe false-leg");
  }
  LLL.debug() << args;
  ValuePtr predicate = args->car();
  ValuePtr result = eval(predicate, env);
  LLL.debug() << result;
  ValuePtr true_leg = args->cdr()->car();
  if (nullptr == true_leg) throw EvalError("missing truthy leg on an if");
  if (is_truthy(result)) {
    return eval(true_leg, env);
  } else {
    if (nullptr == args->cdr()->cdr()) return result;
    ValuePtr false_leg = args->cdr()->cdr();
    if (nullptr == false_leg) return result;
    false_leg = false_leg->car();
    if (nullptr == false_leg) return result;
    return eval(false_leg, env);
  }
}

ValuePtr do_cond(ValuePtr args, Environment& env) {
  if (nullptr == args) return mk_cell(); // nothing was true

  ValuePtr predicate = args->car();
  if (nullptr == predicate) {
    throw ArgumentError("malformed `cond` predicate");
  }
  ValuePtr result = eval(predicate, env);

  if ((nullptr == args->cdr()) ||
      (nullptr == args->cdr()->car())) {
    throw ArgumentError("malformed `cond` true-leg");
  }
  ValuePtr true_leg = args->cdr()->car();

  if (is_truthy(result)) {
    return eval(true_leg, env);
  }

  return do_cond(args->cdr()->cdr(), env);
}

ValuePtr do_row_lambda(ValuePtr args, Environment& env) {
  if (nullptr == args) {
    throw ArgumentError("`row-lambda` expects a body");
  }
  return std::make_shared<RowLambda>(args->car(), env);
}

ValuePtr do_set(ValuePtr args, Environment& env) {
  if (nullptr == args) {
    throw ArgumentError("`set` expects a name and value, got nothin'");
  }
  if (nullptr == std::dynamic_pointer_cast<value::Keyword>(args->car())) {
    throw ArgumentError("`set` expects a keyword for the name");
  }
  ValuePtr name = args->car();

  if (nullptr == args->cdr()) {
    throw ArgumentError("`set` expects a value");
  }
  ValuePtr result = eval(args->cdr()->car(), env);
  env.set(name->to_keyword(), result);
  return name;
}

ValuePtr oqtopus::eval(ValuePtr args, Environment& env) {

  if (std::shared_ptr<value::Keyword> kw =
      std::dynamic_pointer_cast<value::Keyword>(args)) {
    ValuePtr got = env[kw->to_keyword()];
    if (nullptr == got) {
      throw EvalError("keyword " + kw->to_keyword() + " not found in env");
    }
    return got;
  }

  if (args->is_atom()) {
    return args;
  }

  if (!args->is_cell()) {
    throw EvalError("couldn't eval non-cell " + args->inspect());
  }

  if (nullptr == args->car()) {
    throw EvalError("expected to find keyword, got nullptr");
  }

  std::shared_ptr<value::Keyword> kw =
    std::dynamic_pointer_cast<value::Keyword>(args->car());

  if (nullptr == kw) {
    throw EvalError("expected to find keyword, got " + args->car()->inspect());
  }

  if (kw->is_keyword("quote")) {
    if (nullptr == args->cdr()) {
      throw ArgumentError("`quote` needs an argument");
    }
    return args->cdr()->car();
  }
  if (kw->is_keyword("if")) return do_if(args->cdr(), env);
  if (kw->is_keyword("cond")) return do_cond(args->cdr(), env);
  if (kw->is_keyword("row-lambda")) return do_row_lambda(args->cdr(), env);
  if (kw->is_keyword("set")) return do_set(args->cdr(), env);

  // base-case, it's a function

  ValuePtr callable = eval(args->car(), env);
  LLL.info() << callable;
  ValuePtr evald_args = eval_args(args->cdr(), env);
  LLL.info() << evald_args;
  return eval_proc(callable, evald_args, env);

}
