#pragma once
#include "../action.hpp"
#include "../models/list.hpp"
namespace action {
class ListShow : public Action {
public:
  ListShow(Request &rq, Params p) : Action(rq, p){};
  virtual void process(mtl::Context &ctx);
  virtual std::filesystem::path template_name() const;

private:
  std::optional<models::List> list = {};
};
}
