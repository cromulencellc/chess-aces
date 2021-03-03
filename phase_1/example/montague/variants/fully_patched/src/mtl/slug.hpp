#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <sys/uio.h>
namespace mtl {
namespace slug {
class Base {
public:
  virtual ~Base(){};
  virtual struct iovec to_iovec() const = 0;
};
class String : public Base {
public:
  String(std::string s) : held(s){};
  virtual ~String(){};
  virtual struct iovec to_iovec() const override;
private:
  std::string held;
};
class View : public Base {
public:
  View(std::string_view v) : referenced(v){};
  virtual ~View(){};
  virtual struct iovec to_iovec() const override;
private:
  std::string_view referenced;
};
}
using Slug = slug::Base;
}
