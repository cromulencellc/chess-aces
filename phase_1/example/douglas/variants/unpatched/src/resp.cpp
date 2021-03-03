#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

#include "assert.h"
#include "log.h"
#include "resp.h"

using std::string;

RespEntry::RespEntry(std::istream& r) {
  char sigil;
  r >> std::noskipws >> sigil;
  switch (sigil) {
    case '+':
      parse_simple_string(r);
      return;
    case ':':
      parse_integer(r);
      return;
    case '$':
      parse_bulk_string(r);
      return;
    case '*':
      parse_array(r);
      return;
  }

  if (r.eof()) {
    lll("got eof, exiting\n");
    std::exit(0);
  }

  lll("unknown kind for sigil '%c' (%02x)\n", sigil, sigil);
  assert(false);
}

std::string RespEntry::first_string() {
  switch (kind) {
    case string:
      return string_val;
    case array:
      assert(arr_val.size() > 0);
      return arr_val[0].first_string();
    default:
      lll("expected first string to be a string (or sub-array), got %d\n",
          kind);
      assert(false);
  }
}

std::string RespEntry::first_string_upcase() {
  std::string mixed = first_string();
  std::transform(mixed.begin(), mixed.end(), mixed.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return mixed;
}

std::string RespEntry::get_string() {
  assert(RespEntry::string == kind);
  return string_val;
}

long int RespEntry::get_integer() {
  assert(RespEntry::integer == kind);
  return int_val;
}

std::string RespEntry::cast_string() {
  switch (kind) {
    case string:
      return string_val;
    case integer:
      return std::to_string(int_val);
    default:
      assert(false);
  }
}

long int RespEntry::cast_integer() {
  switch (kind) {
    case string:
      return std::stol(string_val);
    case integer:
      return int_val;
    default:
      assert(false);
  }
}

double RespEntry::cast_double() { return std::stod(get_string()); }

std::vector<RespEntry> RespEntry::get_array() {
  assert(RespEntry::array == kind);
  return arr_val;
}

void RespEntry::dump(std::ostream& o) {
  switch (kind) {
    case string:
      return dump_string(o);
    case integer:
      return dump_integer(o);
    case array:
      return dump_array(o);
    case null:
      return dump_null(o);
    default:
      lll("couldn't dump respentry kind %d\n", (int)kind);
      std::exit(-1);
  }
}

void RespEntry::parse_simple_string(std::istream& r) {
  kind = string;
  std::string got;
  std::getline(r, got, '\r');
  expect_lf(r);
  string_val = got;
}

void RespEntry::parse_integer(std::istream& r) {
  kind = integer;
  long int got;
  r >> std::noskipws >> got;
  kind = integer;
  int_val = got;
  expect_crlf(r);
}

void RespEntry::parse_bulk_string(std::istream& r) {
  kind = string;
  long int len;
  r >> std::noskipws >> len;
  expect_crlf(r);
  if (-1 == len) {
    kind = null;
    return;
  }
  assert(len > 0);
  std::string got;
  got.resize(len);
  r.read(&got[0], len);
  expect_crlf(r);
  string_val = got;
}

void RespEntry::parse_array(std::istream& r) {
  kind = array;
  long int count;
  r >> std::noskipws >> count;
  assert(count >= 0);
  expect_crlf(r);
  arr_val.clear();
  arr_val.reserve(count);
  for (int i = 0; i < count; i++) {
    arr_val.push_back(RespEntry(r));
  }
}

void RespEntry::expect_lf(std::istream& r) {
  char lf;
  r >> std::noskipws >> lf;

  if ('\n' != lf) {
    lll("expected lf (%02x) got %02x\n", '\n', lf);
    std::exit(-1);
  }

  return;
}

void RespEntry::expect_crlf(std::istream& r) {
  char cr, lf;
  r >> std::noskipws >> cr >> lf;

  if (('\r' != cr) || ('\n' != lf)) {
    lll("expected crlf (%02x %02x) got %02x %02x\n", '\r', '\n', cr, lf);
    std::exit(-1);
  }

  return;
}

void RespEntry::dump_string(std::ostream& w) {
  w << '$' << string_val.length() << "\r\n";
  w << string_val;
  w << "\r\n";
}

void RespEntry::dump_integer(std::ostream& w) { w << ':' << int_val << "\r\n"; }

void RespEntry::dump_array(std::ostream& w) {
  w << "*" << arr_val.size() << "\r\n";
  for (int i = 0; i < arr_val.size(); i++) {
    arr_val[i].dump(w);
  }
}

void RespEntry::dump_null(std::ostream& w) { w << "*" << -1 << "\r\n"; }
