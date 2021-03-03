#include "response.hpp"
#include "mtl.hpp"
#include "mtl/template.hpp"
#include "action.hpp"
#include "action/server_error.hpp"
#include <numeric>
#include <sstream>
#include <limits.h>
#include <sys/uio.h>
using namespace std::literals::string_literals;
Response::Response(Uuid tx_id, Request rq)
    : status(200), transaction_id(tx_id), request(rq) {
  std::unique_ptr<Action> act;
  ctx = mtl::make_context(tx_id, rq);
  try {
    act = Action::route(request);
    act->process(*ctx);
    tpl = {act->template_name()};
    got = tpl.render(ctx);
  } catch (const std::runtime_error &err) {
    action::ServerError se = {rq};
    se.process(*ctx, err);
    act = std::make_unique<action::ServerError>(se);
    tpl = {se.template_name()};
    got = tpl.render(ctx);
  }
  body = got.to_iovec();
  headers = {{"Content-type", act->content_type()},
             {"Content-length", std::to_string(got.content_length())}};
}
std::string Response::serialize() {
  std::stringstream o;
  o << "HTTP/1.1 ";
  o << status << "\r\n";
  for (response::Header header : headers) {
    o << header.first << ": " << header.second << "\r\n";
  }
  o << "\r\n";
  for (struct iovec chunk : body) {
    o << std::string_view((const char *)chunk.iov_base, chunk.iov_len);
  }
  return o.str();
}
void Response::serialize_to(int fd) {
  std::vector<std::string> chunks = {"HTTP/1.1 ", to_string(status), "\r\n"};
  for (response::Header header : headers) {
    chunks.push_back(header.first);
    chunks.push_back(": ");
    chunks.push_back(header.second);
    chunks.push_back("\r\n");
  }
  chunks.push_back("\r\n");
  std::vector<struct iovec> chunk_vecs;
  chunk_vecs.reserve(chunks.size() + body.size());
  std::transform(chunks.begin(), chunks.end(), std::back_inserter(chunk_vecs),
                 [](std::string & chunk) -> struct iovec {
                   return {.iov_base = chunk.data(), .iov_len = chunk.size()};
                 });
  std::transform(body.begin(), body.end(), std::back_inserter(chunk_vecs),
                 [](struct iovec chunk) { return chunk; });
  size_t cursor = 0;
  while (cursor < chunk_vecs.size()) {
    auto chunk_start = chunk_vecs.begin() + cursor;
    auto chunk_end = chunk_vecs.begin() + cursor + IOV_MAX;
    if (chunk_end > chunk_vecs.end())
      chunk_end = chunk_vecs.end();
    size_t expected_write = std::accumulate(
        chunk_start, chunk_end, 0,
        [](size_t acc, struct iovec iov) { return acc + iov.iov_len; });
    ssize_t actual_write =
        writev(fd, chunk_vecs.data() + cursor, chunk_end - chunk_start);
    if (0 > actual_write) {
      throw response::WritevError();
    }
    if (actual_write != expected_write) {
      throw response::ShortWritevError(expected_write, actual_write);
    }
    cursor += IOV_MAX;
  }
}
response::ShortWritevError::ShortWritevError(size_t expected, ssize_t got) {
  std::stringstream o;
  o << "Expected to writev " << expected << " bytes but wrote " << got
    << " instead.";
  MontagueRuntimeError{o.str()};
}
std::ostream &operator<<(std::ostream &o, Response &rs) {
  o << "Response(" << rs.status << std::endl
    << rs.headers << "Body(" << rs.body.size() << " chunks))" << std::endl;
  return o;
}
