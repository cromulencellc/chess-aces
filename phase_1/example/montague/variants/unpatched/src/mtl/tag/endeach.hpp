#pragma once
#include "../tag.hpp"
namespace mtl {
namespace tag {
class Endeach : public Base {
public:
  static std::optional<Endeach> try_make(std::string_view t);
  Endeach(){};
  virtual ~Endeach(){};
  virtual void resolve(TagBag &_remain) override;
  virtual void render(std::shared_ptr<mtl::Context> _ctx,
                      SlugBag &_dest) override;
  virtual std::string inspect() const override;
};
}
}
