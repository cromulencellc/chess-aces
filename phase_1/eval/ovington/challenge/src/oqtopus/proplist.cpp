#include "proplist.hpp"

#include "../logger.hpp"

using namespace oqtopus;
using namespace oqtopus::proplist;

ValuePtr proplist::proplist_get(ValuePtr pl, value::Keyword key) {
  if (nullptr == pl) return nullptr;

  ValuePtr this_entry = pl->car();

  std::shared_ptr<value::Keyword> this_keyword =
    std::dynamic_pointer_cast<value::Keyword>(this_entry->car());

  if (nullptr == this_keyword) {
    throw OvingtonRuntimeError("couldn't proplist_get on " + pl->inspect());
  }

  if (this_keyword->is_keyword(key.to_keyword())) {
    return this_entry->cdr()->car();
  }

  return proplist_get(pl->cdr(), key);
}

ValuePtr proplist::proplist_set(ValuePtr pl,
                                value::Keyword key, ValuePtr value) {
  std::shared_ptr<Cell> this_entry =
    mk_cell(std::make_shared<value::Keyword>(key),
            mk_cell(value, nullptr));

  return std::make_shared<Cell>(this_entry, pl);
}

ValuePtr proplist::proplist_set(ValuePtr pl, std::string key, int64_t value) {
  return proplist_set(pl,
                      value::Keyword(key),
                      std::make_shared<value::Sint64>
                      ((unsigned long long) value));
}

ValuePtr proplist::proplist_set(ValuePtr pl, std::string key, float64_t value) {
  return proplist_set(pl,
                      value::Keyword(key),
                      std::make_shared<value::Float64>(value));
}
