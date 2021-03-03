#include "document.hpp"

#include "muncher.hpp"

using namespace html;

Document::Document(std::string body) {
  ScanIterator s = {body.cbegin(), body.cend()};
  Muncher::comments_and_whitespace(s);
  doctype = {s};
  root = Node::parse_document(s);
}

void Document::pretty(Io& io) const {
  PrettyPrinter printer(io);

  root->pretty(printer);
}
