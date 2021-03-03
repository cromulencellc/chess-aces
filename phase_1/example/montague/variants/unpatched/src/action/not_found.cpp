#include "not_found.hpp"
using namespace action;
void NotFound::process(mtl::Context &ctx) {
  ctx.assign("path", request.target.path);
}
std::filesystem::path NotFound::template_name() const { return "404.html.mtl"; }
