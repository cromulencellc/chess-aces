#include "each.hpp"
#include "endeach.hpp"
using namespace mtl::tag;
const std::regex each_parser("each\\s+([a-z_,]+)\\s+in\\s+([a-z_,.]+)");
const std::regex target_parser("(([a-z_]+),)?([a-z_]+)");
Each::Each(std::string t, std::string c) : collection(c) {
  std::smatch got;
  std::regex_match(t, got, target_parser);
  if (0 == got.size()) {
    throw MalformedTagError("each", t);
  }
  if (2 == got.size()) {
    value_target = got[1];
    return;
  }
  key_target = {got[2]};
  value_target = got[3];
}
std::optional<Each> Each::try_make(std::string_view t) {
  std::smatch got;
  std::string candidate(t);
  if (!std::regex_match(candidate, got, each_parser)) {
    return {};
  }
  return {Each{got[1], got[2]}};
}
void Each::resolve(TagBag &remain) {
  while (remain.size() > 0) {
    std::shared_ptr<Base> tag = remain.back();
    remain.pop_back();
    std::shared_ptr<Endeach> maybe_endeach =
        std::dynamic_pointer_cast<Endeach>(tag);
    if (maybe_endeach) {
      return;
    } else {
      tag->resolve(remain);
      contents.push_back(tag);
    }
  }
}
void Each::render(std::shared_ptr<mtl::Context> ctx, SlugBag &dest) {
  auto given = std::dynamic_pointer_cast<mtl::context::Hash>(ctx);
  if (nullptr == given) {
    throw context::UnexpectedContextError("each", ctx->inspect());
  }
  std::optional<mtl::context::Hash> iterating_collection_candidate =
      given->traverse(collection);
  if (!iterating_collection_candidate.has_value()) {
    throw context::NoKeyError(collection, given->inspect());
  }
  mtl::context::Hash iterating_collection =
      iterating_collection_candidate.value();
  for (auto pair : iterating_collection) {
    auto ol = std::make_shared<mtl::context::Overlay>(*given);
    ol->add(value_target, pair.second);
    if (key_target.has_value()) {
      ol->add(key_target.value(),
              std::make_shared<mtl::context::Scalar>(pair.first));
    }
    for (std::shared_ptr<Tag> t : contents) {
      t->render(ol, dest);
    }
  }
}
std::string Each::inspect() const {
  std::stringstream buf;
  buf << "tag::Each{";
  if (key_target.has_value()) {
    buf << "key-target=" << key_target.value() << " ";
  }
  buf << "value-target=" << value_target << " ";
  buf << "collection=" << collection << std::endl;
  for (std::shared_ptr<Tag> t : contents) {
    buf << "\t\t" << t->inspect() << std::endl;
  }
  buf << "\t}";
  return buf.str();
}
