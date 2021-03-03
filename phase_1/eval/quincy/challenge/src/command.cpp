#include "command.hpp"

#include <sstream>

#include "command/fetch.hpp"
#include "command/parse-doc.hpp"
#include "command/parse-url.hpp"

class InvalidCommand : public command::Base {
public:
  InvalidCommand(std::string name) : _name(name) {};

  virtual bool execute(Io& io, std::string args) override {
    std::stringstream buf;
    buf << "Invalid command `" << _name << "` called with args `"
        << args << "`.";

    LLL.error() << buf.str();
    io.write_str(buf.str());

    return true;
  }

  std::string _name;
};

using namespace command;

#define CCC(nombre, klass) \
  if (nombre == name) { return std::make_unique<klass>(); }

std::unique_ptr<Base> Base::match(std::string name) {
  CCC("parse-doc", ParseDoc);
  if ("parse-url" == name) return std::make_unique<ParseUrl>();
  if ("fetch" == name) return std::make_unique<Fetch>();

  return std::make_unique<InvalidCommand>(name);
}
