"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

var _pwad_writer = require("./pwad_writer");

var _pwad = require("./pwad");

var _crypto = _interopRequireDefault(require("crypto"));

var original_pwad = process.argv[2];
var new_pwad = process.argv[3];
var pwad = new _pwad.Pwad(original_pwad); // only delete one of these, and definitely at least one of these, 
// since we don't want
// to accidentally make a still-valid map

var sectors_to_delete = _crypto["default"].randomInt(0, 8);

var linedefs_to_delete = _crypto["default"].randomInt(0, 8);

var sidedefs_to_delete = _crypto["default"].randomInt(0, 8);

var vertexes_to_delete = _crypto["default"].randomInt(1, 8);

if (sectors_to_delete > 0) {
  for (var i = 0; i < sectors_to_delete; i++) {
    pwad.map.sectors.pop();
  }
} else if (linedefs_to_delete > 0) {
  for (var _i = 0; _i < linedefs_to_delete; _i++) {
    pwad.map.linedefs.pop();
  }
} else if (sidedefs_to_delete > 0) {
  for (var _i2 = 0; _i2 < sidedefs_to_delete; _i2++) {
    pwad.map.sidedefs.pop();
  }
} else if (vertexes_to_delete > 0) {
  for (var _i3 = 0; _i3 < vertexes_to_delete; _i3++) {
    pwad.map.vertexes.pop();
  }
}

var writer = new _pwad_writer.PwadWriter(pwad.map);
writer.save(new_pwad);