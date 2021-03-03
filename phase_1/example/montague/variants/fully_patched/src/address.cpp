#include "address.hpp"
#include <arpa/inet.h>
using namespace address;
std::optional<Address4> address::cast_address4(Address a) {
  if (AF_INET != a.sa_family) {
    return {};
  }
  return {*(Address4 *)(void *)(&a)};
}
std::optional<Address6> address::cast_address6(Address a) {
  if (AF_INET6 != a.sa_family) {
    return {};
  }
  return {*(Address6 *)(void *)(&a)};
}
std::ostream &operator<<(std::ostream &o, Address &a) {
  std::optional<Address4> a4 = cast_address4(a);
  std::optional<Address6> a6 = cast_address6(a);
  if (a4.has_value())
    return (o << a4.value());
  if (a6.has_value())
    return (o << a6.value());
  o << "Address{sa_family = " << std::hex << a.sa_family << "}";
  return o;
}
std::ostream &operator<<(std::ostream &o, address::Address4 &a) {
  o << "Address4{" << to_string(a) << ":" << a.sin_port << "}";
  return o;
}
std::ostream &operator<<(std::ostream &o, address::Address6 &a) {
  o << "Address6{" << to_string(a) << "}";
  return o;
}
std::string to_string(Address a) {
  std::optional<Address4> a4 = cast_address4(a);
  std::optional<Address6> a6 = cast_address6(a);
  if (a4.has_value())
    return to_string(a4.value());
  if (a6.has_value())
    return to_string(a6.value());
  return "{Unknown address type " + std::to_string(a.sa_family) + "}";
}
std::string to_string(address::Address4 a) {
  std::string addr(INET_ADDRSTRLEN + 1, '\0');
  inet_ntop(AF_INET, &a.sin_addr, addr.data(), INET_ADDRSTRLEN);
  return addr + ":" + std::to_string(a.sin_port);
}
std::string to_string(address::Address6 a) {
  std::string addr(INET6_ADDRSTRLEN + 1, '\0');
  inet_ntop(AF_INET6, &a.sin6_addr, addr.data(), INET6_ADDRSTRLEN);
  return addr + ":" + std::to_string(a.sin6_port);
}
