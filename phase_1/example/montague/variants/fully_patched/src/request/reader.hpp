#pragma once
#include "../error.hpp"
#include <optional>
#include <string>
#include <unistd.h>
namespace request {
class Reader {
public:
  Reader(int i) : fd(i){};
  bool more();
  char get();
  char peek();
  void push_back(char pb);
  std::string read_to(char sigil);
  bool expect(char sigil);

private:
  int fd;
  std::optional<char> held;
  void try_hold_from_fd();
};
class PushBackError : public MontagueRuntimeError {
public:
  PushBackError();
};
class EofError : public MontagueRuntimeError {
public:
  EofError();
};
class ReaderError : public MontagueSystemError {};
}
