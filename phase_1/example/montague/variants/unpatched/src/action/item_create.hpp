#pragma once
#include "../action.hpp"
namespace action {
class ItemCreate : public Action {
public:
  ItemCreate(Request &rq, Params p) : Action(rq, p){};
  virtual void process(mtl::Context &ctx);
  virtual std::filesystem::path template_name() const;
};
}
