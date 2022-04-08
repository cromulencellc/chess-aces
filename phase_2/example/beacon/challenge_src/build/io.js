"use strict";

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } }

function _createClass(Constructor, protoProps, staticProps) { if (protoProps) _defineProperties(Constructor.prototype, protoProps); if (staticProps) _defineProperties(Constructor, staticProps); return Constructor; }

var EventEmitter = require("events");

var Io = /*#__PURE__*/function () {
  function Io(i, o) {
    _classCallCheck(this, Io);

    this.i = i;
    this.i.setEncoding('UTF-8');
    this.o = o;
  }

  _createClass(Io, [{
    key: "destroy",
    value: function destroy() {
      this.i.destroy();

      if (this.o != this.i) {
        this.o.destroy();
      }
    }
  }], [{
    key: "fromStdio",
    value: function fromStdio() {
      return new Io(process.stdin, process.stdout);
    }
  }]);

  return Io;
}();

module.exports = Io;