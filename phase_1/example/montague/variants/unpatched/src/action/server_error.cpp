#include "server_error.hpp"
using namespace action;
void ServerError::process(mtl::Context &ctx) {
  ctx.assign("except", "unknown error");
}
void ServerError::process(mtl::Context &ctx, const std::runtime_error &err) {
  ctx.assign("except", err.what());
}
std::filesystem::path ServerError::template_name() const {
  return "500.html.mtl";
}
