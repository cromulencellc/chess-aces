#pragma once
#include "../action.hpp"
namespace action {
class ListCreate : public Action {
public:
  ListCreate(Request &rq, Params p) : Action(rq, p){};
  virtual void process(mtl::Context &ctx);
  virtual std::filesystem::path template_name() const;
};
}
