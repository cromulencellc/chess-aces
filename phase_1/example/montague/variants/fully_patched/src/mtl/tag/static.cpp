#include "static.hpp"
using namespace mtl::tag;
void Static::render(std::shared_ptr<Context> _ctx, SlugBag &dest) {
  auto slug = std::make_shared<slug::String>(content);
  dest.push_back(slug);
}
std::string Static::inspect() const {
  return "tag::Static{len=" + std::to_string(content.size()) + "}";
}
