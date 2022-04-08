"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.DoomMap = void 0;

var _classCallCheck2 = _interopRequireDefault(require("@babel/runtime/helpers/classCallCheck"));

var _createClass2 = _interopRequireDefault(require("@babel/runtime/helpers/createClass"));

var _defineProperty2 = _interopRequireDefault(require("@babel/runtime/helpers/defineProperty"));

var _default_map = require("./default_map");

var DoomMap = /*#__PURE__*/function () {
  function DoomMap(_ref) {
    var _this = this;

    var linedefs = _ref.linedefs,
        sidedefs = _ref.sidedefs,
        vertexes = _ref.vertexes,
        sectors = _ref.sectors;
    (0, _classCallCheck2["default"])(this, DoomMap);
    (0, _defineProperty2["default"])(this, "linedefs", void 0);
    (0, _defineProperty2["default"])(this, "sidedefs", void 0);
    (0, _defineProperty2["default"])(this, "vertexes", void 0);
    (0, _defineProperty2["default"])(this, "sectors", void 0);
    (0, _defineProperty2["default"])(this, "sectors_by_floor", void 0);
    (0, _defineProperty2["default"])(this, "linedefs_by_drawstyle", void 0);
    (0, _defineProperty2["default"])(this, "min_x", Number.MAX_VALUE);
    (0, _defineProperty2["default"])(this, "min_y", Number.MAX_VALUE);
    (0, _defineProperty2["default"])(this, "max_x", Number.MIN_VALUE);
    (0, _defineProperty2["default"])(this, "max_y", Number.MIN_VALUE);
    this.linedefs = linedefs;
    this.sidedefs = sidedefs;
    this.vertexes = vertexes;
    this.sectors = sectors;
    this.linedefs.forEach(function (ld) {
      return ld.resolve(_this.vertexes, _this.sidedefs, _this.sectors);
    });
    this.sectors_by_floor = new _default_map.DefaultMap(function () {
      return new Set();
    });
    this.sectors.forEach(function (sector) {
      _this.sectors_by_floor.get(sector.texturefloor).add(sector);
    });
    this.linedefs_by_drawstyle = new _default_map.DefaultMap(function () {
      return new Set();
    });
    this.linedefs.forEach(function (ld) {
      _this.linedefs_by_drawstyle.get(ld.drawStyle()).add(ld);
    });
    this.vertexes.forEach(function (vx) {
      if (vx.x > _this.max_x) _this.max_x = vx.x;
      if (vx.x < _this.min_x) _this.min_x = vx.x;
      if (vx.y > _this.max_y) _this.max_y = vx.y;
      if (vx.y < _this.min_y) _this.min_y = vx.y;
    });
  }

  (0, _createClass2["default"])(DoomMap, [{
    key: "to_textmap",
    value: function to_textmap() {
      var lines = [];
      this.vertexes.forEach(function (vtx) {
        lines.push('vertex');
        lines.push('{');
        lines.push("x = ".concat(vtx.x, ";"));
        lines.push("y = ".concat(vtx.y, ";"));
        lines.push('}');
      });
      this.sidedefs.forEach(function (sd) {
        lines.push('sidedef');
        lines.push('{');
        lines.push("sector = ".concat(sd.si, ";"));
        lines.push('}');
      });
      this.sectors.forEach(function (st) {
        lines.push('sector');
        lines.push('{');
        lines.push("texturefloor = \"".concat(st.texturefloor, "\";"));
        lines.push("heightceiling = ".concat(st.heightceiling, ";"));
        lines.push("heightfloor = ".concat(st.heightfloor, ";"));
        lines.push('}');
      });
      this.linedefs.forEach(function (ld) {
        lines.push('linedef');
        lines.push('{');
        lines.push("v1 = ".concat(ld.v1i, ";"));
        lines.push("v2 = ".concat(ld.v2i, ";"));
        lines.push("sidefront = ".concat(ld.fsi, ";"));

        if (undefined != ld.bsi) {
          lines.push("sideback = ".concat(ld.bsi, ";"));
        }

        if (ld.secret) {
          lines.push('secret = true;');
        }

        lines.push('}');
      });
      return lines.join("\n");
    }
  }]);
  return DoomMap;
}();

exports.DoomMap = DoomMap;