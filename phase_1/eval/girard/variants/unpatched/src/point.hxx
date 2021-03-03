#pragma once
#include <vector>
namespace Girard {
class Point {
public:
  Point();
  Point(float, float);
  Point(float, float, float, float, float, float);
  static Point fromBytes(const std::vector<unsigned char> &);
  static Point fromBytes(const std::vector<unsigned char> &, unsigned int &);
  std::vector<unsigned char> toBytes();
  static float readCoordinate(const std::vector<unsigned char> &,
                              unsigned int &);
  static void writeCoordinate(std::vector<unsigned char> &, float);
  float x;
  float y;
  float xin;
  float yin;
  float xout;
  float yout;
private:
};
} // namespace Girard
