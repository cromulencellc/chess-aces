#pragma once
#include "transformer.hxx"
#include <vector>
namespace Girard {
class ShapeFlag {
public:
  enum {
    Transform = 1 << 1,
    Hinting = 1 << 2,
    LevelOfDetailScale = 1 << 3,
    HasTransformers = 1 << 4,
    Translation = 1 << 5
  };
};
class ShapeType {
public:
  enum Value { PathSource = 10 };
  ShapeType() = default;
  ShapeType(int);
  constexpr ShapeType(Value style) : value(style) {}
  operator Value() const { return value; }
  explicit operator bool() = delete;
private:
  Value value;
};
class Shape {
public:
  Shape();
  void setType(ShapeType);
  unsigned char getStyle();
  void setStyle(unsigned char);
  std::vector<unsigned char> getPaths();
  void setPaths(std::vector<unsigned char> &);
  void addPath(unsigned char);
  void removePath(unsigned char);
  bool hasHinting();
  void setHinting(bool);
  float getMinVisibility();
  void setMinVisibility(float);
  float getMaxVisibility();
  void setMaxVisibility(float);
  unsigned char getTransformers();
  Transformer *getTransformer(unsigned char);
  void setTransformer(Transformer *, unsigned char);
  void addTransformer(Transformer *);
  void addTransformer(Transformer *, unsigned char);
  void removeTransformer(unsigned char);
  static Shape fromBytes(const std::vector<unsigned char> &);
  static Shape fromBytes(const std::vector<unsigned char> &, unsigned int &);
  std::vector<unsigned char> toBytes();
private:
  void checkPaths();
  bool hinting;
  float minVisibility;
  float maxVisibility;
  ShapeType type;
  unsigned char style;
  std::vector<unsigned char> paths;
  std::vector<Transformer *> transformers;
};
} // namespace Girard
