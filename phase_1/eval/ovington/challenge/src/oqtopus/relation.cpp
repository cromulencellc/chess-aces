#include "relation.hpp"

#include "environment.hpp"
#include "proplist.hpp"

#include <cassert>

using namespace oqtopus;
using namespace oqtopus::relation;

using namespace oqtopus::value::literals;
using namespace oqtopus::proplist;

RelationPtr Base::de_represent(ValuePtr representation) {
  if ((nullptr == representation) ||
      (nullptr == representation->car())) {
    throw UnknownDerepresentationError(representation);
  }

  if (representation->car()->is_keyword("table")) {
    return std::make_shared<Table>(representation);
  }

  if (representation->car()->is_keyword("temp-table")) {
    return std::make_shared<Temporary>(representation);
  }

  throw UnknownDerepresentationError(representation);
}

std::string Table::derepresent_name(ValuePtr representation) {
  if (!representation->car()->is_keyword("table")) {
    throw UnknownDerepresentationError(representation);
  }

  //                                      keyword  defs  meta
  ValuePtr expected_metadata = representation->cdr()->cdr()->car();
  ValuePtr expected_name =
    proplist_get(expected_metadata, value::Keyword("table-name"));

  assert(nullptr != std::dynamic_pointer_cast<value::Stringz>(expected_name));
  return expected_name->to_string();
}

ValuePtr Table::represent() {
  ValuePtr md = nullptr;
  md = proplist_set(md,
                    value::Keyword("row-count"),
                    std::make_shared<value::Sint64>
                    ((unsigned long long)tbl.row_count()));
  md = proplist_set(md,
                    value::Keyword("table-name"),
                    std::make_shared<value::Stringz>
                    (tbl.name));

  return mk_cell("table"_ok,
                 mk_cell(defs(),
                         mk_cell(md, nullptr)));
}

ValuePtr def_to_cell(odbc::Def def) {
  ValuePtr type_name;
  switch (def.type) {
  case odbc::ColumnType::sint64:
    type_name = "sint64"_ok;
    break;
  case odbc::ColumnType::float64:
    type_name = "float64"_ok;
    break;
  case odbc::ColumnType::stringz:
    type_name = "stringz"_ok;
    break;
  }
  ValuePtr name = std::make_shared<oqtopus::value::Stringz>(def.name);

  return std::make_shared<Cell>(type_name, name);
}

ValuePtr listify_defs(const std::vector<odbc::Def>& defs) {
  std::shared_ptr<Cell> first_cell = std::make_shared<Cell>();

  if (0 == defs.size()) return first_cell; // why tho

  std::shared_ptr<Cell> last;
  std::shared_ptr<Cell> running = first_cell;

  for (const odbc::Def d : defs) {
    std::shared_ptr<Cell> next_cell = std::make_shared<Cell>();
    ValuePtr def_cell = def_to_cell(d);
    running->axr = def_cell;
    running->dxr = next_cell;
    last = running;
    running = next_cell;
  }

  last->dxr = nullptr;
  return first_cell;
}

std::vector<ValuePtr> Table::rows() {
  tbl.reader().rewind();
  std::vector<ValuePtr> to_return;

  while (tbl.reader().more_to_read()) {
    Environment e;
    tbl.reader().read_into_env(e);

    CellPtr first_cell = mk_cell();
    CellPtr last;
    CellPtr running = first_cell;
    for (odbc::Def d : tbl.defs()) {
      CellPtr next_cell = mk_cell();
      running->axr = e[d.name];
      running->dxr = next_cell;
      last = running;
      running = next_cell;
    }

    last->dxr = nullptr;
    to_return.push_back(first_cell);
  }

  return to_return;
}

ValuePtr Table::defs() {
  if (nullptr != _defs) return _defs;

  _defs = listify_defs(tbl.defs());

  return _defs;
}

size_t Table::row_count() {
  return tbl.row_count();
}

Temporary::Temporary(ValuePtr representation) {
  if (!representation->car()->is_keyword("temp-table")) {
    throw UnknownDerepresentationError(representation);
  }

  _defs = representation->cdr()->car();

  ValuePtr found_row = representation->cdr()->cdr()->cdr()->car();

  while (nullptr != found_row) {
    add_row(found_row->car());
    found_row = found_row->cdr();
  }
}

Temporary::Temporary(Base& t) : _rows(), _defs(t.defs()) {}

ValuePtr Temporary::represent() {
    ValuePtr md = nullptr;
    md = proplist_set(md,
                      value::Keyword("row-count"),
                      std::make_shared<value::Sint64>
                      ((unsigned long long) _rows.size()));


  return mk_cell("temp-table"_ok,
                 mk_cell(defs(),
                         mk_cell(md,
                                 mk_cell(represented_rows(), nullptr))));
}

size_t Temporary::row_count() {
  return _rows.size();
}

void Temporary::add_row(ValuePtr r) {
  _rows.push_back(r);
}

ValuePtr Temporary::represented_rows() {
  if (0 == row_count()) return mk_cell();

  CellPtr first_row = mk_cell();
  CellPtr last_row;
  CellPtr running = first_row;

  for (ValuePtr r : rows()) {
    CellPtr next_row = mk_cell();
    running->axr = r;
    running->dxr = next_row;
    last_row = running;
    running = next_row;
  }

  last_row->dxr = nullptr;
  return first_row;
}
