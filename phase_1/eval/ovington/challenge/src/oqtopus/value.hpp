#pragma once

#include "../error.hpp"
#include "../float.hpp"

#include <memory>
#include <string>

namespace oqtopus {
  namespace value {
    enum class Type : char {
                            sint64 = 's',
                            float64 = 'f',
                            stringz = 'z',
                            keyword = 'x',
                            openlist = '(',
                            closelist = ')'
    };

    class Base {
    public:
      virtual ~Base() {};

      virtual bool is_truthy() const { return true; }

      virtual bool is_int() const { return false; }
      virtual bool is_float() const { return false; }

      virtual int64_t to_int() const;
      virtual float64_t to_float() const;

      virtual int64_t cast_int() const;
      virtual float64_t cast_float() const;

      virtual const std::string& to_string() const;
      virtual const std::string& to_keyword() const;

      virtual bool is_openlist() const;
      virtual bool is_closelist() const;

      virtual bool is_cell() const;
      virtual bool is_atom() const;
      virtual std::shared_ptr<Base> car() const;
      virtual std::shared_ptr<Base> cdr() const;

      virtual bool is_keyword(std::string _cand) const;

      virtual std::string inspect() const;

      virtual void serialize(int fd) const;
    };

    class Sint64 : public Base {
    public:
      Sint64(int fd);
      Sint64(int64_t val) : value(val) {};
      Sint64(unsigned long long val); // for ""_o literal
      virtual ~Sint64() {};

      virtual bool is_int() const { return true; }
      virtual int64_t to_int() const;
      virtual int64_t cast_int() const;
      virtual float64_t cast_float() const;

      virtual bool is_atom() const;

      virtual std::string inspect() const;

      virtual void serialize(int fd) const;
    private:
      int64_t value;
    };

    class Float64 : public Base {
    public:
      Float64(int fd);
      Float64(float64_t val) : value(val) {};
      Float64(long double val); // for ""_o literal
      virtual ~Float64() {};

      virtual bool is_float() const { return true; }
      virtual float64_t to_float() const;
      virtual int64_t cast_int() const;
      virtual float64_t cast_float() const;

      virtual bool is_atom() const;

      virtual std::string inspect() const;

      virtual void serialize(int fd) const;
    private:
      float64_t value;
    };

    class Stringz : public Base {
    public:
      Stringz(int fd);
      Stringz(std::string val) : value(val) {};
      virtual ~Stringz() {};

      virtual const std::string& to_string() const;

      virtual bool is_atom() const;

      virtual std::string inspect() const;

      virtual void serialize(int fd) const;
    private:
      std::string value;
    };

    class Keyword : public Base {
    public:
      Keyword(int fd);
      Keyword(std::string val) : value(val) {};
      virtual ~Keyword() {};

      virtual const std::string& to_keyword() const;

      virtual bool is_keyword(std::string cand) const;

      virtual std::string inspect() const;
      virtual void serialize(int fd) const;
    private:
      std::string value;
    };

    class Openlist : public Base {
    public:
      virtual bool is_openlist() const;

      virtual std::string inspect() const;
    };

    class Closelist : public Base {
    public:
      virtual bool is_closelist() const;

      virtual std::string inspect() const;
    };

    namespace literals {
      std::shared_ptr<Sint64> operator ""_o(unsigned long long v);
      std::shared_ptr<Float64> operator ""_o(long double v);
      std::shared_ptr<Stringz> operator ""_oz(const char* v, size_t vl);
      std::shared_ptr<Keyword> operator ""_ok(const char* v, size_t vl);
    }
  }

  class ValueCastError : public OvingtonRuntimeError {
  public:
    ValueCastError(std::string klass_name,
                   std::string method_name,
                   std::string destination_type) :
      OvingtonRuntimeError{"couldn't cast a " + klass_name +
                           " to a " + destination_type +
                           " in " + method_name}
    {}
  };


  class CantSerializeError : public OvingtonRuntimeError {
  public:
    CantSerializeError() :
      OvingtonRuntimeError{"couldn't serialize this"}
    {}
  };

  using Value = value::Base;
  using ValuePtr = std::shared_ptr<Value>;
}

std::ostream& operator<<(std::ostream& os, const oqtopus::ValuePtr v);
