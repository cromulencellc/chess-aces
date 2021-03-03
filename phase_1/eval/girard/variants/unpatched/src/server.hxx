#pragma once
#include "image.hxx"
#include <iostream>
namespace Girard {
const unsigned int MAX_IMAGES = 0x10;
class RequestType {
public:
  enum Value {
    Connect = 0x10,
    Disconnect = 0x11,
    Create = 0x20,
    Destroy = 0x21,
    Load = 0x22,
    Store = 0x23,
    SelectedImage = 0x30,
    SelectImage = 0x31,
    GetStyles = 0x40,
    GetStyle = 0x41,
    SetStyle = 0x42,
    AddStyle = 0x43,
    RemoveStyle = 0x44,
    IsFlat = 0x45,
    IsTransparent = 0x46,
    GetColor = 0x47,
    SetColor = 0x48,
    GetGradient = 0x49,
    SetGradient = 0x4A,
    SetStep = 0x4B,
    AddStep = 0x4C,
    RemoveStep = 0x4D,
    GetGradientTransformer = 0x4E,
    SetGradientTransformer = 0x4F,
    GetPaths = 0x60,
    GetPath = 0x61,
    SetPath = 0x62,
    AddPath = 0x63,
    RemovePath = 0x64,
    GetPoint = 0x65,
    SetPoint = 0x66,
    AddPoint = 0x67,
    RemovePoint = 0x68,
    GetShapes = 0x80,
    GetShape = 0x81,
    SetShape = 0x82,
    AddShape = 0x83,
    RemoveShape = 0x84,
    GetShapeStyle = 0x85,
    SetShapeStyle = 0x86,
    GetShapePaths = 0x87,
    SetShapePaths = 0x88,
    AddShapePath = 0x89,
    RemoveShapePath = 0x8A,
    HasHinting = 0x8B,
    SetHinting = 0x8C,
    GetMinVisibility = 0x90,
    SetMinVisibility = 0x91,
    GetMaxVisibility = 0x92,
    SetMaxVisibility = 0x93,
    GetTransformers = 0x94,
    GetTransformer = 0x95,
    SetTransformer = 0x96,
    AddTransformer = 0x97,
    RemoveTransformer = 0x98,
  };
  RequestType() = default;
  RequestType(unsigned char);
  constexpr RequestType(Value style) : value(style) {}
  operator Value() const { return value; }
  explicit operator bool() = delete;
private:
  Value value;
};
class ResponseType {
public:
  enum {
    Acknowledge = 0x00,
    Success = 0x01,
    Failure = 0x02,
    Result = 0x04,
  };
};
class Server {
public:
  Server() = delete;
  Server(unsigned short);
  Server(std::istream *, std::ostream *);
  void connect();
  void run();
  void disconnect();
private:
  std::vector<unsigned char> getSizedRequest();
  void respondAcknowledge();
  void respondFailure();
  void respondSuccess();
  void respondResult(std::vector<unsigned char> &);
  void handleConnect();
  void handleDisconnect();
  void handleCreate();
  void handleDestroy();
  void handleLoad();
  void handleStore();
  void handleSelectedImage();
  void handleSelectImage();
  void handleGetStyles();
  void handleGetStyle();
  void handleSetStyle();
  void handleAddStyle();
  void handleRemoveStyle();
  void handleIsFlat();
  void handleIsTransparent();
  void handleGetColor();
  void handleSetColor();
  void handleGetGradient();
  void handleSetGradient();
  void handleSetStep();
  void handleAddStep();
  void handleRemoveStep();
  void handleGetGradientTransformer();
  void handleSetGradientTransformer();
  void handleGetPaths();
  void handleGetPath();
  void handleSetPath();
  void handleAddPath();
  void handleRemovePath();
  void handleGetPoint();
  void handleSetPoint();
  void handleAddPoint();
  void handleRemovePoint();
  void handleGetShapes();
  void handleGetShape();
  void handleSetShape();
  void handleAddShape();
  void handleRemoveShape();
  void handleGetShapeStyle();
  void handleSetShapeStyle();
  void handleGetShapePaths();
  void handleSetShapePaths();
  void handleAddShapePath();
  void handleRemoveShapePath();
  void handleHasHinting();
  void handleSetHinting();
  void handleGetMinVisibility();
  void handleSetMinVisibility();
  void handleGetMaxVisibility();
  void handleSetMaxVisibility();
  void handleGetTransformers();
  void handleGetTransformer();
  void handleSetTransformer();
  void handleAddTransformer();
  void handleRemoveTransformer();
  bool connected;
  unsigned short port;
  int client;
  std::istream *cin;
  std::ostream *cout;
  unsigned char selected;
  std::vector<Image> images;
};
} // namespace Girard
