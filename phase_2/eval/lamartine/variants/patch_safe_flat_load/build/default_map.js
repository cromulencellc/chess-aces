"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.DefaultMap = void 0;

var _classCallCheck2 = _interopRequireDefault(require("@babel/runtime/helpers/classCallCheck"));

var _createClass2 = _interopRequireDefault(require("@babel/runtime/helpers/createClass"));

var _defineProperty2 = _interopRequireDefault(require("@babel/runtime/helpers/defineProperty"));

var _Symbol$iterator;

_Symbol$iterator = Symbol.iterator;

var DefaultMap = /*#__PURE__*/function () {
  function DefaultMap(default_maker) {
    (0, _classCallCheck2["default"])(this, DefaultMap);
    (0, _defineProperty2["default"])(this, "default_maker", void 0);
    (0, _defineProperty2["default"])(this, "container", void 0);
    this.container = new Map();
    this.default_maker = default_maker;
  }

  (0, _createClass2["default"])(DefaultMap, [{
    key: "get",
    value: function get(key) {
      var got = this.container.get(key);
      if (undefined != got) return got;
      var made = this.default_maker();
      this.container.set(key, made);
      return made;
    }
  }, {
    key: "set",
    value: function set(key, value) {
      this.container.set(key, value);
    }
  }, {
    key: _Symbol$iterator,
    value: function value() {
      return this.container[Symbol.iterator]();
    }
  }]);
  return DefaultMap;
}();

exports.DefaultMap = DefaultMap;