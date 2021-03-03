#include "tag.hpp"
#include "error.hpp"
#include "tag/each.hpp"
#include "tag/else.hpp"
#include "tag/endeach.hpp"
#include "tag/endif.hpp"
#include "tag/if.hpp"
#include "tag/interp.hpp"
#include "tag/static.hpp"
using namespace mtl;
using namespace mtl::tag;
std::ostream &operator<<(std::ostream &o, const Base &t) {
  o << t.inspect();
  return o;
}
#define PPP(klass)                                                             \
  {                                                                            \
    std::optional<klass> c = klass::try_make(t);                               \
    if (c.has_value())                                                         \
      return std::make_shared<klass>(c.value());                               \
  }
std::shared_ptr<Base> Base::parse(std::string_view t) {
  PPP(Each);
  PPP(Else);
  PPP(Endeach);
  PPP(Interp);
  PPP(If);
  PPP(Endif);
  return std::make_shared<Invalid>(Invalid{t});
}
std::shared_ptr<Base> Base::make_static(std::string_view t) {
  return std::make_shared<Static>(Static{t});
}
std::string Invalid::inspect() const { return "tag::Invalid{" + content + "}"; }
void Invalid::render(std::shared_ptr<mtl::Context> _ctx, SlugBag &dest) {
  dest.push_back(std::make_shared<slug::View>("Invalid tag "));
  dest.push_back(std::make_shared<slug::View>(content));
}
