#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>

#include "command.h"
#include "commands.h"
#include "db.h"
#include "log.h"
#include "net.h"
#include "resp.h"
#include "testbed.h"

using std::string;

int main() {
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif

#ifdef ALWAYS_SWEEP
  std::system("rm -rf /data/*");
#endif

  lll("initializing database...\n");
  Db* db = Db::get_instance();
  db->dump_keys(std::cerr);

  std::istream* in;
  std::ostream* out;

  if (std::getenv("PORT")) {
    Net handler{std::atoi(std::getenv("PORT"))};
    in = handler.get_in();
    out = handler.get_out();
  } else {
    in = &std::cin;
    out = &std::cout;
  }

  lll("ready\n");

  while (true) {
    try {
      RespEntry cmd_line{*in};

      Command* cmd = Command::command_for(*out, cmd_line);
      assert(nullptr != cmd);

      cmd->respond();
      out->flush();

    } catch (const std::exception& e) {
      lll("caught exception %s\n", e.what());
      assert(false);
    }

    if (in->eof()) {
      lll("eof, exiting\n");
      return 0;
    }
  }
}
