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

var util = require('util');

var Duplex = require("stream").Duplex;

var BeaconError = require("./error");

var Logger = require("./logger");

var Token = require("./token");

var NumberParser = /[0-9]+(\.[0-9]*)?[\t-\r \(\)\xA0\u1680\u2000-\u200A\u2028\u2029\u202F\u205F\u3000\uFEFF]/;
var StringParser = /"((\\"|(?:(?!")[\s\S]))*)"[\t-\r \(\)\xA0\u1680\u2000-\u200A\u2028\u2029\u202F\u205F\u3000\uFEFF]/;
var SymbolParser = /([!\*\+\x2D\/<->A-Z_a-z][!\*\+\x2D\/-9<->A-Z_a-z]*)[\t-\r \(\)\xA0\u1680\u2000-\u200A\u2028\u2029\u202F\u205F\u3000\uFEFF]/;
var OpenParser = /\(/;
var CloseParser = /\)/;
var QuoteUnescaper = /\\"/g;

var ScanError = /*#__PURE__*/function (_BeaconError) {
  _inherits(ScanError, _BeaconError);

  var _super = _createSuper(ScanError);

  function ScanError() {
    _classCallCheck(this, ScanError);

    return _super.apply(this, arguments);
  }

  return ScanError;
}(BeaconError);

var TokenStream = /*#__PURE__*/function (_Duplex) {
  _inherits(TokenStream, _Duplex);

  var _super2 = _createSuper(TokenStream);

  function TokenStream(options) {
    var _this;

    _classCallCheck(this, TokenStream);

    _this = _super2.call(this, _objectSpread(_objectSpread({}, {
      readableObjectMode: true
    }), options));
    _this.tokens = [];
    _this.max_held_tokens = 16;
    _this.prev_chunk = "";
    _this.still_receiving = true;
    return _this;
  }

  _createClass(TokenStream, [{
    key: "toString",
    value: function toString() {
      return "TokenStream[".concat(this.tokens, "]");
    }
  }, {
    key: "isFull",
    value: function isFull() {
      return this.tokens.length === this.max_held_tokens;
    } // accepts a write from upstream; do the lexing here

  }, {
    key: "_write",
    value: function _write(chunk, encoding, callback) {
      if ('buffer' == encoding) {
        chunk = chunk.toString();
      } else if ('UTF-8' != encoding) {
        var err = new ScanError("unknown character encoding ".concat(encoding));
        return callback(err);
      }

      try {
        this.consumeChunk(chunk);
        Logger.INFO("consumed chunks ".concat(this.tokens));
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
    } // passes tokens downstream

  }, {
    key: "_read",
    value: function _read() {
      while (true) {
        if (0 == this.tokens.length) {
          if (!this.still_receiving) {
            this.push(null);
            return;
          }

          setImmediate(this._read.bind(this));
          return;
        }

        var cur_tok = this.tokens.shift();
        var got = this.push(cur_tok);
        if (false == got) return;
      }
    }
  }, {
    key: "consumeChunk",
    value: function consumeChunk(new_chunk) {
      var chunk = this.prev_chunk + new_chunk;
      var string_pos = chunk.search(StringParser);
      var number_pos = chunk.search(NumberParser);
      var symbol_pos = chunk.search(SymbolParser);
      var open_pos = chunk.search(OpenParser);
      var close_pos = chunk.search(CloseParser);
      var beyond_pos = chunk.length + 1;
      if (string_pos < 0) string_pos = beyond_pos;
      if (number_pos < 0) number_pos = beyond_pos;
      if (symbol_pos < 0) symbol_pos = beyond_pos;
      if (open_pos < 0) open_pos = beyond_pos;
      if (close_pos < 0) close_pos = beyond_pos;
      var min_pos = beyond_pos;
      if (string_pos < min_pos) min_pos = string_pos;
      if (number_pos < min_pos) min_pos = number_pos;
      if (symbol_pos < min_pos) min_pos = symbol_pos;
      if (open_pos < min_pos) min_pos = open_pos;
      if (close_pos < min_pos) min_pos = close_pos;

      if (min_pos == beyond_pos) {
        // nothing matched, done
        this.prev_chunk = new_chunk; // hold on for next time

        return undefined;
      }

      this.prev_chunk = "";
      if (min_pos == string_pos) return this.consumeString(chunk);
      if (min_pos == number_pos) return this.consumeNumber(chunk);
      if (min_pos == symbol_pos) return this.consumeSymbol(chunk);
      if (min_pos == open_pos) return this.consumeOpen(chunk);
      if (min_pos == close_pos) return this.consumeClose(chunk);
      throw ScanError("couldn't figure out how to consume ".concat(chunk));
    }
  }, {
    key: "consumeString",
    value: function consumeString(chunk) {
      var md = chunk.match(StringParser);

      if (undefined == md.index) {
        throw ScanError("tried to consumeString but failed ".concat(chunk));
      }

      var unescaped_contents = md[1].replace(QuoteUnescaper, '"');
      this.tokens.push(new Token.String(unescaped_contents));
      var remain = chunk.substr(md.index + md[0].length - 1);
      return this.consumeChunk(remain);
    }
  }, {
    key: "consumeNumber",
    value: function consumeNumber(chunk) {
      var md = chunk.match(NumberParser);

      if (undefined == md.index) {
        throw new ScanError("tried to consumeNumber but failed ".concat(chunk));
      }

      var val;

      if (undefined != md[1]) {
        val = parseFloat(md[0]);
      } else {
        val = parseInt(md[0]);
      }

      this.tokens.push(new Token.Number(val));
      var remain = chunk.substr(md.index + md[0].length - 1);
      return this.consumeChunk(remain);
    }
  }, {
    key: "consumeSymbol",
    value: function consumeSymbol(chunk) {
      var md = chunk.match(SymbolParser);

      if (undefined == md.index) {
        throw new ScanError("tried to consumeSymbol but failed ".concat(chunk));
      }

      var contents = md[1];
      this.tokens.push(new Token.Atom(contents));
      var remain = chunk.substr(md.index + md[1].length);
      return this.consumeChunk(remain);
    }
  }, {
    key: "consumeOpen",
    value: function consumeOpen(chunk) {
      var md = chunk.match(OpenParser);

      if (undefined == md.index) {
        throw new ScanError("tried to consumeOpen but failed ".concat(chunk));
      }

      this.tokens.push(new Token.OpenList());
      var remain = chunk.substr(md.index + 1);
      return this.consumeChunk(remain);
    }
  }, {
    key: "consumeClose",
    value: function consumeClose(chunk) {
      var md = chunk.match(CloseParser);

      if (undefined == md.index) {
        throw new ScanError("tried to consumeClose but failed ".concat(chunk));
      }

      this.tokens.push(new Token.CloseList());
      var remain = chunk.substr(md.index + 1);
      return this.consumeChunk(remain);
    }
  }]);

  return TokenStream;
}(Duplex);

util.inherits(TokenStream, Duplex);
module.exports = TokenStream;