#pragma once
#include "models/item.hpp"
#include "models/list.hpp"
#include "mtl/context.hpp"
namespace presenter {
using SharedHash = std::shared_ptr<mtl::context::Hash>;
SharedHash list(models::List l);
SharedHash item(const models::Item i);
}
