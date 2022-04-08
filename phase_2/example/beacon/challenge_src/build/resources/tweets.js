'use strict';

var express = require("express");

var multer = require("multer");

var Logger = require("../logger");

var Tweet = require("../models/tweet");

var tweets = express.Router();
var ROUTE_PREFIX = '/tweets';
var upload = multer({
  storage: multer.diskStorage({})
});
tweets.get('/', function (req, resp) {
  Tweet.list().then(function (tweets) {
    resp.render('tweets/index', {
      tweets: tweets
    });
  })["catch"](function (err) {
    return resp.render('error', {
      error: err
    });
  });
});
tweets.get(/^\/page\/([^\/]+)$/, function (req, resp) {
  Tweet.list(req.params[0]).then(function (tweets) {
    resp.render('tweets/index', {
      tweets: tweets
    });
  })["catch"](function (err) {
    return resp.render('error', {
      error: err
    });
  });
});
tweets.post('/', upload.single('tweet[photo]'), function (req, resp) {
  Logger.DEBUG(JSON.stringify(req.body));
  var tweet_attrs = {
    body: req.body.tweet.body
  };

  if (req.file) {
    tweet_attrs.uploaded_photo = req.file;
  }

  Logger.DEBUG(JSON.stringify(tweet_attrs));
  Tweet.create(tweet_attrs).then(function (tweet) {
    resp.redirect("".concat(ROUTE_PREFIX, "/").concat(tweet.id));
  })["catch"](function (err) {
    return resp.render('error', {
      error: err
    });
  });
});
tweets.get(/^\/([^\/]+)$/, function (req, resp) {
  Tweet.load(req.params[0]).then(function (tweet) {
    return resp.render('tweets/show', {
      tweet: tweet
    });
  })["catch"](function (err) {
    return resp.sendStatus(400);
  });
});
tweets.get(/^\/([^\/]+)\/photo$/, function (req, resp) {
  Logger.DEBUG(req.params[0]);
  var tweet = new Tweet({
    id: req.params[0]
  });
  return resp.sendFile(tweet.photoPath());
});
module.exports = tweets;