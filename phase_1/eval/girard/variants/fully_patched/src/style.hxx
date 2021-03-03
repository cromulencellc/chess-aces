#pragma once
#include "color.hxx"
#include <vector>
namespace Girard {
class StyleType {
public:
  enum Value {
    SolidColor = 1,
    Gradient = 2,
    SolidColorNoAlpha = 3,
    SolidGray = 4,
    SolidGrayNoAlpha = 5
  };
  StyleType() = default;
  StyleType(int);
  constexpr StyleType(Value style) : value(style) {}
  operator Value() const { return value; }
  explicit operator bool() = delete;
private:
  Value value;
};
class Style {
public:
  Style();
  Style(Color);
  Style(Gradient);
  Color getColor();
  Gradient getGradient();
  void setColor(Color);
  void setGradient(Gradient);
  GradientStep getStep(unsigned int);
  void setStep(GradientStep, unsigned int);
  void addStep(GradientStep, unsigned int);
  void removeStep(unsigned int);
  bool hasGradient();
  bool hasTransparency();
  static Style fromBytes(const std::vector<unsigned char> &);
  static Style fromBytes(const std::vector<unsigned char> &, unsigned int &);
  std::vector<unsigned char> toBytes();
private:
  void updateTransparency();
  bool flat;
  bool transparent;
  Color color;
  Gradient gradient;
};
} // namespace Girard
