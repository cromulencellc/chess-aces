#pragma once
#include "../action.hpp"
namespace action {
class Admin : public Action {
public:
  Admin(Request &rq, Params p) : Action(rq, p){};
  virtual void process(mtl::Context &ctx);
  virtual std::filesystem::path template_name() const;
};
}
