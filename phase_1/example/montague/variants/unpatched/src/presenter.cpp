#include "presenter.hpp"
using namespace presenter;
SharedHash hhh();
SharedHash presenter::list(models::List l) {
  SharedHash ret = hhh();
  ret->assign("id", to_string(l.id));
  ret->assign("name", l.name);
  SharedHash items = hhh();
  for (std::shared_ptr<models::Item> i : l.items()) {
    items->assign(to_string(i->id), item(*i));
  }
  ret->assign("items", items);
  return ret;
}
SharedHash presenter::item(const models::Item i) {
  SharedHash ret = hhh();
  ret->assign("id", to_string(i.id));
  ret->assign("content", i.content);
  ret->assign("complete", i.complete ? "true" : "false");
  return ret;
}
