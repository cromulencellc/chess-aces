'use strict';

var express = require("express");

var Logger = require("../logger");

var index = express.Router();
index.get('/', function (req, resp) {
  resp.render('index');
});
index.get('/query-parsing', function (req, resp) {
  resp.render('console', {
    data: JSON.stringify(req.query)
  });
});
module.exports = index;