#pragma once
#include <netinet/in.h>
#include <optional>
#include <ostream>
#include <sys/socket.h>
using Address = struct sockaddr;
namespace address {
using Address4 = struct sockaddr_in;
using Address6 = struct sockaddr_in6;
std::optional<Address4> cast_address4(Address a);
std::optional<Address6> cast_address6(Address a);
}
std::ostream &operator<<(std::ostream &o, Address &a);
std::ostream &operator<<(std::ostream &o, address::Address4 &a);
std::ostream &operator<<(std::ostream &o, address::Address6 &a);
std::string to_string(Address a);
std::string to_string(address::Address4 a);
std::string to_string(address::Address6 a);
