#include "favicon.hpp"

#include "../../logger.hpp"

using namespace http;
using namespace http::resources;

bool Favicon::handles(const std::filesystem::path& path) {
  if ("/favicon.ico" == path) return true;

  return false;
}

void Favicon::validate_path(std::filesystem::path& p) {
  LLL.info() << "validating favicon path";
  p = std::filesystem::path("/data/static/favicon.ico");
}
