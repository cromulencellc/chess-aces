#pragma once
#include "../model.hpp"
namespace models {
class Item;
class List : public Model {
public:
  List(std::string id_str);
  static List create(std::string name);
  static std::vector<List> all();
  static std::optional<List> find(std::string id_str);
  std::filesystem::path dir() const;
  void save();
  std::string name;
  std::vector<std::shared_ptr<Item>> items();
  std::vector<Uuid> item_ids = {};
private:
  List(std::string n, Uuid i) : name(n) { id = i; };
  bool loaded_items = false;
  std::vector<std::shared_ptr<Item>> _items = {};
};
}
