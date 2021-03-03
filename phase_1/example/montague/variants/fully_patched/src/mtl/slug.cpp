#include "slug.hpp"
using namespace mtl::slug;
struct iovec String::to_iovec() const {
  return {.iov_base = (void *)held.data(), .iov_len = held.size()};
}
struct iovec View::to_iovec() const {
  return {.iov_base = (void *)referenced.data(), .iov_len = referenced.size()};
}
