#include "static.hpp"

#include "../default_mime_type.hpp"
#include "../mime_types.hpp"
#include "../sendfile_response.hpp"

#include "../../logger.hpp"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace http;
using namespace http::resources;

const std::filesystem::path static_base = "/static/";
const std::filesystem::path static_root_dir = "/data/static/";

bool Static::handles(const std::filesystem::path& path) {
  LLL.info() << "checking for handling of " << path;
  if (path.lexically_normal().string().size() <= static_base.string().size()) {
    return false;
  } // long enough to be under static
  if (path.lexically_relative(static_base).string().find("../") !=
      std::string::npos) {
    return false;
  } // under static

  return true;
}

Static::Static(const Request& rq) : Resource(rq) {
  std::filesystem::path p = rq.path();

  path_inside_static = p.lexically_proximate(static_base);
}

Static::~Static() {
  if (read_fd >= 0) ::close(read_fd);
}

bool Static::allowed_method() {
  if (req.method_name() == Method::Name::GET) return true;
  if (req.method_name() == Method::Name::HEAD) return true;

  return Resource::allowed_method();
}

bool Static::forbidden() {
  try_open();
  if (read_fd >= 0) return false;
  if (read_error == EACCES) return true;
  return false;
}

bool Static::exists() {
  try_open();
  if (read_fd >= 0) return true;
  if (read_error == ENOENT) return false;
  return false;
}


bool Static::acceptable_content_type() {
  auto accept_header = req.headers.find("Accept");
  if (req.headers.cend() == accept_header) return true;
  std::string types = accept_header->second;
  if (std::string::npos != types.find("*/*")) return true;

  std::string content_type = DefaultMimeType;
  auto maybe_type = MimeTypes.find(path_inside_static.extension());
  if (MimeTypes.cend() != maybe_type) {
    content_type = maybe_type->second;
  }

  if (std::string::npos != types.find(content_type)) return true;

  return false;
}

void Static::validate_path(std::filesystem::path& p) {
  if (p.lexically_normal().string().size() <=
      static_root_dir.string().size()) {
    throw RuntimeError("tried to initialize Static with unexpected path");
  }
  if (p.lexically_relative(static_root_dir).string().find("../") !=
      std::string::npos) {
    throw RuntimeError("tried to initialize Static with unexpected path");
  }
}

void Static::try_open() {
  if (did_try_open) return;
  did_try_open = true;
  std::filesystem::path p = static_root_dir / path_inside_static;
  validate_path(p);
  LLL.debug() << "trying to open " << p.string();
  read_fd = open(p.c_str(), O_RDONLY);
  LLL.debug() << "got fd " << read_fd << " and errno " << errno;
  if (-1 == read_fd) read_error = errno;
}

ResponsePtr Static::get_response() {
  try_open();
  if (read_fd < 0) {
    throw http::RuntimeError("can't get response with a negative fd");
  }

  std::string content_type = DefaultMimeType;
  auto maybe_type = MimeTypes.find(path_inside_static.extension());
  if (MimeTypes.cend() != maybe_type) {
    content_type = maybe_type->second;
  }

  ResponsePtr resp = std::make_shared<SendfileResponse>(read_fd, content_type);
  read_fd = -2; // someone else's problem now
  return resp;
}
