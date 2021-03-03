#include "router.hpp"

#include "error.hpp"

#include "../logger.hpp"

#include "resources/favicon.hpp"
#include "resources/orders.hpp"
#include "resources/static.hpp"

using namespace http;

#define rrr(res) if (resources::res::handles(req.path())) {     \
    return std::make_unique<resources::res>(req);               \
  }

ResourcePtr Router::route(const Request& req) {
  LLL.info() << "trying to route " << req.path();
  rrr(Favicon);
  rrr(Orders);
  rrr(Static);

  LLL.info() << "couldn't route " << req.path();

  throw NotFound();
}
