"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports["default"] = void 0;

var _process = _interopRequireDefault(require("process"));

var _util = _interopRequireDefault(require("util"));

var LevelNames = ["_INVALID", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"];
var LogLevels = new Map();
LevelNames.forEach(function (name, index, _arr) {
  LogLevels.set(name, index);
});
var log_level = LogLevels.get(_process["default"].env['LOG_LEVEL'] || 'DEBUG') || 1;

function maybe_log(candidate_level, message) {
  if (candidate_level < log_level) return;
  console.log("[".concat(LevelNames[candidate_level], "] ").concat(_util["default"].format(message)));
}

var _default = {
  NOOP: function NOOP(_message) {},
  DEBUG: function DEBUG(message) {
    return maybe_log(1, message);
  },
  INFO: function INFO(message) {
    return maybe_log(2, message);
  },
  WARN: function WARN(message) {
    return maybe_log(3, message);
  },
  ERROR: function ERROR(message) {
    return maybe_log(4, message);
  },
  FATAL: function FATAL(message) {
    return maybe_log(5, message);
  }
};
exports["default"] = _default;