#include "interp.hpp"
using namespace mtl::tag;
const std::regex interp_parser("=\\s+([a-z_,\\.]+)");
std::optional<Interp> Interp::try_make(std::string_view t) {
  std::smatch got;
  std::string candidate(t);
  if (!std::regex_match(candidate, got, interp_parser)) {
    return {};
  }
  return {Interp{got[1]}};
}
void Interp::render(std::shared_ptr<Context> ctx, SlugBag &dest) {
  std::optional<std::string> got;
  try {
    got = ctx->get(key);
  } catch (const mtl::Error &tpe) {
    LLL.error() << "MTL template error: " << tpe.what();
    dest.push_back(std::make_shared<slug::String>("MTL template error: "));
    dest.push_back(std::make_shared<slug::String>(tpe.what()));
    return;
  }
  if (!got.has_value())
    return;
  dest.push_back(std::make_shared<slug::String>(got.value()));
  return;
}
std::string Interp::inspect() const { return "tag::Interp{key=" + key + "}"; }
