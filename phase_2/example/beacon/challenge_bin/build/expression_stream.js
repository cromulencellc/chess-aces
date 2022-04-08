'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function"); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, writable: true, configurable: true } }); if (superClass) _setPrototypeOf(subClass, superClass); }

function _setPrototypeOf(o, p) { _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) { o.__proto__ = p; return o; }; return _setPrototypeOf(o, p); }

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = _getPrototypeOf(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

function _possibleConstructorReturn(self, call) { if (call && (_typeof(call) === "object" || typeof call === "function")) { return call; } return _assertThisInitialized(self); }

function _assertThisInitialized(self) { if (self === void 0) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return self; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

function _getPrototypeOf(o) { _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) { return o.__proto__ || Object.getPrototypeOf(o); }; return _getPrototypeOf(o); }

var Duplex = require('stream').Duplex;

var Expression = require('./expression');

var Token = require('./token');

var ExpressionStream = /*#__PURE__*/function (_Duplex) {
  _inherits(ExpressionStream, _Duplex);

  var _super = _createSuper(ExpressionStream);

  function ExpressionStream(options) {
    var _this;

    _classCallCheck(this, ExpressionStream);

    _this = _super.call(this, _objectSpread(_objectSpread({}, {
      readableObjectMode: true,
      writableObjectMode: true
    }), options));
    _this.stack = [];
    _this.still_receiving = true;
    _this.parsed_exprs = [];
    return _this;
  }

  _createClass(ExpressionStream, [{
    key: "toString",
    value: function toString() {
      return "ExpressionStream(".concat(this.stack.length, " frames)");
    }
  }, {
    key: "_write",
    value: function _write(chunk, _encoding, callback) {
      try {
        this.consume(chunk);
      } catch (err) {
        return callback(err);
      }

      return callback(null);
    }
  }, {
    key: "_destroy",
    value: function _destroy(err, callback) {
      this.still_receiving = false;
      callback(err);
    }
  }, {
    key: "_final",
    value: function _final(callback) {
      this.still_receiving = false;
      callback();
    }
  }, {
    key: "_read",
    value: function _read() {
      while (true) {
        if (0 == this.parsed_exprs.length) {
          if (!this.still_receiving) {
            this.push(null);
            return;
          }

          setImmediate(this._read.bind(this));
          return;
        }

        var cur_expr = this.parsed_exprs.shift();
        var got = this.push(cur_expr);
        if (false == got) return;
      }
    }
  }, {
    key: "consume",
    value: function consume(chunk) {
      if (chunk instanceof Token.OpenList) {
        return this.stack.push(new Expression());
      }

      if (chunk instanceof Token.CloseList) return this.consumeClose();
      var top_frame = this.stack[this.stack.length - 1];
      return top_frame.push(chunk);
    }
  }, {
    key: "consumeClose",
    value: function consumeClose() {
      var finished_expr = this.stack.pop();

      if (0 == this.stack.length) {
        return this.parsed_exprs.push(finished_expr);
      }

      var new_top_frame = this.stack[this.stack.length - 1];
      return new_top_frame.push(finished_expr);
    }
  }]);

  return ExpressionStream;
}(Duplex);

module.exports = ExpressionStream;