'use strict';

function _typeof(obj) { "@babel/helpers - typeof"; if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof = function _typeof(obj) { return typeof obj; }; } else { _typeof = function _typeof(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof(obj); }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(Object(source), true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(Object(source)).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

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

var Duplex = require('stream').Duplex;

var BeaconError = require('./error');

var Logger = require('./logger');

var Env = require('./env');

var Expression = require('./expression');

var Token = require('./token');

var SpecialForms = ['if', 'cond', 'quote', 'set', 'lambda'];

function a(str) {
  return new Token.Atom(str);
}

var EvalError = /*#__PURE__*/function (_BeaconError) {
  _inherits(EvalError, _BeaconError);

  var _super = _createSuper(EvalError);

  function EvalError() {
    _classCallCheck(this, EvalError);

    return _super.apply(this, arguments);
  }

  return EvalError;
}(BeaconError);

var Evaluator = /*#__PURE__*/function (_Duplex) {
  _inherits(Evaluator, _Duplex);

  var _super2 = _createSuper(Evaluator);

  function Evaluator(options) {
    var _this;

    _classCallCheck(this, Evaluator);

    _this = _super2.call(this, _objectSpread(_objectSpread({}, {
      readableObjectMode: true,
      writableObjectMode: true
    }), options));
    _this.results = [];
    _this.env = Env["default"]();
    _this.still_receiving = true;
    Logger.INFO(_this.env);
    return _this;
  }

  _createClass(Evaluator, [{
    key: "_write",
    value: function _write(chunk, _encoding, callback) {
      try {
        var got_result = this.eval(chunk);
        Logger.INFO(got_result);
        this.results.push(got_result);
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
        if (0 == this.results.length) {
          if (!this.still_receiving) {
            this.push(null);
            return;
          }

          setImmediate(this._read.bind(this));
          return;
        }

        var got = this.push(this.results.shift());
        if (false == got) return;
      }
    }
  }, {
    key: "eval",
    value: function _eval(expr) {
      var _this2 = this;

      var first = expr.shift();
      Logger.INFO(first);

      if (a('quote').eq(first)) {
        return expr;
      }

      if (a('if').eq(first)) return this.do_if(expr);
      if (a('cond').eq(first)) return this.do_cond(expr);
      if (a('set').eq(first)) return this.do_set(expr); // if (a('lambda') == first) return this.do_lambda(expr)

      var rest = expr.map(function (a) {
        return _this2.inner_eval(a);
      });
      Logger.INFO("eval ".concat(first, "(").concat(rest, ")"));

      if (first instanceof Token.Atom) {
        var sym = first.atom;
        var got = this.env.get(sym);
        return got(rest);
      }

      throw new EvalError("couldn't eval ".concat(first, " (typeof ").concat(_typeof(first), ")"));
    }
  }, {
    key: "inner_eval",
    value: function inner_eval(arg) {
      if (arg instanceof Expression) return this.eval(arg);
      if (arg instanceof Token.Atom) return this.env[arg];
      if (arg instanceof Token.Base) return arg.castJS();
      return arg;
    }
  }, {
    key: "is_truthy",
    value: function is_truthy(thing) {
      if (thing instanceof Array) return 0 != thing.length;
      return true;
    }
  }, {
    key: "do_if",
    value: function do_if(expr) {
      var predicate = expr[0];
      var true_leg = expr[1];
      var false_leg = expr[2];
      var result = this.inner_eval(predicate);
      if (this.is_truthy(result)) return this.inner_eval(true_leg);
      if (false_leg) return this.inner_eval(false_leg);
      return [];
    }
  }, {
    key: "do_cond",
    value: function do_cond(expr) {
      if (0 == expr.length) return [];
      var predicate = expr.shift();
      var leg = expr.shift();
      var result = this.inner_eval(predicate);
      if (this.is_truthy(result)) return this.inner_eval(leg);
      return this.do_cond(expr);
    }
  }, {
    key: "do_set",
    value: function do_set(expr) {
      var name = expr[0];
      var value = expr[1];
      Logger.INFO("set ".concat(name, " to ").concat(value, " ").concat(_typeof(value)));
      this.env[name] = value;
      return value;
    }
  }]);

  return Evaluator;
}(Duplex);

module.exports = Evaluator;