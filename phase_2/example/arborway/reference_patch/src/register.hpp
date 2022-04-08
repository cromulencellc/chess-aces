#ifndef __REGISTER_HPP__
#define __REGISTER_HPP__
#include "uint256_t.h"
#include "utils.hpp"
#include <stdint.h>
#define LOWBYTE 0
#define HIGHBYTE 1
#define WORD 2
#define DWORD 4
#define QWORD 8
#define DQWORD 16
#define QQWORD 32
typedef struct rtype {
  std::string n;
  std::string parent;
  uint64_t type;
  uint64_t bits;
} rtype;
class reg {
public:
  uint64_t bits;
  uint64_t val;
  uint256_t ymm;
  std::string name;
  reg();
  reg(std::string name, uint64_t bits);
  void set(uint64_t nv, uint64_t loc);
  void set(uint256_t nv, uint64_t loc);
  uint64_t get(uint64_t loc);
  uint256_t get(uint256_t loc);
  reg &operator+=(const uint64_t rhs) {
    if (bits == 64) {
      val += rhs;
    } else {
      ymm += uint256_t(rhs);
    }
    return *this;
  }
  reg &operator+=(const uint256_t rhs) {
    ymm += rhs;
    return *this;
  }
  reg &operator-=(const uint64_t rhs) {
    if (bits == 64) {
      val -= rhs;
    } else {
      ymm -= uint256_t(rhs);
    }
    return *this;
  }
  reg &operator-=(const uint256_t rhs) {
    ymm -= rhs;
    return *this;
  }
  inline bool operator==(std::string &rhs) {
    return cppstrncasecmp(this->name, rhs);
  }
};
#endif