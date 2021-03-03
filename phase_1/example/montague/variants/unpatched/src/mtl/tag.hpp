#pragma once
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>
#include "../logger.hpp"
#include "context.hpp"
#include "slug.hpp"
#include "slug_bag.hpp"
namespace mtl {
namespace tag {
class Base {
public:
  static std::shared_ptr<Base> parse(std::string_view t);
  static std::shared_ptr<Base> make_static(std::string_view t);
  virtual ~Base() = default;
  virtual void resolve(std::vector<std::shared_ptr<Base>> &remain){};
  virtual void render(std::shared_ptr<mtl::Context> ctx, SlugBag &dest) = 0;
  virtual std::string inspect() const = 0;
};
class Invalid : public Base {
public:
  Invalid(std::string_view c) : content(std::string(c)){};
  Invalid(std::string c) : content(c){};
  virtual ~Invalid() = default;
  virtual void resolve(std::vector<std::shared_ptr<Base>> &_remain){};
  virtual void render(std::shared_ptr<mtl::Context> _ctx, SlugBag &dest);
  virtual std::string inspect() const;
private:
  std::string content;
};
}
using Tag = tag::Base;
using TagBag = std::vector<std::shared_ptr<Tag>>;
}
std::ostream &operator<<(std::ostream &o, const mtl::Tag &t);
