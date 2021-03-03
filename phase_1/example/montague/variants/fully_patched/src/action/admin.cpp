#include "admin.hpp"
#include <fstream>
using namespace action;
std::string load_token() {
  std::ifstream token_reader("/token", std::ios_base::in);
  if (!token_reader.good())
    throw MontagueSystemError();
  std::string token(32, '\0');
  token_reader.read(token.data(), token.size());
  token.resize(token_reader.gcount());
  token_reader.close();
  return token;
}
std::string load_password() {
  std::ifstream password_reader("/data/password",
                                std::ios_base::in | std::ios_base::ate);
  if (!password_reader.good())
    throw MontagueSystemError();
  std::string password(password_reader.tellg(), '\0');
  password_reader.seekg(0);
  password_reader.read(password.data(), password.size());
  return password;
}
std::string admin_password = load_password();
std::string admin_token = load_token();
void Admin::process(mtl::Context &ctx) {
  std::shared_ptr<mtl::context::Hash> req_mebbe =
      std::dynamic_pointer_cast<mtl::context::Hash>(
          ctx.find_something("request"));
  if (nullptr == req_mebbe) {
    throw MontagueRuntimeError("couldn't find request for admin auth");
  }
  req_mebbe->erase("_montague_authenticated");
  ctx.assign("token", admin_token);
  auto body = request.decode_body();
  if (!body.has_value())
    return;
  auto password_finder = body->find("password");
  if (body->end() == password_finder)
    return;
  if (admin_password != password_finder->second)
    return;
  req_mebbe->assign("_montague_authenticated", "true");
}
std::filesystem::path Admin::template_name() const { return "admin.html.mtl"; }
