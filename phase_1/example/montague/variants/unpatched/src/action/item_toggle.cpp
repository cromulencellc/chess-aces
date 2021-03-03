#include "item_toggle.hpp"
#include "../models/item.hpp"
#include "../presenter.hpp"
using namespace action;
using List = models::List;
using Item = models::Item;
namespace context = mtl::context;
void ItemToggle::process(mtl::Context &ctx) {
  list = models::List::find(params["list_id"]);
  if (!list.has_value()) {
    return;
  }
  std::optional<Item> got = Item::find(list.value(), params["item_id"]);
  if (!got.has_value()) {
    return;
  }
  item.emplace(got.value());
  item->complete = !item->complete;
  item->save();
  ctx.assign("list", presenter::list(list.value()));
}
std::filesystem::path ItemToggle::template_name() const {
  if (!item.has_value())
    return "404.html.mtl";
  return "list/show.html.mtl";
}
