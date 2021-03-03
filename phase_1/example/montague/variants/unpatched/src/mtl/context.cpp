#include "context.hpp"
#include <regex>
#include <sstream>
using namespace mtl::context;
const std::regex variable_parser("([a-z_,]+)(\\.(.+))?");
void Base::assign(std::string _key, std::string _value) {
  throw AssignUnimplementedrror();
}
void Base::assign(std::string _key, std::shared_ptr<Base> _value) {
  throw AssignUnimplementedrror();
}
std::shared_ptr<Base> Base::find_something(std::string _rest) {
  return {nullptr};
}
_maybe_result Hash::get(std::string rest) {
  std::smatch got;
  if (!std::regex_match(rest, got, variable_parser)) {
    return {};
  }
  std::string candidate_key = got[1];
  auto take = find(candidate_key);
  if (end() == take) {
    return {};
  }
  if (got[0] == got[1]) {
    std::shared_ptr<Scalar> scl =
        std::dynamic_pointer_cast<Scalar>(take->second);
    if (!scl) {
      throw UnexpectedContextError(candidate_key, take->second->inspect());
    }
    return {*scl};
  }
  std::shared_ptr<Hash> hsh = std::dynamic_pointer_cast<Hash>(take->second);
  if (!hsh) {
    throw UnexpectedContextError(candidate_key, take->second->inspect());
  }
  return hsh->get(got[3]);
}
std::shared_ptr<Base> Hash::find_something(std::string rest) {
  std::smatch got;
  if (!std::regex_match(rest, got, variable_parser)) {
    return {};
  }
  std::string candidate_key = got[1];
  auto take = find(candidate_key);
  if (end() == take) {
    return {};
  }
  if (got[0] == got[1]) {
    return {take->second};
  }
  return take->second->find_something(got[3]);
}
std::string Hash::inspect() const {
  std::stringstream buf;
  buf << "context::Hash{" << std::endl;
  for (auto el : *this) {
    buf << "\t" << el.first << " -> " << el.second << std::endl;
  }
  buf << "}";
  return buf.str();
}
void Hash::assign(std::string key, std::string value) {
  assign(key, std::make_shared<Scalar>(value));
}
void Hash::assign(std::string key, std::shared_ptr<Base> value) {
  insert_or_assign(key, value);
}
std::optional<Hash> Hash::traverse(std::string rest) {
  std::smatch got;
  if (!std::regex_match(rest, got, variable_parser)) {
    return {};
  }
  std::string candidate_key = got[1];
  auto take = find(candidate_key);
  if (end() == take) {
    return {};
  }
  std::shared_ptr<Hash> hsh = std::dynamic_pointer_cast<Hash>(take->second);
  if (!hsh) {
    throw UnexpectedContextError(candidate_key, take->second->inspect());
  }
  if (got[0] == got[1]) {
    return {*hsh};
  }
  return hsh->traverse(got[3]);
}
void Overlay::add(std::string key, std::shared_ptr<Base> value) {
  mine[key] = value;
}
_maybe_result Overlay::get(std::string rest) {
  _maybe_result candidate = mine.get(rest);
  if (candidate.has_value())
    return candidate;
  return underlay.get(rest);
}
std::shared_ptr<Base> Overlay::find_something(std::string rest) {
  auto my_found = mine.find_something(rest);
  if (nullptr != my_found)
    return my_found;
  return underlay.find_something(rest);
}
std::string Overlay::inspect() const {
  std::stringstream buf;
  buf << "context::Overlay{" << std::endl;
  buf << "mine: " << mine.inspect() << std::endl;
  buf << "underlay: " << underlay.inspect() << std::endl;
  buf << "}";
  return buf.str();
}
_maybe_result Scalar::get(std::string rest) {
  throw UnexpectedContextError(rest, this->inspect());
}
std::string Scalar::inspect() const { return "context::Scalar{" + *this + "}"; }
