#pragma once

#include "../resource.hpp"

#include "../../yaml/column_collection.hpp"
#include "../../yaml/row.hpp"

namespace http {
  namespace resources {
    class Orders : public Resource {
    public:
      static bool handles(const std::filesystem::path& path);

      Orders(const Request& rq);
      virtual ~Orders() {};

      bool allowed_method() override;
      bool exists() override;

      ResponsePtr get_response() override;

    protected:
      bool wanted_index = false;
      std::string order_id;

      bool provided_postcode = false;
      std::string postcode;

      void try_load_single_order();

      bool did_load_order = false;
      bool failed_to_load_order = false;

      std::unique_ptr<yaml::ColumnCollection> order_columns = nullptr;
      std::unique_ptr<yaml::Row> current_order = nullptr;

      bool is_index() const;

      ResponsePtr get_index_response();
      ResponsePtr get_mini_order_response();
      ResponsePtr get_full_order_response();
    };
  }
}
