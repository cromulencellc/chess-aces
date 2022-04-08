#include <string>

class MaybeWord {
  public:
  MaybeWord(std::wstring w) : has_word(true), word(w), error() {};
  MaybeWord(const std::exception&& e) : has_word(false), word(), error(e.what()) {};

  bool has_word;
  std::wstring word;
  std::string error;
};