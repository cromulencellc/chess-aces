#pragma once
#include "../tag.hpp"
#include "endeach.hpp"
namespace mtl {
namespace tag {
class Each : public Base {
public:
  static std::optional<Each> try_make(std::string_view t);
  Each(std::string t, std::string c);
  virtual ~Each(){};
  virtual void resolve(TagBag &remain) override;
  virtual void render(std::shared_ptr<Context> ctx,
                      SlugBag &destination) override;
  virtual std::string inspect() const override;
private:
  std::optional<std::string> key_target;
  std::string value_target;
  std::string collection;
  TagBag contents = {};
};
}
}
