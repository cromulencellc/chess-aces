#include "logger.hpp"
#include "mtl.hpp"
using namespace mtl;
using hpt = std::shared_ptr<context::Hash>;
using spt = std::shared_ptr<context::Scalar>;
hpt hhh() { return std::make_shared<context::Hash>(); }
spt sss(std::string v) { return std::make_shared<context::Scalar>(v); }
std::shared_ptr<Context> mtl::make_context(Uuid tx_id, Request rq) {
  hpt txn = hhh();
  txn->insert_or_assign("id", sss(to_string(tx_id)));
  hpt req = hhh();
  for (request::Header h : rq.headers) {
    req->insert_or_assign(h.first, sss(h.second));
  }
  req->insert_or_assign("_method", sss(to_string(rq.method)));
  req->insert_or_assign("_target", sss(to_string(rq.target)));
  req->insert_or_assign("_version", sss(to_string(rq.version)));
  req->insert_or_assign("_client_address", sss(to_string(rq.client_address)));
  hpt ctx = hhh();
  ctx->insert_or_assign("transaction", txn);
  ctx->insert_or_assign("request", req);
  LLL.info() << txn->inspect();
  return ctx;
}
std::shared_ptr<Context> mtl::fake_context() {
  hpt txn = hhh();
  (*txn)["id"] = sss(to_string(Uuid{}));
  hpt req = hhh();
  req->insert_or_assign(
      "Accept",
      sss("text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"));
  req->insert_or_assign("Accept-Encoding", sss("gzip, deflate"));
  req->insert_or_assign("Accept-Language", sss("en-US,en;q=0.5"));
  req->insert_or_assign("Cache-Control", sss("max-age=0"));
  req->insert_or_assign("Connection", sss("keep-alive"));
  req->insert_or_assign("Host", sss("localhost:32768"));
  req->insert_or_assign("Upgrade-Insecure-Requests", sss("1"));
  req->insert_or_assign("User-Agent",
                        sss("Mozilla/5.0 (Macintosh; Intel Mac OS X 10.14; "
                            "rv:69.0) Gecko/20100101 Firefox/69.0"));
  req->insert_or_assign("_method", sss("GET"));
  req->insert_or_assign("_target", sss("/asdf"));
  req->insert_or_assign("_version", sss("HTTP/1.1"));
  hpt ctx = hhh();
  ctx->insert_or_assign("transaction", txn);
  ctx->insert_or_assign("request", req);
  LLL.info() << ctx->inspect();
  return ctx;
}
