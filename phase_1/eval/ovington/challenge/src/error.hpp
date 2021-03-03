#pragma once

#include <errno.h>
#include <stdexcept>
#include <string>
#include <system_error>

class OvingtonError {};

class OvingtonSystemError : public OvingtonError,
                            public std::system_error {
public:
  OvingtonSystemError() : OvingtonError(),
                          std::system_error{errno, std::system_category()} {};
};

class OvingtonRuntimeError : public OvingtonError,
                             public std::runtime_error {
public:
  OvingtonRuntimeError() : OvingtonError(),
                           std::runtime_error("Ovington Runtime Error") {};
  OvingtonRuntimeError(std::string explanation) :
    OvingtonError(),
    std::runtime_error(explanation) {};
};
