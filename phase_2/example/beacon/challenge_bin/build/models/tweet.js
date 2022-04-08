'use strict';

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var fs = require('fs');

var path = require('path');

var process = require('process');

var sprintf = require('sprintf-js').sprintf;

var Db = require("../db");

var Logger = require("../logger");

var extend = require("../extend").mutable;

var collection = new Db().getCollection("tweets");
var PHOTO_DIR = process.env.PHOTO_DIR || '/data/photos';
fs.mkdirSync(PHOTO_DIR, {
  recursive: true
}); // reformat ids before 5138 CE
//            01601922097303

var MAX_NOW = 99999999999999;
var MIN_NOW = 0;
var ID_FMT = "%014d-%06d";
var MIN_ID = sprintf(ID_FMT, 0, 0);

function mk_id() {
  var now = new Date().valueOf();
  var inv_now = MAX_NOW - now;
  return sprintf(ID_FMT, inv_now, Math.floor(Math.random() * 1000000));
}

var Tweet = /*#__PURE__*/function () {
  function Tweet(options) {
    _classCallCheck(this, Tweet);

    if ('string' == typeof options) {
      options = JSON.parse(options);
    } else if (Buffer.isBuffer(options)) {
      options = JSON.parse(options.toString());
    }

    extend(this, options);
  }

  _createClass(Tweet, [{
    key: "value",
    value: function value() {
      var running = {
        id: this.id,
        body: this.body,
        created_at: this.created_at
      };

      if (this.mime_type) {
        running.mime_type = this.mime_type;
      } else if (this.uploaded_photo) {
        running.mime_type = this.uploaded_photo.mimetype;
      }

      return JSON.stringify(running);
    }
  }, {
    key: "createdAt",
    value: function createdAt() {
      if (undefined != this._createdAt) return this._createdAt;
      return this._createdAt = new Date(this.created_at);
    }
  }, {
    key: "afterCreate",
    value: function afterCreate(resolve, reject) {
      var _this = this;

      Logger.DEBUG("doing afterCreate with ".concat(this.uploaded_photo));

      if (this.uploaded_photo) {
        fs.copyFile(this.uploaded_photo.path, this.photoPath(), fs.constants.COPYFILE_FICLONE, function (err) {
          if (err) return reject(err);
          return resolve(_this);
        });
        return;
      }

      Logger.DEBUG("did afterCreate with no photo");
      resolve(this);
    }
  }, {
    key: "photoPath",
    value: function photoPath() {
      return path.join(PHOTO_DIR, this.id);
    }
  }], [{
    key: "create",
    value: function create(options) {
      var id = mk_id();
      var nuevo = new Tweet(extend(options, {
        id: id,
        created_at: Date.now()
      }));
      var db_px = collection.put(id, nuevo.value());
      var creat_px = new Promise(function (resolve, reject) {
        db_px["catch"](function (err) {
          return reject(err);
        }).then(function () {
          return nuevo.afterCreate(resolve, reject);
        });
      });
      return creat_px;
    }
  }, {
    key: "list",
    value: function list() {
      var before = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : MIN_ID;
      var limit = arguments.length > 1 && arguments[1] !== undefined ? arguments[1] : 25;
      var rs = collection.createValueStream({
        gt: before,
        limit: limit
      });
      var got_tweets = [];
      var list_px = new Promise(function (resolve, reject) {
        rs.on('error', function (err) {
          return reject(err);
        });
        rs.on('data', function (data) {
          return got_tweets.push(new Tweet(data));
        });
        var did_already_close = false;

        var maybe_resolve = function maybe_resolve() {
          if (did_already_close) return;
          did_already_close = true;
          resolve(got_tweets);
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
            resolve(new Tweet(data.toString()));
          } catch (err) {
            reject(err);
          }
        });
      });
      return load_px;
    }
  }]);

  return Tweet;
}();

module.exports = Tweet;