#include "item.hpp"
#include <istream>
using namespace models;
const size_t MAX_CONTENT_LEN = 1024;
Item::Item(const List &l, std::string id_str)
    : content(MAX_CONTENT_LEN, '\0'), list_id(l.id), _list(l) {
  id = {id_str};
  std::ifstream reader = {l.dir() / id_str, std::ios_base::in};
  if (!reader.good())
    throw MontagueSystemError();
  char complete_sigil;
  reader >> complete_sigil;
  reader.read(content.data(), MAX_CONTENT_LEN);
  content.resize(reader.gcount());
  complete = ('1' == complete_sigil);
}
Item Item::create(const List &l, std::string content) {
  Item candidate = {l, content, Uuid()};
  candidate.save();
  return candidate;
}
std::optional<Item> Item::find(const List &l, std::string id_str) {
  std::ifstream candidate_reader = {l.dir() / id_str,
                                    std::ios_base::in | std::ios_base::ate};
  if (!candidate_reader.good())
    return {};
  if (0 == candidate_reader.tellg())
    return {};
  return {{l, id_str}};
}
void Item::save() {
  std::ofstream item_writer = {_list.dir() / to_string(id),
                               std::ios_base::out | std::ios_base::trunc};
  char complete_sigil = complete ? '1' : '0';
  item_writer << complete_sigil << content;
}
