#include "item_create.hpp"
#include "../models/item.hpp"
#include "../presenter.hpp"
using namespace action;
using List = models::List;
using Item = models::Item;
namespace context = mtl::context;
void ItemCreate::process(mtl::Context &ctx) {
  std::optional<List> list = models::List::find(params["list_id"]);
  if (!list.has_value()) {
    throw MontagueRuntimeError("Couldn't create item without list");
  }
  auto body = request.decode_body();
  if (!body.has_value()) {
    throw MontagueRuntimeError("Couldn't create item without body");
  }
  auto content_finder = body->find("content");
  if (body->end() == content_finder) {
    throw MontagueRuntimeError("Couldn't create item without body");
  }
  Item i = Item::create(list.value(), content_finder->second);
  ctx.assign("list", presenter::list(list.value()));
}
std::filesystem::path ItemCreate::template_name() const {
  return "list/show.html.mtl";
}
