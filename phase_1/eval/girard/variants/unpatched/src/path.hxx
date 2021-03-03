#pragma once
#include "point.hxx"
#include <vector>
namespace Girard {
class PathFlag {
public:
  enum { Closed = 1 << 1, UsesCommands = 1 << 2, NoCurves = 1 << 3 };
};
class PathCommand {
public:
  enum Value { HorizontalLine = 0, VerticalLine = 1, Line = 2, Curve = 3 };
  PathCommand() = default;
  PathCommand(int);
  constexpr PathCommand(Value style) : value(style) {}
  operator Value() const { return value; }
  explicit operator bool() = delete;
private:
  Value value;
};
class Path {
public:
  Path();
  Point getPoint(unsigned int);
  std::vector<Point> getPoints();
  void setPoint(Point, unsigned int);
  void addPoint(Point);
  void addPoint(Point, unsigned int);
  void removePoint(unsigned int);
  bool isClosed();
  bool isCurved();
  static Path fromBytes(const std::vector<unsigned char> &);
  static Path fromBytes(const std::vector<unsigned char> &, unsigned int &);
  std::vector<unsigned char> toBytes();
private:
  bool closed;
  bool curved;
  std::vector<Point> points;
};
} // namespace Girard
