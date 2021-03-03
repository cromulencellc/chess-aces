#include "scan_iterator.hpp"

#include "error.hpp"

using namespace html;

bool ScanIterator::kicked() const {
  return end <= *this;
}

void ScanIterator::expect_not_kicked() const {
  if (kicked()) throw UnexpectedlyKicked();
}

bool ScanIterator::has(size_t n) const {
  return end <= *(this + n - 1);
}

enum class ScanResult { all_match, unexpected_kick, didnt_match };

ScanResult eat_recurse(std::string::const_iterator want,
                  std::string::const_iterator want_end,
                  ScanIterator& have) {
  if (want_end == want) return ScanResult::all_match;
  if (have.kicked()) return ScanResult::unexpected_kick;
  if (*want != *have) return ScanResult::didnt_match;

  have++;

  return eat_recurse(want + 1, want_end, have);
}

ScanResult eati_recurse(std::string::const_iterator want,
                  std::string::const_iterator want_end,
                  ScanIterator& have) {
  if (want_end == want) return ScanResult::all_match;
  if (have.kicked()) return ScanResult::unexpected_kick;
  if (std::tolower(*want) != std::tolower(*have)) {
    return ScanResult::didnt_match;
  }

  have++;

  return eati_recurse(want + 1, want_end, have);
}

void ScanIterator::eat(std::string want) {
  const char* first = &**this;
  ScanResult got = eat_recurse(want.cbegin(), want.cend(), *this);
  switch (got) {
  case ScanResult::all_match:
    return;
  case ScanResult::unexpected_kick:
    throw UnexpectedlyKicked(want);
  case ScanResult::didnt_match:
    const char* mismatched = &**this;
    std::string misgot = {first, (size_t)(mismatched - first)};
    throw DidntGet(want, misgot);
  }
}

void ScanIterator::eati(std::string want) {
  const char* first = &**this;
  ScanResult got = eati_recurse(want.cbegin(), want.cend(), *this);
  switch (got) {
  case ScanResult::all_match:
    return;
  case ScanResult::unexpected_kick:
    throw UnexpectedlyKicked(want);
  case ScanResult::didnt_match:
    const char* mismatched = &**this;
    std::string misgot = {first, (size_t)(mismatched - first)};
    throw DidntGet(want, misgot);
  };
}

bool ScanIterator::can_eati(std::string want) {
  ScanIterator temp = {(_ScanIteratorBase) *this, end};

  ScanResult got = eati_recurse(want.cbegin(), want.cend(), temp);
  if (ScanResult::all_match == got) return true;

  return false;
}

bool ScanIterator::can_eat(std::string want) {
  ScanIterator temp = {(_ScanIteratorBase) *this, end};

  ScanResult got = eat_recurse(want.cbegin(), want.cend(), temp);
  if (ScanResult::all_match == got) return true;

  return false;
}

std::string ScanIterator::scan_to(char delim) {
  std::string got;

  while (delim != **this) {
    got.push_back(**this);
    (*this)++;

    if (kicked()) throw UnexpectedlyKicked(std::string(delim, 1));
  }

  (*this)++;
  return got;
}
