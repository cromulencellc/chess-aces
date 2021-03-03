#include "error.hpp"

#include <sstream>

#ifdef BACKTRACES
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>

#include "logger.hpp"

Error::Error() {
  valid_frames = backtrace(frames.data(), BACKTRACE_LIMIT);
}

std::string Error::inspect() const {
  char** backtrace_lines = backtrace_symbols(frames.data(), valid_frames);

  std::stringstream buf;
  buf << "Backtrace (most recent call first):" << std::endl;
  Dl_info info;
  for (size_t cur = 0; cur < valid_frames; cur++) {
    int dla_got = dladdr(frames[cur], &info);
    if (0 == dla_got) {
      LLL.fatal() << "couldn't dladdr while backtracing, rip in pieces";
      std::exit(-1);
    }
    int dem_got;
    const char* demangled;
    size_t demangled_len;
    bool demangled_needs_free = true;
    demangled = abi::__cxa_demangle(info.dli_sname,
                                    nullptr, &demangled_len,
                                    &dem_got);
    if (-2 == dem_got) {
      demangled = info.dli_sname;
      demangled_needs_free = false;
    } else if (0 != dem_got) {
      LLL.fatal() << "couldn't demangle " << info.dli_sname
                  << " while bactracing, got " << dem_got;
      demangled = info.dli_sname;
      demangled_needs_free = false;
    }
    buf << std::hex << "\t"
        << demangled << "+0x"
        << (ptrdiff_t)frames[cur] - (ptrdiff_t)info.dli_saddr
        << " in " << info.dli_fname << " @ 0x"
        << (ptrdiff_t)frames[cur] - (ptrdiff_t)info.dli_fbase << ' '
        << std::endl;
    if (demangled_needs_free) free((void*)demangled);
  }
  buf << std::dec;

  free(backtrace_lines);

  return buf.str();
}
#else
std::string Error::inspect() const { return ""; }
#endif

std::string RuntimeError::inspect() const {
  std::stringstream buf;

  buf << "runtime error: " << what() << std::endl;
  buf << Error::inspect();

  return buf.str();
}

std::string SystemError::inspect() const {
  std::stringstream buf;

  buf << "system error code " << code() << " " << what() << std::endl;
  buf << Error::inspect();

  return buf.str();
}
