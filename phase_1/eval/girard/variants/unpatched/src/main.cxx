#include "image.hxx"
#include "server.hxx"
#include "testbed.hxx"
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
int main(int argc, char **argv) {
#ifdef DEBUG
  if (argc > 2) {
    std::cout << "Usage: " << argv[0] << " <filename.hvif>" << std::endl;
    return 0;
  }
  std::string path(argv[1]);
  Girard::Image image;
  try {
    image = Girard::Image::load(path);
  } catch (std::runtime_error &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  path.replace(path.end() - 5, path.end(), "_copy.hvif");
  try {
    image.store(path);
  } catch (std::runtime_error &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
#endif
#ifndef DEBUG
  (void)argc;
  (void)argv;
#endif
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  if (std::getenv("PORT")) {
    Girard::Server server(std::atoi(std::getenv("PORT")));
    try {
      server.run();
    } catch (std::exception &e) {
      server.disconnect();
    }
  } else {
    Girard::Server server(&std::cin, &std::cout);
    try {
      server.run();
    } catch (std::exception &e) {
      server.disconnect();
    }
  }
  return 0;
}
