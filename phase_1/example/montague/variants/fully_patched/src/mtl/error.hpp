#pragma once
#include <stdexcept>
#include <string>
#include "../error.hpp"
namespace mtl {
class Error : public MontagueRuntimeError {
public:
  Error(std::string explanation) : MontagueRuntimeError(explanation){};
};
class UnresolvableError : public Error {
public:
  UnresolvableError(std::string tag_name)
      : mtl::Error("Tag " + tag_name + " cannot be resolved") {}
};
class UnrenderableError : public Error {
public:
  UnrenderableError(std::string tag_name)
      : mtl::Error{"Tag " + tag_name + " cannot be rendered"} {}
};
class MalformedTagError : public Error {
public:
  MalformedTagError(std::string tag_name, std::string malformed_bit)
      : mtl::Error{"Tag " + tag_name + " is malformed around " +
                   malformed_bit} {}
};
class TemplateReadError : public Error {
public:
  TemplateReadError(std::string path)
      : mtl::Error{"Template " + path + " not good to read."} {}
};
class TemplateSizeError : public Error {
public:
  TemplateSizeError(std::string path, size_t given_size)
      : mtl::Error{"Template " + path + " was too big or small at " +
                   std::to_string(given_size) + " bytes."} {}
};
}
