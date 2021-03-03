#include "else.hpp"
#include "../error.hpp"
using namespace mtl;
using namespace mtl::tag;
const std::regex else_parser("else");
std::optional<Else> Else::try_make(std::string_view t) {
  if (!std::regex_match(std::string(t), else_parser))
    return {};
  return {Else{}};
}
void Else::resolve(TagBag &_remain) { throw UnresolvableError("else"); }
void Else::render(std::shared_ptr<mtl::Context> _ctx, SlugBag &_dest) {
  throw UnrenderableError("else");
}
std::string Else::inspect() const { return "tag::Else{}"; }
