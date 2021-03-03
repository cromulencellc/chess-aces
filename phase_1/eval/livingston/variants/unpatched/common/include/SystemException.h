#ifndef SYSTEMEXCEPTION_H
#define SYSTEMEXCEPTION_H
#include <exception>
#include <string>
class SystemException : public std::exception {
public:
  SystemException(std::string &message);
  SystemException(const char *message);
  virtual const char *what() const throw();
  friend std::ostream &operator<<(std::ostream &out, const SystemException &e);
private:
  std::string data;
};
#endif