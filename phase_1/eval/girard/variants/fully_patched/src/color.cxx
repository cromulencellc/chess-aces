#ifdef DEBUG
#include <iostream>
#endif
#include "style.hxx"
#include <stdexcept>
namespace Girard {
Color::Color() : red(0), green(0), blue(0), alpha(0) {}
Color::Color(const unsigned char r, const unsigned char g,
             const unsigned char b) {
  this->red = r;
  this->green = g;
  this->blue = b;
  this->alpha = 255;
}
Color::Color(const unsigned char r, const unsigned char g,
             const unsigned char b, const unsigned char a) {
  this->red = r;
  this->green = g;
  this->blue = b;
  this->alpha = a;
}
Color Color::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return Color::fromBytes(bytes, idx);
}
Color Color::fromBytes(const std::vector<unsigned char> &bytes,
                       unsigned int &idx) {
  StyleType type = bytes.at(idx++);
#ifdef DEBUG
  std::cout << "Decoding color type " << type << std::endl;
#endif
  switch (type) {
  case StyleType::SolidGrayNoAlpha: {
    unsigned char gray = bytes.at(idx++);
    return Color(gray, gray, gray);
  }
  case StyleType::SolidGray: {
    unsigned char gray = bytes.at(idx++);
    unsigned char alpha = bytes.at(idx++);
    return Color(gray, gray, gray, alpha);
  }
  case StyleType::SolidColorNoAlpha: {
    unsigned char red = bytes.at(idx++);
    unsigned char green = bytes.at(idx++);
    unsigned char blue = bytes.at(idx++);
    return Color(red, green, blue);
  }
  case StyleType::SolidColor: {
    unsigned char red = bytes.at(idx++);
    unsigned char green = bytes.at(idx++);
    unsigned char blue = bytes.at(idx++);
    unsigned char alpha = bytes.at(idx++);
    return Color(red, green, blue, alpha);
  }
  default:
    throw std::range_error("Not a valid color style");
  }
}
std::vector<unsigned char> Color::toBytes() {
  std::vector<unsigned char> bytes;
#ifdef DEBUG
  std::cout << "Encoding color " << this->red << " " << this->green << " "
            << this->blue << " " << this->alpha << std::endl;
#endif
  if (this->red == this->green && this->green == this->blue) {
    if (this->alpha == 255) {
      bytes.emplace_back(StyleType::SolidGrayNoAlpha);
      bytes.emplace_back(this->red);
    } else {
      bytes.emplace_back(StyleType::SolidGray);
      bytes.emplace_back(this->red);
      bytes.emplace_back(this->alpha);
    }
  } else {
    if (this->alpha == 255) {
      bytes.emplace_back(StyleType::SolidColorNoAlpha);
      bytes.emplace_back(this->red);
      bytes.emplace_back(this->green);
      bytes.emplace_back(this->blue);
    } else {
      bytes.emplace_back(StyleType::SolidColor);
      bytes.emplace_back(this->red);
      bytes.emplace_back(this->green);
      bytes.emplace_back(this->blue);
      bytes.emplace_back(this->alpha);
    }
  }
  return bytes;
}
GradientType::GradientType(int type) {
  switch (type) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    value = static_cast<Value>(type);
    break;
  default:
    throw std::range_error("Not a valid gradient type");
  }
}
Gradient::Gradient() : transformer(nullptr), type(GradientType::Linear) {}
Gradient::Gradient(GradientType type) : transformer(nullptr), type(type) {}
GradientType Gradient::getType() { return this->type; }
void Gradient::setType(GradientType type) { this->type = type; }
GradientStep Gradient::getStep(unsigned int idx) { return this->steps.at(idx); }
std::vector<GradientStep> Gradient::getSteps() { return this->steps; }
void Gradient::setStep(GradientStep step, unsigned int idx) {
  auto &old = this->steps.at(idx);
  old = step;
}
void Gradient::addStep(GradientStep step) { this->steps.emplace_back(step); }
void Gradient::addStep(GradientStep step, unsigned int idx) {
  if (idx <= this->steps.size()) {
    this->steps.insert(this->steps.begin() + idx, step);
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
void Gradient::removeStep(unsigned int idx) {
  if (idx < this->steps.size()) {
    this->steps.erase(this->steps.begin() + idx);
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
AffineTransformer *Gradient::getTransformer() { return this->transformer; }
void Gradient::setTransformer(AffineTransformer *transformer) {
  this->transformer = transformer;
}
Gradient Gradient::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return Gradient::fromBytes(bytes, idx);
}
Gradient Gradient::fromBytes(const std::vector<unsigned char> &bytes,
                             unsigned int &idx) {
  StyleType styleType = bytes.at(idx++);
  if (styleType != StyleType::Gradient) {
    throw std::range_error("Not a valid gradient style");
  }
  Gradient gradient(GradientType(bytes.at(idx++)));
  unsigned char flags = bytes.at(idx++);
  unsigned char stops = bytes.at(idx++);
#ifdef DEBUG
  std::cout << "Decoding gradient of type " << static_cast<unsigned int>(flags)
            << " with " << static_cast<unsigned int>(stops) << " stops"
            << std::endl;
#endif
  if ((flags & GradientFlag::Transform) != 0) {
    gradient.transformer = AffineTransformer::fromBytes(bytes, --idx);
  }
  for (int j = 0; j < stops; j++) {
    unsigned char stop = bytes.at(idx++);
    if ((flags & GradientFlag::Grays) != 0) {
      unsigned char gray = bytes.at(idx++);
      if ((flags & GradientFlag::NoAlpha) == 0) {
        unsigned char alpha = bytes.at(idx++);
        gradient.addStep(GradientStep{stop, Color(gray, gray, gray, alpha)});
      } else {
        gradient.addStep(GradientStep{stop, Color(gray, gray, gray)});
      }
    } else {
      unsigned char red = bytes.at(idx++);
      unsigned char green = bytes.at(idx++);
      unsigned char blue = bytes.at(idx++);
      if ((flags & GradientFlag::NoAlpha) == 0) {
        unsigned char alpha = bytes.at(idx++);
        gradient.addStep(GradientStep{stop, Color(red, green, blue, alpha)});
      } else {
        gradient.addStep(GradientStep{stop, Color(red, green, blue)});
      }
    }
  }
  return gradient;
}
std::vector<unsigned char> Gradient::toBytes() {
  std::vector<unsigned char> bytes;
  bytes.emplace_back(this->type);
  unsigned char flags = 0;
  bool allGray = true;
  bool noAlpha = true;
  for (auto step : this->steps) {
    if (!(step.color.red == step.color.green &&
          step.color.green == step.color.blue)) {
      allGray = false;
    }
    if (step.color.alpha != 255) {
      noAlpha = false;
    }
  }
  if (allGray) {
    flags |= GradientFlag::Grays;
  }
  if (noAlpha) {
    flags |= GradientFlag::NoAlpha;
  }
  if (this->transformer != nullptr) {
    flags |= GradientFlag::Transform;
  }
  bytes.emplace_back(flags);
#ifdef DEBUG
  std::cout << "Encoding gradient of type " << static_cast<unsigned int>(flags)
            << " with " << this->steps.size() << " stops" << std::endl;
#endif
  bytes.emplace_back(static_cast<unsigned char>(this->steps.size()));
  if (transformer != nullptr) {
    auto transformerBytes = this->transformer->toBytes();
    bytes.insert(bytes.end(), transformerBytes->begin() + 1,
                 transformerBytes->end());
  }
  for (auto step : this->steps) {
    bytes.emplace_back(step.stop);
    if (allGray) {
      if (noAlpha) {
        bytes.emplace_back(step.color.red);
      } else {
        bytes.emplace_back(step.color.red);
        bytes.emplace_back(step.color.alpha);
      }
    } else {
      if (noAlpha) {
        bytes.emplace_back(step.color.red);
        bytes.emplace_back(step.color.green);
        bytes.emplace_back(step.color.blue);
      } else {
        bytes.emplace_back(step.color.red);
        bytes.emplace_back(step.color.green);
        bytes.emplace_back(step.color.blue);
        bytes.emplace_back(step.color.alpha);
      }
    }
  }
  return bytes;
}
} // namespace Girard
