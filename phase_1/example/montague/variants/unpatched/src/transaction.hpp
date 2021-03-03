#pragma once
#include <iostream>
#include "address.hpp"
#include "uuid.hpp"
class Transaction {
public:
  Transaction(int f, Address c_a) : fd(f), client_address(c_a){};
  void service();
  Uuid id = {};
private:
  int fd;
  Address client_address;
};
