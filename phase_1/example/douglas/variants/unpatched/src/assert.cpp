#include <cstdlib>
#include <iostream>

#include "assert.h"

void __assert_fail(const char *__assertion, const char *__file,

                   unsigned int __line, const char *__function) {
  
  
  
  
  std::cerr << __file << ":" << __line << ": " << __function << ": Assertion `"
            << __assertion << "` failed." << std::endl;

  std::exit(-1);
}
