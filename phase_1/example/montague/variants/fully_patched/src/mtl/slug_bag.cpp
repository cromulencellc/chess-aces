#include "slug_bag.hpp"
#include <algorithm>
#include <numeric>
using namespace mtl;
std::size_t SlugBag::content_length() {
  auto sizer = [](std::size_t acc, std::shared_ptr<Slug> i) {
    return acc + i->to_iovec().iov_len;
  };
  return std::accumulate(begin(), end(), 0, sizer);
}
std::vector<struct iovec> SlugBag::to_iovec() {
  std::vector<struct iovec> ret;
  std::transform(begin(), end(), std::back_inserter(ret),
                 [](std::shared_ptr<Slug> i) { return i->to_iovec(); });
  return ret;
}
