#include "list_show.hpp"
#include "../presenter.hpp"
using namespace action;
namespace context = mtl::context;
void ListShow::process(mtl::Context &ctx) {
  list = models::List::find(params["list_id"]);
  if (!list.has_value()) {
    ctx.assign("path", request.target.path);
    return;
  }
  std::shared_ptr<context::Hash> list_ctx = presenter::list(list.value());
  ctx.assign("list", list_ctx);
}
std::filesystem::path ListShow::template_name() const {
  if (!list.has_value())
    return "404.html.mtl";
  return "list/show.html.mtl";
}
