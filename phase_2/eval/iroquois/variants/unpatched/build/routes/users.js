"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = __importDefault(require("express"));
const authentication_1 = require("../authentication");
const user_1 = require("../models/user");
let ROUTE_PREFIX = '/users';
let users = express_1.default.Router();
users.post('/', authentication_1.prohibit_logged_in(), (req, resp) => {
    if (req.body.user.password_confirmation !=
        req.body.user.password) {
        req.flash('error', "password and password confirmation didn't match");
        resp.redirect('/');
        return;
    }
    try {
        let new_user = user_1.User.create({
            name: req.body.user.name,
            password: req.body.user.password
        });
    }
    catch (err) {
        req.flash('error', `couldn't save user: ${err}`);
        resp.redirect('/');
        return;
    }
    req.flash('info', "created user, please log in");
    resp.redirect('/');
    return;
});
exports.default = users;
