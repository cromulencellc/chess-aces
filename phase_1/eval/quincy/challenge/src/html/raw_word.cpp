#include "void_word.hpp"

#include <string>

using namespace html;

bool is_raw_element_name(const std::string candidate) {
  auto e = candidate.cend();
  auto i = candidate.cbegin();
  if ((e > (i + 0)) && ('s' == *(i + 0))) {
    if ((e > (i + 1)) && ('c' == *(i + 1))) {
      if ((e > (i + 2)) && ('r' == *(i + 2))) {
        if ((e > (i + 3)) && ('i' == *(i + 3))) {
          if ((e > (i + 4)) && ('p' == *(i + 4))) {
            if ((e > (i + 5)) && ('t' == *(i + 5))) {
              return true;
            }
            return false;
          }
          return false;
        }
        return false;
      }
      return false;
    }
    if ((e > (i + 1)) && ('t' == *(i + 1))) {
      if ((e > (i + 2)) && ('y' == *(i + 2))) {
        if ((e > (i + 3)) && ('l' == *(i + 3))) {
          if ((e > (i + 4)) && ('e' == *(i + 4))) {
            return true;
          }
          return false;
        }
        return false;
      }
      return false;
    }
    return false;
  }
  if ((e > (i + 0)) && ('t' == *(i + 0))) {
    if ((e > (i + 1)) && ('e' == *(i + 1))) {
      if ((e > (i + 2)) && ('x' == *(i + 2))) {
        if ((e > (i + 3)) && ('t' == *(i + 3))) {
          if ((e > (i + 4)) && ('a' == *(i + 4))) {
            if ((e > (i + 5)) && ('r' == *(i + 5))) {
              if ((e > (i + 6)) && ('e' == *(i + 6))) {
                if ((e > (i + 7)) && ('a' == *(i + 7))) {
                  return true;
                }
                return false;
              }
              return false;
            }
            return false;
          }
          return false;
        }
        return false;
      }
      return false;
    }
    if ((e > (i + 1)) && ('i' == *(i + 1))) {
      if ((e > (i + 2)) && ('t' == *(i + 2))) {
        if ((e > (i + 3)) && ('l' == *(i + 3))) {
          if ((e > (i + 4)) && ('e' == *(i + 4))) {
            return true;
          }
          return false;
        }
        return false;
      }
      return false;
    }
    return false;
  }
  return false;
}
