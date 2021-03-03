#include "orders.hpp"

#include "../../logger.hpp"
#include "../html_response.hpp"

#include "../../yaml/error.hpp"
#include "../../yaml/table.hpp"
#include "../../yaml/program.hpp"

#include <regex>

using namespace http;
using namespace http::resources;

const std::regex five_digits("\\d\\d\\d\\d\\d");

#define catch_and_rethrow_yaml_errors \
  catch (const yaml::RuntimeError& e) { \
    throw http::RuntimeError(e.inspect()); \
  }


Orders::Orders(const Request& rq) try : Resource(rq) {
  std::filesystem::path p = rq.path();

  if ("" == p.stem()) {
    wanted_index = true;
    return;
  }

  order_id = p.stem().string();

  if ("" != req.target->query()) {
    postcode = req.target->query();

    LLL.debug() << "postcode " << postcode;

    if (false == std::regex_match(postcode, five_digits)) {
      throw BadRequest("postcode must be five digits");
    }
    provided_postcode = true;
  }
} catch_and_rethrow_yaml_errors;
