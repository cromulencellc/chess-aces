#pragma once

#include "error.hpp"
#include "logger.hpp"

namespace odbc {
  enum class ColumnType : char {
                                sint64 = 's',
                                float64 = 'f',
                                stringz = 'z'
  };

  class OdbcError : public OvingtonRuntimeError {
  public:
    OdbcError() : OvingtonRuntimeError("odbc error") {};
    OdbcError(std::string what) : OvingtonRuntimeError(what) {};
  };
}
