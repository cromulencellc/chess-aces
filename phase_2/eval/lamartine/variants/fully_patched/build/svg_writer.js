"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.SvgWriter = void 0;

var _slicedToArray2 = _interopRequireDefault(require("@babel/runtime/helpers/slicedToArray"));

var _classCallCheck2 = _interopRequireDefault(require("@babel/runtime/helpers/classCallCheck"));

var _createClass2 = _interopRequireDefault(require("@babel/runtime/helpers/createClass"));

var _defineProperty2 = _interopRequireDefault(require("@babel/runtime/helpers/defineProperty"));

var _fs = _interopRequireDefault(require("fs"));

var _path = _interopRequireDefault(require("path"));

var _xml = _interopRequireDefault(require("xml"));

var _map_classes = require("./map_classes");

function _createForOfIteratorHelper(o, allowArrayLike) { var it; if (typeof Symbol === "undefined" || o[Symbol.iterator] == null) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = o[Symbol.iterator](); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

var SvgWriter = /*#__PURE__*/function () {
  function SvgWriter(filename, map) {
    (0, _classCallCheck2["default"])(this, SvgWriter);
    (0, _defineProperty2["default"])(this, "map", void 0);
    (0, _defineProperty2["default"])(this, "out_stream", void 0);
    (0, _defineProperty2["default"])(this, "svg_root", void 0);
    (0, _defineProperty2["default"])(this, "svg_stream", void 0);
    this.map = map;
    this.out_stream = _fs["default"].createWriteStream(filename);
    this.svg_root = this._mk_root();
    this.svg_stream = (0, _xml["default"])({
      svg: this.svg_root
    }, {
      stream: true
    });
    this.svg_stream.pipe(this.out_stream);
  }

  (0, _createClass2["default"])(SvgWriter, [{
    key: "write",
    value: function write() {
      this._write_patterns();

      this._write_bg();

      this._write_sectors();

      this._write_linedefs();

      this.svg_root.close();
    }
  }, {
    key: "_mk_root",
    value: function _mk_root() {
      return _xml["default"].element({
        _attr: {
          viewBox: [this.map.min_x - 1, this.map.min_y - 1, 2 + this.map.max_x - this.map.min_x, 2 + this.map.max_y - this.map.min_y].join(' '),
          transform: 'scale(1, -1)',
          xmlns: "http://www.w3.org/2000/svg",
          fill: 'black'
        }
      });
    }
  }, {
    key: "_write_patterns",
    value: function _write_patterns() {
      var _iterator = _createForOfIteratorHelper(this.map.sectors_by_floor),
          _step;

      try {
        for (_iterator.s(); !(_step = _iterator.n()).done;) {
          var _step$value = (0, _slicedToArray2["default"])(_step.value, 2),
              floor = _step$value[0],
              _sxs = _step$value[1];

          var image_path = _path["default"].format({
            dir: '/static/flats',
            name: _path["default"].basename(floor.toLowerCase())
          });

          var image_data = _fs["default"].readFileSync(image_path);

          this.svg_root.push({
            pattern: [{
              _attr: {
                id: floor,
                viewBox: '0,0,64,64',
                width: '128px',
                height: '128px',
                patternUnits: 'userSpaceOnUse'
              }
            }, {
              image: [{
                _attr: {
                  href: 'data:image/png;base64,' + image_data.toString('base64'),
                  x: 0,
                  y: 0,
                  height: 64,
                  width: 64
                }
              }]
            }]
          });
        }
      } catch (err) {
        _iterator.e(err);
      } finally {
        _iterator.f();
      }
    }
  }, {
    key: "_write_bg",
    value: function _write_bg() {
      this.svg_root.push({
        rect: {
          _attr: {
            fill: 'black',
            x: this.map.min_x - 1,
            y: this.map.min_y - 1,
            width: 2 + this.map.max_x - this.map.min_x,
            height: 2 + this.map.max_y - this.map.min_y
          }
        }
      });
    }
  }, {
    key: "_write_sectors",
    value: function _write_sectors() {
      var _this = this;

      var _iterator2 = _createForOfIteratorHelper(this.map.sectors_by_floor),
          _step2;

      try {
        var _loop = function _loop() {
          var _step2$value = (0, _slicedToArray2["default"])(_step2.value, 2),
              floor = _step2$value[0],
              sxs = _step2$value[1];

          paths = [];
          sxs.forEach(function (sect) {
            var polys = sect.polygons();
            var cmds = [];
            polys.forEach(function (poly) {
              var first_vtx = poly[0];
              cmds.push("M ".concat(first_vtx.x, ",").concat(first_vtx.y));
              poly.slice(1, -1).forEach(function (v) {
                cmds.push("L ".concat(v.x, ",").concat(v.y));
              });
              cmds.push('Z');
            });
            var attrs = {
              d: cmds.join(' '),
              fill: "url(#".concat(floor, ")"),
              'fill-rule': 'evenodd'
            };

            _this.svg_root.push({
              path: {
                _attr: attrs
              }
            });
          });
        };

        for (_iterator2.s(); !(_step2 = _iterator2.n()).done;) {
          var paths;

          _loop();
        }
      } catch (err) {
        _iterator2.e(err);
      } finally {
        _iterator2.f();
      }
    }
  }, {
    key: "_write_linedefs",
    value: function _write_linedefs() {
      var draw_styles = new Map([[_map_classes.DrawStyle.one_sided, '#f00'], [_map_classes.DrawStyle.different_ceilings, '#ff0'], [_map_classes.DrawStyle.different_floors, '#940'], [_map_classes.DrawStyle.secret, '#ff0']]);

      var _iterator3 = _createForOfIteratorHelper(this.map.linedefs_by_drawstyle),
          _step3;

      try {
        for (_iterator3.s(); !(_step3 = _iterator3.n()).done;) {
          var _step3$value = (0, _slicedToArray2["default"])(_step3.value, 2),
              style = _step3$value[0],
              lds = _step3$value[1];

          if (_map_classes.DrawStyle.no_draw == style) continue;
          var color = draw_styles.get(style);

          if (undefined == color) {
            throw new Error("couldn't figuire out how to draw style ${style}");
          }

          var cmds = [];
          lds.forEach(function (ld) {
            cmds.push("M ".concat(ld.v1.x, ",").concat(ld.v1.y, " L ").concat(ld.v2.x, " ").concat(ld.v2.y));
          });
          var attrs = {
            d: cmds.join(' '),
            stroke: draw_styles.get(style)
          };
          this.svg_root.push({
            path: {
              _attr: attrs
            }
          });
        }
      } catch (err) {
        _iterator3.e(err);
      } finally {
        _iterator3.f();
      }
    }
  }]);
  return SvgWriter;
}();

exports.SvgWriter = SvgWriter;