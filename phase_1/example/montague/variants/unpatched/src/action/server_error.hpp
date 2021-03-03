#pragma once
#include "../action.hpp"
#include "../error.hpp"
namespace action {
class ServerError : public Action {
public:
  ServerError(Request &rq) : Action(rq){};
  virtual ~ServerError(){};
  virtual void process(mtl::Context &ctx) override;
  virtual void process(mtl::Context &ctx, const std::runtime_error &err);
  virtual std::filesystem::path template_name() const override;
};
}
