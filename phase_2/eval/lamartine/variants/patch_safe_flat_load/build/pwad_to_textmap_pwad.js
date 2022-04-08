"use strict";

var _pwad_writer = require("./pwad_writer");

var _pwad = require("./pwad");

var original_pwad = process.argv[2];
var new_pwad = process.argv[3];
var pwad = new _pwad.Pwad(original_pwad);
var writer = new _pwad_writer.PwadWriter(pwad.map);
writer.save(new_pwad);