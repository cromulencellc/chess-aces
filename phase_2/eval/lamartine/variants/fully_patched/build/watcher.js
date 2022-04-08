"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.Watcher = void 0;

var _classCallCheck2 = _interopRequireDefault(require("@babel/runtime/helpers/classCallCheck"));

var _createClass2 = _interopRequireDefault(require("@babel/runtime/helpers/createClass"));

var _defineProperty2 = _interopRequireDefault(require("@babel/runtime/helpers/defineProperty"));

var _fs = _interopRequireDefault(require("fs"));

var _path = _interopRequireDefault(require("path"));

var _util = _interopRequireDefault(require("util"));

var _process2 = require("process");

var _logger = _interopRequireDefault(require("./logger"));

var _pwad = require("./pwad");

var _svg_writer = require("./svg_writer");

var DEFAULT_INTERVAL = 500; // milliseconds

var Watcher = /*#__PURE__*/function () {
  function Watcher() {
    (0, _classCallCheck2["default"])(this, Watcher);
    (0, _defineProperty2["default"])(this, "interval", void 0);
    (0, _defineProperty2["default"])(this, "did_start", false);
    (0, _defineProperty2["default"])(this, "keep_going", false);

    _fs["default"].mkdirSync('/data/incoming', {
      recursive: true
    });

    _fs["default"].mkdirSync('/data/enqueued', {
      recursive: true
    });

    _fs["default"].mkdirSync('/data/processing', {
      recursive: true
    });

    _fs["default"].mkdirSync('/data/finalizing', {
      recursive: true
    });

    _fs["default"].mkdirSync('/data/done', {
      recursive: true
    });

    this.interval = Number(process.env.WATCHER_INTERVAL || DEFAULT_INTERVAL);
  }

  (0, _createClass2["default"])(Watcher, [{
    key: "start",
    value: function start() {
      if (this.did_start) return;
      setTimeout(this._running.bind(this), this.interval);
      this.keep_going = true;
      this.did_start = true;
    }
  }, {
    key: "_running",
    value: function _running() {
      var entries = _fs["default"].readdirSync('/data/enqueued');

      var runner_pxss = entries.map(this._process);

      try {
        Promise.all(runner_pxss).then(this._maybe_requeue.bind(this));
      } catch (e) {
        _logger["default"].FATAL("unhandled watcher error!");

        _logger["default"].FATAL(e);

        _logger["default"].FATAL("you probably need to clean out the work dirs");

        (0, _process2.exit)(-1);
      }
    }
  }, {
    key: "_maybe_requeue",
    value: function _maybe_requeue() {
      if (!this.keep_going) return;
      setTimeout(this._running.bind(this), this.interval);
    }
  }, {
    key: "_process",
    value: function _process(file) {
      var parsed_path = _path["default"].parse(file);

      var queue_path = _path["default"].join('/data/enqueued', parsed_path.base);

      var inflight_path = _path["default"].join('/data/processing', parsed_path.base);

      var inflight_svg_path = _path["default"].join('/data/finalizing', parsed_path.base);

      var done_path = _path["default"].join('/data/done', parsed_path.name);

      return _fs["default"].promises.rename(queue_path, inflight_path).then(function () {
        return new Promise(function (resolve, _reject) {
          return resolve(new _pwad.Pwad(inflight_path));
        });
      }).then(function (wad) {
        return new Promise(function (resolve, _reject) {
          var writer = new _svg_writer.SvgWriter(inflight_svg_path, wad.map);
          return resolve(writer.write());
        });
      }).then(function () {
        return _fs["default"].promises.rename(inflight_svg_path, done_path);
      }).then(function () {
        return _fs["default"].promises.unlink(inflight_path);
      })["catch"](function (reason) {
        _logger["default"].ERROR("barfed processing ".concat(inflight_path, " for ").concat(_util["default"].inspect(reason)));

        return _fs["default"].promises.copyFile('/static/lamartine-failed.svg', done_path);
      });
    }
  }]);
  return Watcher;
}();

exports.Watcher = Watcher;