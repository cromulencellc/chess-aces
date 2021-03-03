#pragma once
#include "../tag.hpp"
namespace mtl {
namespace tag {
class Interp : public Base {
public:
  static std::optional<Interp> try_make(std::string_view t);
  Interp(std::string k) : key(k){};
  ~Interp(){};
  virtual void render(std::shared_ptr<Context> ctx, SlugBag &dest) override;
  virtual std::string inspect() const override;
private:
  std::string key;
};
}
}
