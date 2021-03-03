#pragma once

namespace yaml {
  class Query {
  public:
    Query(std::string query_str);

  private:
    std::vector<OpcodePtr> operations;
  };
}
