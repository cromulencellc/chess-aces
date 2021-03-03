#pragma once
#include "../action.hpp"
namespace action {
class NotFound : public Action {
public:
  NotFound(Request &rq) : Action(rq){};
  virtual ~NotFound(){};
  virtual void process(mtl::Context &ctx);
  virtual std::filesystem::path template_name() const;
};
}
