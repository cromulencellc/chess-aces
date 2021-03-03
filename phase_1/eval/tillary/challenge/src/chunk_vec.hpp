#pragma once

#include <vector>

#include "chunk.hpp"

using _ChunkVec_base = std::vector<Chunk>;

class ChunkVec : public _ChunkVec_base {
public:
  void push_back(Chunk&& value);

  void push_back(const std::string& value) {
    return push_back(Chunk(value));
  }

  bool write(int fd);

  size_t content_length();
};
