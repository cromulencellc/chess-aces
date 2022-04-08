"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.Sectors = exports.Vertexes = exports.Sidedefs = exports.Linedefs = exports.LumpSizeError = exports.Lump = void 0;

var _createClass2 = _interopRequireDefault(require("@babel/runtime/helpers/createClass"));

var _assertThisInitialized2 = _interopRequireDefault(require("@babel/runtime/helpers/assertThisInitialized"));

var _inherits2 = _interopRequireDefault(require("@babel/runtime/helpers/inherits"));

var _possibleConstructorReturn2 = _interopRequireDefault(require("@babel/runtime/helpers/possibleConstructorReturn"));

var _getPrototypeOf2 = _interopRequireDefault(require("@babel/runtime/helpers/getPrototypeOf"));

var _wrapNativeSuper2 = _interopRequireDefault(require("@babel/runtime/helpers/wrapNativeSuper"));

var _classCallCheck2 = _interopRequireDefault(require("@babel/runtime/helpers/classCallCheck"));

var _defineProperty2 = _interopRequireDefault(require("@babel/runtime/helpers/defineProperty"));

var _map_classes = require("./map_classes");

var _too_big_error = require("./too_big_error");

var _util = _interopRequireDefault(require("util"));

var _util$inspect$custom;

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = (0, _getPrototypeOf2["default"])(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = (0, _getPrototypeOf2["default"])(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return (0, _possibleConstructorReturn2["default"])(this, result); }; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

function cleanBuf(b) {
  return b.toString().replace(/\0+$/, '');
}

var Lump = function Lump(name, expected_size, length) {
  (0, _classCallCheck2["default"])(this, Lump);
  (0, _defineProperty2["default"])(this, "name", void 0);
  (0, _defineProperty2["default"])(this, "length", void 0);
  (0, _defineProperty2["default"])(this, "expected_count", void 0);
  this.name = name;
  this.length = length;

  if (0 != length % expected_size) {
    throw new LumpSizeError(name, expected_size);
  }

  this.expected_count = length / expected_size;

  if (this.expected_count > Number.MAX_SAFE_INTEGER) {
    throw new _too_big_error.TooBigError();
  }
};

exports.Lump = Lump;

var LumpSizeError = /*#__PURE__*/function (_Error) {
  (0, _inherits2["default"])(LumpSizeError, _Error);

  var _super = _createSuper(LumpSizeError);

  function LumpSizeError(name, expectation) {
    (0, _classCallCheck2["default"])(this, LumpSizeError);
    return _super.call(this, "".concat(name, " content wasn't divisible by ").concat(expectation));
  }

  return LumpSizeError;
}( /*#__PURE__*/(0, _wrapNativeSuper2["default"])(Error));

exports.LumpSizeError = LumpSizeError;
_util$inspect$custom = _util["default"].inspect.custom;

var Linedefs = /*#__PURE__*/function (_Lump) {
  (0, _inherits2["default"])(Linedefs, _Lump);

  var _super2 = _createSuper(Linedefs);

  function Linedefs(reader, length) {
    var _this;

    (0, _classCallCheck2["default"])(this, Linedefs);
    _this = _super2.call(this, 'LINEDEFS', Linedefs.expected_size, length);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this), "linedefs", void 0);
    _this.linedefs = [];

    for (var i = 0; i < _this.expected_count; i++) {
      var v1 = reader.readUInt16();
      var v2 = reader.readUInt16();
      var flags = reader.readUInt16();

      var _special_type = reader.readUInt16();

      var _sector_tag = reader.readUInt16();

      var fsi = reader.readUInt16();
      var bsi_maybe = reader.readUInt16();
      if (0xffff == bsi_maybe) bsi_maybe = undefined;
      var secret = (0x20 & flags) > 0;

      _this.linedefs.push(new _map_classes.Linedef(v1, v2, fsi, bsi_maybe, secret));
    }

    return _this;
  }

  (0, _createClass2["default"])(Linedefs, [{
    key: _util$inspect$custom,
    value: function value() {
      return "Linedefs(".concat(this.linedefs.length, ")");
    }
  }]);
  return Linedefs;
}(Lump);

exports.Linedefs = Linedefs;
(0, _defineProperty2["default"])(Linedefs, "expected_size", 14);

var Sidedefs = /*#__PURE__*/function (_Lump2) {
  (0, _inherits2["default"])(Sidedefs, _Lump2);

  var _super3 = _createSuper(Sidedefs);

  function Sidedefs(reader, length) {
    var _this2;

    (0, _classCallCheck2["default"])(this, Sidedefs);
    _this2 = _super3.call(this, 'SIDEDEFS', Sidedefs.expected_size, length);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this2), "sidedefs", void 0);
    _this2.sidedefs = [];

    for (var i = 0; i < _this2.expected_count; i++) {
      var _xo = reader.readInt16();

      var _yo = reader.readInt16();

      var _upper = reader.readBytes(8);

      var _lower = reader.readBytes(8);

      var _middle = reader.readBytes(8);

      var si = reader.readUInt16();

      _this2.sidedefs.push(new _map_classes.Sidedef(si));
    }

    return _this2;
  }

  return Sidedefs;
}(Lump);

exports.Sidedefs = Sidedefs;
(0, _defineProperty2["default"])(Sidedefs, "expected_size", 30);

var Vertexes = /*#__PURE__*/function (_Lump3) {
  (0, _inherits2["default"])(Vertexes, _Lump3);

  var _super4 = _createSuper(Vertexes);

  function Vertexes(reader, length) {
    var _this3;

    (0, _classCallCheck2["default"])(this, Vertexes);
    _this3 = _super4.call(this, 'VERTEXES', Vertexes.expected_size, length);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this3), "vertexes", void 0);
    _this3.vertexes = [];

    for (var i = 0; i < _this3.expected_count; i++) {
      var x = reader.readInt16();
      var y = reader.readInt16();

      _this3.vertexes.push(new _map_classes.Vertex(x, y));
    }

    return _this3;
  }

  return Vertexes;
}(Lump);

exports.Vertexes = Vertexes;
(0, _defineProperty2["default"])(Vertexes, "expected_size", 4);

var Sectors = /*#__PURE__*/function (_Lump4) {
  (0, _inherits2["default"])(Sectors, _Lump4);

  var _super5 = _createSuper(Sectors);

  function Sectors(reader, length) {
    var _this4;

    (0, _classCallCheck2["default"])(this, Sectors);
    _this4 = _super5.call(this, 'SECTORS', Sectors.expected_size, length);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "sectors", void 0);
    _this4.sectors = [];

    for (var i = 0; i < _this4.expected_count; i++) {
      var heightfloor = reader.readInt16();
      var heightceiling = reader.readInt16();
      var texturefloor = cleanBuf(reader.readBytes(8));

      var _textureceiling = reader.readBytes(8);

      var _lightlevel = reader.readInt16();

      var _special = reader.readInt16();

      var _tag = reader.readInt16();

      _this4.sectors.push(new _map_classes.Sector(texturefloor, heightceiling, heightfloor));
    }

    return _this4;
  }

  return Sectors;
}(Lump);

exports.Sectors = Sectors;
(0, _defineProperty2["default"])(Sectors, "expected_size", 26);