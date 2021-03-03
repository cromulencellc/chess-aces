#ifdef DEBUG
#include <iostream>
#endif
#include "path.hxx"
#include "shape.hxx"
#include <set>
#include <stdexcept>
namespace Girard {
ShapeType::ShapeType(int type) {
  switch (type) {
  case 10:
    value = static_cast<Value>(type);
    break;
  default:
    throw std::range_error("Not a valid style type");
  }
}
Shape::Shape() {
  this->hinting = false;
  this->minVisibility = 0.0;
  this->maxVisibility = 4.0;
}
void Shape::setType(ShapeType t) { this->type = t; }
unsigned char Shape::getStyle() { return this->style; }
void Shape::setStyle(unsigned char idx) { this->style = idx; }
std::vector<unsigned char> Shape::getPaths() { return this->paths; }
void Shape::setPaths(std::vector<unsigned char> &paths) {
  this->paths = paths;
  this->checkPaths();
}
void Shape::addPath(unsigned char idx) {
  this->paths.push_back(idx);
  this->checkPaths();
}
void Shape::removePath(unsigned char idx) {
  if (idx < this->paths.size()) {
    this->paths.erase(this->paths.begin() + idx);
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
void Shape::checkPaths() {
  std::set<unsigned char> paths(this->paths.begin(), this->paths.end());
  this->paths.assign(paths.begin(), paths.end());
}
bool Shape::hasHinting() { return this->hinting; }
void Shape::setHinting(bool hinting) { this->hinting = hinting; }
float Shape::getMinVisibility() { return this->minVisibility; }
void Shape::setMinVisibility(float min) { this->minVisibility = min; }
float Shape::getMaxVisibility() { return this->maxVisibility; }
void Shape::setMaxVisibility(float max) { this->maxVisibility = max; }
unsigned char Shape::getTransformers() { return this->transformers.size(); }
Transformer *Shape::getTransformer(unsigned char idx) {
  return this->transformers.at(idx);
}
void Shape::setTransformer(Transformer *t, unsigned char idx) {
  Transformer *old = this->transformers.at(idx);
  this->transformers.at(idx) = t;
  delete old;
}
void Shape::addTransformer(Transformer *t) {
  this->transformers.emplace_back(t);
}
void Shape::addTransformer(Transformer *t, unsigned char idx) {
  if (idx <= this->transformers.size()) {
    this->transformers.insert(this->transformers.begin() + idx, t);
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
void Shape::removeTransformer(unsigned char idx) {
  if (idx < this->transformers.size()) {
    this->transformers.erase(this->transformers.begin() + idx);
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
Shape Shape::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return Shape::fromBytes(bytes, idx);
}
Shape Shape::fromBytes(const std::vector<unsigned char> &bytes,
                       unsigned int &idx) {
  Shape shape;
  shape.setType(ShapeType(bytes.at(idx++)));
  shape.style = bytes.at(idx++);
  unsigned char npaths = bytes.at(idx++);
  for (int i = 0; i < npaths; i++) {
    shape.paths.emplace_back(bytes.at(idx++));
  }
  shape.checkPaths();
  unsigned char flag = bytes.at(idx++);
  bool transform = (flag & ShapeFlag::Transform) != 0;
  shape.setHinting((flag & ShapeFlag::Hinting) != 0);
  bool lod = (flag & ShapeFlag::LevelOfDetailScale) != 0;
  bool hasTransformer = (flag & ShapeFlag::HasTransformers) != 0;
  bool translation = (flag & ShapeFlag::Translation) != 0;
#ifdef DEBUG
  std::cout << "Decoding shape of type " << +shape.type << " with style "
            << +shape.style << " with " << +npaths << " paths and flags "
            << +flag << std::endl;
#endif
  if (transform) {
    shape.transformers.emplace_back(AffineTransformer::fromBytes(bytes, --idx));
  } else if (translation) {
    float tx = Point::readCoordinate(bytes, idx);
    float ty = Point::readCoordinate(bytes, idx);
    if (tx != 0.0 || ty != 0.0) {
      shape.transformers.emplace_back(
          new AffineTransformer(1.0, 1.0, 0.0, 0.0, tx, ty));
    }
  }
  if (lod) {
    unsigned char min = bytes.at(idx++);
    unsigned char max = bytes.at(idx++);
    shape.minVisibility = min / 63.75;
    shape.maxVisibility = max / 63.75;
  }
  if (hasTransformer) {
    unsigned char ntransformers = bytes.at(idx++);
    for (int i = 0; i < ntransformers; i++) {
      shape.transformers.emplace_back(Transformer::fromBytes(bytes, idx));
    }
  }
  return shape;
}
std::vector<unsigned char> Shape::toBytes() {
  std::vector<unsigned char> bytes;
  bytes.emplace_back(this->type);
  bytes.emplace_back(this->style);
  bytes.emplace_back(this->paths.size());
  for (auto path : this->paths) {
    bytes.emplace_back(path);
  }
  unsigned char flag = 0;
  unsigned char flagidx = bytes.size();
  bytes.emplace_back(flag);
#ifdef DEBUG
  std::cout << "Encoding shape of type " << +this->type << " with style "
            << +this->style << " with " << this->paths.size() << " paths and "
            << this->transformers.size() << " transformers" << std::endl;
#endif
  bool packedFirst = false;
  if (this->transformers.size() == 1 &&
      this->transformers.at(0)->type == TransformerType::Affine) {
    packedFirst = true;
    auto t = reinterpret_cast<AffineTransformer *>(this->transformers.at(0));
    if (t->sx == 1.0 && t->sy == 1.0 && t->shx == 0.0 && t->shy == 0.0) {
      Point::writeCoordinate(bytes, t->tx);
      Point::writeCoordinate(bytes, t->ty);
      bytes.at(flagidx) |= ShapeFlag::Translation;
    } else {
      auto tbytes = t->toBytes();
      bytes.insert(bytes.end(), tbytes->begin() + 1, tbytes->end());
      bytes.at(flagidx) |= ShapeFlag::Transform;
    }
  }
  if (this->minVisibility != 0.0 || this->maxVisibility != 4.0) {
    bytes.emplace_back(this->minVisibility * 63.75);
    bytes.emplace_back(this->maxVisibility * 63.75);
    bytes.at(flagidx) |= ShapeFlag::LevelOfDetailScale;
  }
  if (this->transformers.size() > 0 &&
      !(this->transformers.size() == 1 && packedFirst)) {
    if (!packedFirst) {
      bytes.emplace_back(this->transformers.size());
    } else {
      bytes.emplace_back(this->transformers.size() - 1);
    }
    bool skip = packedFirst;
    for (auto transformer : this->transformers) {
      if (skip) {
        skip = false;
        continue;
      }
      std::vector<unsigned char> *transformerBytes;
      switch (transformer->type) {
      case TransformerType::Identity:
        continue;
      case TransformerType::Affine:
        transformerBytes =
            reinterpret_cast<AffineTransformer *>(transformer)->toBytes();
        break;
      case TransformerType::Contour:
        transformerBytes =
            reinterpret_cast<ContourTransformer *>(transformer)->toBytes();
        break;
      case TransformerType::Perspective:
        transformerBytes =
            reinterpret_cast<PerspectiveTransformer *>(transformer)->toBytes();
        break;
      case TransformerType::Stroke:
        transformerBytes =
            reinterpret_cast<StrokeTransformer *>(transformer)->toBytes();
        break;
      default:
        continue;
      }
      bytes.insert(bytes.end(), transformerBytes->begin(),
                   transformerBytes->end());
    }
    bytes.at(flagidx) |= ShapeFlag::HasTransformers;
  }
  if (this->hinting) {
    bytes.at(flagidx) |= ShapeFlag::Hinting;
  }
  return bytes;
}
} // namespace Girard
