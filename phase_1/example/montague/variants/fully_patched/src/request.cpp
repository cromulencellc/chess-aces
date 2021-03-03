#include "request.hpp"
#include "request/body_reader.hpp"
#include "request/headers.hpp"
#include "request/method.hpp"
#include "request/reader.hpp"
#include "request/target.hpp"
#include "request/version.hpp"
#include "logger.hpp"
using Method = request::Method;
using Reader = request::Reader;
Request::Request(int i, Address c_a) {
  Reader in = {i};
  method = Method::parse(in);
  target = {in};
  version = {in};
  headers = {in};
  client_address = c_a;
  request::body_reader::ContentLength body_reader = {i, headers};
  raw_body = body_reader.raw_body();
}
bool Request::match(std::string candidate_path,
                    request::Method::Name candidate_method) {
  if (method.name != candidate_method)
    return false;
  if (target.path != candidate_path)
    return false;
  return true;
}
std::optional<request::body_parser::DecodedForm> Request::decode_body() {
  request::body_parser::FormEncoded decoder = {raw_body};
  return decoder.form_data;
}
std::ostream &operator<<(std::ostream &o, Request &rq) {
  o << "Request(" << std::endl
    << rq.method << " " << rq.target << " " << rq.version << std::endl
    << rq.headers << std::endl
    << "raw_body(len=" << rq.raw_body.size() << ")" << std::endl
    << ")" << std::endl;
  return o;
}
