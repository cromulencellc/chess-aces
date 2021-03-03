#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H
#include "SystemException.h"
#include <iostream>
#define ISTREAM_READ(stream, target, err)                                      \
  {                                                                            \
    if (!stream.eof()) {                                                       \
      stream >> target;                                                        \
    } else {                                                                   \
      SystemException e(err);                                                  \
      throw e;                                                                 \
    }                                                                          \
  }
class Serializable {
public:
  friend std::ostream &operator<<(std::ostream &out, const Serializable &obj) {
    return obj.serialize(out);
  }
  friend std::istream &operator>>(std::istream &in, Serializable &obj) {
    return obj.dserialize(in);
  }
protected:
  virtual std::ostream &serialize(std::ostream &out) const = 0;
  virtual std::istream &dserialize(std::istream &in) = 0;
};
#endif