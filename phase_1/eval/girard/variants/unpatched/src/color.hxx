#pragma once
#include "transformer.hxx"
#include <vector>
namespace Girard {
class Color {
public:
  Color();
  Color(const unsigned char, const unsigned char, const unsigned char);
  Color(const unsigned char, const unsigned char, const unsigned char,
        const unsigned char);
  static Color fromBytes(const std::vector<unsigned char> &);
  static Color fromBytes(const std::vector<unsigned char> &, unsigned int &);
  std::vector<unsigned char> toBytes();
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
private:
};
class GradientFlag {
public:
  enum { Transform = 1 << 1, NoAlpha = 1 << 2, Grays = 1 << 4 };
};
class GradientType {
public:
  enum Value {
    Linear = 0,
    Circular = 1,
    Diamond = 2,
    Conic = 3,
    Xy = 4,
    SqrtXy = 5
  };
  GradientType() = default;
  GradientType(int);
  constexpr GradientType(Value style) : value(style) {}
  operator Value() const { return value; }
  explicit operator bool() = delete;
private:
  Value value;
};
struct GradientStep {
  unsigned char stop;
  Color color;
};
class Gradient {
public:
  Gradient();
  Gradient(GradientType);
  GradientType getType();
  void setType(GradientType);
  GradientStep getStep(unsigned int);
  std::vector<GradientStep> getSteps();
  void setStep(GradientStep, unsigned int);
  void addStep(GradientStep);
  void addStep(GradientStep, unsigned int);
  void removeStep(unsigned int);
  AffineTransformer *getTransformer();
  void setTransformer(AffineTransformer *);
  static Gradient fromBytes(const std::vector<unsigned char> &);
  static Gradient fromBytes(const std::vector<unsigned char> &, unsigned int &);
  std::vector<unsigned char> toBytes();
private:
  AffineTransformer *transformer;
  GradientType type;
  std::vector<GradientStep> steps;
};
} // namespace Girard
