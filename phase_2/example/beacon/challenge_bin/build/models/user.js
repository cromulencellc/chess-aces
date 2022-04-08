'use strict';

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var crypto = require('crypto');

var Db = require("../db");

var Logger = require("../logger");

var extend = require("../extend").mutable;

var collection = new Db().getCollection("users");

var User = /*#__PURE__*/function () {
  function User(options) {
    _classCallCheck(this, User);

    if ('string' == typeof options) {
      options = JSON.parse(options);
    } else if (Buffer.isBuffer(options)) {
      options = JSON.parse(options.toString());
    }

    extend(this, options);
  }

  _createClass(User, [{
    key: "authenticate",
    value: function authenticate(candidate_password) {
      var _this = this;

      var expected_digest = Buffer.from(this.password_digest, 'base64');
      var scrypt_px = new Promise(function (resolve, reject) {
        crypto.scrypt(candidate_password, Buffer.from(_this.password_salt, 'base64'), 64, function (err, got_digest) {
          if (err) reject(err);
          resolve(crypto.timingSafeEqual(expected_digest, got_digest));
        });
      });
      return scrypt_px;
    }
  }, {
    key: "beforeCreate",
    value: function beforeCreate() {
      this.password_salt = crypto.randomBytes(32);
      this.password_digest = crypto.scryptSync(Buffer.from(this.password), this.password_salt, 64);
    }
  }, {
    key: "value",
    value: function value() {
      return JSON.stringify({
        name: this.name,
        password_salt: this.password_salt,
        password_digest: this.password_digest,
        is_admin: this.is_admin
      });
    }
  }], [{
    key: "create",
    value: function create(options) {
      var nuevo = new User(options);
      nuevo.beforeCreate();
      var db_px = collection.put(options.name, nuevo.value());
      var creat_px = new Promise(function (resolve, reject) {
        db_px["catch"](function (err) {
          return reject(err);
        }).then(function () {
          return resolve(nuevo);
        });
      });
      return creat_px;
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
            resolve(new User(data.toString()));
          } catch (err) {
            reject(err);
          }
        });
      });
      return load_px;
    }
  }, {
    key: "maybe_load",
    value: function maybe_load(id) {
      Logger.INFO("getting ".concat(id));
      var get_px = collection.get(id);
      var load_px = new Promise(function (resolve, reject) {
        get_px.then(function (data) {
          Logger.DEBUG(data);

          try {
            resolve(new User(data.toString()));
          } catch (err) {
            Logger.DEBUG(err);
            reject(err);
          }
        })["catch"](function (err) {
          Logger.DEBUG(err);
          if (err.notFound) resolve(null);else reject(err);
        });
      });
      return load_px;
    }
  }]);

  return User;
}();

module.exports = User;