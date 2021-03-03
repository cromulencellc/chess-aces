#include "void_word.hpp"

#include "../logger.hpp"

#include <string>

bool html::is_void_element_name(const std::string candidate) {
  LLL.debug() << "checking " << candidate << " for void-ness";
  auto e = candidate.cend();
  auto i = candidate.cbegin();
  if ((e > (i + 0)) && ('a' == *(i + 0))) {
    if ((e > (i + 1)) && ('r' == *(i + 1))) {
      if ((e > (i + 2)) && ('e' == *(i + 2))) {
        if ((e > (i + 3)) && ('a' == *(i + 3))) {
          return true;
        }
        return false;
      }
      return false;
    }
    return false;
  }
  if ((e > (i + 0)) && ('b' == *(i + 0))) {
    if ((e > (i + 1)) && ('a' == *(i + 1))) {
      if ((e > (i + 2)) && ('s' == *(i + 2))) {
        if ((e > (i + 3)) && ('e' == *(i + 3))) {
          return true;
        }
        return false;
      }
      return false;
    }
    if ((e > (i + 1)) && ('r' == *(i + 1))) {
      return true;
    }
    return false;
  }
  if ((e > (i + 0)) && ('c' == *(i + 0))) {
    if ((e > (i + 1)) && ('o' == *(i + 1))) {
      if ((e > (i + 2)) && ('l' == *(i + 2))) {
        return true;
      }
      return false;
    }
    return false;
  }
  if ((e > (i + 0)) && ('e' == *(i + 0))) {
    if ((e > (i + 1)) && ('m' == *(i + 1))) {
      if ((e > (i + 2)) && ('b' == *(i + 2))) {
        if ((e > (i + 3)) && ('e' == *(i + 3))) {
          if ((e > (i + 4)) && ('d' == *(i + 4))) {
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
  if ((e > (i + 0)) && ('h' == *(i + 0))) {
    if ((e > (i + 1)) && ('r' == *(i + 1))) {
      return true;
    }
    return false;
  }
  if ((e > (i + 0)) && ('i' == *(i + 0))) {
    if ((e > (i + 1)) && ('m' == *(i + 1))) {
      if ((e > (i + 2)) && ('g' == *(i + 2))) {
        return true;
      }
      return false;
    }
    if ((e > (i + 1)) && ('n' == *(i + 1))) {
      if ((e > (i + 2)) && ('p' == *(i + 2))) {
        if ((e > (i + 3)) && ('u' == *(i + 3))) {
          if ((e > (i + 4)) && ('t' == *(i + 4))) {
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
  if ((e > (i + 0)) && ('l' == *(i + 0))) {
    if ((e > (i + 1)) && ('i' == *(i + 1))) {
      if ((e > (i + 2)) && ('n' == *(i + 2))) {
        if ((e > (i + 3)) && ('k' == *(i + 3))) {
          return true;
        }
        return false;
      }
      return false;
    }
    return false;
  }
  if ((e > (i + 0)) && ('m' == *(i + 0))) {
    if ((e > (i + 1)) && ('e' == *(i + 1))) {
      if ((e > (i + 2)) && ('t' == *(i + 2))) {
        if ((e > (i + 3)) && ('a' == *(i + 3))) {
          return true;
        }
        return false;
      }
      return false;
    }
    return false;
  }
  if ((e > (i + 0)) && ('p' == *(i + 0))) {
    if ((e > (i + 1)) && ('a' == *(i + 1))) {
      if ((e > (i + 2)) && ('r' == *(i + 2))) {
        if ((e > (i + 3)) && ('a' == *(i + 3))) {
          if ((e > (i + 4)) && ('m' == *(i + 4))) {
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
  if ((e > (i + 0)) && ('s' == *(i + 0))) {
    if ((e > (i + 1)) && ('o' == *(i + 1))) {
      if ((e > (i + 2)) && ('u' == *(i + 2))) {
        if ((e > (i + 3)) && ('r' == *(i + 3))) {
          if ((e > (i + 4)) && ('c' == *(i + 4))) {
            if ((e > (i + 5)) && ('e' == *(i + 5))) {
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
  if ((e > (i + 0)) && ('t' == *(i + 0))) {
    if ((e > (i + 1)) && ('r' == *(i + 1))) {
      if ((e > (i + 2)) && ('a' == *(i + 2))) {
        if ((e > (i + 3)) && ('c' == *(i + 3))) {
          if ((e > (i + 4)) && ('k' == *(i + 4))) {
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
  if ((e > (i + 0)) && ('w' == *(i + 0))) {
    if ((e > (i + 1)) && ('b' == *(i + 1))) {
      if ((e > (i + 2)) && ('r' == *(i + 2))) {
        return true;
      }
      return false;
    }
    return false;
  }
  return false;
}
