#include "environment.hpp"

#include "proplist_lambda.hpp"
#include "native_lambda.hpp"
#include "table_lambda.hpp"

#include <sstream>

using namespace oqtopus;

using namespace std::literals::string_literals;
using namespace oqtopus::value::literals;

#define mknl(fn) std::make_shared<NativeLambda>(fn)

Environment oqtopus::_make_default_environment() {
  return {_Environment_Base({
                             {"+"s, mknl(builtin::add)},
                             {"-"s, mknl(builtin::subtract)},
                             {"*"s, mknl(builtin::multiply)},
                             {"/"s, mknl(builtin::divide)},
                             {"car"s, mknl(builtin::car)},
                             {"cdr"s, mknl(builtin::cdr)},

                             {"<"s, mknl(builtin::lt)},
                             {"<="s, mknl(builtin::lteq)},
                             {"=="s, mknl(builtin::eq)},
                             {">="s, mknl(builtin::gteq)},
                             {">"s, mknl(builtin::gt)},
                             {"!="s, mknl(builtin::neq)},

                             {"inspect"s, mknl(builtin::inspect)},

                             {"true"s, 1_o},
                             {"false"s, std::make_shared<Cell>()},

                             {"table"s, mknl(table_lambda::table)},
                             {"count-rows"s, mknl(table_lambda::count_rows)},
                             {"detect"s, mknl(table_lambda::detect)},
                             {"select"s, mknl(table_lambda::select)},
                             {"all"s, mknl(table_lambda::all)},
                             {"rows"s, mknl(table_lambda::rows)},
                             {"sort-by"s, mknl(table_lambda::sort_by)},
                             {"stats"s, mknl(table_lambda::stats)},
                             {"map"s, mknl(table_lambda::map)},
                             {"reduce"s, mknl(table_lambda::reduce)},
                             {"defs"s, mknl(table_lambda::defs)},

                             {"plget"s, mknl(proplist_lambda::plget)},
                             {"plset"s, mknl(proplist_lambda::plset)}
      })};
}

Environment::Environment() :
  mine({}),
  parent(std::make_shared<Environment>(default_environment)) {};

ValuePtr Environment::operator[](std::string k) {
  auto got = mine.find(k);
  if (mine.end() != got) return got->second;

  if (nullptr == parent) return nullptr;

  return (*parent)[k];
}

void Environment::reparent(Environment& e) {
  parent = std::make_shared<Environment>(e);
}

void Environment::set(std::string k, ValuePtr v) {
  mine.insert_or_assign(k, v);
}

std::string Environment::inspect() const {
  std::stringstream dest;
  dest << "Environment(parent: " << std::hex << parent << std::dec << "\n";
  for (auto pair : mine) {
    dest << "\t" << pair.first << ": " << pair.second << "\n";
  }
  dest << ")";

  return dest.str();
}


std::ostream& operator<<(std::ostream& os, const oqtopus::Environment& e) {
  os << e.inspect();

  return os;
}
