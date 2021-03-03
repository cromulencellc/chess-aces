#pragma once
#include "mtl/context.hpp"
#include "request.hpp"
#include "uuid.hpp"
#include <memory>
namespace mtl {
std::shared_ptr<Context> make_context(Uuid tx_id, Request rq);
std::shared_ptr<Context> fake_context();
}
