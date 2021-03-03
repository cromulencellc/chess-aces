#pragma once
#include "logger.hpp"
#include "mtl/context.hpp"
#include "params.hpp"
#include "request.hpp"
#include <filesystem>
#include <memory>
#include <optional>
class Action {
public:
  static std::unique_ptr<Action> route(Request &rq);
  Action(Request &rq, Params ps) : request(rq), params(ps){};
  Action(Request &rq) : request(rq), params({}){};
  virtual ~Action(){};
  virtual void process(mtl::Context &ctx) = 0;
  virtual std::filesystem::path template_name() const = 0;
  virtual std::string content_type() const;
protected:
  Request &request;
  Params params;
  std::shared_ptr<mtl::context::Scalar> str(std::string s);
};
