#pragma once

#include "cell.hpp"

namespace oqtopus {
  namespace proplist {
    ValuePtr proplist_get(ValuePtr pl, value::Keyword key);

    ValuePtr proplist_set(ValuePtr pl, value::Keyword key, ValuePtr value);

    ValuePtr proplist_set(ValuePtr pl, std::string key, int64_t value);
    ValuePtr proplist_set(ValuePtr pl, std::string key, float64_t value);
  }
}
