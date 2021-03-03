#include <string>
#include <vector>

#include "resp.h"
#include "tree_by_score.h"
#include "tree_by_value.h"
#include "zpair.h"

using std::string;

class Zset {
 public:
  Zset(string k);

  bool member(string candidate);
  long int rank(string candidate);
  double score(string candidate);

  std::vector<ZScorePair> range(long int start, long int stop);

  void add(double score, string candidate);
  void remove(string candidate);

  long int count();

  std::vector<ZScorePair> to_vec();
  std::vector<RespEntry> to_resp();

  void write();
  void write(string dest);

  void dump(long int ctr);

 private:
  string key;
  TreeByScore* root_score = nullptr;
  TreeByValue* root_value = nullptr;
};
