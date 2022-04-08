'use strict';

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var Db = require("../db");

var Logger = require("../logger");

var extend = require("../extend").mutable;

var collection = new Db().getCollection("orders");

var Order = /*#__PURE__*/function () {
  function Order(options) {
    _classCallCheck(this, Order);

    if ('string' == typeof options) {
      options = JSON.parse(options);
    } else if (Buffer.isBuffer(options)) {
      options = JSON.parse(options.toString());
    }

    extend(this, options);
  }

  _createClass(Order, null, [{
    key: "list",
    value: function list() {
      var limit = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 25;
      var rs = collection.createValueStream({
        limit: limit
      });
      var got_orders = [];
      var list_px = new Promise(function (resolve, reject) {
        rs.on('error', function (err) {
          return reject(err);
        });
        rs.on('data', function (data) {
          return got_orders.push(new Order(data));
        });
        var did_already_close = false;

        var maybe_resolve = function maybe_resolve() {
          if (did_already_close) return;
          did_already_close = true;
          resolve(got_orders);
        };

        rs.on('close', maybe_resolve);
        rs.on('end', maybe_resolve);
      });
      return list_px;
    }
  }, {
    key: "load",
    value: function load(id) {
      var get_px = collection.get(id);
      var load_px = new Promise(function (resolve, reject) {
        get_px["catch"](function (err) {
          return reject(err);
        }).then(function (data) {
          try {
            resolve(new Order(data.toString()));
          } catch (err) {
            reject(err);
          }
        });
      });
      return load_px;
    }
  }]);

  return Order;
}();

module.exports = Order;