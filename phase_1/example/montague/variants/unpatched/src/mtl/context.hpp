#pragma once
#include <map>
#include <memory>
#include <optional>
#include <string>
#include "error.hpp"
namespace mtl {
namespace context {
using _maybe_result = std::optional<std::string>;
class Base {
public:
  virtual ~Base() = default;
  virtual _maybe_result get(std::string rest) = 0;
  virtual std::string inspect() const = 0;
  virtual std::shared_ptr<Base> find_something(std::string rest);
  virtual void assign(std::string _key, std::string _value);
  virtual void assign(std::string _key, std::shared_ptr<Base> value);
};
using _Hash_base_class = std::map<std::string, std::shared_ptr<Base>>;
class Hash : public _Hash_base_class, public Base {
public:
  Hash() : _Hash_base_class(){};
  virtual ~Hash() = default;
  virtual _maybe_result get(std::string rest) override;
  virtual std::string inspect() const override;
  virtual std::shared_ptr<Base> find_something(std::string rest) override;
  virtual void assign(std::string key, std::string value) override;
  virtual void assign(std::string key, std::shared_ptr<Base> value) override;
  std::optional<Hash> traverse(std::string rest);
};
using _Scalar_base_class = std::string;
class Scalar : public _Scalar_base_class, public Base {
public:
  Scalar() : _Scalar_base_class(){};
  Scalar(std::string c) : _Scalar_base_class(c){};
  virtual ~Scalar(){};
  virtual _maybe_result get(std::string rest);
  virtual std::string inspect() const;
};
class Overlay : public Base {
public:
  Overlay(Hash h) : underlay(h){};
  void add(std::string key, std::shared_ptr<Base> value);
  virtual _maybe_result get(std::string rest) override;
  virtual std::shared_ptr<Base> find_something(std::string rest) override;
  virtual std::string inspect() const override;

private:
  Hash underlay;
  Hash mine = {};
};
class NoKeyError : public Error {
public:
  NoKeyError(std::string key, std::string collection)
      : Error{"Key " + key + " not found in collection " + collection} {};
};
class UnexpectedContextError : public Error {
public:
  UnexpectedContextError(std::string key, std::string collection)
      : Error{"Key " + key + " had wrong kind of context: " + collection} {};
};
class AssignUnimplementedrror : public Error {
public:
  AssignUnimplementedrror()
      : Error{"Couldn't assign on this context subclass"} {};
};
}
using Context = context::Base;
}
