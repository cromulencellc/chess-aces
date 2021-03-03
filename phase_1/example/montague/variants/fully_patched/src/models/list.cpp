#include "item.hpp"
#include "list.hpp"
#include "../logger.hpp"
using namespace models;
const size_t MAX_NAME_LEN = 256;
std::filesystem::path lists_dir = "/data/lists";
std::vector<List> List::all() {
  std::vector<List> ret;
  for (const std::filesystem::directory_entry &e :
       std::filesystem::directory_iterator(lists_dir)) {
    if (!e.is_directory())
      continue;
    ret.push_back({e.path().filename()});
  }
  return ret;
}
List List::create(std::string name) {
  List candidate = {name, Uuid()};
  candidate.save();
  return candidate;
}
std::optional<List> List::find(std::string id) {
  try {
    List got = {id};
    return {got};
  } catch (const MontagueSystemError &mse) {
    return {};
  }
}
List::List(std::string id_str) : name(MAX_NAME_LEN, '\0') {
  id = {id_str};
  std::ifstream name_reader = {dir() / "_name", std::ios_base::in};
  if (!name_reader.good()) {
    throw MontagueSystemError();
  }
  name_reader.read(name.data(), MAX_NAME_LEN);
  size_t read_len = name_reader.gcount();
  name.resize(read_len);
}
std::filesystem::path List::dir() const { return lists_dir / to_string(id); }
void List::save() {
  std::error_code creation_error;
  std::filesystem::create_directory(dir(), creation_error);
  if (creation_error && (std::errc::file_exists == creation_error)) {
    throw MontagueSystemError();
  }
  std::ofstream name_writer = {dir() / "_name", std::ios_base::out};
  name_writer << name;
}
std::vector<std::shared_ptr<Item>> List::items() {
  if (loaded_items)
    return _items;
  for (const std::filesystem::directory_entry &e :
       std::filesystem::directory_iterator(dir())) {
    LLL.info() << "potential item " << e.path();
    if (!e.is_regular_file())
      continue;
    std::filesystem::path filename = e.path().filename();
    if ('_' == filename.string()[0])
      continue;
    _items.push_back(std::make_shared<Item>(*this, filename));
  }
  loaded_items = true;
  return _items;
}
