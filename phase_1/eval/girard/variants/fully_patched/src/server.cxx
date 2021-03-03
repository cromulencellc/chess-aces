#include "server.hxx"
#include <algorithm>
#include <ext/stdio_filebuf.h>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
namespace Girard {
RequestType::RequestType(unsigned char type) {
  switch (type) {
  case 0x10:
  case 0x11:
  case 0x20:
  case 0x21:
  case 0x22:
  case 0x23:
  case 0x30:
  case 0x31:
  case 0x40:
  case 0x41:
  case 0x42:
  case 0x43:
  case 0x44:
  case 0x45:
  case 0x46:
  case 0x47:
  case 0x48:
  case 0x49:
  case 0x4A:
  case 0x4B:
  case 0x4C:
  case 0x4D:
  case 0x4E:
  case 0x4F:
  case 0x60:
  case 0x61:
  case 0x62:
  case 0x63:
  case 0x64:
  case 0x65:
  case 0x66:
  case 0x67:
  case 0x68:
  case 0x80:
  case 0x81:
  case 0x82:
  case 0x83:
  case 0x84:
  case 0x85:
  case 0x86:
  case 0x87:
  case 0x88:
  case 0x89:
  case 0x8A:
  case 0x8B:
  case 0x8C:
  case 0x90:
  case 0x91:
  case 0x92:
  case 0x93:
  case 0x94:
  case 0x95:
  case 0x96:
  case 0x97:
  case 0x98:
    value = static_cast<Value>(type);
    break;
  default:
    throw std::range_error("Not a valid request type value");
  }
}
Server::Server(unsigned short port) {
  this->connected = false;
  this->port = port;
  this->connect();
  this->selected = 0;
}
Server::Server(std::istream *in, std::ostream *out) {
  this->connected = false;
  this->port = 0;
  this->client = -1;
  this->cin = in;
  this->cout = out;
  this->selected = 0;
}
void Server::connect() {
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  int opt = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)) != 0) {
    throw std::runtime_error("Failed to set socket reuse option");
  }
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(this->port);
  if (bind(sock, reinterpret_cast<const struct sockaddr *>(&address),
           sizeof(address)) != 0) {
    throw std::runtime_error("Failed to bind socket to given port");
  }
  if (listen(sock, 1) != 0) {
    throw std::runtime_error("Failed to listen on open socket");
  }
  this->client = accept(sock, nullptr, nullptr);
  this->cin = new std::istream(new __gnu_cxx::stdio_filebuf<char>(
      dup(client), std::ios::binary | std::ios::in));
  this->cout = new std::ostream(new __gnu_cxx::stdio_filebuf<char>(
      dup(client), std::ios::binary | std::ios::out));
}
void Server::disconnect() {
  if (this->client != -1) {
    close(this->client);
    this->client = -1;
  }
}
void Server::run() {
  unsigned char byte;
  RequestType request;
  std::vector<unsigned char> data;
  while (true) {
    try {
      *this->cin >> std::noskipws >> byte;
      request = byte;
    } catch (std::range_error &e) {
      continue;
    }
    switch (request) {
    case RequestType::Connect:
      this->handleConnect();
      break;
    case RequestType::Disconnect:
      this->handleDisconnect();
      return;
    case RequestType::Create:
      this->handleCreate();
      break;
    case RequestType::Destroy:
      this->handleDestroy();
      break;
    case RequestType::Load:
      this->handleLoad();
      break;
    case RequestType::Store:
      this->handleStore();
      break;
    case RequestType::SelectedImage:
      this->handleSelectedImage();
      break;
    case RequestType::SelectImage:
      this->handleSelectImage();
      break;
    case RequestType::GetStyles:
      this->handleGetStyles();
      break;
    case RequestType::GetStyle:
      this->handleGetStyle();
      break;
    case RequestType::SetStyle:
      this->handleSetStyle();
      break;
    case RequestType::AddStyle:
      this->handleAddStyle();
      break;
    case RequestType::RemoveStyle:
      this->handleRemoveStyle();
      break;
    case RequestType::IsFlat:
      this->handleIsFlat();
      break;
    case RequestType::IsTransparent:
      this->handleIsTransparent();
      break;
    case RequestType::GetColor:
      this->handleGetColor();
      break;
    case RequestType::SetColor:
      this->handleSetColor();
      break;
    case RequestType::GetGradient:
      this->handleGetGradient();
      break;
    case RequestType::SetGradient:
      this->handleSetGradient();
      break;
    case RequestType::SetStep:
      this->handleSetStep();
      break;
    case RequestType::AddStep:
      this->handleAddStep();
      break;
    case RequestType::RemoveStep:
      this->handleRemoveStep();
      break;
    case RequestType::GetGradientTransformer:
      this->handleGetGradientTransformer();
      break;
    case RequestType::SetGradientTransformer:
      this->handleSetGradientTransformer();
      break;
    case RequestType::GetPaths:
      this->handleGetPaths();
      break;
    case RequestType::GetPath:
      this->handleGetPath();
      break;
    case RequestType::SetPath:
      this->handleSetPath();
      break;
    case RequestType::AddPath:
      this->handleAddPath();
      break;
    case RequestType::RemovePath:
      this->handleRemovePath();
      break;
    case RequestType::GetPoint:
      this->handleGetPoint();
      break;
    case RequestType::SetPoint:
      this->handleSetPoint();
      break;
    case RequestType::AddPoint:
      this->handleAddPoint();
      break;
    case RequestType::RemovePoint:
      this->handleRemovePoint();
      break;
    case RequestType::GetShapes:
      this->handleGetShapes();
      break;
    case RequestType::GetShape:
      this->handleGetShape();
      break;
    case RequestType::SetShape:
      this->handleSetShape();
      break;
    case RequestType::AddShape:
      this->handleAddShape();
      break;
    case RequestType::RemoveShape:
      this->handleRemoveShape();
      break;
    case RequestType::GetShapeStyle:
      this->handleGetShapeStyle();
      break;
    case RequestType::SetShapeStyle:
      this->handleSetShapeStyle();
      break;
    case RequestType::GetShapePaths:
      this->handleGetShapePaths();
      break;
    case RequestType::SetShapePaths:
      this->handleSetShapePaths();
      break;
    case RequestType::AddShapePath:
      this->handleAddShapePath();
      break;
    case RequestType::RemoveShapePath:
      this->handleRemoveShapePath();
      break;
    case RequestType::HasHinting:
      this->handleHasHinting();
      break;
    case RequestType::SetHinting:
      this->handleSetHinting();
      break;
    case RequestType::GetMinVisibility:
      this->handleGetMinVisibility();
      break;
    case RequestType::SetMinVisibility:
      this->handleSetMinVisibility();
      break;
    case RequestType::GetMaxVisibility:
      this->handleGetMaxVisibility();
      break;
    case RequestType::SetMaxVisibility:
      this->handleSetMaxVisibility();
      break;
    case RequestType::GetTransformers:
      this->handleGetTransformers();
      break;
    case RequestType::GetTransformer:
      this->handleGetTransformer();
      break;
    case RequestType::SetTransformer:
      this->handleSetTransformer();
      break;
    case RequestType::AddTransformer:
      this->handleAddTransformer();
      break;
    case RequestType::RemoveTransformer:
      this->handleRemoveTransformer();
      break;
    default:
      break;
    }
  }
}
std::vector<unsigned char> Server::getSizedRequest() {
  unsigned char byte;
  unsigned short size;
  *this->cin >> std::noskipws >> byte;
  size = byte;
  *this->cin >> std::noskipws >> byte;
  size |= (byte << 8);
  std::vector<unsigned char> data;
  for (unsigned int i = 0; i < size; i++) {
    *this->cin >> std::noskipws >> byte;
    data.emplace_back(byte);
  }
  return data;
}
void Server::respondAcknowledge() {
  *this->cout << static_cast<char>(ResponseType::Acknowledge) << std::flush;
}
void Server::respondFailure() {
  *this->cout << static_cast<char>(ResponseType::Failure) << std::flush;
}
void Server::respondSuccess() {
  *this->cout << static_cast<char>(ResponseType::Success) << std::flush;
}
void Server::respondResult(std::vector<unsigned char> &data) {
  unsigned short size = data.size();
  *this->cout << static_cast<char>(ResponseType::Result)
              << static_cast<char>(size) << static_cast<char>(size >> 8);
  for (auto c : data) {
    *this->cout << c;
  }
  *this->cout << std::flush;
}
void Server::handleConnect() {
  this->respondAcknowledge();
  this->connected = true;
}
void Server::handleDisconnect() {
  this->respondAcknowledge();
  this->connected = false;
  this->disconnect();
}
void Server::handleCreate() {
  if (this->images.size() == MAX_IMAGES) {
    this->respondFailure();
    return;
  }
  this->images.emplace_back(Image());
  this->respondSuccess();
  this->selected = this->images.size() - 1;
}
void Server::handleDestroy() {
  unsigned char iidx;
  *this->cin >> std::noskipws >> iidx;
  if (iidx < this->images.size()) {
    this->images.erase(this->images.begin() + iidx);
    this->respondSuccess();
    if (iidx <= this->selected && this->selected != 0) {
      this->selected -= 1;
    }
  } else {
    this->respondFailure();
  }
}
void Server::handleLoad() {
  auto data = this->getSizedRequest();
  if (this->images.size() == MAX_IMAGES) {
    this->respondFailure();
    return;
  }
  try {
    this->images.emplace_back(Image::load(data));
    this->respondSuccess();
    this->selected = this->images.size() - 1;
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleStore() {
  if (this->selected >= this->images.size()) {
    this->respondFailure();
    return;
  }
  try {
    std::vector<unsigned char> data;
    this->images.at(this->selected).store(data);
    this->respondResult(data);
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleSelectedImage() {
  if (this->selected >= this->images.size()) {
    this->respondFailure();
    return;
  }
  std::vector<unsigned char> data;
  data.emplace_back(this->selected);
  this->respondResult(data);
}
void Server::handleSelectImage() {
  unsigned char iidx;
  *this->cin >> std::noskipws >> iidx;
  if (iidx < this->images.size()) {
    this->selected = iidx;
    this->respondSuccess();
  } else {
    this->respondFailure();
  }
}
void Server::handleGetStyles() {
  try {
    std::vector<unsigned char> data;
    data.emplace_back(this->images.at(this->selected).getStyles());
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleGetStyle() {
  unsigned char sidx;
  *this->cin >> std::noskipws >> sidx;
  try {
    auto data = this->images.at(selected).getStyle(sidx).toBytes();
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetStyle() {
  auto data = this->getSizedRequest();
  try {
    unsigned char sidx = data.at(0);
    data.erase(data.begin());
    auto style = Style::fromBytes(data);
    this->images.at(this->selected).setStyle(style, sidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleAddStyle() {
  auto data = this->getSizedRequest();
  try {
    auto style = Style::fromBytes(data);
    this->images.at(this->selected).addStyle(style);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleRemoveStyle() {
  unsigned char sidx;
  *this->cin >> std::noskipws >> sidx;
  try {
    this->images.at(this->selected).removeStyle(sidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleIsFlat() {
  unsigned char sidx;
  *this->cin >> std::noskipws >> sidx;
  try {
    auto style = this->images.at(this->selected).getStyle(sidx);
    std::vector<unsigned char> data;
    if (!style.hasGradient()) {
      data.emplace_back(1);
      this->respondResult(data);
    } else {
      data.emplace_back(0);
      this->respondResult(data);
    }
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleIsTransparent() {
  unsigned char sidx;
  *this->cin >> std::noskipws >> sidx;
  try {
    auto style = this->images.at(this->selected).getStyle(sidx);
    std::vector<unsigned char> data;
    if (style.hasTransparency()) {
      data.emplace_back(1);
      this->respondResult(data);
    } else {
      data.emplace_back(0);
      this->respondResult(data);
    }
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleGetColor() {
  unsigned char sidx;
  *this->cin >> std::noskipws >> sidx;
  try {
    auto style = this->images.at(this->selected).getStyle(sidx);
    if (!style.hasGradient()) {
      auto data = style.getColor().toBytes();
      this->respondResult(data);
    } else {
      this->respondFailure();
    }
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetColor() {
  auto data = this->getSizedRequest();
  try {
    unsigned char sidx = data.at(0);
    data.erase(data.begin());
    auto color = Color::fromBytes(data);
    auto style = this->images.at(this->selected).getStyle(sidx);
    style.setColor(color);
    this->images.at(this->selected).setStyle(style, sidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleGetGradient() {
  unsigned char sidx;
  *this->cin >> std::noskipws >> sidx;
  try {
    auto style = this->images.at(this->selected).getStyle(sidx);
    if (style.hasGradient()) {
      auto data = style.getGradient().toBytes();
      this->respondResult(data);
    } else {
      this->respondFailure();
    }
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetGradient() {
  auto data = this->getSizedRequest();
  try {
    unsigned char sidx = data.at(0);
    data.erase(data.begin());
    auto gradient = Gradient::fromBytes(data);
    auto style = this->images.at(this->selected).getStyle(sidx);
    style.setGradient(gradient);
    this->images.at(this->selected).setStyle(style, sidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleSetStep() {
  auto data = this->getSizedRequest();
  try {
    unsigned char sidx = data.at(0);
    data.erase(data.begin());
    unsigned char gidx = data.at(0);
    data.erase(data.begin());
    unsigned int idx = 1;
    data.insert(data.begin() + idx, StyleType::SolidColor);
    auto color = Color::fromBytes(data, idx);
    auto step = GradientStep{data.at(0), color};
    auto style = this->images.at(this->selected).getStyle(sidx);
    style.setStep(step, gidx);
    this->images.at(this->selected).setStyle(style, sidx);
    this->respondSuccess();
  } catch (std::logic_error &e) {
    this->respondFailure();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleAddStep() {
  auto data = this->getSizedRequest();
  try {
    unsigned char sidx = data.at(0);
    data.erase(data.begin());
    unsigned char gidx = data.at(0);
    data.erase(data.begin());
    unsigned int idx = 1;
    data.insert(data.begin() + idx, StyleType::SolidColor);
    auto color = Color::fromBytes(data, idx);
    auto step = GradientStep{data.at(0), color};
    auto style = this->images.at(this->selected).getStyle(sidx);
    style.addStep(step, gidx);
    this->images.at(this->selected).setStyle(style, sidx);
    this->respondSuccess();
  } catch (std::logic_error &e) {
    this->respondFailure();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleRemoveStep() {
  auto data = this->getSizedRequest();
  try {
    unsigned char sidx = data.at(0);
    data.erase(data.begin());
    unsigned char gidx = data.at(0);
    data.erase(data.begin());
    auto style = this->images.at(this->selected).getStyle(sidx);
    style.removeStep(gidx);
    this->images.at(this->selected).setStyle(style, sidx);
    this->respondSuccess();
  } catch (std::logic_error &e) {
    this->respondFailure();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleGetGradientTransformer() {
  unsigned char sidx;
  *this->cin >> std::noskipws >> sidx;
  try {
    auto style = this->images.at(this->selected).getStyle(sidx);
    if (style.hasGradient()) {
      auto transformer = style.getGradient().getTransformer();
      if (transformer != nullptr) {
        auto data = transformer->toBytes();
        if (data->size() > 0) {
          data->erase(data->begin());
          this->respondResult(*data);
        } else {
          this->respondFailure();
        }
      } else {
        this->respondFailure();
      }
    } else {
      this->respondFailure();
    }
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetGradientTransformer() {
  auto data = this->getSizedRequest();
  try {
    unsigned char sidx = data.at(0);
    data.erase(data.begin());
    auto style = this->images.at(this->selected).getStyle(sidx);
    if (style.hasGradient()) {
      auto gradient = style.getGradient();
      gradient.setTransformer(
          reinterpret_cast<AffineTransformer *>(Transformer::fromBytes(data)));
      style.setGradient(gradient);
      this->images.at(this->selected).setStyle(style, sidx);
      this->respondSuccess();
    } else {
      this->respondFailure();
    }
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleGetPaths() {
  try {
    std::vector<unsigned char> data;
    data.emplace_back(this->images.at(this->selected).getPaths());
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleGetPath() {
  unsigned char byte;
  *this->cin >> std::noskipws >> byte;
  try {
    std::vector<unsigned char> data =
        this->images.at(selected).getPath(byte).toBytes();
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetPath() {
  auto data = this->getSizedRequest();
  try {
    unsigned char byte = data.at(0);
    data.erase(data.begin());
    auto path = Path::fromBytes(data);
    this->images.at(this->selected).setPath(path, byte);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleAddPath() {
  auto data = this->getSizedRequest();
  try {
    auto path = Path::fromBytes(data);
    this->images.at(this->selected).addPath(path);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleRemovePath() {
  unsigned char byte;
  *this->cin >> std::noskipws >> byte;
  try {
    this->images.at(this->selected).removePath(byte);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleGetPoint() {
  unsigned char pidx;
  *this->cin >> std::noskipws >> pidx;
  unsigned char ptidx;
  *this->cin >> std::noskipws >> ptidx;
  try {
    auto path = this->images.at(this->selected).getPath(pidx);
    auto point = path.getPoint(ptidx);
    auto data = point.toBytes();
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetPoint() {
  auto data = this->getSizedRequest();
  try {
    unsigned char pidx = data.at(0);
    data.erase(data.begin());
    auto point = Point::fromBytes(data);
    auto path = this->images.at(this->selected).getPath(pidx);
    unsigned char ptidx = data.at(0);
    data.erase(data.begin());
    path.setPoint(point, ptidx);
    this->images.at(this->selected).setPath(path, pidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleAddPoint() {
  auto data = this->getSizedRequest();
  try {
    unsigned char pidx = data.at(0);
    data.erase(data.begin());
    auto point = Point::fromBytes(data);
    auto path = this->images.at(this->selected).getPath(pidx);
    unsigned char ptidx = data.at(0);
    data.erase(data.begin());
    path.addPoint(point, ptidx);
    this->images.at(this->selected).setPath(path, pidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  }
}
void Server::handleRemovePoint() {
  unsigned char pidx;
  *this->cin >> std::noskipws >> pidx;
  unsigned char ptidx;
  *this->cin >> std::noskipws >> ptidx;
  try {
    auto path = this->images.at(this->selected).getPath(pidx);
    path.removePoint(ptidx);
    this->images.at(this->selected).setPath(path, pidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleGetShapes() {
  try {
    std::vector<unsigned char> data;
    data.emplace_back(this->images.at(this->selected).getShapes());
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleGetShape() {
  unsigned char byte;
  *this->cin >> std::noskipws >> byte;
  try {
    std::vector<unsigned char> data =
        this->images.at(selected).getShape(byte).toBytes();
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetShape() {
  auto data = this->getSizedRequest();
  try {
    unsigned char byte = data.at(0);
    data.erase(data.begin());
    auto shape = Shape::fromBytes(data);
    this->images.at(this->selected).setShape(shape, byte);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleAddShape() {
  auto data = this->getSizedRequest();
  try {
    auto shape = Shape::fromBytes(data);
    this->images.at(this->selected).addShape(shape);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::range_error &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleRemoveShape() {
  unsigned char byte;
  *this->cin >> std::noskipws >> byte;
  try {
    this->images.at(this->selected).removeShape(byte);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleGetShapeStyle() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    std::vector<unsigned char> data;
    data.emplace_back(shape.getStyle());
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetShapeStyle() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  unsigned char sidx;
  *this->cin >> std::noskipws >> sidx;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    shape.setStyle(sidx);
    this->images.at(this->selected).setShape(shape, shidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleGetShapePaths() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    auto data = shape.getPaths();
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetShapePaths() {
  auto data = this->getSizedRequest();
  try {
    unsigned char shidx = data.at(0);
    data.erase(data.begin());
    auto shape = this->images.at(this->selected).getShape(shidx);
    shape.setPaths(data);
    this->images.at(this->selected).setShape(shape, shidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleAddShapePath() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  unsigned char path;
  *this->cin >> std::noskipws >> path;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    shape.addPath(path);
    this->images.at(this->selected).setShape(shape, shidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleRemoveShapePath() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  unsigned char path;
  *this->cin >> std::noskipws >> path;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    auto paths = shape.getPaths();
    auto it = std::find(paths.begin(), paths.end(), path);
    if (it != paths.end()) {
      paths.erase(it);
      shape.setPaths(paths);
      this->images.at(this->selected).setShape(shape, shidx);
      this->respondSuccess();
    } else {
      this->respondFailure();
    }
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleHasHinting() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  try {
    std::vector<unsigned char> data;
    if (this->images.at(this->selected).getShape(shidx).hasHinting()) {
      data.emplace_back(1);
    } else {
      data.emplace_back(0);
    }
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetHinting() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  unsigned char hint;
  *this->cin >> std::noskipws >> hint;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    shape.setHinting((hint != 0) ? true : false);
    this->images.at(this->selected).setShape(shape, shidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleGetMinVisibility() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    std::vector<unsigned char> data;
    data.emplace_back(shape.getMinVisibility() * 63.75);
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetMinVisibility() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  unsigned char min;
  *this->cin >> std::noskipws >> min;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    shape.setMinVisibility(min / 63.75);
    this->images.at(this->selected).setShape(shape, shidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleGetMaxVisibility() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    std::vector<unsigned char> data;
    data.emplace_back(shape.getMaxVisibility() * 63.75);
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetMaxVisibility() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  unsigned char max;
  *this->cin >> std::noskipws >> max;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    shape.setMaxVisibility(max / 63.75);
    this->images.at(this->selected).setShape(shape, shidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleGetTransformers() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    std::vector<unsigned char> data;
    data.emplace_back(shape.getTransformers());
    this->respondResult(data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleGetTransformer() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  unsigned char tidx;
  *this->cin >> std::noskipws >> tidx;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    auto data = shape.getTransformer(tidx)->toBytes();
    this->respondResult(*data);
  } catch (std::out_of_range &e) {
    this->respondFailure();
  }
}
void Server::handleSetTransformer() {
  auto data = this->getSizedRequest();
  try {
    unsigned char shidx = data.at(0);
    data.erase(data.begin());
    unsigned char tidx = data.at(0);
    data.erase(data.begin());
    auto xformer = Transformer::fromBytes(data);
    auto shape = this->images.at(this->selected).getShape(shidx);
    shape.setTransformer(xformer, tidx);
    this->images.at(this->selected).setShape(shape, shidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleAddTransformer() {
  auto data = this->getSizedRequest();
  try {
    unsigned char shidx = data.at(0);
    data.erase(data.begin());
    unsigned char tidx = data.at(0);
    data.erase(data.begin());
    auto xform = Transformer::fromBytes(data);
    auto shape = this->images.at(this->selected).getShape(shidx);
    shape.addTransformer(xform, tidx);
    this->images.at(this->selected).setShape(shape, shidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
void Server::handleRemoveTransformer() {
  unsigned char shidx;
  *this->cin >> std::noskipws >> shidx;
  unsigned char tidx;
  *this->cin >> std::noskipws >> tidx;
  try {
    auto shape = this->images.at(this->selected).getShape(shidx);
    shape.removeTransformer(tidx);
    this->images.at(this->selected).setShape(shape, shidx);
    this->respondSuccess();
  } catch (std::out_of_range &e) {
    this->respondFailure();
  } catch (std::runtime_error &e) {
    this->respondFailure();
  }
}
} // namespace Girard
