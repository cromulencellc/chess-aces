#include "if.hpp"
#include "else.hpp"
#include "endif.hpp"
using namespace mtl::tag;
const std::regex if_parser("if\\s+([a-z_.]+)");
std::optional<If> If::try_make(std::string_view t) {
  std::smatch got;
  std::string candidate(t);
  if (!std::regex_match(candidate, got, if_parser)) {
    return {};
  }
  return {If{got[1]}};
}
If::If(std::string p) : predicand(p) {}
void If::resolve(TagBag &remain) {
  bool in_else = false;
  while (remain.size() > 0) {
    std::shared_ptr<Base> tag = remain.back();
    remain.pop_back();
    std::shared_ptr<Endif> maybe_endif = std::dynamic_pointer_cast<Endif>(tag);
    if (maybe_endif) {
      return;
    }
    std::shared_ptr<Else> maybe_else = std::dynamic_pointer_cast<Else>(tag);
    if (nullptr != maybe_else) {
      in_else = true;
      continue;
    }
    tag->resolve(remain);
    if (in_else) {
      else_contents.push_back(tag);
    } else {
      contents.push_back(tag);
    }
  }
}
void If::render(std::shared_ptr<mtl::Context> ctx, SlugBag &dest) {
  auto given = std::dynamic_pointer_cast<mtl::context::Hash>(ctx);
  if (nullptr == given) {
    throw context::UnexpectedContextError("if", ctx->inspect());
  }
  std::shared_ptr<mtl::Context> found_predicand =
      ctx->find_something(predicand);
  if (nullptr == found_predicand) {
    render_else(ctx, dest);
    return;
  }
  std::shared_ptr<mtl::context::Hash> maybe_hash_predicand =
      std::dynamic_pointer_cast<mtl::context::Hash>(found_predicand);
  std::shared_ptr<mtl::context::Scalar> maybe_string_predicand =
      std::dynamic_pointer_cast<mtl::context::Scalar>(found_predicand);
  if (maybe_hash_predicand && (0 == maybe_hash_predicand->size())) {
    render_else(ctx, dest);
    return;
  }
  if (maybe_string_predicand && (0 == maybe_string_predicand->size())) {
    render_else(ctx, dest);
    return;
  }
  for (std::shared_ptr<Tag> t : contents) {
    t->render(ctx, dest);
  }
}
void If::render_else(std::shared_ptr<mtl::Context> ctx, SlugBag &dest) {
  for (std::shared_ptr<Tag> t : else_contents) {
    t->render(ctx, dest);
  }
}
std::string If::inspect() const {
  std::stringstream buf;
  buf << "tag::If{predicand=" << predicand << "}";
  return buf.str();
}
