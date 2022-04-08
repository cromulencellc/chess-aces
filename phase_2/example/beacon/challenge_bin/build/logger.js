"use strict";

var process = require("process");

var util = require('util');

var Logger = function () {
  var _this = this;

  var LevelNames = ["_INVALID", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"];
  var LogLevels = {};
  LevelNames.forEach(function (name, index, _arr) {
    LogLevels[name] = index;
  });
  var LogLevel = LogLevels[process.env['LOG_LEVEL']] || LogLevels.DEBUG;

  var do_log = function do_log(level_name) {
    return function (message) {
      console.log("[" + level_name + "] " + util.format(message));
    };
  };

  var dont_log = function dont_log(_message) {};

  LevelNames.forEach(function (name, index, _arr) {
    if (index >= LogLevel) {
      _this[name] = do_log(name);
    } else {
      _this[name] = dont_log;
    }
  });
  this.LogLevels = LogLevels;
  return this;
}();

module.exports = Logger;