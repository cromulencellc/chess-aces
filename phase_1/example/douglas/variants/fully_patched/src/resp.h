#pragma once

#include <string>
#include <vector>

class RespEntry {
 public:
  RespEntry(std::istream& r);

  RespEntry(std::string new_str) : kind(string), string_val(new_str){};
  RespEntry(long int new_int) : kind(integer), int_val(new_int){};
  RespEntry(std::vector<RespEntry> new_arr) : kind(array), arr_val(new_arr){};
  RespEntry(double new_float)
      : kind(string), string_val(std::to_string(new_float)){};
  RespEntry(std::nullptr_t nullp) : kind(null){};

  RespEntry(const RespEntry& o)
      : kind(o.kind),
        string_val(o.string_val),
        int_val(o.int_val),
        arr_val(o.arr_val){};
  RespEntry() : kind(null){};

  std::string first_string();
  std::string first_string_upcase();

  std::string get_string();
  long int get_integer();
  std::vector<RespEntry> get_array();

  std::string cast_string();
  long int cast_integer();
  double cast_double();

  void dump(std::ostream& o);

  enum _kind { string, integer, array, null };
  _kind kind;
  std::string string_val;
  long int int_val;
  std::vector<RespEntry> arr_val;

 private:
  void parse_simple_string(std::istream& r);
  void parse_integer(std::istream& r);
  void parse_bulk_string(std::istream& r);
  void parse_array(std::istream& r);

  void expect_lf(std::istream& r);
  void expect_crlf(std::istream& r);

  void dump_string(std::ostream& w);
  void dump_integer(std::ostream& w);
  void dump_array(std::ostream& w);
  void dump_null(std::ostream& w);
};
