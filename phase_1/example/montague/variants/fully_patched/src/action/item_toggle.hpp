#pragma once
#include "../action.hpp"
#include "../models/item.hpp"
#include "../models/list.hpp"
namespace action {
class ItemToggle : public Action {
public:
  ItemToggle(Request &rq, Params p) : Action(rq, p){};
  virtual void process(mtl::Context &ctx);
  virtual std::filesystem::path template_name() const;
private:
  std::optional<models::List> list = {};
  std::optional<models::Item> item = {};
};
}
