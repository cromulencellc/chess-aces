'use strict';

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var path = require("path");

var process = require('process');

var levelup = require("levelup");

var leveldown = require("leveldown");

var promisify = require("./promisify");

var Db = /*#__PURE__*/function () {
  function Db() {
    var options = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : {};

    _classCallCheck(this, Db);

    this.data_path = options['data_path'] || process.env.DATA_PATH || '/data';
    this.collections = {};
  }

  _createClass(Db, [{
    key: "getCollection",
    value: function getCollection(collectionName) {
      if (this.collections[collectionName]) {
        return this.collections[collectionName];
      }

      var coll_path = path.join(this.data_path, collectionName);
      var coll = levelup(leveldown(coll_path));
      this.collections[collectionName] = coll;
      return coll;
    }
  }]);

  return Db;
}();

module.exports = Db;