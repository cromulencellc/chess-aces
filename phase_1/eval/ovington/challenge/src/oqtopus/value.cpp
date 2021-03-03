#include "value.hpp"

#include "../checked_io.hpp"
#include "../logger.hpp"

#include <cassert>

using namespace oqtopus;
using namespace oqtopus::value;

#define fail_cast(klass, meth, dest_type)               \
  dest_type klass::meth() const {                       \
    throw ValueCastError(#klass, #meth, #dest_type);    \
  }

fail_cast(Base, to_int, int64_t);
fail_cast(Base, to_float, float64_t);
fail_cast(Base, cast_int, int64_t);
fail_cast(Base, cast_float, float64_t);
fail_cast(Base, to_string, const std::string&);
fail_cast(Base, to_keyword, const std::string&);

fail_cast(Base, car, ValuePtr);
fail_cast(Base, cdr, ValuePtr);

bool Base::is_openlist() const { return false; }
bool Base::is_closelist() const { return false; }
bool Base::is_cell() const { return false; }
bool Base::is_atom() const { return false; }

int64_t Sint64::to_int() const { return value; }
int64_t Sint64::cast_int() const { return value; }
float64_t Sint64::cast_float() const { return (float64_t) value; }

float64_t Float64::to_float() const { return value; }
int64_t Float64::cast_int() const { return (int64_t) value; }
float64_t Float64::cast_float() const { return value; }

const std::string& Stringz::to_string() const { return value; }
const std::string& Keyword::to_keyword() const { return value; }

bool Openlist::is_openlist() const { return true; }
bool Closelist::is_closelist() const { return true; }

bool Base::is_keyword(std::string _cand) const { return false; }

bool Keyword::is_keyword(std::string cand) const {
  return (value == cand);
}

std::string Base::inspect() const {
  return "?oqtopus::value?";
}

void Base::serialize(int _fd) const {
  throw CantSerializeError();
}

Sint64::Sint64(int fd) {
  checked_buf_read(fd, value);
}

Sint64::Sint64(unsigned long long val) {
  int64_t candidate_value = (int64_t)val;
  assert((unsigned long long)candidate_value == val);
  value = candidate_value;
}

bool Sint64::is_atom() const {
  return true;
}

std::string Sint64::inspect() const {
  return std::to_string(value);
}

void Sint64::serialize(int fd) const {
  checked_write(fd, (char)value::Type::sint64);
  checked_write(fd, value);
}

Float64::Float64(int fd) {
  checked_buf_read(fd, value);
}

Float64::Float64(long double val) {
  float64_t candidate_value = (float64_t) val;
  assert((long double)candidate_value == val);
  value = candidate_value;
}

bool Float64::is_atom() const {
  return true;
}

std::string Float64::inspect() const {
  return std::to_string(value);
}

void Float64::serialize(int fd) const {
  //  LLL.info() << std::to_string(value);
  checked_write(fd, (char) value::Type::float64);
  checked_write(fd, value);
}

Stringz::Stringz(int fd) : value() {
  char buf = 0;
  while (true) {
    checked_buf_read(fd, buf);
    if (0 == buf) break;
    value.push_back(buf);
  }
}

bool Stringz::is_atom() const {
  return true;
}

std::string Stringz::inspect() const {
  return "\"" + value + "\"";
}

void Stringz::serialize(int fd) const {
  checked_write(fd, (char) value::Type::stringz);
  checked_write_str(fd, value);
  checked_write(fd, (char) 0);
}

Keyword::Keyword(int fd) : value() {
  char len = 0;
  checked_buf_read(fd, len);
  value.resize(len, 0);

  for ( size_t i = 0; i < len; i++) {
    char buf = 0;
    checked_buf_read(fd, buf);
    value[i] = buf;
  }
}

std::string Keyword::inspect() const {
  return value;
}

void Keyword::serialize(int fd) const {
  checked_write(fd, (char) value::Type::keyword);
  checked_write(fd, (char) value.size());
  checked_write_str(fd, value);
}

std::string Openlist::inspect() const {
  return "(";
}

std::string Closelist::inspect() const {
  return ")";
}

std::ostream& operator<<(std::ostream& os, const oqtopus::ValuePtr v) {
  if (nullptr == v) {
    os << "nullptr";
  } else {
    os << v->inspect();
  }
  return os;
}


std::shared_ptr<Sint64> literals::operator ""_o(unsigned long long v) {
  return std::make_shared<Sint64>(v);
  }
std::shared_ptr<Float64> literals::operator ""_o(long double v) {
  return std::make_shared<Float64>(v);
  }
std::shared_ptr<Stringz> literals::operator ""_oz(const char* v, size_t vl) {
  return std::make_shared<Stringz>(std::string(v, vl));
  }
std::shared_ptr<Keyword> literals::operator ""_ok(const char* v, size_t vl) {
  return std::make_shared<Keyword>(std::string(v, vl));
  }
