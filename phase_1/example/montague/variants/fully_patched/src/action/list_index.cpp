#include "list_index.hpp"
#include "../models/list.hpp"
using namespace action;
using List = models::List;
namespace context = mtl::context;
void ListIndex::process(mtl::Context &ctx) {
  std::vector<List> all = List::all();
  std::shared_ptr<context::Hash> lists = std::make_shared<context::Hash>();
  for (List l : all) {
    lists->assign(to_string(l.id), l.name);
  }
  ctx.assign("lists", lists);
}
std::filesystem::path ListIndex::template_name() const {
  return "list/index.html.mtl";
}
