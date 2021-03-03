#pragma once

#include "../error.hpp"

#include "node_ptr.hpp"

#include <string>

namespace html {
  class SystemError : public ::SystemError {};

  class RuntimeError : public ::RuntimeError {
  public:
    RuntimeError(std::string why) : ::RuntimeError(why) {};
  };


  class UnexpectedlyKicked : public RuntimeError {
  public:
    UnexpectedlyKicked(std::string wanted) :
      RuntimeError("iterator kicked when expecting `" + wanted + "`") {}

    UnexpectedlyKicked() :
      RuntimeError("iterator kicked unexpectedly") {}
  };

  class DidntGet : public RuntimeError {
  public:
    DidntGet(std::string wanted, std::string got) :
      RuntimeError("didn't get `" + wanted + "`, instead got `" + got + "`") {}
  };

  class NeededWS : public RuntimeError {
  public:
    NeededWS(char got) :
      RuntimeError("needed whitespace, got `" + std::string{got} + "`") {}
  };

  class AlreadyParsed : public RuntimeError {
  public:
    AlreadyParsed(std::string n) :
      RuntimeError("this node's already been parsed, dev goofed! `"
                   + n
                   + "`") {}
  };

  class DidntParseSelf : public RuntimeError {
  public:
    // yeah the different pointer types are different, deal
    DidntParseSelf(std::string expected, std::string got) :
      RuntimeError("wanted to parse `"
                   + expected
                   + "` but got `"
                   + got
                   + "`, dev error") {}
  };

  class CantReallyParse : public RuntimeError {
  public:
    CantReallyParse(std::string n) :
      RuntimeError("you can't really parse `" + n +"`, dev goofed") {}
  };

  class UnknownEntityError : public RuntimeError {
  public:
    UnknownEntityError(std::string key) :
      html::RuntimeError("unknown entity with key `" + key + "`") {};
  };
}
