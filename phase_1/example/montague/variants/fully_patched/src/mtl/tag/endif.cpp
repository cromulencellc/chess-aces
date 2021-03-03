#include "endif.hpp"
#include "../error.hpp"
using namespace mtl;
using namespace mtl::tag;
const std::regex endif_parser("endif");
std::optional<Endif> Endif::try_make(std::string_view t) {
  if (!std::regex_match(std::string(t), endif_parser))
    return {};
  return {Endif{}};
}
void Endif::resolve(TagBag &_remain) { throw UnresolvableError("endif"); }
void Endif::render(std::shared_ptr<mtl::Context> _ctx, SlugBag &_dest) {
  throw UnrenderableError("endif");
}
std::string Endif::inspect() const { return "tag::Endif{}"; }
