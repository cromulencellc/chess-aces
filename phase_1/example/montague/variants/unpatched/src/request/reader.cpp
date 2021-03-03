#include "reader.hpp"
using namespace request;
bool Reader::more() {
  try_hold_from_fd();
  if (held.has_value())
    return true;
  return false;
}
char Reader::get() {
  try_hold_from_fd();
  if (held.has_value()) {
    char tmp = held.value();
    held.reset();
    return tmp;
  }
  throw EofError();
}
char Reader::peek() {
  try_hold_from_fd();
  if (held.has_value()) {
    return held.value();
  }
  throw EofError();
}
void Reader::push_back(char pb) {
  if (held.has_value()) {
    throw PushBackError();
  }
  held.emplace(pb);
}
std::string Reader::read_to(char sigil) {
  std::string going = "";
  while (true) {
    try_hold_from_fd();
    if (!held.has_value()) {
      throw EofError();
    }
    if (sigil == *held)
      return going;
    going.push_back(*held);
    held.reset();
  }
}
bool Reader::expect(char sigil) {
  char got = get();
  return (got == sigil);
}
void Reader::try_hold_from_fd() {
  if (held.has_value())
    return;
  char buf = 0;
  ssize_t got = read(fd, &buf, sizeof(buf));
  if (-1 == got) {
    throw ReaderError();
  }
  if (1 == got) {
    held.emplace(buf);
  }
}
PushBackError::PushBackError()
    : MontagueRuntimeError("Cannot push_back more than one byte") {}
EofError::EofError()
    : MontagueRuntimeError("Couldn't read past eof (check if there's more())") {
}
