#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "command.h"
#include "commands.h"
#include "log.h"
#include "resp.h"

#define CCC(klass) \
  if (klass::handles(cmd)) return new klass(out, cmd);

Command* Command::command_for(std::ostream& out, RespEntry& cmd) {
  CCC(Ping);
  CCC(Get);
  CCC(Set);
  CCC(Append);
  CCC(Strlen);
  CCC(Mget);
  CCC(Mset);

  CCC(Sadd);
  CCC(Srem);
  CCC(Sismember);
  CCC(Smembers);
  CCC(Scard);

  CCC(Sdiff);
  CCC(Sinter);
  CCC(Sunion);

  CCC(Sdiffstore);
  CCC(Sinterstore);
  CCC(Sunionstore);

  CCC(Zadd);
  CCC(Zcard);
  CCC(Zrank);
  CCC(Zrem);
  CCC(Zscore);

  CCC(ZrangeWithscores);
  CCC(Zrange);
  CCC(ZrevrangeWithscores);
  CCC(Zrevrange);

  return new Command(out, cmd);
}

void Command::respond() {
  lll("Processing unknown command %s\n", argv.first_string().c_str());
  std::exit(-1);
}

bool Command::handles(RespEntry _cmd) { return true; }
