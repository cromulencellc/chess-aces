#include "list_create.hpp"
#include "../models/list.hpp"
using namespace action;
using List = models::List;
namespace context = mtl::context;
void ListCreate::process(mtl::Context &ctx) {
  auto body = request.decode_body();
  if (!body.has_value()) {
    throw MontagueRuntimeError("Couldn't create list without body");
  }
  auto name_finder = body->find("name");
  if (body->end() == name_finder) {
    throw MontagueRuntimeError("Couldn't create list without name");
  }
  List l = List::create(name_finder->second);
  std::shared_ptr<context::Hash> list = std::make_shared<context::Hash>();
  list->assign("id", to_string(l.id));
  list->assign("name", l.name);
  list->assign("items", std::make_shared<context::Hash>());
  ctx.assign("list", list);
}
std::filesystem::path ListCreate::template_name() const {
  return "list/show.html.mtl";
}
