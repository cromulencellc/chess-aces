#pragma once
#include "../model.hpp"
#include "list.hpp"
namespace models {
class Item : public Model {
public:
  Item(const List &l, std::string id_str);
  static Item create(const List &l, std::string c);
  static std::optional<Item> find(const List &l, std::string id_str);
  void save();
  std::string content;
  bool complete;
  Uuid list_id;
private:
  Item(const List &l, std::string c, Uuid i) : content(c), _list(l) { id = i; };
  const List &_list;
};
}
