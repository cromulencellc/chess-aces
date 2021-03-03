#include "endeach.hpp"
#include "../error.hpp"
using namespace mtl;
using namespace mtl::tag;
const std::regex endeach_parser("endeach");
std::optional<Endeach> Endeach::try_make(std::string_view t) {
  if (!std::regex_match(std::string(t), endeach_parser))
    return {};
  return {Endeach{}};
}
void Endeach::resolve(TagBag &_remain) { throw UnresolvableError("endeach"); }
void Endeach::render(std::shared_ptr<mtl::Context> _ctx, SlugBag &_dest) {
  throw UnrenderableError("endeach");
}
std::string Endeach::inspect() const { return "tag::Endeach{}"; }
