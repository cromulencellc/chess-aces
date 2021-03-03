#ifdef DEBUG
#include <iostream>
#endif
#include "image.hxx"
#include <fstream>
#include <iterator>
namespace Girard {
Image::Image() {}
Image Image::load(const std::string &path) {
#ifdef DEBUG
  std::cout << "Loading image by file path: " << path << std::endl;
#endif
  std::ifstream ifs(path, std::ifstream::binary);
  if (!ifs) {
    throw std::runtime_error("Could not open input file");
  }
  ifs.seekg(0, ifs.end);
  auto size = ifs.tellg();
  ifs.seekg(0, ifs.beg);
  if (size > 0x10000) {
    throw std::runtime_error("Input file size is too large");
  }
  std::vector<unsigned char> buffer(size);
  ifs.read(reinterpret_cast<char *>(&buffer[0]), size);
  if (!ifs) {
    throw std::runtime_error("Could not read entire input file");
  }
  ifs.close();
  return load(buffer);
}
Image Image::load(const std::vector<unsigned char> &buffer) {
#ifdef DEBUG
  std::cout << "Loading image by buffer (size " << buffer.size() << ")"
            << std::endl;
#endif
  Image image;
  if (buffer.size() < 4) {
    throw std::runtime_error("Input file is too small");
  }
  unsigned int idx = 0;
  for (auto c : MAGIC) {
    if (buffer.at(idx++) != c) {
      throw std::runtime_error("Input file does not contain expected magic");
    }
  }
  try {
    image.styles = decodeStyles(buffer, idx);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Ran out of bytes while decoding styles");
  }
  try {
    image.paths = decodePaths(buffer, idx);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Ran out of bytes while decoding paths");
  }
  try {
    image.shapes = decodeShapes(buffer, idx);
  } catch (std::out_of_range &e) {
    throw std::runtime_error("Ran out of bytes while decoding shapes");
  }
#ifdef DEBUG
  if (idx != buffer.size()) {
    std::cout << "There are " << buffer.size() - idx
              << " extra bytes in the file" << std::endl;
  }
#endif
  for (auto shape : image.shapes) {
    try {
      auto style = image.styles.at(shape.getStyle());
      for (auto pathIdx : shape.getPaths()) {
        auto path = image.paths.at(pathIdx);
      }
    } catch (std::out_of_range &e) {
      throw std::runtime_error("Shape style or path out of range");
    }
  }
#ifdef DEBUG
  std::cout << "Image loaded" << std::endl;
#endif
  return image;
}
std::vector<Style> Image::decodeStyles(const std::vector<unsigned char> &buffer,
                                       unsigned int &idx) {
  std::vector<Style> styles;
  int nentries = buffer.at(idx++);
#ifdef DEBUG
  std::cout << "Decoding " << nentries << " styles at index " << idx - 1
            << std::endl;
#endif
  for (int i = 0; i < nentries; i++) {
    try {
      styles.emplace_back(Style::fromBytes(buffer, idx));
    } catch (std::range_error &e) {
#ifdef DEBUG
      std::cout << e.what() << std::endl;
#endif
      idx++;
      continue;
    }
  }
  return styles;
}
std::vector<Path> Image::decodePaths(const std::vector<unsigned char> &buffer,
                                     unsigned int &idx) {
  std::vector<Path> paths;
  int nentries = buffer.at(idx++);
#ifdef DEBUG
  std::cout << "Decoding " << nentries << " paths at index " << idx - 1
            << std::endl;
#endif
  for (int i = 0; i < nentries; i++) {
    try {
      paths.emplace_back(Path::fromBytes(buffer, idx));
    } catch (std::range_error &e) {
#ifdef DEBUG
      std::cout << e.what() << std::endl;
#endif
      idx++;
      continue;
    }
  }
  return paths;
}
std::vector<Shape> Image::decodeShapes(const std::vector<unsigned char> &buffer,
                                       unsigned int &idx) {
  std::vector<Shape> shapes;
  int nentries = buffer.at(idx++);
#ifdef DEBUG
  std::cout << "Decoding " << nentries << " shapes at index " << idx - 1
            << std::endl;
#endif
  for (int i = 0; i < nentries; i++) {
    try {
      shapes.emplace_back(Shape::fromBytes(buffer, idx));
    } catch (std::range_error &e) {
#ifdef DEBUG
      std::cout << e.what() << std::endl;
#endif
      idx++;
      continue;
    }
  }
  return shapes;
}
void Image::store(const std::string &path) {
#ifdef DEBUG
  std::cout << "Storing image by file path: " << path << std::endl;
#endif
  std::ofstream ofs(path, std::ifstream::binary);
  if (!ofs) {
    throw std::runtime_error("Could not open output file");
  }
  std::vector<unsigned char> buffer;
  this->store(buffer);
  ofs.write(reinterpret_cast<char *>(buffer.data()), buffer.size());
  ofs.close();
}
void Image::store(std::vector<unsigned char> &buffer) {
#ifdef DEBUG
  std::cout << "Storing image to buffer" << std::endl;
#endif
  for (auto c : MAGIC) {
    buffer.push_back(static_cast<unsigned char>(c));
  }
  this->encodeStyles(buffer);
  this->encodePaths(buffer);
  this->encodeShapes(buffer);
#ifdef DEBUG
  std::cout << "Image stored (size " << buffer.size() << ")" << std::endl;
#endif
}
void Image::encodeStyles(std::vector<unsigned char> &buffer) {
#ifdef DEBUG
  std::cout << "Encoding " << styles.size() << " styles" << std::endl;
#endif
  buffer.emplace_back(this->styles.size());
  for (auto style : this->styles) {
    auto styleBytes = style.toBytes();
    buffer.insert(buffer.end(), styleBytes.begin(), styleBytes.end());
  }
}
void Image::encodePaths(std::vector<unsigned char> &buffer) {
#ifdef DEBUG
  std::cout << "Encoding " << paths.size() << " paths" << std::endl;
#endif
  buffer.emplace_back(this->paths.size());
  for (auto path : this->paths) {
    auto pathBytes = path.toBytes();
    buffer.insert(buffer.end(), pathBytes.begin(), pathBytes.end());
  }
}
void Image::encodeShapes(std::vector<unsigned char> &buffer) {
#ifdef DEBUG
  std::cout << "Encoding " << shapes.size() << " shapes" << std::endl;
#endif
  buffer.emplace_back(this->shapes.size());
  for (auto shape : this->shapes) {
    auto shapeBytes = shape.toBytes();
    buffer.insert(buffer.end(), shapeBytes.begin(), shapeBytes.end());
  }
}
unsigned char Image::getStyles() { return this->styles.size(); }
Style Image::getStyle(unsigned char idx) { return this->styles.at(idx); }
void Image::setStyle(Style &style, unsigned char idx) {
  auto &old = this->styles.at(idx);
  old = style;
}
void Image::addStyle(Style &style) { this->styles.emplace_back(style); }
void Image::removeStyle(unsigned char idx) {
  for (auto shape : this->shapes) {
    if (shape.getStyle() == idx) {
      throw std::runtime_error("Style is currently in-use by a shape");
    }
  }
  if (idx < this->styles.size()) {
    this->styles.erase(this->styles.begin() + idx);
    for (unsigned int i = 0; i < this->shapes.size(); i++) {
      auto style = this->shapes.at(i).getStyle();
      if (style > idx) {
        this->shapes[i].setStyle(style - 1);
      }
    }
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
unsigned char Image::getPaths() { return this->paths.size(); }
Path Image::getPath(unsigned char idx) { return this->paths.at(idx); }
void Image::setPath(Path &path, unsigned char idx) {
  auto &old = this->paths.at(idx);
  old = path;
}
void Image::addPath(Path &path) { this->paths.emplace_back(path); }
void Image::removePath(unsigned char idx) {
  for (auto shape : this->shapes) {
    for (auto path : shape.getPaths()) {
      if (path == idx) {
        throw std::runtime_error("Path is currently in-use by a shape");
      }
    }
  }
  if (idx < this->paths.size()) {
    this->paths.erase(this->paths.begin() + idx);
    for (unsigned int i = 0; i < this->shapes.size(); i++) {
      auto paths = this->shapes.at(i).getPaths();
      for (unsigned int j = 0; j < paths.size(); j++) {
        if (paths.at(j) > i) {
          paths[j] -= 1;
        }
      }
    }
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
void Image::checkShape(Shape &shape) {
  if (shape.getStyle() > this->styles.size()) {
    throw std::runtime_error("Shape contains an invalid style reference");
  }
  for (auto path : shape.getPaths()) {
    if (path > this->paths.size()) {
      throw std::runtime_error("Shape contains an invalid path reference");
    }
  }
}
unsigned char Image::getShapes() { return this->shapes.size(); }
Shape Image::getShape(unsigned char idx) { return this->shapes.at(idx); }
void Image::setShape(Shape &shape, unsigned char idx) {
  this->checkShape(shape);
  auto &old = this->shapes.at(idx);
  old = shape;
}
void Image::addShape(Shape &shape) {
  this->checkShape(shape);
  this->shapes.emplace_back(shape);
}
void Image::removeShape(unsigned char idx) {
  if (idx < this->shapes.size()) {
    this->shapes.erase(this->shapes.begin() + idx);
  } else {
    throw std::out_of_range("Requested index is not in range of vector");
  }
}
} // namespace Girard
