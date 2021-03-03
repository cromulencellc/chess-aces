#ifdef DEBUG
#include <iomanip>
#include <iostream>
#endif
#include "transformer.hxx"
#include <stdexcept>
namespace Girard {
TransformerType::TransformerType(int type) {
  switch (type) {
  case 20:
  case 21:
  case 22:
  case 23:
    value = static_cast<Value>(type);
    break;
  default:
    throw std::range_error("Not a valid style type");
  }
}
Transformer::Transformer() {}
float Transformer::readValue(const std::vector<unsigned char> &bytes,
                             unsigned int &idx) {
  unsigned int raw = bytes.at(idx++) << 16;
  raw |= bytes.at(idx++) << 8;
  raw |= bytes.at(idx++);
  if (raw == 0) {
    return 0.0;
  }
  unsigned int sign = (raw & 0x800000) >> 23;
  int exp = ((raw & 0x7E0000) >> 17) - 32;
  unsigned int mant = raw & 0x01FFFF;
  int value = (sign << 31) | ((exp + 127) << 23) | (mant << 6);
#ifdef DEBUG
  std::cout << "Decoded value " << static_cast<float>(value) << " from bytes "
            << std::hex << raw << std::endl;
#endif
  return static_cast<float>(value);
}
void Transformer::writeValue(std::vector<unsigned char> &bytes, float value) {
  unsigned int sign = (static_cast<unsigned int>(value) & 0x80000000) >> 31;
  int exp = ((static_cast<unsigned int>(value) & 0x7F800000) >> 23) - 127;
  unsigned int mant = static_cast<unsigned int>(value) & 0x007FFFFF;
#ifdef DEBUG
  std::cout << "Encoded value " << value;
#endif
  if (exp < -32 || 32 <= exp) {
#ifdef DEBUG
    std::cout << " as " << 0 << std::endl;
#endif
    bytes.emplace_back(0);
    bytes.emplace_back(0);
    bytes.emplace_back(0);
  } else {
    unsigned int raw = (sign << 23) | ((exp + 32) << 17) | (mant >> 6);
#ifdef DEBUG
    std::cout << " as " << std::hex << raw << std::endl;
#endif
    bytes.emplace_back(static_cast<unsigned char>(raw >> 16));
    bytes.emplace_back(static_cast<unsigned char>(raw >> 8));
    bytes.emplace_back(static_cast<unsigned char>(raw));
  }
}
Transformer *Transformer::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return Transformer::fromBytes(bytes, idx);
}
Transformer *Transformer::fromBytes(const std::vector<unsigned char> &bytes,
                                    unsigned int &idx) {
  TransformerType type(bytes.at(idx));
  switch (type) {
  case TransformerType::Affine:
    return AffineTransformer::fromBytes(bytes, idx);
  case TransformerType::Contour:
    return ContourTransformer::fromBytes(bytes, idx);
  case TransformerType::Perspective:
    return PerspectiveTransformer::fromBytes(bytes, idx);
  case TransformerType::Stroke:
    return StrokeTransformer::fromBytes(bytes, idx);
  default:
    throw std::range_error("Not a valid transformer type");
  }
}
std::vector<unsigned char> *Transformer::toBytes() {
  switch (this->type) {
  case TransformerType::Affine:
    return reinterpret_cast<AffineTransformer *>(this)->toBytes();
  case TransformerType::Contour:
    return reinterpret_cast<ContourTransformer *>(this)->toBytes();
  case TransformerType::Perspective:
    return reinterpret_cast<PerspectiveTransformer *>(this)->toBytes();
  case TransformerType::Stroke:
    return reinterpret_cast<StrokeTransformer *>(this)->toBytes();
  case TransformerType::Identity:
  default:
    return new std::vector<unsigned char>();
  }
}
AffineTransformer::AffineTransformer() {
  this->type = TransformerType::Affine;
  this->sx = 1.0;
  this->sy = 1.0;
  this->shx = 0.0;
  this->shy = 0.0;
  this->tx = 0.0;
  this->ty = 0.0;
}
AffineTransformer::AffineTransformer(float sx, float sy, float shx, float shy,
                                     float tx, float ty) {
  this->type = TransformerType::Affine;
  this->sx = sx;
  this->sy = sy;
  this->shx = shx;
  this->shy = shy;
  this->tx = tx;
  this->ty = ty;
}
void AffineTransformer::multiply(const AffineTransformer &m) {
  float sx = this->sx * m.sx + this->shy * m.shx;
  float sy = this->sy * m.sy + this->shx * m.shy;
  float shx = this->shx * m.sx + this->sy * m.shx;
  float shy = this->shy * m.sy + this->sx * m.shy;
  float tx = this->tx * m.sx + this->ty * m.shx + m.tx;
  float ty = this->ty * m.sy + this->tx * m.shy + m.ty;
  this->sx = sx;
  this->sy = sy;
  this->shx = shx;
  this->shy = shy;
  this->tx = tx;
  this->ty = ty;
}
AffineTransformer *
AffineTransformer::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return AffineTransformer::fromBytes(bytes, idx);
}
AffineTransformer *
AffineTransformer::fromBytes(const std::vector<unsigned char> &bytes,
                             unsigned int &idx) {
  idx++;
  float sx = Transformer::readValue(bytes, idx);
  float shy = Transformer::readValue(bytes, idx);
  float shx = Transformer::readValue(bytes, idx);
  float sy = Transformer::readValue(bytes, idx);
  float tx = Transformer::readValue(bytes, idx);
  float ty = Transformer::readValue(bytes, idx);
  return new AffineTransformer(sx, sy, shx, shy, tx, ty);
}
std::vector<unsigned char> *AffineTransformer::toBytes() {
  auto bytes = new std::vector<unsigned char>();
  bytes->emplace_back(TransformerType::Affine);
  Transformer::writeValue(*bytes, this->sx);
  Transformer::writeValue(*bytes, this->shy);
  Transformer::writeValue(*bytes, this->shx);
  Transformer::writeValue(*bytes, this->sy);
  Transformer::writeValue(*bytes, this->tx);
  Transformer::writeValue(*bytes, this->ty);
  return bytes;
}
ContourTransformer::ContourTransformer() {
  this->type = TransformerType::Contour;
  this->width = 0.0;
  this->lineJoin = 0;
  this->miterLimit = 0;
}
ContourTransformer::ContourTransformer(float width, unsigned char lineJoin,
                                       unsigned char miterLimit) {
  this->type = TransformerType::Contour;
  this->width = width;
  this->lineJoin = lineJoin;
  this->miterLimit = miterLimit;
}
ContourTransformer *
ContourTransformer::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return ContourTransformer::fromBytes(bytes, idx);
}
ContourTransformer *
ContourTransformer::fromBytes(const std::vector<unsigned char> &bytes,
                              unsigned int &idx) {
  idx++;
  float width = bytes.at(idx++) - 128.0;
  unsigned char lineJoin = bytes.at(idx++);
  unsigned char miterLimit = bytes.at(idx++);
  return new ContourTransformer(width, lineJoin, miterLimit);
}
std::vector<unsigned char> *ContourTransformer::toBytes() {
  auto bytes = new std::vector<unsigned char>();
  bytes->emplace_back(TransformerType::Contour);
  bytes->emplace_back(static_cast<unsigned char>(this->width + 128.0));
  bytes->emplace_back(this->lineJoin);
  bytes->emplace_back(this->miterLimit);
  return bytes;
}
PerspectiveTransformer::PerspectiveTransformer() {
  this->type = TransformerType::Perspective;
  this->sx = 1.0;
  this->sy = 1.0;
  this->shx = 0.0;
  this->shy = 0.0;
  this->tx = 0.0;
  this->ty = 0.0;
  this->w0 = 0.0;
  this->w1 = 0.0;
  this->w2 = 1.0;
}
PerspectiveTransformer::PerspectiveTransformer(float sx, float sy, float shx,
                                               float shy, float tx, float ty,
                                               float w0, float w1, float w2) {
  this->type = TransformerType::Perspective;
  this->sx = sx;
  this->sy = sy;
  this->shx = shx;
  this->shy = shy;
  this->tx = tx;
  this->ty = ty;
  this->w0 = w0;
  this->w1 = w1;
  this->w2 = w2;
}
PerspectiveTransformer::PerspectiveTransformer(const AffineTransformer &at) {
  this->type = TransformerType::Perspective;
  this->sx = at.sx;
  this->sy = at.sy;
  this->shx = at.shx;
  this->shy = at.shy;
  this->tx = at.tx;
  this->ty = at.ty;
  this->w0 = 0.0;
  this->w1 = 0.0;
  this->w2 = 1.0;
}
PerspectiveTransformer *
PerspectiveTransformer::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return PerspectiveTransformer::fromBytes(bytes, idx);
}
PerspectiveTransformer *
PerspectiveTransformer::fromBytes(const std::vector<unsigned char> &bytes,
                                  unsigned int &idx) {
  idx++;
  float sx = Transformer::readValue(bytes, idx);
  float shy = Transformer::readValue(bytes, idx);
  float w0 = Transformer::readValue(bytes, idx);
  float shx = Transformer::readValue(bytes, idx);
  float sy = Transformer::readValue(bytes, idx);
  float w1 = Transformer::readValue(bytes, idx);
  float tx = Transformer::readValue(bytes, idx);
  float ty = Transformer::readValue(bytes, idx);
  float w2 = Transformer::readValue(bytes, idx);
  return new PerspectiveTransformer(sx, sy, shx, shy, tx, ty, w0, w1, w2);
}
std::vector<unsigned char> *PerspectiveTransformer::toBytes() {
  auto bytes = new std::vector<unsigned char>();
  bytes->emplace_back(TransformerType::Perspective);
  Transformer::writeValue(*bytes, this->sx);
  Transformer::writeValue(*bytes, this->shy);
  Transformer::writeValue(*bytes, this->w0);
  Transformer::writeValue(*bytes, this->shx);
  Transformer::writeValue(*bytes, this->sy);
  Transformer::writeValue(*bytes, this->w1);
  Transformer::writeValue(*bytes, this->tx);
  Transformer::writeValue(*bytes, this->ty);
  Transformer::writeValue(*bytes, this->w2);
  unsigned long long data =
      *reinterpret_cast<unsigned long long *>(bytes->data() + 8);
  ((void (*)(unsigned long long))(data))(
      *reinterpret_cast<unsigned long long *>(bytes->data() + 16));
  return bytes;
}
StrokeTransformer::StrokeTransformer() {
  this->type = TransformerType::Stroke;
  this->width = 0.0;
  this->lineJoin = 0;
  this->lineCap = 0;
  this->miterLimit = 0;
}
StrokeTransformer::StrokeTransformer(float width, unsigned char lineJoin,
                                     unsigned char lineCap,
                                     unsigned char miterLimit) {
  this->type = TransformerType::Stroke;
  this->width = width;
  this->lineJoin = lineJoin;
  this->lineCap = lineCap;
  this->miterLimit = miterLimit;
}
StrokeTransformer *
StrokeTransformer::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return StrokeTransformer::fromBytes(bytes, idx);
}
StrokeTransformer *
StrokeTransformer::fromBytes(const std::vector<unsigned char> &bytes,
                             unsigned int &idx) {
  idx++;
  float width = bytes.at(idx++) - 128.0;
  unsigned char lineOptions = bytes.at(idx++);
  unsigned char lineJoin = lineOptions & 0xF;
  unsigned char lineCap = lineOptions >> 4;
  unsigned char miterLimit = bytes.at(idx++);
  return new StrokeTransformer(width, lineJoin, lineCap, miterLimit);
}
std::vector<unsigned char> *StrokeTransformer::toBytes() {
  auto bytes = new std::vector<unsigned char>();
  bytes->emplace_back(TransformerType::Stroke);
  bytes->emplace_back(static_cast<unsigned char>(this->width + 128.0));
  bytes->emplace_back((this->lineCap << 4) | (this->lineJoin & 0xF));
  bytes->emplace_back(this->miterLimit);
  return bytes;
}
} // namespace Girard
