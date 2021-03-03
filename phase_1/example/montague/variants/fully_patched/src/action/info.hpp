#pragma once
#include "../action.hpp"
namespace action {
class Info : public Action {
public:
  Info(Request &rq, Params p) : Action(rq, p){};
  virtual void process(mtl::Context &ctx);
  virtual std::filesystem::path template_name() const;
};
}
