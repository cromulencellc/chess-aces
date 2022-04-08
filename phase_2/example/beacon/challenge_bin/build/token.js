'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

var BeaconError = require('./error');

var CastError = /*#__PURE__*/function (_BeaconError) {
  _inherits(CastError, _BeaconError);

  var _super = _createSuper(CastError);

  function CastError(from, to_name) {
    _classCallCheck(this, CastError);

    return _super.call(this, "failed to cast ".concat(from, " to a ").concat(to_name));
  }

  return CastError;
}(BeaconError);

var Base = /*#__PURE__*/function () {
  function Base() {
    _classCallCheck(this, Base);
  }

  _createClass(Base, [{
    key: "castJS",
    value: function castJS() {
      throw CastError(this, 'JS thing');
    }
  }]);

  return Base;
}();

var Atom = /*#__PURE__*/function (_Base) {
  _inherits(Atom, _Base);

  var _super2 = _createSuper(Atom);

  function Atom(atom_str) {
    var _this;

    _classCallCheck(this, Atom);

    _this = _super2.call(this);
    _this.atom = atom_str;
    return _this;
  }

  _createClass(Atom, [{
    key: "toString",
    value: function toString() {
      return "Atom(".concat(this.atom, ")");
    }
  }, {
    key: "eq",
    value: function eq(other) {
      if (!other instanceof Atom) return false;
      if (this.atom != other.atom) return false;
      return true;
    }
  }]);

  return Atom;
}(Base);

var OpenList = /*#__PURE__*/function (_Base2) {
  _inherits(OpenList, _Base2);

  var _super3 = _createSuper(OpenList);

  function OpenList() {
    _classCallCheck(this, OpenList);

    return _super3.apply(this, arguments);
  }

  _createClass(OpenList, [{
    key: "toString",
    value: function toString() {
      return '(';
    }
  }]);

  return OpenList;
}(Base);

var CloseList = /*#__PURE__*/function (_Base3) {
  _inherits(CloseList, _Base3);

  var _super4 = _createSuper(CloseList);

  function CloseList() {
    _classCallCheck(this, CloseList);

    return _super4.apply(this, arguments);
  }

  _createClass(CloseList, [{
    key: "toString",
    value: function toString() {
      return ')';
    }
  }]);

  return CloseList;
}(Base);

var Number = /*#__PURE__*/function (_Base4) {
  _inherits(Number, _Base4);

  var _super5 = _createSuper(Number);

  function Number(num) {
    var _this2;

    _classCallCheck(this, Number);

    _this2 = _super5.call(this);
    _this2.num = num;
    return _this2;
  }

  _createClass(Number, [{
    key: "toString",
    value: function toString() {
      return this.num.toString();
    }
  }, {
    key: "castJS",
    value: function castJS() {
      return this.num;
    }
  }]);

  return Number;
}(Base);

var String = /*#__PURE__*/function (_Base5) {
  _inherits(String, _Base5);

  var _super6 = _createSuper(String);

  function String(str) {
    var _this3;

    _classCallCheck(this, String);

    _this3 = _super6.call(this);
    _this3.str = str;
    return _this3;
  }

  _createClass(String, [{
    key: "toString",
    value: function toString() {
      return "\"".concat(this.str, "\"");
    }
  }, {
    key: "castJS",
    value: function castJS() {
      return this.str;
    }
  }]);

  return String;
}(Base);

module.exports = {
  Atom: Atom,
  Base: Base,
  Number: Number,
  String: String,
  OpenList: OpenList,
  CloseList: CloseList
};