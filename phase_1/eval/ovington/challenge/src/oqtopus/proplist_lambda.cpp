#include "proplist_lambda.hpp"

#include "proplist.hpp"

using namespace oqtopus::proplist;

using namespace oqtopus;

ValuePtr proplist_lambda::plget(ValuePtr i) {
  std::string key = i->car()->to_string();
  ValuePtr pl = i->cdr()->car();

  return proplist_get(pl, key);
}

ValuePtr proplist_lambda::plset(ValuePtr i) {
  std::string key = i->car()->to_string();
  ValuePtr value = i->cdr()->car();
  ValuePtr pl = i->cdr()->cdr()->car();

  return proplist_set(pl, key, value);
}
