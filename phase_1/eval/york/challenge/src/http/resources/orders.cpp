#include "orders.hpp"

#include "../../logger.hpp"
#include "../html_response.hpp"

#include "../../yaml/error.hpp"
#include "../../yaml/table.hpp"
#include "../../yaml/program.hpp"

#include <regex>
#include <sstream>

using namespace http;
using namespace http::resources;

using namespace std::string_literals;

const std::filesystem::path orders_base = "/orders/";

class OrderFinder {
public:
  OrderFinder() {};
  OrderFinder(std::string order_id) :
    did_specify_oid(true),
    oid(order_id) {};

  OrderFinder(std::string order_id, std::string zip) :
    did_specify_oid(true),
    oid(order_id),
    did_specify_postcode(true),
    post(zip) {};

  std::string get_query() const {
    if (did_specify_postcode) return get_postie_query();
    if (did_specify_oid) return get_basic_query();

    return get_all_query();
  }

  yaml::Program get_program() const {
    return yaml::Program(get_query());
  }

private:
  bool did_specify_oid = false;
  std::string oid;
  bool did_specify_postcode = false;
  std::string post;

  std::string get_postie_query() const {
    std::stringstream parts;

    parts << "\"shipping address\0*\""s
#ifdef PATCH_ESCAPING_YAML_QUERIES
          << escape(post)
#else
          << post
#endif
          << "\0~\"billing address\0*\""s
#ifdef PATCH_ESCAPING_YAML_QUERIES
          << escape(post)
#else
          << post
#endif
          << "\0~|"s // stack is (billing or shipping match)
          << "\"order number\0*\""s
#ifdef PATCH_ESCAPING_YAML_QUERIES
          << escape(oid)
#else
          << oid
#endif
          << "\0="s; // stack is (address match) (order eq)

    parts << "&"; // stack is (address match and order eq)

    return parts.str();
  }

  std::string get_basic_query() const {
    std::stringstream parts;
    parts << "\"order number\0*\""s
#ifdef PATCH_ESCAPING_YAML_QUERIES
          << escape(oid)
#else
          << oid
#endif
          << "\0="s;

    return parts.str();
  }

  std::string get_all_query() const {
    return "t";
  }

#ifdef PATCH_ESCAPING_YAML_QUERIES
  std::string escape(const std::string& field) const {
    std::string ret = field;
    size_t first_null = ret.find_first_of('\0');
    if (std::string::npos != first_null) {
      ret.resize(first_null);
    }

    return ret;
  }

#endif
};

bool Orders::handles(const std::filesystem::path& path) {
  LLL.debug() << path;
  std::string lexrel = path.lexically_relative(orders_base).string();

  if (std::string::npos != lexrel.find("../")) {
    return false;

  }

  return true;
}

#define catch_and_rethrow_yaml_errors \
  catch (const yaml::RuntimeError& e) { \
    throw http::RuntimeError(e.inspect()); \
  }


bool Orders::allowed_method() {
  if (req.method_name() == Method::Name::GET) return true;
  if (req.method_name() == Method::Name::HEAD) return true;

  if (req.method_name() == Method::Name::TRACE) return true;

  return false;
}

bool Orders::exists() {
  if (is_index()) return true;

  try_load_single_order();

  if (did_load_order) return true;

  return false;
}

void Orders::try_load_single_order() try {
  if (did_load_order) return;
  if (failed_to_load_order) return;

  LLL.info() << "loading order " + order_id
             << " postcode? " << provided_postcode;

  OrderFinder of;

  if (provided_postcode) {
    of = OrderFinder(order_id, std::string(req.target->query()));
  } else {
    of = (order_id);
  }

  LLL.debug() << of.get_query();

  yaml::Table t("orders");

  order_columns = std::make_unique<yaml::ColumnCollection>(t.get_columns());

  try {
    current_order = t.find_first_row(of.get_program());
  } catch (const yaml::RuntimeError& e) {
    throw http::RuntimeError(e.what());
  }

  if (nullptr != current_order) {
    did_load_order = true;
  }
} catch_and_rethrow_yaml_errors;

bool Orders::is_index() const {
  return wanted_index;
}

ResponsePtr Orders::get_response() {
  if (is_index()) return get_index_response();
  if (provided_postcode) return get_full_order_response();
  return get_mini_order_response();
}

ResponsePtr Orders::get_index_response() try {
  yaml::Table t("orders");
  OrderFinder of;

  size_t id_column = t.column_index_for("order number");

  std::shared_ptr<HtmlResponse> resp =
    std::make_shared<HtmlResponse>();

  resp->raw("<!doctype html>");
  resp->open_tag("html");
  resp->open_tag("head");
  resp->quick_tag("title", "orders");
  resp->close_tag();
  resp->open_tag("body");
  resp->raw("\n");
  resp->quick_tag("h1", "orders");

  resp->open_tag("table");
  resp->open_tag("thead");
  resp->open_tag("tr");
  resp->quick_tag("th", "order number");
  resp->quick_tag("th", "actions");
  resp->close_tag(); // tr
  resp->close_tag(); // thead

  resp->open_tag("tbody");

  std::unique_ptr<yaml::Row> cur_row;

  while (nullptr != (cur_row = t.find_next_row(of.get_program()))) {
    resp->raw("\n");
    std::string cur_id = cur_row->at(id_column);
    resp->open_tag("tr", "id='order-" + cur_id + "'");
    resp->quick_tag("td", cur_id);
    resp->open_tag("td");
    std::string href = "href=\"/orders/";
    href += cur_id;
    href += "\"";
    resp->open_tag("a", href);
    resp->text("view");
    resp->close_tag(); // a
    resp->close_tag(); // td
    resp->close_tag(); // tr
  }
  resp->raw("\n");

  resp->close_tag(); // tbody
  resp->close_tag(); // table

  resp->close_tag(); // body
  resp->close_tag(); // html

  return resp;
} catch_and_rethrow_yaml_errors;

#define col_get(col_name) \
  current_order->at(order_columns->column_index_for(col_name))

ResponsePtr Orders::get_full_order_response() try {
  std::shared_ptr<HtmlResponse> resp =
    std::make_shared<HtmlResponse>();

  resp->raw("<!doctype html>");
  resp->open_tag("html");
  resp->open_tag("head");
  resp->open_tag("title");
  resp->text("order ");
  resp->text(order_id);
  resp->close_tag();
  resp->close_tag();
  resp->open_tag("body");
  resp->raw("\n");

  resp->open_tag("h1");
  resp->text("order ");
  resp->text(order_id);
  resp->close_tag();

  resp->open_tag("dl");

  resp->raw("\n");
  resp->quick_tag("dt", "order number");
  resp->quick_tag("dd", order_id, "id='order_number'");

  resp->raw("\n");
  resp->quick_tag("dt", "primary account number (PAN)");
  resp->quick_tag("dd", col_get("PAN"), "id='pan'");

  resp->raw("\n");
  resp->quick_tag("dt", "expiration");
  resp->quick_tag("dd", col_get("exp"), "id='exp'");

  resp->raw("\n");
  resp->quick_tag("dt", "card verification value (CVV)");
  resp->quick_tag("dd", col_get("cvv"), "id='cvv'");

  resp->raw("\n");
  resp->quick_tag("dt", "shipping address");
  resp->open_tag("dd", "id='ship'");
  resp->text_breaks(col_get("shipping address"));
  resp->close_tag(); // dd

  resp->raw("\n");
  resp->quick_tag("dt", "billing address");
  resp->open_tag("dd", "id='bill'");
  resp->text_breaks(col_get("billing address"));
  resp->close_tag(); // dd

  resp->close_tag(); // dl

  resp->close_tag(); // body
  resp->close_tag(); // html

  return resp;
} catch_and_rethrow_yaml_errors;

ResponsePtr Orders::get_mini_order_response() try {
  std::shared_ptr<HtmlResponse> resp =
    std::make_shared<HtmlResponse>();

  resp->raw("<!doctype html>");
  resp->open_tag("html");
  resp->open_tag("head");
  resp->open_tag("title");
  resp->text("order ");
  resp->text(order_id);
  resp->close_tag();
  resp->close_tag();
  resp->open_tag("body");
  resp->raw("\n");

  resp->open_tag("h1");
  resp->text("order ");
  resp->text(order_id);
  resp->close_tag();

  resp->open_tag("dl");

  resp->raw("\n");
  resp->quick_tag("dt", "order number");
  resp->quick_tag("dd", order_id, "id='order_number'");

  resp->raw("\n");
  resp->quick_tag("dt", "primary account number (PAN)");
  std::string pan_col = col_get("PAN");
  std::stringstream masked_pan;
  if (pan_col.size() == 16) {
    masked_pan << pan_col.substr(0, 4)
               << " "
               << pan_col.substr(4, 2)
               << "** **** "
               << pan_col.substr(12, 4);
  } else if (pan_col.size() == 15) {
    masked_pan << pan_col.substr(0, 4)
               << " "
               << pan_col.substr(4, 2)
               << "**** *"
               << pan_col.substr(11, 4);
  }

  resp->quick_tag("dd", masked_pan.str(), "id='pan'");

  resp->close_tag(); // dl

  resp->close_tag(); // body
  resp->close_tag(); // html

  return resp;
} catch_and_rethrow_yaml_errors;
