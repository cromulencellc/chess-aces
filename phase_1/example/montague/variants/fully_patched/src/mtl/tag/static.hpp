#pragma once
#include "../tag.hpp"
namespace mtl {
namespace tag {
class Static : public Base {
public:
  Static(std::string_view c) : content(std::string(c)){};
  ~Static(){};
  virtual void render(std::shared_ptr<Context> _ctx, SlugBag &dest) override;
  virtual std::string inspect() const override;
private:
  std::string content;
};
}
}
