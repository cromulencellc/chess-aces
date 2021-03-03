#include <iostream>
#include "logger.hpp"
std::string to_string(LogLevel ll) {
  switch (ll) {
  case LogLevel::debug:
    return "DEBUG";
  case LogLevel::info:
    return "INFO";
  case LogLevel::warn:
    return "WARN";
  case LogLevel::error:
    return "ERROR";
  case LogLevel::fatal:
    return "FATAL";
  }
}
std::ostream &operator<<(std::ostream &os, LogLevel ll) {
  os << to_string(ll);
  return os;
}
std::ostream &Logger::fatal() { return get_logger(LogLevel::fatal); }
std::ostream &Logger::error() { return get_logger(LogLevel::error); }
std::ostream &Logger::warn() { return get_logger(LogLevel::warn); }
std::ostream &Logger::info() { return get_logger(LogLevel::info); }
std::ostream &Logger::debug() { return get_logger(LogLevel::debug); }
std::ostream &Logger::get_logger(LogLevel candidate_level) {
  if (level > candidate_level) {
    return null_stream;
  }
  std::cerr << "[" << candidate_level << "] " << file << ":" << line << " "
            << function << ": ";
  needs_newline = true;
  return std::cerr;
}
Logger::~Logger() {
  if (needs_newline) {
    std::cerr << std::endl;
  }
}
