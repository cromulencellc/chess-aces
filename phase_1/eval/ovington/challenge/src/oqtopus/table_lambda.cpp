#include "table_lambda.hpp"

#include "../logger.hpp"
#include "proplist.hpp"
#include "relation.hpp"
#include "row_lambda.hpp"
#include "../odbc/table.hpp"


#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <utility>
#include <variant>

using namespace std::literals::string_literals;
using namespace oqtopus::value::literals;

using namespace oqtopus::proplist;

using namespace oqtopus;

using ArgE = ArgumentError;
using FuncE = FunctionError;

ValuePtr table_lambda::table(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected string argument to `table`");
  if (nullptr == std::dynamic_pointer_cast<value::Stringz>(i->car())) {
    throw ArgE("expected string argument to `table`");
  }

  std::string table_name = i->car()->to_string();

  relation::Table table = {table_name};

  return table.represent();
}

ValuePtr table_lambda::count_rows(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `count-rows`");

  RelationPtr rel = Relation::de_represent(i->car());
  return std::make_shared<value::Sint64>
    ((unsigned long long)rel->row_count());
}

void cells_to_environment(Environment& e, ValuePtr cell, ValuePtr def) {
  if (nullptr == cell) return;
  if ((nullptr == def) ||
      (nullptr == def->car()) ||
      (nullptr == def->car()->cdr())) {
    throw FuncE("malformed defs");
  }

  ValuePtr cur_def_name = def->car()->cdr();

  if (nullptr == std::dynamic_pointer_cast<value::Stringz>(cur_def_name)) {
    throw FuncE("malformed defs: wanted all defs to have string names");
  }
  e.set(cur_def_name->to_string(),
        cell->car());
  return cells_to_environment(e,
                              cell->cdr(), def->cdr());
}

ValuePtr table_lambda::detect(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `detect`");
  RelationPtr rel = Relation::de_represent(i->car());
  if ((nullptr == i->cdr()) ||
      (nullptr == i->cdr()->car())) {
    throw ArgE("expected row lambda argument to `detect`");
  }
  ValuePtr expected_row_lambda = i->cdr()->car();
  std::shared_ptr<RowLambda> row_lambda =
    std::dynamic_pointer_cast<RowLambda>(expected_row_lambda);

  if (nullptr == row_lambda) {
    throw ArgE("expected row lambda argument to `detect`");
  }

  for (ValuePtr r : rel->rows()) {
    Environment e;

    cells_to_environment(e, r, rel->defs());

    ValuePtr got = row_lambda->call(e);

    if (got && got->is_truthy()) {
      return r;
    }
  }

  LLL.info() << "detected nothing";
  return std::make_shared<Cell>();
}

ValuePtr table_lambda::select(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `select`");
  RelationPtr rel = Relation::de_represent(i->car());
  if ((nullptr == i->cdr()) ||
      (nullptr == i->cdr()->car())) {
    throw ArgE("expected row lambda argument to `select`");
  }

  ValuePtr expected_row_lambda = i->cdr()->car();
  std::shared_ptr<RowLambda> row_lambda =
    std::dynamic_pointer_cast<RowLambda>(expected_row_lambda);

  if (nullptr == row_lambda) {
    throw ArgE("expected row lambda argument to `detect`");
  }
  relation::Temporary selected = {*rel};

  for (ValuePtr r : rel->rows()) {
    Environment e;
    cells_to_environment(e, r, rel->defs());

    ValuePtr got = row_lambda->call(e);

    if (got && got->is_truthy()) {
      selected.add_row(r);
    }
  }

  return selected.represent();
}

ValuePtr table_lambda::all(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `all`");
  RelationPtr rel = Relation::de_represent(i->car());

  std::shared_ptr<RowLambda> always =
    std::make_shared<RowLambda>("true"_ok,
                                Environment());

  return table_lambda::select(mk_cell(rel->represent(),
                                      mk_cell(always, nullptr)));
}

ValuePtr table_lambda::rows(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `rows`");
  RelationPtr rel = Relation::de_represent(i->car());

  if (0 == rel->row_count()) return mk_cell();

  CellPtr first_row = mk_cell();
  CellPtr last;
  CellPtr running = first_row;

  for (ValuePtr row : rel->rows()) {
    CellPtr next_row = mk_cell();
    running->axr = row;
    running->dxr = next_row;
    last = running;
    running = next_row;
  }

  last->dxr = nullptr;

  return first_row;
}

ValuePtr table_lambda::defs(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `defs`");
  RelationPtr rel = Relation::de_represent(i->car());

  return (rel->defs());
}

using SortKey = std::variant<int64_t, float64_t>;
using SortGubbin = std::pair<ValuePtr, SortKey>;

std::ostream& operator<<(std::ostream& o, SortKey k) {
  if (0 == k.index()) o << std::get<0>(k);
  if (1 == k.index()) o << std::get<1>(k);

  return o;
}

ValuePtr table_lambda::sort_by(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `sort-by`");
  RelationPtr rel = Relation::de_represent(i->car());
  if ((nullptr == i->cdr()) ||
      (nullptr == i->cdr()->car())) {
    throw ArgE("expected row lambda argument to `sort-by`");
  }

  ValuePtr expected_row_lambda = i->cdr()->car();
  std::shared_ptr<RowLambda> row_lambda =
    std::dynamic_pointer_cast<RowLambda>(expected_row_lambda);

  if (nullptr == row_lambda) {
    throw ArgE("expected row lambda argument to `sort-by`");
  }

  std::vector<SortGubbin> gubbins;
  for (ValuePtr row : rel->rows()) {
    Environment e;
    cells_to_environment(e, row, rel->defs());

    ValuePtr got = row_lambda->call(e);

    if (nullptr == got) {
      throw FuncE("sort key was nullptr but wanted sint64 or float64");
    }

    SortKey k;
    if (got->is_float()) {
      k = got->to_float();
    } else if (got->is_int()) {
      k = got->to_int();
    } else {
      throw FuncE("sort key wasn't sint64 or float64");
    }

    gubbins.push_back({row, k});
  }

  std::sort(gubbins.begin(), gubbins.end(),
            [](SortGubbin l, SortGubbin r) {

              LLL.info() << "comparing " << l.second << " < " << r.second
                         << " = " << (l.second < r.second);
              return l.second < r.second;
            });

  relation::Temporary sorted = {*rel};

  for (SortGubbin g : gubbins) {
    sorted.add_row(g.first);
  }

  return sorted.represent();
}

ValuePtr table_lambda::stats(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `stats`");
  RelationPtr rel = Relation::de_represent(i->car());

  if (0 == rel->row_count()) {
    throw FuncE("expected non-zero row count for `stats`");
  }

  if ((nullptr == i->cdr()) ||
      (nullptr == i->cdr()->car())) {
    throw ArgE("expected row lambda argument to `stats`");
  }
  ValuePtr expected_row_lambda = i->cdr()->car();
  std::shared_ptr<RowLambda> row_lambda =
    std::dynamic_pointer_cast<RowLambda>(expected_row_lambda);

  if (nullptr == row_lambda) {
    throw ArgE("expected row lambda argument to `stats");
  }

  std::vector<float64_t> values;
  for (ValuePtr row : rel->rows()) {
    Environment e;
    cells_to_environment(e, row, rel->defs());

    ValuePtr got = row_lambda->call(e);

    if (nullptr == got) {
      throw FuncE("expected row lambda to not return nullptr");
    }

    if ((! got->is_float()) && (! got->is_int())) {
      throw FuncE("expected row lambda to return float or int");
    }

    values.push_back(got->cast_float());
  }

  std::sort(values.begin(), values.end());

  // for (float64_t v : values) {
  //   LLL.info() << v;
  // }

  size_t count = values.size();

  if (0 == count) {
    throw FuncE("expected non-zero row count for `stats`");
  }

  float64_t mean = std::accumulate(values.begin(),
                                   values.end(),
                                   0.0) / count;

  size_t midpoint = count / 2;
  float64_t median = values[midpoint];

  float64_t squared_differences =
    std::accumulate(values.begin(), values.end(), 0.0,
                          [mean](float64_t memo, float64_t obj) {
                            float64_t d = mean - obj;
                            return memo + (d * d);
                          });

  // LLL.info() << squared_differences;

  float64_t stddev = sqrt(squared_differences / (count - 1));

  ValuePtr pl = nullptr;
  pl = proplist_set(pl, "mean", mean);
  pl = proplist_set(pl, "count", (int64_t)count);
  pl = proplist_set(pl, "median", median);
  pl = proplist_set(pl, "stddev", stddev);

  return pl;
}

ValuePtr table_lambda::map(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `map`");
  RelationPtr rel = Relation::de_represent(i->car());


  if ((nullptr == i->cdr()) ||
      (nullptr == i->cdr()->car())) {
    throw ArgE("expected row lambda argument to `map`");
  }
  ValuePtr expected_row_lambda = i->cdr()->car();

  std::shared_ptr<RowLambda> row_lambda =
    std::dynamic_pointer_cast<RowLambda>(expected_row_lambda);
  if (nullptr == row_lambda) {
    throw ArgE("expected row lambda argument to `map");
  }

  relation::Temporary dest =
    {
     mk_cell("temp-table"_ok,
             mk_cell(
                     mk_cell(mk_cell("_"_ok, "_"_oz), nullptr),
                     mk_cell(mk_cell(), mk_cell())))
    };

  for (ValuePtr r : rel->rows()) {
    Environment e;
    cells_to_environment(e, r, rel->defs());

    ValuePtr got = row_lambda->call(e);
    dest.add_row(mk_cell(got, nullptr));
  }

  return dest.represent();
}

ValuePtr table_lambda::reduce(ValuePtr i) {
  if (nullptr == i) throw ArgE("expected table argument to `reduce`");
  RelationPtr rel = Relation::de_represent(i->car());

  if ((nullptr == i->cdr()) ||
      (nullptr == i->cdr()->car())) {
    throw ArgE("expected initial value argument to `reduce`");
  }
  ValuePtr running = i->cdr()->car();

  if (nullptr == i->cdr()->cdr()->car()) {
    throw ArgE("expected row lambda argument to `reduce`");
  }
  ValuePtr expected_row_lambda = i->cdr()->cdr()->car();
  std::shared_ptr<RowLambda> row_lambda =
    std::dynamic_pointer_cast<RowLambda>(expected_row_lambda);

  if (nullptr == row_lambda) {
    throw ArgE("expected row lambda argument to `map");
  }

  for (ValuePtr r : rel->rows()) {
    LLL.info() << r;
    Environment e;
    cells_to_environment(e, r, rel->defs());
    e.set("_memo", running);

    running = row_lambda->call(e);
  }

  LLL.info() << running;
  return running;
}
