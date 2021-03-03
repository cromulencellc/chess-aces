#pragma once

#include <map>
#include <string>

namespace http {
  const std::map<std::string, std::string> MimeTypes
    {
     {".css", "text/css"},
     {".js", "application/javascript"},
     {".html", "text/html"},
     {".jpg", "image/jpeg"},
     {".png", "image/png"},
     {".txt", "text/plain"},
     {".ico", "image/x-icon"}
    };
}
