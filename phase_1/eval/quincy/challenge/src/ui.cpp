#include "ui.hpp"

#include "command.hpp"

#include <memory>
#include <string>

void Ui::service() {
  io.write_str("quincy\n");

  while (true) {
    io.write_str("> ");
    std::string command_line;
    try {
      command_line = io.read_str_delim('\n');
    } catch (const IncompleteReadError& ire) {
      LLL.info() << "got IncompleteReadError, probably a benign disconnect";
      return;
    } catch (const ReadZeroError& rze) {
      LLL.info() << "got ReadZeroError, probably a benign disconnect";
      return;
    }

    if ('\n' != command_line[command_line.size() - 1]) {
      LLL.error() << "command line too long";
      io.write_str("command line too long, disconnecting");
      return;
    }

    command_line.pop_back();

    size_t first_space = command_line.find_first_of(' ');

    std::string first_word = command_line.substr(0, first_space);
    std::string rest = command_line.substr(first_space + 1); // default to end

    std::unique_ptr<Command> cmd = Command::match(first_word);

    bool keep_going = cmd->execute(io, rest);

    if (! keep_going) return;
  }
}
