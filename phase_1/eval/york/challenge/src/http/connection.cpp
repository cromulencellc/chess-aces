#include "connection.hpp"

#include "../chunk_vec.hpp"
#include "../logger.hpp"

#include "blank_response.hpp"
#include "error.hpp"
#include "error_response.hpp"
#include "redirect_response.hpp"
#include "router.hpp"

#include <unistd.h>

using namespace http;

Connection::Connection(int client_fd, Address client_address) :
  FdServiceable(client_fd), client_addr(client_address) {
  LLL.info() << "connection opened from " << client_addr;
}

Connection::Connection() :
  FdServiceable(0, 1) {
  LLL.info() << "connection opened on stdio";
}

ResponsePtr get_response(Request& req);

void Connection::service(int fd) {
  try {
    LLL.info() << "servicing " << fd << ": " << req.want;
    if (!req.want.in_valid_state()) {
      LLL.info() << "invalid request state, close it up";
      is_finished = true;
    }
    if (req.want.want_line) {
      bool got_full_line = io.read_buf_delim(in_buf, '\n');
      if (got_full_line) {
        // clip off the crlf
        bool ends_with_crlf = true;
        if ('\n' != in_buf.back()) ends_with_crlf = false;
        in_buf.pop_back();
        if ('\r' != in_buf.back()) ends_with_crlf = false;
        in_buf.pop_back();

        if (!ends_with_crlf) {
          LLL.debug() << in_buf;
          throw BadRequest("malformed line, wanted a crlf at end");
        }

        req.consume(in_buf);
        in_buf.clear();
      }
    }
    if (req.want.want_bytes.has_value()) {
      io.read_buf_count(in_buf, req.want.want_bytes.value());

      req.consume(in_buf);

      in_buf.clear();
    }
    if (req.want.ready_to_respond) {
      ResponsePtr resp = get_response(req);

      if (nullptr == resp) {
        throw http::RuntimeError("null response (resource didn't implement)");
      }

      resp->send_headers(io.o);

      if (req.method_name() != Method::Name::HEAD) {
        resp->stream_body_to(io.o);
      }

      // reset request
      req = Request();
    }
  }
  catch (const http::RuntimeError& re) {
    LLL.error() << "caught http runtime error "
                << re.status_code()
                << " "
                << re.status_desc()
                << std::endl
                << re.inspect();
    ErrorResponse error_resp = ErrorResponse(re);
    error_resp.send_headers(io.o);
    error_resp.stream_body_to(io.o);
    is_finished = true;
  }
}


ResponsePtr get_response(Request& req) {
  ResourcePtr resource = Router::route(req);

  if (! resource->known_method()) throw NotImplemented();
  if (! resource->allowed_method()) throw MethodNotAllowed();

  if (Method::Name::TRACE == req.method_name()) {
    return resource->trace_response();
  }

  if (resource->forbidden()) throw Forbidden();

  if (! resource->acceptable_content_type()) throw UnsupportedMediaType();

  if (resource->exists()) {
    // chase the existenz flow
    if (! ((Method::Name::POST == req.method_name()) ||
           (Method::Name::PUT == req.method_name()) ||
           (Method::Name::DELETE == req.method_name()))) {
      // idempotent, 200-track
    } else {
      resource->process();

      if (Method::Name::DELETE == req.method_name()) {
        if (resource->not_committed()) {
          return make_blank(202, "Accepted");
        }

        if (! resource->has_response_content()) {
          return make_blank(204, "No Content");
        }

        // fall through to normal delete_response
      } else if (Method::Name::POST == req.method_name()) {
        if (resource->should_redirect()) {
          return make_redirect(303, "See Other",
                               resource->redirect_location());
        }

        if (resource->did_create()) {
          return make_blank(201, "Created");
        }
      }
    }
  } else {
    // chase the creation flow
    if (Method::Name::POST != req.method_name()) {
      throw NotFound();
    }

    if (! resource->allow_post()) {
      throw NotFound();
    }

    resource->process();

    if (resource->should_redirect()) {
      return make_redirect(303, "See Other",
                           resource->redirect_location());
    }

    if (resource->did_create()) {
      return make_blank(201, "Created");
    }
  }

  switch (req.method_name()) {
  case Method::Name::GET:
    return resource->get_response();
  case Method::Name::HEAD:
    return resource->head_response();
  case Method::Name::POST:
    return resource->post_response();
  case Method::Name::DELETE:
    return resource->delete_response();
  case Method::Name::PUT:
    return resource->put_response();
  default:
    return nullptr;
  }
}
