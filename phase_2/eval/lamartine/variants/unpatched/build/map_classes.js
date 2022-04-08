"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.Linedef = exports.DrawStyle = exports.Sidedef = exports.Sector = exports.Vertex = exports.MalformedPolygonsError = exports.UndrawableLinedefError = exports.LinedefWithoutFrontSidedefError = exports.NoSuchObjectError = void 0;

var _slicedToArray2 = _interopRequireDefault(require("@babel/runtime/helpers/slicedToArray"));

var _classPrivateFieldGet2 = _interopRequireDefault(require("@babel/runtime/helpers/classPrivateFieldGet"));

var _createClass2 = _interopRequireDefault(require("@babel/runtime/helpers/createClass"));

var _assertThisInitialized2 = _interopRequireDefault(require("@babel/runtime/helpers/assertThisInitialized"));

var _defineProperty2 = _interopRequireDefault(require("@babel/runtime/helpers/defineProperty"));

var _classCallCheck2 = _interopRequireDefault(require("@babel/runtime/helpers/classCallCheck"));

var _inherits2 = _interopRequireDefault(require("@babel/runtime/helpers/inherits"));

var _possibleConstructorReturn2 = _interopRequireDefault(require("@babel/runtime/helpers/possibleConstructorReturn"));

var _getPrototypeOf2 = _interopRequireDefault(require("@babel/runtime/helpers/getPrototypeOf"));

var _wrapNativeSuper2 = _interopRequireDefault(require("@babel/runtime/helpers/wrapNativeSuper"));

var _util = _interopRequireDefault(require("util"));

function _createForOfIteratorHelper(o, allowArrayLike) { var it; if (typeof Symbol === "undefined" || o[Symbol.iterator] == null) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = o[Symbol.iterator](); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = (0, _getPrototypeOf2["default"])(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = (0, _getPrototypeOf2["default"])(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return (0, _possibleConstructorReturn2["default"])(this, result); }; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

var NoSuchObjectError = /*#__PURE__*/function (_Error) {
  (0, _inherits2["default"])(NoSuchObjectError, _Error);

  var _super = _createSuper(NoSuchObjectError);

  function NoSuchObjectError(index, type) {
    (0, _classCallCheck2["default"])(this, NoSuchObjectError);
    return _super.call(this, "the ".concat(type, " collection has no index ").concat(index));
  }

  return NoSuchObjectError;
}( /*#__PURE__*/(0, _wrapNativeSuper2["default"])(Error));

exports.NoSuchObjectError = NoSuchObjectError;

var LinedefWithoutFrontSidedefError = /*#__PURE__*/function (_Error2) {
  (0, _inherits2["default"])(LinedefWithoutFrontSidedefError, _Error2);

  var _super2 = _createSuper(LinedefWithoutFrontSidedefError);

  function LinedefWithoutFrontSidedefError(linedef) {
    (0, _classCallCheck2["default"])(this, LinedefWithoutFrontSidedefError);
    return _super2.call(this, "linedef ".concat(_util["default"].inspect(linedef), " is missing a front sidedef"));
  }

  return LinedefWithoutFrontSidedefError;
}( /*#__PURE__*/(0, _wrapNativeSuper2["default"])(Error));

exports.LinedefWithoutFrontSidedefError = LinedefWithoutFrontSidedefError;

var UndrawableLinedefError = /*#__PURE__*/function (_Error3) {
  (0, _inherits2["default"])(UndrawableLinedefError, _Error3);

  var _super3 = _createSuper(UndrawableLinedefError);

  function UndrawableLinedefError(linedef) {
    (0, _classCallCheck2["default"])(this, UndrawableLinedefError);
    return _super3.call(this, "couldn't figure out how to draw ".concat(_util["default"].inspect(linedef)));
  }

  return UndrawableLinedefError;
}( /*#__PURE__*/(0, _wrapNativeSuper2["default"])(Error));

exports.UndrawableLinedefError = UndrawableLinedefError;

var MalformedPolygonsError = /*#__PURE__*/function (_Error4) {
  (0, _inherits2["default"])(MalformedPolygonsError, _Error4);

  var _super4 = _createSuper(MalformedPolygonsError);

  function MalformedPolygonsError(polys) {
    (0, _classCallCheck2["default"])(this, MalformedPolygonsError);
    return _super4.call(this, "couldn't make sensible polygons out of ".concat(_util["default"].inspect(polys)));
  }

  return MalformedPolygonsError;
}( /*#__PURE__*/(0, _wrapNativeSuper2["default"])(Error));

exports.MalformedPolygonsError = MalformedPolygonsError;

function check_get(index, collection) {
  if (index >= collection.length) {
    throw new NoSuchObjectError(index, collection[0].constructor.name);
  }

  return collection[index];
}

var MapClass = function MapClass() {
  (0, _classCallCheck2["default"])(this, MapClass);
};

var Vertex = /*#__PURE__*/function (_MapClass) {
  (0, _inherits2["default"])(Vertex, _MapClass);

  var _super5 = _createSuper(Vertex);

  function Vertex(x, y) {
    var _this;

    (0, _classCallCheck2["default"])(this, Vertex);
    _this = _super5.call(this);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this), "x", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this), "y", void 0);
    _this.x = x;
    _this.y = y;
    return _this;
  }

  (0, _createClass2["default"])(Vertex, null, [{
    key: "fromBlock",
    value: function fromBlock(block) {
      return new Vertex(block.getNum('x'), block.getNum('y'));
    }
  }]);
  return Vertex;
}(MapClass);

exports.Vertex = Vertex;

var _polygons = new WeakMap();

var Sector = /*#__PURE__*/function (_MapClass2) {
  (0, _inherits2["default"])(Sector, _MapClass2);

  var _super6 = _createSuper(Sector);

  function Sector(texturefloor, heightceiling, heightfloor) {
    var _this2;

    (0, _classCallCheck2["default"])(this, Sector);
    _this2 = _super6.call(this);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this2), "texturefloor", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this2), "heightceiling", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this2), "heightfloor", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this2), "linedefs", new Set());

    _polygons.set((0, _assertThisInitialized2["default"])(_this2), {
      writable: true,
      value: new Array()
    });

    _this2.texturefloor = texturefloor;
    _this2.heightceiling = heightceiling;
    _this2.heightfloor = heightfloor;
    return _this2;
  }

  (0, _createClass2["default"])(Sector, [{
    key: "polygons",
    value: function polygons() {
      if (0 < (0, _classPrivateFieldGet2["default"])(this, _polygons).length) return (0, _classPrivateFieldGet2["default"])(this, _polygons);
      var valid_polygons = [];
      var unmatched_lines = new Array();

      var _iterator = _createForOfIteratorHelper(this.linedefs.entries()),
          _step;

      try {
        for (_iterator.s(); !(_step = _iterator.n()).done;) {
          var _step$value = (0, _slicedToArray2["default"])(_step.value, 2),
              linedef = _step$value[0],
              _ld = _step$value[1];

          unmatched_lines.push(linedef);
        }
      } catch (err) {
        _iterator.e(err);
      } finally {
        _iterator.f();
      }

      var current_polygon = [];
      var first_ld = unmatched_lines.pop();
      current_polygon.unshift(first_ld.v1);
      current_polygon.unshift(first_ld.v2);

      var _loop = function _loop() {
        var want_vertex = current_polygon[0];
        var matching_line = unmatched_lines.find(function (line) {
          if (line.v1 == want_vertex) return true;
          if (line.v2 == want_vertex) return true;
          return false;
        });

        if (undefined == matching_line) {
          // start new polygon
          valid_polygons.push(current_polygon);
          var new_line = unmatched_lines.pop();
          current_polygon = [];
          current_polygon.unshift(new_line.v1);
          current_polygon.unshift(new_line.v2);
          return "continue";
        }

        var rm = unmatched_lines.indexOf(matching_line);
        unmatched_lines.splice(rm, 1);

        if (matching_line.v1 == want_vertex) {
          current_polygon.unshift(matching_line.v2);
        } else {
          current_polygon.unshift(matching_line.v1);
        }
      };

      while (0 < unmatched_lines.length) {
        var _ret = _loop();

        if (_ret === "continue") continue;
      }

      valid_polygons.push(current_polygon);

      for (var _i = 0, _valid_polygons = valid_polygons; _i < _valid_polygons.length; _i++) {
        var p = _valid_polygons[_i];

        if (3 <= p.length) {
          (0, _classPrivateFieldGet2["default"])(this, _polygons).push(p);
        }
      }

      if (0 == (0, _classPrivateFieldGet2["default"])(this, _polygons).length) {
        throw new MalformedPolygonsError((0, _classPrivateFieldGet2["default"])(this, _polygons));
      }

      return (0, _classPrivateFieldGet2["default"])(this, _polygons);
    }
  }], [{
    key: "fromBlock",
    value: function fromBlock(block) {
      var texturefloor = block.getStr('texturefloor');
      var heightceiling = block.getNum('heightceiling', 0);
      var heightfloor = block.getNum('heightfloor', 0);
      return new Sector(texturefloor, heightceiling, heightfloor);
    }
  }]);
  return Sector;
}(MapClass);

exports.Sector = Sector;

var Sidedef = /*#__PURE__*/function (_MapClass3) {
  (0, _inherits2["default"])(Sidedef, _MapClass3);

  var _super7 = _createSuper(Sidedef);

  function Sidedef(si) {
    var _this3;

    (0, _classCallCheck2["default"])(this, Sidedef);
    _this3 = _super7.call(this);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this3), "si", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this3), "sector", void 0);
    _this3.si = si;
    return _this3;
  }

  (0, _createClass2["default"])(Sidedef, [{
    key: "resolve",
    value: function resolve(sectors) {
      this.sector = check_get(this.si, sectors);
    }
  }], [{
    key: "fromBlock",
    value: function fromBlock(block) {
      return new Sidedef(block.getNum('sector'));
    }
  }]);
  return Sidedef;
}(MapClass);

exports.Sidedef = Sidedef;
var DrawStyle;
exports.DrawStyle = DrawStyle;

(function (DrawStyle) {
  DrawStyle[DrawStyle["no_draw"] = 0] = "no_draw";
  DrawStyle[DrawStyle["one_sided"] = 1] = "one_sided";
  DrawStyle[DrawStyle["different_ceilings"] = 2] = "different_ceilings";
  DrawStyle[DrawStyle["different_floors"] = 3] = "different_floors";
  DrawStyle[DrawStyle["secret"] = 4] = "secret";
})(DrawStyle || (exports.DrawStyle = DrawStyle = {}));

var Linedef = /*#__PURE__*/function (_MapClass4) {
  (0, _inherits2["default"])(Linedef, _MapClass4);

  var _super8 = _createSuper(Linedef);

  function Linedef(v1i, v2i, fsi, bsi, secret) {
    var _this4;

    (0, _classCallCheck2["default"])(this, Linedef);
    _this4 = _super8.call(this);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "v1i", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "v2i", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "v1", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "v2", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "fsi", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "bsi", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "fs", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "bs", void 0);
    (0, _defineProperty2["default"])((0, _assertThisInitialized2["default"])(_this4), "secret", void 0);
    _this4.v1i = v1i;
    _this4.v2i = v2i;
    _this4.fsi = fsi;
    _this4.bsi = bsi;
    _this4.secret = secret;
    return _this4;
  }

  (0, _createClass2["default"])(Linedef, [{
    key: "resolve",
    value: function resolve(vertexes, sidedefs, sectors) {
      this.v1 = check_get(this.v1i, vertexes);
      this.v2 = check_get(this.v2i, vertexes);
      this.fs = check_get(this.fsi, sidedefs);
      this.fs.resolve(sectors);
      this.fs.sector.linedefs.add(this);

      if (undefined != this.bsi) {
        this.bs = check_get(this.bsi, sidedefs);
        this.bs.resolve(sectors);
        this.bs.sector.linedefs.add(this);
      }
    }
  }, {
    key: "drawStyle",
    value: function drawStyle() {
      if (undefined == this.fs) {
        throw new LinedefWithoutFrontSidedefError(this);
      }

      if (this.secret) return DrawStyle.secret;
      if (undefined == this.bs) return DrawStyle.one_sided;

      if (this.bs.sector.heightfloor != this.fs.sector.heightfloor) {
        return DrawStyle.different_floors;
      }

      if (this.bs.sector.heightceiling != this.fs.sector.heightceiling) {
        return DrawStyle.different_ceilings;
      }

      return DrawStyle.no_draw;
    }
  }], [{
    key: "fromBlock",
    value: function fromBlock(block) {
      return new Linedef(block.getNum('v1'), block.getNum('v2'), block.getNum('sidefront'), block.getNumU('sideback'), !!block.get('secret'));
    }
  }]);
  return Linedef;
}(MapClass);

exports.Linedef = Linedef;