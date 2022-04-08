"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports.PwadMap = exports.Pwad = exports.MissingLumpError = exports.TooManyMapsError = exports.NotPwadError = exports.pwad_magic = void 0;

var _createClass2 = _interopRequireDefault(require("@babel/runtime/helpers/createClass"));

var _defineProperty2 = _interopRequireDefault(require("@babel/runtime/helpers/defineProperty"));

var _classCallCheck2 = _interopRequireDefault(require("@babel/runtime/helpers/classCallCheck"));

var _inherits2 = _interopRequireDefault(require("@babel/runtime/helpers/inherits"));

var _possibleConstructorReturn2 = _interopRequireDefault(require("@babel/runtime/helpers/possibleConstructorReturn"));

var _getPrototypeOf2 = _interopRequireDefault(require("@babel/runtime/helpers/getPrototypeOf"));

var _wrapNativeSuper2 = _interopRequireDefault(require("@babel/runtime/helpers/wrapNativeSuper"));

var _csbinary = require("csbinary");

var _fs = _interopRequireDefault(require("fs"));

var _pwad_lumps = require("./pwad_lumps");

var _util = _interopRequireDefault(require("util"));

var _doom_map = require("./doom_map");

var _udmf_classes = require("./udmf_classes");

var _util$inspect$custom, _util$inspect$custom2;

function _createSuper(Derived) { var hasNativeReflectConstruct = _isNativeReflectConstruct(); return function _createSuperInternal() { var Super = (0, _getPrototypeOf2["default"])(Derived), result; if (hasNativeReflectConstruct) { var NewTarget = (0, _getPrototypeOf2["default"])(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return (0, _possibleConstructorReturn2["default"])(this, result); }; }

function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Boolean.prototype.valueOf.call(Reflect.construct(Boolean, [], function () {})); return true; } catch (e) { return false; } }

var pwad_magic = Buffer.from('PWAD', 'binary');
exports.pwad_magic = pwad_magic;

var NotPwadError = /*#__PURE__*/function (_Error) {
  (0, _inherits2["default"])(NotPwadError, _Error);

  var _super = _createSuper(NotPwadError);

  function NotPwadError(wad_kind) {
    (0, _classCallCheck2["default"])(this, NotPwadError);
    return _super.call(this, "expected PWAD but got ".concat(wad_kind));
  }

  return NotPwadError;
}( /*#__PURE__*/(0, _wrapNativeSuper2["default"])(Error));

exports.NotPwadError = NotPwadError;

var TooManyMapsError = /*#__PURE__*/function (_Error2) {
  (0, _inherits2["default"])(TooManyMapsError, _Error2);

  var _super2 = _createSuper(TooManyMapsError);

  function TooManyMapsError(telltale_name) {
    (0, _classCallCheck2["default"])(this, TooManyMapsError);
    return _super2.call(this, "multiple ".concat(telltale_name, " lumps, expected only one"));
  }

  return TooManyMapsError;
}( /*#__PURE__*/(0, _wrapNativeSuper2["default"])(Error));

exports.TooManyMapsError = TooManyMapsError;

var MissingLumpError = /*#__PURE__*/function (_Error3) {
  (0, _inherits2["default"])(MissingLumpError, _Error3);

  var _super3 = _createSuper(MissingLumpError);

  function MissingLumpError(telltale_name) {
    (0, _classCallCheck2["default"])(this, MissingLumpError);
    return _super3.call(this, "needed but couldn't find ".concat(telltale_name, " lump"));
  }

  return MissingLumpError;
}( /*#__PURE__*/(0, _wrapNativeSuper2["default"])(Error));

exports.MissingLumpError = MissingLumpError;
_util$inspect$custom = _util["default"].inspect.custom;

var Pwad = /*#__PURE__*/function () {
  function Pwad(filename) {
    (0, _classCallCheck2["default"])(this, Pwad);
    (0, _defineProperty2["default"])(this, "filename", void 0);
    (0, _defineProperty2["default"])(this, "header", void 0);
    (0, _defineProperty2["default"])(this, "directory", void 0);
    (0, _defineProperty2["default"])(this, "map", void 0);
    this.filename = filename;
    var reader = new _csbinary.BinaryReader((0, _csbinary.File)(_fs["default"].openSync(filename, 'r')));
    this.header = new Header(reader);
    this.directory = new Directory(reader, this.header.directory_location, this.header.lump_count);

    if (this.directory.has('TEXTMAP')) {
      var udmf_dir = this.directory.get('TEXTMAP');
      reader.file.seek(udmf_dir.start, _csbinary.SeekOrigin.Begin);
      var udmf_lump = reader.readBytes(udmf_dir.size).toString();
      this.map = new _udmf_classes.UdmfMap(udmf_lump);
    } else {
      this.map = new PwadMap(reader, this.directory);
    }
  }

  (0, _createClass2["default"])(Pwad, [{
    key: _util$inspect$custom,
    value: function value() {
      return "Pwad(".concat(this.filename, ", directory=").concat(_util["default"].inspect(this.directory), ")");
    }
  }]);
  return Pwad;
}();

exports.Pwad = Pwad;

var PwadMap = /*#__PURE__*/function (_DoomMap) {
  (0, _inherits2["default"])(PwadMap, _DoomMap);

  var _super4 = _createSuper(PwadMap);

  function PwadMap(reader, directory) {
    (0, _classCallCheck2["default"])(this, PwadMap);
    var linedefs_dir = directory.get('LINEDEFS');
    reader.file.seek(linedefs_dir.start, _csbinary.SeekOrigin.Begin);
    var linedefs_lump = new _pwad_lumps.Linedefs(reader, linedefs_dir.size);
    var sidedefs_dir = directory.get('SIDEDEFS');
    reader.file.seek(sidedefs_dir.start, _csbinary.SeekOrigin.Begin);
    var sidedefs_lump = new _pwad_lumps.Sidedefs(reader, sidedefs_dir.size);
    var vertexes_dir = directory.get('VERTEXES');
    reader.file.seek(vertexes_dir.start, _csbinary.SeekOrigin.Begin);
    var vertexes_lump = new _pwad_lumps.Vertexes(reader, vertexes_dir.size);
    var sectors_dir = directory.get('SECTORS');
    reader.file.seek(sectors_dir.start, _csbinary.SeekOrigin.Begin);
    var sectors_lump = new _pwad_lumps.Sectors(reader, sectors_dir.size);
    return _super4.call(this, {
      linedefs: linedefs_lump.linedefs,
      sidedefs: sidedefs_lump.sidedefs,
      vertexes: vertexes_lump.vertexes,
      sectors: sectors_lump.sectors
    });
  }

  return PwadMap;
}(_doom_map.DoomMap);

exports.PwadMap = PwadMap;

var Header = function Header(reader) {
  (0, _classCallCheck2["default"])(this, Header);
  (0, _defineProperty2["default"])(this, "wad_kind", void 0);
  (0, _defineProperty2["default"])(this, "lump_count", void 0);
  (0, _defineProperty2["default"])(this, "directory_location", void 0);
  reader.file.seek(0, _csbinary.SeekOrigin.Begin);
  this.wad_kind = reader.readBytes(4);
  this.lump_count = reader.readUInt32();
  this.directory_location = reader.readUInt32();
};

var Directory = /*#__PURE__*/function () {
  function Directory(reader, directory_location, lump_count) {
    (0, _classCallCheck2["default"])(this, Directory);
    (0, _defineProperty2["default"])(this, "entries", new Map());
    reader.file.seek(directory_location, _csbinary.SeekOrigin.Begin);

    for (var i = 0; i < lump_count; i++) {
      var ent = new DirectoryEntry(reader);

      if (this.entries.has(ent.name)) {
        throw new TooManyMapsError(ent.name);
      }

      this.entries.set(ent.name, ent);
    }
  }

  (0, _createClass2["default"])(Directory, [{
    key: "has",
    value: function has(lump_name) {
      return this.entries.has(lump_name);
    }
  }, {
    key: "get",
    value: function get(lump_name) {
      var got = this.entries.get(lump_name);
      if (undefined == got) throw new MissingLumpError(lump_name);
      return got;
    }
  }]);
  return Directory;
}();

_util$inspect$custom2 = _util["default"].inspect.custom;

var DirectoryEntry = /*#__PURE__*/function () {
  function DirectoryEntry(reader) {
    (0, _classCallCheck2["default"])(this, DirectoryEntry);
    (0, _defineProperty2["default"])(this, "start", void 0);
    (0, _defineProperty2["default"])(this, "size", void 0);
    (0, _defineProperty2["default"])(this, "name", void 0);
    this.start = reader.readUInt32();
    this.size = reader.readUInt32();
    this.name = reader.readBytes(8).toString().replace(/\0+$/, '');
  }

  (0, _createClass2["default"])(DirectoryEntry, [{
    key: _util$inspect$custom2,
    value: function value() {
      return "{".concat(this.name, " @ 0x").concat(this.start.toString(16), "+").concat(this.size.toString(16), "}");
    }
  }]);
  return DirectoryEntry;
}();