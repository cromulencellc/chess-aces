#include "transaction.hpp"
#include "logger.hpp"
#include "request.hpp"
#include "response.hpp"
void Transaction::service() {
  Request rq = {fd, client_address};
  Response rs = {id, rq};
  LLL.info() << id << client_address << rq << rs;
  rs.serialize_to(fd);
}
