#include "template.hpp"
#include <filesystem>
#include <fstream>
#include <regex>
using namespace mtl;
const std::regex tag_parser("\\{\\{\\s*(.+?)\\s*\\}\\}");
const size_t MAX_TEMPLATE_SIZE = (1 << 24);
Template::Template() : path(""), loaded(true), content(""), _tags() {}
Template::Template(std::string template_name) {
  const char *template_base = getenv("TEMPLATE_PATH");
  if (nullptr == template_base) {
    template_base = "/data/templates";
  }
  std::filesystem::path base = {template_base};
  path = base / template_name;
}
std::vector<std::shared_ptr<Tag>> Template::tags() {
  if (!loaded)
    parse_tags();
  return _tags;
}
void Template::parse_tags() {
  std::ifstream template_file = {path, std::ios_base::in | std::ios_base::ate};
  if (!template_file.good()) {
    throw TemplateReadError(path);
  }
  auto template_len = template_file.tellg();
  if ((MAX_TEMPLATE_SIZE < template_len) || (0 == template_len)) {
    throw TemplateSizeError(path, template_len);
  }
  content.resize(template_len);
  template_file.seekg(0);
  template_file.read(content.data(), content.size());
  auto first_token =
      std::sregex_iterator(content.begin(), content.end(), tag_parser);
  auto past_final_token = std::sregex_iterator();
  std::size_t last_tag_end = 0;
  TagBag potential_tags = {};
  for (std::sregex_iterator i = first_token; i != past_final_token; ++i) {
    std::smatch token = *i;
    std::size_t cur_tag_start = token.position();
    if (cur_tag_start != last_tag_end) {
      std::string_view before_static = {content.data() + last_tag_end,
                                        cur_tag_start - last_tag_end};
      potential_tags.push_back(Tag::make_static(before_static));
    }
    potential_tags.push_back(Tag::parse(token[1].str()));
    last_tag_end = token.position() + token[0].length();
  }
  if (last_tag_end != (content.size() - 1)) {
    std::string_view final_static = {content.data() + last_tag_end,
                                     content.size() - last_tag_end};
    potential_tags.push_back(Tag::make_static(final_static));
  }
  TagBag remain = {};
  remain.resize(potential_tags.size());
  for (std::size_t i = 0; i < potential_tags.size(); i++) {
    remain[potential_tags.size() - (i + 1)] = potential_tags[i];
  }
  while (remain.size() > 0) {
    std::shared_ptr<Tag> t = remain.back();
    remain.pop_back();
    _tags.push_back(t);
    t->resolve(remain);
  }
  loaded = true;
}
SlugBag Template::render(std::shared_ptr<Context> ctx) {
  LLL.info() << ctx->inspect();
  SlugBag dest;
  for (std::shared_ptr<Tag> t : tags()) {
    t->render(ctx, dest);
  }
  return dest;
}
std::ostream &operator<<(std::ostream &o, mtl::Template &t) {
  o << "Template(" << std::endl;
  for (std::shared_ptr<Tag> t : t.tags()) {
    o << "\t" << *t << std::endl;
  }
  o << ")" << std::endl;
  return o;
}
