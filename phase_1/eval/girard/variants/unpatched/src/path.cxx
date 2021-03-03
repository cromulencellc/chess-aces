#ifdef DEBUG
#include <iostream>
#endif
#include "path.hxx"
#include <stdexcept>
namespace Girard {
PathCommand::PathCommand(int type) {
  switch (type) {
  case 0:
  case 1:
  case 2:
  case 3:
    value = static_cast<Value>(type);
    break;
  default:
    throw std::range_error("Not a valid style type");
  }
}
Path::Path() : closed(false), curved(false) {}
Point Path::getPoint(unsigned int idx) { return this->points.at(idx); }
std::vector<Point> Path::getPoints() { return this->points; }
void Path::setPoint(Point point, unsigned int idx) {
  auto &old = this->points.at(idx);
  old = point;
}
void Path::addPoint(Point point) { this->points.emplace_back(point); }
void Path::addPoint(Point point, unsigned int idx) {
  if (idx <= this->points.size()) {
    this->points.insert(this->points.begin() + idx, point);
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
void Path::removePoint(unsigned int idx) {
  if (idx < this->points.size()) {
    this->points.erase(this->points.begin() + idx);
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
bool Path::isClosed() { return this->closed; }
bool Path::isCurved() { return this->curved; }
Path Path::fromBytes(const std::vector<unsigned char> &bytes) {
  unsigned int idx = 0;
  return Path::fromBytes(bytes, idx);
}
Path Path::fromBytes(const std::vector<unsigned char> &bytes,
                     unsigned int &idx) {
  Path path;
  unsigned char flags = bytes.at(idx++);
  unsigned int npoints = bytes.at(idx++);
#ifdef DEBUG
  std::cout << "Decoding " << npoints << " points with flags " << +flags
            << std::endl;
#endif
  if ((flags & PathFlag::Closed) != 0) {
    path.closed = true;
  }
  if ((flags & PathFlag::UsesCommands) != 0) {
    unsigned char ncmdbytes = (npoints + 3) / 4;
    auto cmds = new PathCommand[npoints + 3];
    for (unsigned int j = 0; j < ncmdbytes; j++) {
      unsigned char cmdbyte = bytes.at(idx++);
      cmds[j * 4] = cmdbyte & 3;
      cmds[(j * 4) + 1] = (cmdbyte >> 2) & 3;
      cmds[(j * 4) + 2] = (cmdbyte >> 4) & 3;
      cmds[(j * 4) + 3] = cmdbyte >> 6;
    }
    Point last;
    for (unsigned int j = 0; j < npoints; j++) {
#ifdef DEBUG
      std::cout << "Decoding point with command " << +cmds[j] << std::endl;
#endif
      switch (cmds[j]) {
      case PathCommand::HorizontalLine: {
        float x = Point::readCoordinate(bytes, idx);
        path.addPoint(Point(x, last.y));
        last.x = x;
        break;
      }
      case PathCommand::VerticalLine: {
        float y = Point::readCoordinate(bytes, idx);
        path.addPoint(Point(last.x, y));
        last.y = y;
        break;
      }
      case PathCommand::Line: {
        float x = Point::readCoordinate(bytes, idx);
        float y = Point::readCoordinate(bytes, idx);
        path.addPoint(Point(x, y));
        last.x = x;
        last.y = y;
        break;
      }
      case PathCommand::Curve: {
        path.curved = true;
        float x = Point::readCoordinate(bytes, idx);
        float y = Point::readCoordinate(bytes, idx);
        float xin = Point::readCoordinate(bytes, idx);
        float yin = Point::readCoordinate(bytes, idx);
        float xout = Point::readCoordinate(bytes, idx);
        float yout = Point::readCoordinate(bytes, idx);
        path.addPoint(Point(x, y, xin, yin, xout, yout));
        last.x = x;
        last.y = y;
        break;
      }
      default:
        throw new std::range_error("Unknown path command");
      }
    }
  } else {
    for (unsigned int j = 0; j < npoints; j++) {
      if ((flags & PathFlag::NoCurves) == 0) {
        path.curved = true;
        float x = Point::readCoordinate(bytes, idx);
        float y = Point::readCoordinate(bytes, idx);
        float xin = Point::readCoordinate(bytes, idx);
        float yin = Point::readCoordinate(bytes, idx);
        float xout = Point::readCoordinate(bytes, idx);
        float yout = Point::readCoordinate(bytes, idx);
        path.addPoint(Point(x, y, xin, yin, xout, yout));
      } else {
        float x = Point::readCoordinate(bytes, idx);
        float y = Point::readCoordinate(bytes, idx);
        path.addPoint(Point(x, y));
      }
    }
  }
  return path;
}
std::vector<unsigned char> Path::toBytes() {
  std::vector<unsigned char> bytes;
  unsigned char flags = 0;
  if (this->closed) {
    flags |= PathFlag::Closed;
  }
  unsigned char straight = 0;
  unsigned char line = 0;
  unsigned char curve = 0;
  Point last;
  for (auto point : this->points) {
    if (point.x == point.xin && point.xin == point.xout &&
        point.y == point.yin && point.yin == point.yout) {
      if (point.x == last.x || point.y == last.y) {
        straight++;
      } else {
        line++;
      }
    } else {
      curve++;
    }
    last = point;
  }
  unsigned int commandLen =
      this->points.size() + straight * 2 + line * 4 + curve * 12;
  unsigned int plainLen = this->points.size() * 12;
  if (curve == 0) {
    flags |= PathFlag::NoCurves;
  }
  if (commandLen < plainLen) {
    flags |= PathFlag::UsesCommands;
  }
#ifdef DEBUG
  std::cout << "Encoding " << this->points.size() << " points with flags "
            << +flags << std::endl;
#endif
  bytes.emplace_back(flags);
  bytes.emplace_back(this->points.size());
  last = Point();
  unsigned int cmdIdx = bytes.size();
  int cmdLoc = 0;
  if (commandLen < plainLen) {
    for (unsigned int i = 0; i < (this->points.size() + 3) / 4; i++) {
      bytes.emplace_back(0);
    }
  }
  for (auto point : this->points) {
    if (commandLen < plainLen) {
      unsigned char command = 0;
      if (point.x == point.xin && point.xin == point.xout &&
          point.y == point.yin && point.yin == point.yout) {
        if (point.x == last.x || point.y == last.y) {
          if (point.x == last.x) {
            command = PathCommand::VerticalLine;
            Point::writeCoordinate(bytes, point.y);
          } else {
            command = PathCommand::HorizontalLine;
            Point::writeCoordinate(bytes, point.x);
          }
        } else {
          command = PathCommand::Line;
          Point::writeCoordinate(bytes, point.x);
          Point::writeCoordinate(bytes, point.y);
        }
      } else {
        command = PathCommand::Curve;
        Point::writeCoordinate(bytes, point.x);
        Point::writeCoordinate(bytes, point.y);
        Point::writeCoordinate(bytes, point.xin);
        Point::writeCoordinate(bytes, point.yin);
        Point::writeCoordinate(bytes, point.xout);
        Point::writeCoordinate(bytes, point.yout);
      }
      last = point;
      bytes.at(cmdIdx) |= command << (2 * cmdLoc);
      cmdLoc++;
      if (cmdLoc > 3) {
        cmdLoc = 0;
        cmdIdx++;
      }
    } else {
      if (curve == 0) {
        Point::writeCoordinate(bytes, point.x);
        Point::writeCoordinate(bytes, point.y);
      } else {
        Point::writeCoordinate(bytes, point.x);
        Point::writeCoordinate(bytes, point.y);
        Point::writeCoordinate(bytes, point.xin);
        Point::writeCoordinate(bytes, point.yin);
        Point::writeCoordinate(bytes, point.xout);
        Point::writeCoordinate(bytes, point.yout);
      }
    }
  }
  return bytes;
}
} // namespace Girard
