#!/usr/bin/env ts-node-script
"use strict";

var _interopRequireDefault = require("@babel/runtime/helpers/interopRequireDefault");

Object.defineProperty(exports, "__esModule", {
  value: true
});
exports["default"] = void 0;

var _regenerator = _interopRequireDefault(require("@babel/runtime/regenerator"));

var _asyncToGenerator2 = _interopRequireDefault(require("@babel/runtime/helpers/asyncToGenerator"));

var _fs = _interopRequireDefault(require("fs"));

var _util = _interopRequireDefault(require("util"));

var _connectFlash = _interopRequireDefault(require("connect-flash"));

var _cookieSession = _interopRequireDefault(require("cookie-session"));

var _csurf = _interopRequireDefault(require("csurf"));

var _express = _interopRequireDefault(require("express"));

var _multer = _interopRequireDefault(require("multer"));

var _uuid = require("uuid");

var _logger = _interopRequireDefault(require("./logger"));

var _watcher = require("./watcher");

var _path = _interopRequireDefault(require("path"));

function main(_x) {
  return _main.apply(this, arguments);
}

function _main() {
  _main = (0, _asyncToGenerator2["default"])( /*#__PURE__*/_regenerator["default"].mark(function _callee(_argv) {
    var watcher, upload, csrf_middleware, app, port;
    return _regenerator["default"].wrap(function _callee$(_context) {
      while (1) {
        switch (_context.prev = _context.next) {
          case 0:
            if (!process.env.CHESS) {
              _logger["default"].FATAL("This application is for research purposes only");

              process.exit(1);
            }

            watcher = new _watcher.Watcher();
            watcher.start();
            upload = (0, _multer["default"])({
              storage: _multer["default"].diskStorage({
                destination: '/data/incoming',
                filename: function filename(req, file, cb) {
                  cb(null, (0, _uuid.v4)() + '.map');
                }
              }),
              limits: {
                fileSize: 5 * 1024 * 1024
              }
            });
            csrf_middleware = (0, _csurf["default"])();
            app = (0, _express["default"])();
            app.set('view engine', 'pug');
            app.set('views', process.env.VIEW_DIR || '/static/view');
            app.use(_express["default"].urlencoded({
              extended: true
            }));
            app.use(function (req, resp, next) {
              _logger["default"].INFO("".concat(req.method, " ").concat(req.path, " ").concat(_util["default"].inspect(req.params)));

              next();
            });
            app.use((0, _cookieSession["default"])({
              secret: process.env.SESSION_SECRET || 'OvertoneLatherSurfaceSpreeSetbackBoil'
            }));
            app.use((0, _connectFlash["default"])());
            app.use('/static', _express["default"]["static"](process.env.STATIC_DIR || '/static/static'));
            app.use('/favicon.ico', _express["default"]["static"]('/static/static/favicon.ico'));
            app.get('/', csrf_middleware, function (req, resp) {
              _fs["default"].promises.readdir('/data/done').then(function (files) {
                var already_converted = files.map(function (fn) {
                  return _path["default"].basename(fn, '.svg');
                });
                resp.render('index', {
                  csrf: req.csrfToken(),
                  flash: req.flash(),
                  already_converted: already_converted
                });
              });
            });
            app.post('/upload', upload.single('map'), csrf_middleware, function (req, resp) {
              var uploaded_path = _path["default"].parse(req.file.path);

              _fs["default"].promises.rename(_path["default"].format(uploaded_path), _path["default"].format({
                dir: '/data/enqueued',
                name: uploaded_path.name
              })).then(function () {
                resp.redirect(303, "/maps/".concat(uploaded_path.name, ".html"));
              });
            });
            app.get(/^\/maps\/([0-9a-f]{8}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{12})\.svg$/, function (req, resp) {
              _logger["default"].INFO(_path["default"].join('/data/done', req.params[0]));

              _fs["default"].promises.readFile(_path["default"].join('/data/done', req.params[0])).then(function (svg_data) {
                resp.contentType('image/svg+xml').send(svg_data);
              })["catch"](function (err) {
                _fs["default"].promises.access(_path["default"].join('/data/processing', req.params[0])).then(function () {
                  resp.status(425).contentType('text/html').header('Refresh', '5').send("still processing, will refresh automatically&hellip;");
                })["catch"](function () {
                  _fs["default"].promises.access(_path["default"].join('/data/enqueued', req.params[0])).then(function () {
                    resp.status(425).contentType('text/html').header('Refresh', '5').send("still enqueued, will refresh automatically&hellip;");
                  })["catch"](function (e) {
                    _logger["default"].ERROR(e);

                    resp.status(404).contentType('text/html').send("missing, or race condition'd");
                  });
                });
              });
            });
            app.get(/^\/maps\/([0-9a-f]{8}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{4}\-[0-9a-f]{12})\.html$/, function (req, resp) {
              _logger["default"].INFO(_path["default"].join('/data/done', req.params[0]));

              _fs["default"].promises.readFile(_path["default"].join('/data/done', req.params[0])).then(function (svg_data) {
                resp.render('show', {
                  flash: req.flash(),
                  id: req.params[0],
                  svg_data: svg_data
                });
              })["catch"](function (err) {
                _logger["default"].ERROR(err);

                _fs["default"].promises.access(_path["default"].join('/data/processing', req.params[0])).then(function () {
                  resp.status(425).header('Refresh', '5').send("still processing, will refresh automatically&hellip;");
                })["catch"](function () {
                  _fs["default"].promises.access(_path["default"].join('/data/enqueued', req.params[0])).then(function () {
                    resp.status(425).header('Refresh', '5').send("still enqueued, will refresh automatically&hellip;");
                  })["catch"](function (e) {
                    _logger["default"].ERROR(e);

                    resp.status(404).send("missing, or race condition'd");
                  });
                });
              });
            });
            port = parseInt(process.env.PORT || '3038');
            app.listen(port, function () {
              _logger["default"].INFO("lamartine listening on port ".concat(port));
            });

          case 20:
          case "end":
            return _context.stop();
        }
      }
    }, _callee);
  }));
  return _main.apply(this, arguments);
}

var _default = main(process.argv)["catch"](function (err) {
  return _logger["default"].FATAL(err);
});

exports["default"] = _default;