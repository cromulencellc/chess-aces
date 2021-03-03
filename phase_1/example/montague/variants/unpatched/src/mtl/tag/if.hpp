#pragma once
#include "../tag.hpp"
#include "endif.hpp"
namespace mtl {
namespace tag {
class If : public Base {
public:
  static std::optional<If> try_make(std::string_view t);
  If(std::string p);
  virtual ~If(){};
  virtual void resolve(TagBag &remain) override;
  virtual void render(std::shared_ptr<Context> ctx,
                      SlugBag &destination) override;
  virtual std::string inspect() const override;
private:
  std::string predicand;
  TagBag contents = {};
  TagBag else_contents = {};
  void render_else(std::shared_ptr<Context> ctx, SlugBag &destination);
};
}
}
