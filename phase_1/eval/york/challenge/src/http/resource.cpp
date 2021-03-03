#include "resource.hpp"

#include "trace_response.hpp"

using namespace http;

using Meth = http::Method::Name;

const std::array<Meth, 6> known_methods = {
                                           Meth::GET,
                                           Meth::HEAD,
                                           Meth::POST,
                                           Meth::PUT,
                                           Meth::DELETE,
                                           Meth::TRACE
};

bool Resource::known_method() {
  for (Meth cand : known_methods) {
    if (req.method_name() == cand) return true;
  }

  return false;
}

bool Resource::acceptable_content_type() {
  auto accept_header = req.headers.find("Accept");
  if (req.headers.cend() == accept_header) return true;
  std::string types = accept_header->second;
  if (std::string::npos != types.find("*/*")) return true;

  return false;
}

ResponsePtr Resource::trace_response() {
  return std::make_shared<TraceResponse>(req, this);
}
