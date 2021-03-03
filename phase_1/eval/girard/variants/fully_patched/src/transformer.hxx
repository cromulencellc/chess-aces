#pragma once
#include <vector>
namespace Girard {
class TransformerType {
public:
  enum Value {
    Identity = -1,
    Affine = 20,
    Contour = 21,
    Perspective = 22,
    Stroke = 23
  };
  TransformerType() = default;
  TransformerType(int);
  constexpr TransformerType(Value style) : value(style) {}
  operator Value() const { return value; }
  explicit operator bool() = delete;
private:
  Value value;
};
class Transformer {
public:
  Transformer();
  static float readValue(const std::vector<unsigned char> &, unsigned int &);
  static void writeValue(std::vector<unsigned char> &, float);
  static Transformer *fromBytes(const std::vector<unsigned char> &);
  static Transformer *fromBytes(const std::vector<unsigned char> &,
                                unsigned int &);
  std::vector<unsigned char> *toBytes();
  TransformerType type;
private:
};
class AffineTransformer : public Transformer {
public:
  AffineTransformer();
  AffineTransformer(float, float, float, float, float, float);
  void multiply(const AffineTransformer &);
  static AffineTransformer *fromBytes(const std::vector<unsigned char> &);
  static AffineTransformer *fromBytes(const std::vector<unsigned char> &,
                                      unsigned int &);
  std::vector<unsigned char> *toBytes();
  float sx;
  float sy;
  float shx;
  float shy;
  float tx;
  float ty;
private:
};
class ContourTransformer : public Transformer {
public:
  ContourTransformer();
  ContourTransformer(float, unsigned char, unsigned char);
  static ContourTransformer *fromBytes(const std::vector<unsigned char> &);
  static ContourTransformer *fromBytes(const std::vector<unsigned char> &,
                                       unsigned int &);
  std::vector<unsigned char> *toBytes();
  float width;
  unsigned char lineJoin;
  unsigned char miterLimit;
private:
};
class PerspectiveTransformer : public Transformer {
public:
  PerspectiveTransformer();
  PerspectiveTransformer(float, float, float, float, float, float, float, float,
                         float);
  PerspectiveTransformer(const AffineTransformer &);
  static PerspectiveTransformer *fromBytes(const std::vector<unsigned char> &);
  static PerspectiveTransformer *fromBytes(const std::vector<unsigned char> &,
                                           unsigned int &);
  std::vector<unsigned char> *toBytes();
  float sx;
  float sy;
  float shx;
  float shy;
  float tx;
  float ty;
  float w0;
  float w1;
  float w2;
private:
};
class StrokeTransformer : public Transformer {
public:
  StrokeTransformer();
  StrokeTransformer(float, unsigned char, unsigned char, unsigned char);
  static StrokeTransformer *fromBytes(const std::vector<unsigned char> &);
  static StrokeTransformer *fromBytes(const std::vector<unsigned char> &,
                                      unsigned int &);
  std::vector<unsigned char> *toBytes();
  float width;
  unsigned char lineJoin;
  unsigned char lineCap;
  unsigned char miterLimit;
private:
};
} // namespace Girard
