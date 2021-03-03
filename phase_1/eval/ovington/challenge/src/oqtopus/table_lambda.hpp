#pragma once

#include "cell.hpp"

namespace oqtopus {
  namespace table_lambda {
    ValuePtr table(ValuePtr i);
    ValuePtr count_rows(ValuePtr i);
    ValuePtr detect(ValuePtr i);
    ValuePtr select(ValuePtr i);
    ValuePtr all(ValuePtr i);
    ValuePtr rows(ValuePtr i);
    ValuePtr sort_by(ValuePtr i);
    ValuePtr stats(ValuePtr i);
    ValuePtr map(ValuePtr i);
    ValuePtr reduce(ValuePtr i);
    ValuePtr defs(ValuePtr i);
  }
}
