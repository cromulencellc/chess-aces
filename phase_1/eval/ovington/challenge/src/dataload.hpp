#pragma once

#include "error.hpp"
#include "uuid.hpp"

#include "odbc/def.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

class Dataload {
public:
  Dataload(int in_fd, int out_fd);
  Uuid name = {};

  static void do_dl(int in_fd, int out_fd);
private:
  std::filesystem::path base_filename;

  std::vector<odbc::Def> defs;
};
