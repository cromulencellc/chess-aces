#pragma once
#include "../tag.hpp"
namespace mtl {
namespace tag {
class Else : public Base {
public:
  static std::optional<Else> try_make(std::string_view t);
  Else(){};
  virtual ~Else(){};
  virtual void resolve(TagBag &_remain) override;
  virtual void render(std::shared_ptr<mtl::Context> _ctx,
                      SlugBag &_dest) override;
  virtual std::string inspect() const override;
};
}
}
