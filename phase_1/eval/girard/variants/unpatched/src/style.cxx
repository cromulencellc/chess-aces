#ifdef DEBUG
#include <iostream>
#endif
#include "style.hxx"
#include <stdexcept>
namespace Girard {
StyleType::StyleType(int type) {
  switch (type) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    value = static_cast<Value>(type);
    break;
  default:
    throw std::range_error("Not a valid style type value");
  }
}
Style::Style() : flat(true), transparent(false), color(Color()) {}
Style::Style(Color color) {
  this->flat = true;
  this->color = color;
  if (color.alpha != 255) {
    this->transparent = true;
  } else {
    this->transparent = false;
  }
}
Style::Style(Gradient gradient) {
  this->flat = false;
  this->gradient = gradient;
  this->transparent = false;
  for (auto step : gradient.getSteps()) {
    if (step.color.alpha != 255) {
      this->transparent = true;
      break;
    }
  }
}
Color Style::getColor() { return this->color; }
Gradient Style::getGradient() { return this->gradient; }
void Style::setColor(Color color) {
  this->color = color;
  this->flat = true;
  this->updateTransparency();
}
void Style::setGradient(Gradient gradient) {
  this->gradient = gradient;
  this->flat = false;
  this->transparent = false;
  this->updateTransparency();
}
GradientStep Style::getStep(unsigned int idx) {
  if (!this->flat) {
    return this->gradient.getStep(idx);
  } else {
    throw std::logic_error("Style does not contain gradient");
  }
}
void Style::setStep(GradientStep step, unsigned int idx) {
  if (!this->flat) {
    this->gradient.setStep(step, idx);
    this->updateTransparency();
  } else {
    throw std::logic_error("Style does not contain gradient");
  }
}
void Style::addStep(GradientStep step, unsigned int idx) {
  if (!this->flat) {
    this->gradient.addStep(step, idx);
    this->updateTransparency();
  } else {
    throw std::logic_error("Style does not contain gradient");
  }
}
void Style::removeStep(unsigned int idx) {
  if (!this->flat) {
    this->gradient.removeStep(idx);
    this->updateTransparency();
  } else {
    throw std::logic_error("Style does not contain gradient");
  }
}
bool Style::hasGradient() { return !this->flat; }
bool Style::hasTransparency() { return this->transparent; }
void Style::updateTransparency() {
  if (this->flat) {
    if (color.alpha != 255) {
      this->transparent = true;
    } else {
      this->transparent = false;
    }
  } else {
    for (auto step : gradient.getSteps()) {
      if (step.color.alpha != 255) {
        this->transparent = true;
        return;
      }
    }
    this->transparent = false;
  }
}
Style Style::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return Style::fromBytes(bytes, idx);
}
Style Style::fromBytes(const std::vector<unsigned char> &bytes,
                       unsigned int &idx) {
  StyleType type(bytes.at(idx));
#ifdef DEBUG
  std::cout << "Processing style of type " << type << std::endl;
#endif
  switch (type) {
  case StyleType::SolidGrayNoAlpha:
  case StyleType::SolidGray:
  case StyleType::SolidColorNoAlpha:
  case StyleType::SolidColor:
    return Style(Color::fromBytes(bytes, idx));
    break;
  case StyleType::Gradient:
    return Style(Gradient::fromBytes(bytes, idx));
    break;
  default:
    throw std::range_error("Not a valid style type");
  }
}
std::vector<unsigned char> Style::toBytes() {
  std::vector<unsigned char> bytes;
  if (this->flat) {
    auto colorBytes = this->color.toBytes();
    bytes.insert(bytes.end(), colorBytes.begin(), colorBytes.end());
  } else {
    bytes.emplace_back(StyleType::Gradient);
    auto gradientBytes = this->gradient.toBytes();
    bytes.insert(bytes.end(), gradientBytes.begin(), gradientBytes.end());
  }
  return bytes;
}
} // namespace Girard
