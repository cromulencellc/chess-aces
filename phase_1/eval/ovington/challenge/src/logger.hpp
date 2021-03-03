#pragma once

#ifndef LOG_LEVEL
#define LOG_LEVEL LogLevel::debug
#endif

#include <fstream>
#include <iostream>
#include <string>

enum class LogLevel {
                      debug,
                      info,
                      warn,
                      error,
                      fatal
};

std::string to_string(LogLevel ll);
std::ostream& operator<<(std::ostream& os, LogLevel ll);

class Logger {
public:
  static const LogLevel level = LOG_LEVEL;

  Logger(const char* f,
         unsigned int l,
         const char* fn)
    : file(f), line(l), function(fn) {}

  Logger& operator=(const Logger&) = delete;
  Logger(const Logger&) = delete;

  ~Logger();

  std::ostream& fatal();
  std::ostream& error();
  std::ostream& warn();
  std::ostream& info();
  std::ostream& debug();

  std::ostream& get_logger(LogLevel candidate_level);

private:
  const char* file;
  unsigned int line;
  const char* function;

  bool needs_newline = false;
  std::ofstream null_stream = {};
};

#ifndef NO_LOG
#define LLL Logger(__FILE__, __LINE__, __extension__ __PRETTY_FUNCTION__)
#else
#define LLL std::ofstream()
#endif
