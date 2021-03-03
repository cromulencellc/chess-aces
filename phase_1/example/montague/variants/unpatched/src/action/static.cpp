#include "static.hpp"
using namespace action;
const std::filesystem::path static_root = {"/mnt/base_data/static"};
const size_t MAX_FILE_SIZE = (1 << 24);
const std::string default_mime_type = "application/octet-stream";
const std::map<std::string, std::string> mime_types{
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".html", "text/html"}};
void Static::process(mtl::Context &ctx) {
  std::filesystem::path file = static_root / params["filename"];
  std::string normalized = {file.lexically_normal()};
  std::string chunk = normalized.substr(0, ((std::string)static_root).size());
  if (static_root != chunk) {
    not_found = true;
    return;
  }
  std::ifstream file_reader = {file, std::ios_base::in};
  std::string file_data(MAX_FILE_SIZE, '\0');
  file_reader.read(file_data.data(), file_data.size());
  file_data.resize(file_reader.gcount());
  extension = file.extension();
  ctx.assign("data", file_data);
}
std::filesystem::path Static::template_name() const {
  if (not_found)
    return "404.html.mtl";
  return "static.mtl";
}
std::string Static::content_type() const {
  auto candidate = mime_types.find(extension);
  if (mime_types.end() == candidate)
    return default_mime_type;
  return candidate->second;
}
