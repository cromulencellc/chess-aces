#pragma once
#include <memory>
#include <sys/uio.h>
#include <vector>
#include "slug.hpp"
namespace mtl {
using _SlugBag_parent_class = std::vector<std::shared_ptr<Slug>>;
class SlugBag : public _SlugBag_parent_class {
public:
  std::size_t content_length();
  std::vector<struct iovec> to_iovec();
};
}
