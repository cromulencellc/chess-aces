#ifdef DEBUG
#include <iomanip>
#include <iostream>
#endif
#include "path.hxx"
#include <stdexcept>
namespace Girard {
Point::Point() : x(0.0), y(0.0), xin(0.0), yin(0.0), xout(0.0), yout(0.0) {}
Point::Point(float x, float y) : x(x), y(y), xin(x), yin(y), xout(x), yout(y) {}
Point::Point(float x, float y, float xin, float yin, float xout, float yout) {
  this->x = x;
  this->y = y;
  this->xin = xin;
  this->yin = yin;
  this->xout = xout;
  this->yout = yout;
}
Point Point::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return Point::fromBytes(bytes, idx);
}
Point Point::fromBytes(const std::vector<unsigned char> &bytes,
                       unsigned int &idx) {
  Point point;
  point.x = Point::readCoordinate(bytes, idx);
  point.y = Point::readCoordinate(bytes, idx);
  point.xin = Point::readCoordinate(bytes, idx);
  point.yin = Point::readCoordinate(bytes, idx);
  point.xout = Point::readCoordinate(bytes, idx);
  point.yout = Point::readCoordinate(bytes, idx);
  return point;
}
std::vector<unsigned char> Point::toBytes() {
  std::vector<unsigned char> bytes;
  Point::writeCoordinate(bytes, this->x);
  Point::writeCoordinate(bytes, this->y);
  Point::writeCoordinate(bytes, this->xin);
  Point::writeCoordinate(bytes, this->yin);
  Point::writeCoordinate(bytes, this->xout);
  Point::writeCoordinate(bytes, this->yout);
  return bytes;
}
float Point::readCoordinate(const std::vector<unsigned char> &bytes,
                            unsigned int &idx) {
  float coordinate;
#ifdef DEBUG
  std::cout << "Decoding bytes " << std::hex << +bytes.at(idx);
#endif
  unsigned char value = bytes.at(idx++);
  if (value & 0x80) {
#ifdef DEBUG
    std::cout << std::hex << +bytes.at(idx);
#endif
    unsigned short larger = ((value & 0x7F) << 8) | bytes.at(idx++);
    coordinate = (larger / 102.0) - 128.0;
  } else {
    coordinate = value - 32.0;
  }
#ifdef DEBUG
  std::cout << " to coordinate " << coordinate << std::endl;
#endif
  return coordinate;
}
void Point::writeCoordinate(std::vector<unsigned char> &bytes,
                            float coordinate) {
  if (coordinate < -128.0) {
    coordinate = -128.0;
  } else if (coordinate > 192.0) {
    coordinate = 192.0;
  }
  if (-32.0 <= coordinate && coordinate <= 95.0 &&
      coordinate == static_cast<int>(coordinate)) {
    unsigned char shorter =
        static_cast<unsigned char>(static_cast<char>(coordinate + 32.0));
#ifdef DEBUG
    std::cout << "Encoding coordinate " << coordinate << " to byte " << std::hex
              << +shorter << std::endl;
#endif
    bytes.push_back(shorter);
  } else {
    unsigned short larger = static_cast<unsigned short>(
        static_cast<short>((coordinate + 128.0) * 102.0));
    larger |= 0x8000;
#ifdef DEBUG
    std::cout << "Encoding coordinate " << coordinate << " to word " << std::hex
              << larger << std::endl;
#endif
    bytes.push_back(static_cast<unsigned char>(larger >> 8));
    bytes.push_back(static_cast<unsigned char>(larger));
  }
}
} // namespace Girard
