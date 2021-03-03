#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "io.hpp"

namespace html {
  class PrettyPrinter;

  using PrintFunc = std::function<void(PrettyPrinter&)>;
  using WrapVec = std::vector<std::string_view>;

  class PrettyPrinter {
  public:
    PrettyPrinter(Io& io);
    ~PrettyPrinter();

    void indent(PrintFunc indented);
    void wrap(WrapVec units);
    void word(const std::string_view& unit);

  private:
    Io& _io;
    int _indentation = 0;
    int current_position = 0;

    void end_line();
    void start_line();

    bool enough_space(const std::string_view& unit);

    PrettyPrinter(Io& io, int indentation);
  };
}
