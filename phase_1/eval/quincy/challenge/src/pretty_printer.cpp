#include "pretty_printer.hpp"

#include "logger.hpp"

using namespace html;

const int terminal_width = 80; // deal with it
const char indenter = ' ';
const char ender = '\n';
const int indent_unit = 2;

PrettyPrinter::PrettyPrinter(Io& io) : _io(io) {
  start_line();
}

PrettyPrinter::PrettyPrinter(Io& io, int indentation) :
  _io(io), _indentation(indentation) {
  start_line();
}

PrettyPrinter::~PrettyPrinter() {
  end_line();
}

void PrettyPrinter::indent(PrintFunc indented) {
  PrettyPrinter child = {_io, _indentation + indent_unit};
  return indented(child);
}

void PrettyPrinter::wrap(WrapVec units) {
  if (0 == units.size()) return;
  start_line();
  for (std::string_view& unit : units) {
    word(unit);
  }
  end_line();
}

void PrettyPrinter::word(const std::string_view& unit) {
  if (!enough_space(unit)) {
    end_line();
    start_line();
  }

  _io.write_str(unit);
  current_position += unit.size();
}

bool PrettyPrinter::enough_space(const std::string_view& unit) {
  if (current_position == _indentation) return true; // start
  int remain = terminal_width - current_position;
  if (remain < 0) return false; // already past end
  if (unit.size() > remain) return false; // would go past end

  return true;
}

void PrettyPrinter::end_line() {
  if (0 == current_position) return;
  _io.write_outof(ender);
  current_position = 0;
}

void PrettyPrinter::start_line() {
  do  {
    _io.write_outof(indenter);
    current_position++;
  } while (current_position < _indentation);
}
