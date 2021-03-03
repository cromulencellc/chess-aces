#pragma once

#include "cell.hpp"

#include "../error.hpp"
#include "../odbc/table.hpp"

#include <vector>

namespace oqtopus {
  namespace relation {
    class Base {
    public:
      static std::shared_ptr<Base> de_represent(ValuePtr representation);

      virtual ValuePtr represent() = 0;
      virtual ValuePtr defs() = 0;
      virtual std::vector<ValuePtr> rows() = 0;
      virtual size_t row_count() = 0;
    };

    class Table : public Base {
    public:
      Table(std::string table_name) : tbl(table_name) {}
      Table(ValuePtr representation) : tbl(derepresent_name(representation)) {};
      virtual ~Table() {};

      virtual ValuePtr represent();

      virtual std::vector<ValuePtr> rows();
      virtual ValuePtr defs();
      virtual size_t row_count();

    private:
      odbc::Table tbl;
      ValuePtr _defs;

      std::string derepresent_name(ValuePtr representation);
    };

    class Temporary : public Base {
    public:
      Temporary(ValuePtr representation); // de-represent
      Temporary(Base& t); // copy defs, not rows

      virtual ~Temporary() {};

      virtual ValuePtr represent();
      virtual ValuePtr defs() { return _defs;}
      virtual std::vector<ValuePtr> rows() { return _rows; }
      virtual size_t row_count();

      void add_row(ValuePtr r);

      ValuePtr represented_rows();

    private:
      std::vector<ValuePtr> _rows;
      ValuePtr _defs;
    };

    class UnknownDerepresentationError : public OvingtonRuntimeError {

    public:
      UnknownDerepresentationError(ValuePtr representation) :
        OvingtonRuntimeError("couldn't figure out how to de-represent " +
                             representation->inspect()) {}
    };
  }

  using Relation = relation::Base;
  using RelationPtr = std::shared_ptr<Relation>;
}
