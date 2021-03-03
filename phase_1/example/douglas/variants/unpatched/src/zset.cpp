#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "assert.h"
#include "db.h"
#include "resp.h"
#include "tree_by_value.h"
#include "zpair.h"
#include "zset.h"

using std::map;
using std::string;
using std::vector;

Zset::Zset(string k) {
  key = k;

  RespEntry content = Db::get_instance()->deserialize(key);
  if (RespEntry::null == content.kind) {
    return;
  }

  vector<RespEntry> content_vec = content.get_array();

  for (RespEntry e : content_vec) {
    vector<RespEntry> tup = e.get_array();
    double s = tup[0].cast_double();
    string m = tup[1].get_string();

    add(s, m);
  }
}

bool Zset::member(string candidate) {
  if (nullptr == root_value) return false;

  return root_value->member(candidate);
}

long int Zset::rank(string candidate) {
  assert(member(candidate));
  double found_score = score(candidate);
  std::vector<ZScorePair> sp = root_score->to_vec();

  long int established_rank = 0;
  for (auto i = sp.begin(); i->first < found_score; i++) {
    established_rank++;
  }

  return established_rank;
}

std::vector<ZScorePair> Zset::range(long int start, long int stop) {
  long int total = count();
  if (0 == total) return {};

  if (start < 0) start = total + start;
  if (stop < 0) stop = total + stop;

  if (start >= total) return {};
  if (start > stop) return {};
  if (stop > total) stop = total;

  std::vector<ZScorePair> capture = {};
  capture.reserve(stop - start + 1);

  assert(nullptr != root_score);
  std::vector<ZScorePair> sp = root_score->to_vec();
  for (long int current_rank = 0; current_rank < total; current_rank++) {
    if ((current_rank >= start) && (current_rank <= stop)) {
      capture.push_back(sp[current_rank]);
    }
  }

  return capture;
}

double Zset::score(string candidate) {
  assert(member(candidate));
  return root_value->get_score(candidate);
}

void Zset::add(double score, string candidate) {
  remove(candidate);
  if (nullptr == root_score) {
    root_score = new TreeByScore(score, candidate);
  } else {
    root_score->add(score, candidate);
  }
  if (nullptr == root_value) {
    root_value = new TreeByValue(candidate, score);
  } else {
    root_value->add(candidate, score);
  }
}

void Zset::remove(string candidate) {
  if (!member(candidate)) return;

  double found_score = score(candidate);

  if ((1 == root_score->count())) {
    delete root_score;
    root_score = nullptr;
    delete root_value;
    root_value = nullptr;

    return;
  }
  root_score->remove(found_score, candidate);
  root_value->remove(candidate);
}

long int Zset::count() {
  long int v_count = 0;
  if (nullptr != root_value) v_count = root_value->count();
  long int s_count = 0;
  if (nullptr != root_score) s_count = root_score->count();

  assert(s_count == v_count);

  return s_count;
}

std::vector<ZScorePair> Zset::to_vec() {
  long int c = count();
  if (0 == c) return {};

  return root_score->to_vec();
}

std::vector<RespEntry> Zset::to_resp() {
  std::vector<RespEntry> entries = {};
  entries.reserve(count());
  for (ZScorePair p : to_vec()) {
    std::vector<RespEntry> pair = {RespEntry{p.first}, RespEntry{p.second}};
    entries.push_back(RespEntry{pair});
  }

  return entries;
}

void Zset::write() { write(key); }

void Zset::write(string dest) {
  Db::get_instance()->serialize(dest, RespEntry{to_resp()});
}

void Zset::dump(long int ctr) {
  if (nullptr == root_score) return;
  std::stringstream filename;
  filename << "/mnt/tmp/zset-dump-" << key << "-" << ctr << ".dot";
  std::cerr << filename.str() << std::endl;
  std::ofstream writer(filename.str(), std::ios::binary | std::ios::trunc);
  writer << "digraph G{" << std::endl;
  root_score->dump_graphviz(writer);
  writer << "}" << std::endl;
  writer.close();
}
