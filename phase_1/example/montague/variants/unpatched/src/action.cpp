#include "action.hpp"
#include "action/admin.hpp"
#include "action/info.hpp"
#include "action/item_create.hpp"
#include "action/item_toggle.hpp"
#include "action/list_create.hpp"
#include "action/list_index.hpp"
#include "action/list_show.hpp"
#include "action/not_found.hpp"
#include "action/static.hpp"
#include <regex>
#include "router.hpp"
std::unique_ptr<Action> Action::route(Request &request) {
  r(GET, "/", {}, ListIndex);
  r(GET, "/_info", {}, Info);
  r(POST, "/lists", {}, ListCreate);
  r(GET, "/lists/([a-fA-F0-9\\-]+)", {"list_id"}, ListShow);
  r(POST, "/lists/([a-fA-F0-9\\-]+)/items", {"list_id"}, ItemCreate);
  r(POST, "/lists/([a-fA-F0-9\\-]+)/items/([a-fA-F0-9\\-]+)/toggle",
    bx({"list_id", "item_id"}), ItemToggle);
  r(GET, "/admin", {}, Admin);
  r(POST, "/admin", {}, Admin);
  r(GET, "/static/(.+)", {"filename"}, Static);
  return std::make_unique<action::NotFound>(request);
}
std::string Action::content_type() const { return "text/html"; }
