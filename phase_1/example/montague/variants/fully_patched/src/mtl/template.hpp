#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <sys/uio.h>
#include "context.hpp"
#include "error.hpp"
#include "slug_bag.hpp"
#include "tag.hpp"
namespace mtl {
class Template {
public:
  Template();
  Template(std::string template_name);
  std::vector<std::shared_ptr<Tag>> tags();
  SlugBag render(std::shared_ptr<Context> ctx);
private:
  std::filesystem::path path;
  bool loaded = false;
  std::string content;
  TagBag _tags;
  void parse_tags();
};
}
std::ostream &operator<<(std::ostream &o, mtl::Template &t);
