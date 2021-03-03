#pragma once
#include "path.hxx"
#include "shape.hxx"
#include "style.hxx"
#include <string>
#include <vector>
namespace Girard {
const std::string MAGIC = "ncif";
class Image {
public:
  Image();
  static Image load(const std::string &);
  static Image load(const std::vector<unsigned char> &);
  void store(const std::string &);
  void store(std::vector<unsigned char> &);
  unsigned char getStyles();
  Style getStyle(unsigned char);
  void setStyle(Style &, unsigned char);
  void addStyle(Style &);
  void removeStyle(unsigned char);
  unsigned char getPaths();
  Path getPath(unsigned char);
  void setPath(Path &, unsigned char);
  void addPath(Path &);
  void removePath(unsigned char);
  unsigned char getShapes();
  Shape getShape(unsigned char);
  void setShape(Shape &, unsigned char);
  void addShape(Shape &);
  void removeShape(unsigned char);
private:
  static std::vector<Style> decodeStyles(const std::vector<unsigned char> &,
                                         unsigned int &);
  static std::vector<Path> decodePaths(const std::vector<unsigned char> &,
                                       unsigned int &);
  static std::vector<Shape> decodeShapes(const std::vector<unsigned char> &,
                                         unsigned int &);
  void encodeStyles(std::vector<unsigned char> &);
  void encodePaths(std::vector<unsigned char> &);
  void encodeShapes(std::vector<unsigned char> &);
  void checkShape(Shape &);
  std::vector<Style> styles;
  std::vector<Path> paths;
  std::vector<Shape> shapes;
};
} // namespace Girard
