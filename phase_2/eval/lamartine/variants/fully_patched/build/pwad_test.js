#!/usr/bin/env ts-node-script
"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

var _pwad = require("./pwad");

var _process = _interopRequireDefault(require("process"));

var _svg_writer = require("./svg_writer");

var pwad_path = _process["default"].argv[2];
var pwad = new _pwad.Pwad(pwad_path);
var svg_path = _process["default"].argv[3];
var writer = new _svg_writer.SvgWriter(svg_path, pwad.map);
writer.write();