#pragma once
#include "../action.hpp"
namespace action {
class Static : public Action {
public:
  Static(Request &rq, Params p) : Action(rq, p){};
  virtual void process(mtl::Context &ctx);
  virtual std::filesystem::path template_name() const;
  virtual std::string content_type() const;
  bool not_found = false;
  std::string extension;
};
}
