'use strict';

function _createForOfIteratorHelper(o, allowArrayLike) { var it; if (typeof Symbol === "undefined" || o[Symbol.iterator] == null) { if (Array.isArray(o) || (it = _unsupportedIterableToArray(o)) || allowArrayLike && o && typeof o.length === "number") { if (it) o = it; var i = 0; var F = function F() {}; return { s: F, n: function n() { if (i >= o.length) return { done: true }; return { done: false, value: o[i++] }; }, e: function e(_e2) { throw _e2; }, f: F }; } throw new TypeError("Invalid attempt to iterate non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); } var normalCompletion = true, didErr = false, err; return { s: function s() { it = o[Symbol.iterator](); }, n: function n() { var step = it.next(); normalCompletion = step.done; return step; }, e: function e(_e3) { didErr = true; err = _e3; }, f: function f() { try { if (!normalCompletion && it["return"] != null) it["return"](); } finally { if (didErr) throw err; } } }; }

function _slicedToArray(arr, i) { return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _unsupportedIterableToArray(arr, i) || _nonIterableRest(); }

function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method."); }

function _unsupportedIterableToArray(o, minLen) { if (!o) return; if (typeof o === "string") return _arrayLikeToArray(o, minLen); var n = Object.prototype.toString.call(o).slice(8, -1); if (n === "Object" && o.constructor) n = o.constructor.name; if (n === "Map" || n === "Set") return Array.from(o); if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen); }

function _arrayLikeToArray(arr, len) { if (len == null || len > arr.length) len = arr.length; for (var i = 0, arr2 = new Array(len); i < len; i++) { arr2[i] = arr[i]; } return arr2; }

function _iterableToArrayLimit(arr, i) { if (typeof Symbol === "undefined" || !(Symbol.iterator in Object(arr))) return; var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"] != null) _i["return"](); } finally { if (_d) throw _e; } } return _arr; }

function _arrayWithHoles(arr) { if (Array.isArray(arr)) return arr; }

var util = require('util');

var express = require("express");

var Logger = require("../logger");

var Order = require("../models/order");

var orders = express.Router();
var ROUTE_PREFIX = '/orders'; // 12345678-1234-1234-1234-123456789012
// d1589325-9fdf-49a2-a65a-eaeac8b569c4

var matcher_makers = {
  gteq: function gteq(key, value) {
    var fn = function fn(obj) {
      return obj[key] >= value;
    };

    fn[util.inspect.custom] = function () {
      return "".concat(key, " gteq ").concat(value);
    };

    return fn;
  },
  lteq: function lteq(key, value) {
    var fn = function fn(obj) {
      return obj[key] <= value;
    };

    fn[util.inspect.custom] = function () {
      return "".concat(key, " lteq ").concat(value);
    };

    return fn;
  }
};

var always_match = function always_match(_obj) {
  return true;
};

always_match[util.inspect.custom] = function () {
  return 'always';
};

orders.get('/', function (req, resp) {
  //  Logger.DEBUG(req.query)
  var filters = [always_match];

  if (req.query.filter) {
    for (var _i = 0, _Object$entries = Object.entries(req.query.filter); _i < _Object$entries.length; _i++) {
      var _Object$entries$_i = _slicedToArray(_Object$entries[_i], 2),
          masher = _Object$entries$_i[0],
          value = _Object$entries$_i[1];

      var _masher$split = masher.split(':'),
          _masher$split2 = _slicedToArray(_masher$split, 2),
          field = _masher$split2[0],
          matcher = _masher$split2[1];

      if ('' == value) continue; //    Logger.DEBUG(`${field} ${matcher} => ${value}`)

      filters.push(matcher_makers[matcher](field, value));
    }
  }

  Logger.DEBUG(filters);
  Order.list().then(function (orders) {
    var matched_orders = orders.filter(function (order) {
      var _iterator = _createForOfIteratorHelper(filters),
          _step;

      try {
        for (_iterator.s(); !(_step = _iterator.n()).done;) {
          var filter = _step.value;

          if (!filter(order)) {
            return false;
          }
        }
      } catch (err) {
        _iterator.e(err);
      } finally {
        _iterator.f();
      }

      return true;
    });
    resp.render('orders/index', {
      orders: matched_orders
    });
  })["catch"](function (err) {
    return resp.render('error', {
      error: err
    });
  });
});
orders.get(/^\/([0-9a-f]{8}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{12})$/, function (req, resp) {
  Order.load(req.params[0]).then(function (order) {
    var template = 'orders/show';
    if (order.pin == req.query.pin) template = 'orders/show_full';
    resp.render(template, {
      order: order
    });
  })["catch"](function (err) {
    if (err.notFound) {
      resp.sendStatus(404);
      return;
    }

    console.log(err);
    resp.sendStatus(400);
  });
});
module.exports = orders;