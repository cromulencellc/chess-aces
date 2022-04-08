"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.UdmfMap = exports.Assignment = exports.Block = exports.Keyword = exports.QuotedString = exports.Identifier = exports.UnknownBlockEntryError = void 0;

var _typeof2 = _interopRequireDefault(require("@babel/runtime/helpers/typeof"));

var _createClass2 = _interopRequireDefault(require("@babel/runtime/helpers/createClass"));

var _defineProperty2 = _interopRequireDefault(require("@babel/runtime/helpers/defineProperty"));

var _classCallCheck2 = _interopRequireDefault(require("@babel/runtime/helpers/classCallCheck"));

var _inherits2 = _interopRequireDefault(require("@babel/runtime/helpers/inherits"));

var _possibleConstructorReturn2 = _interopRequireDefault(require("@babel/runtime/helpers/possibleConstructorReturn"));

var _getPrototypeOf2 = _interopRequireDefault(require("@babel/runtime/helpers/getPrototypeOf"));

var _wrapNativeSuper2 = _interopRequireDefault(require("@babel/runtime/helpers/wrapNativeSuper"));

var _util = _interopRequireDefault(require("util"));

var _map_classes = require("./map_classes");

var _doom_map = require("./doom_map");

var _udmf_parser = require("./udmf_parser");

var _logger = _interopRequireDefault(require("./logger"));

var _util$inspect$custom, _util$inspect$custom2, _util$inspect$custom3, _util$inspect$custom4, _util$inspect$custom5;

function _createForOfIteratorHelper(o, allowArrayLike) { var it; if (typeof Symbol === "undefined" || o[Symbol.iterator] == null) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e) { throw _e; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = o[Symbol.iterator](); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e2) { didErr = true; err = _e2; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = (0, _getPrototypeOf2["default"])(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = (0, _getPrototypeOf2["default"])(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return (0, _possibleConstructorReturn2["default"])(this, result); }; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

var UnknownBlockEntryError = /*#__PURE__*/function (_Error) {
  (0, _inherits2["default"])(UnknownBlockEntryError, _Error);

  var _super = _createSuper(UnknownBlockEntryError);

  function UnknownBlockEntryError(attribute, type, block) {
    (0, _classCallCheck2["default"])(this, UnknownBlockEntryError);
    return _super.call(this, "wanted ".concat(attribute, ":").concat(type, " in ").concat(_util["default"].inspect(block)));
  }

  return UnknownBlockEntryError;
}( /*#__PURE__*/(0, _wrapNativeSuper2["default"])(Error));

exports.UnknownBlockEntryError = UnknownBlockEntryError;
_util$inspect$custom = _util["default"].inspect.custom;

var Identifier = /*#__PURE__*/function () {
  function Identifier(text) {
    (0, _classCallCheck2["default"])(this, Identifier);
    (0, _defineProperty2["default"])(this, "text", void 0);
    this.text = text;
  }

  (0, _createClass2["default"])(Identifier, [{
    key: _util$inspect$custom,
    value: function value() {
      return "Identifier(".concat(this.text, ")");
    }
  }]);
  return Identifier;
}();

exports.Identifier = Identifier;
_util$inspect$custom2 = _util["default"].inspect.custom;

var QuotedString = /*#__PURE__*/function () {
  function QuotedString(text) {
    (0, _classCallCheck2["default"])(this, QuotedString);
    (0, _defineProperty2["default"])(this, "text", void 0);
    this.text = JSON.parse(text);
  }

  (0, _createClass2["default"])(QuotedString, [{
    key: _util$inspect$custom2,
    value: function value() {
      return "QuotedString(".concat(this.text, ")");
    }
  }]);
  return QuotedString;
}();

exports.QuotedString = QuotedString;
_util$inspect$custom3 = _util["default"].inspect.custom;

var Keyword = /*#__PURE__*/function () {
  function Keyword(text) {
    (0, _classCallCheck2["default"])(this, Keyword);
    (0, _defineProperty2["default"])(this, "text", void 0);
    this.text = text;
  }

  (0, _createClass2["default"])(Keyword, [{
    key: _util$inspect$custom3,
    value: function value() {
      return "Keyword(".concat(this.text, ")");
    }
  }]);
  return Keyword;
}();

exports.Keyword = Keyword;
_util$inspect$custom4 = _util["default"].inspect.custom;

var Block = /*#__PURE__*/function () {
  function Block(name, content) {
    (0, _classCallCheck2["default"])(this, Block);
    (0, _defineProperty2["default"])(this, "name", void 0);
    (0, _defineProperty2["default"])(this, "content", void 0);
    this.name = name;
    this.content = content;
  }

  (0, _createClass2["default"])(Block, [{
    key: _util$inspect$custom4,
    value: function value() {
      return "Block(".concat(this.name.text, "): ").concat(_util["default"].inspect(this.content));
    }
  }, {
    key: "get",
    value: function get(key) {
      var _iterator = _createForOfIteratorHelper(this.content),
          _step;

      try {
        for (_iterator.s(); !(_step = _iterator.n()).done;) {
          var assn = _step.value;

          if (key == assn.name.text) {
            return assn.value;
          }
        }
      } catch (err) {
        _iterator.e(err);
      } finally {
        _iterator.f();
      }

      return undefined;
    }
  }, {
    key: "getNum",
    value: function getNum(key, default_number) {
      var got = this.get(key);

      if (undefined == got && undefined != default_number) {
        return default_number;
      }

      if ('number' == typeof got) return got;
      throw new UnknownBlockEntryError(key, 'number | default', this);
    }
  }, {
    key: "getNumU",
    value: function getNumU(key) {
      var got = this.get(key);
      if (undefined == got) return undefined;
      if ('number' == typeof got) return got;
      throw new UnknownBlockEntryError(key, 'number | undefined', this);
    }
  }, {
    key: "getStr",
    value: function getStr(key) {
      var got = this.get(key);
      if ('string' == typeof got) return got;
      throw new UnknownBlockEntryError(key, 'string', this);
    }
  }]);
  return Block;
}();

exports.Block = Block;
_util$inspect$custom5 = _util["default"].inspect.custom;

var Assignment = /*#__PURE__*/function () {
  function Assignment(name, value) {
    (0, _classCallCheck2["default"])(this, Assignment);
    (0, _defineProperty2["default"])(this, "name", void 0);
    (0, _defineProperty2["default"])(this, "value", void 0);
    this.name = name;
    this.value = value;

    if (value instanceof Keyword && '0' == value.text) {
      this.value = 0;
    } else if (value instanceof QuotedString) {
      this.value = value.text;
    }
  }

  (0, _createClass2["default"])(Assignment, [{
    key: _util$inspect$custom5,
    value: function value() {
      return "Assignment(".concat(this.name.text, " := ").concat(_util["default"].inspect(this.value), ")");
    }
  }]);
  return Assignment;
}();

exports.Assignment = Assignment;

var UdmfMap = /*#__PURE__*/function (_DoomMap) {
  (0, _inherits2["default"])(UdmfMap, _DoomMap);

  var _super2 = _createSuper(UdmfMap);

  function UdmfMap(udmf_content) {
    (0, _classCallCheck2["default"])(this, UdmfMap);

    try {
      var parsed = (0, _udmf_parser.parse)(udmf_content);
    } catch (e) {
      if (!(e instanceof _udmf_parser.SyntaxError)) throw e;

      _logger["default"].ERROR("syntax error at ".concat(_util["default"].inspect(e.location)));

      _logger["default"].ERROR(e.message);

      throw e;
    }

    var got_vertexes = new Array();
    var got_linedefs = new Array();
    var got_sidedefs = new Array();
    var got_sectors = new Array();

    var _iterator2 = _createForOfIteratorHelper(parsed),
        _step2;

    try {
      for (_iterator2.s(); !(_step2 = _iterator2.n()).done;) {
        var element = _step2.value;

        if (element instanceof Assignment) {} else if (element instanceof Block) {
          switch (element.name.text) {
            case 'vertex':
              got_vertexes.push(_map_classes.Vertex.fromBlock(element));
              break;

            case 'linedef':
              got_linedefs.push(_map_classes.Linedef.fromBlock(element));
              break;

            case 'sidedef':
              got_sidedefs.push(_map_classes.Sidedef.fromBlock(element));
              break;

            case 'sector':
              got_sectors.push(_map_classes.Sector.fromBlock(element));
              break;

            default:
              _logger["default"].NOOP("unhandled element ".concat(element.name.text));

          }
        } else {
          _logger["default"].INFO("unknown type ".concat((0, _typeof2["default"])(element)));
        }
      }
    } catch (err) {
      _iterator2.e(err);
    } finally {
      _iterator2.f();
    }

    return _super2.call(this, {
      linedefs: got_linedefs,
      sidedefs: got_sidedefs,
      vertexes: got_vertexes,
      sectors: got_sectors
    });
  }

  return UdmfMap;
}(_doom_map.DoomMap);

exports.UdmfMap = UdmfMap;