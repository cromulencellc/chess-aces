"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.PwadWriter = void 0;

var _classCallCheck2 = _interopRequireDefault(require("@babel/runtime/helpers/classCallCheck"));

var _createClass2 = _interopRequireDefault(require("@babel/runtime/helpers/createClass"));

var _defineProperty2 = _interopRequireDefault(require("@babel/runtime/helpers/defineProperty"));

var _csbinary = require("csbinary");

var _pwad = require("./pwad");

var _fs = _interopRequireDefault(require("fs"));

var _logger = _interopRequireDefault(require("./logger"));

var PwadWriter = /*#__PURE__*/function () {
  function PwadWriter(map) {
    (0, _classCallCheck2["default"])(this, PwadWriter);
    (0, _defineProperty2["default"])(this, "map", void 0);
    this.map = map;
  }

  (0, _createClass2["default"])(PwadWriter, [{
    key: "save",
    value: function save(filename) {
      var textmap = this.map.to_textmap();
      var textmap_start = 4 + 4 + 4 + // header
      4 + 4 + 8 + // startmap
      4 + 4 + 8 + // textmap
      4 + 4 + 8 + // endmap
      0;
      var writer = new _csbinary.BinaryWriter((0, _csbinary.File)(_fs["default"].openSync(filename, 'w')));
      writer.writeBuffer(_pwad.pwad_magic);
      writer.writeUInt32(3); // lump count

      writer.writeUInt32(4 + 4 + 4);

      _logger["default"].INFO("file header 0x".concat(writer.file.tell().toString(16)));

      writer.writeUInt32(textmap_start);
      writer.writeUInt32(0);
      writer.writeRawString("E1M1\0\0\0\0");

      _logger["default"].INFO("E1M1 0x".concat(writer.file.tell().toString(16)));

      writer.writeUInt32(textmap_start);
      writer.writeUInt32(textmap.length);
      writer.writeRawString("TEXTMAP\0");

      _logger["default"].INFO("TEXTMAP 0x".concat(writer.file.tell().toString(16)));

      writer.writeUInt32(textmap_start + textmap.length);
      writer.writeUInt32(0);
      writer.writeRawString("ENDMAP\0\0");

      _logger["default"].INFO("ENDMAP 0x".concat(writer.file.tell().toString(16)));

      if (textmap_start != writer.file.tell()) {
        _logger["default"].FATAL("calculated 0x".concat(textmap_start.toString(16)));

        _logger["default"].FATAL("got 0x".concat(writer.file.tell().toString(16)));

        throw new Error("wad toc longer than expected");
      }

      writer.writeRawString(textmap);
      writer.close();
    }
  }]);
  return PwadWriter;
}();

exports.PwadWriter = PwadWriter;