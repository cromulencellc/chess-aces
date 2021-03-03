#pragma once
using MethName = request::Method::Name;
#define r(meth, rxp, binds, klass)                                             \
  {                                                                            \
    std::optional<action::klass> candidate =                                   \
        match<action::klass>(MethName::meth, std::regex(rxp), request, binds); \
    if (candidate.has_value()) {                                               \
      return std::make_unique<action::klass>(candidate.value());               \
    }                                                                          \
  }
template <typename T>
std::optional<T> match(MethName method_to_match, std::regex path_matcher,
                       Request &request, std::vector<std::string> binds) {
  if (method_to_match != request.method.name)
    return {};
  std::string path = request.target.path.string();
  std::smatch got;
  if (!std::regex_match(path, got, path_matcher)) {
    return {};
  }
  Params params;
  for (std::size_t i = 0; i < binds.size(); i++) {
    params.insert_or_assign(binds[i], got[i + 1]);
  }
  return std::make_optional<T>(request, params);
}
std::vector<std::string> bx(std::vector<std::string> binds) { return binds; }
