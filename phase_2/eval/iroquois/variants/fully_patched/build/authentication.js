"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.prohibit_logged_in = exports.require_logged_in = exports.log_out = exports.process_login = exports.passport = void 0;
const passport_1 = __importDefault(require("passport"));
exports.passport = passport_1.default;
const passport_local_1 = __importDefault(require("passport-local"));
const user_1 = require("./models/user");
const LocalStrategy = passport_local_1.default.Strategy;
passport_1.default.serializeUser((user, done) => {
    done(undefined, user.id);
});
passport_1.default.deserializeUser((id, done) => {
    let maybe_user = user_1.User.find(id);
    if (undefined == maybe_user) {
        return done('not found', undefined);
    }
    return done(null, maybe_user);
});
passport_1.default.use(new LocalStrategy({ passReqToCallback: true }, (req, username, password, done) => {
    let user = user_1.User.findByName(username);
    if (undefined == user) {
        req.flash('error', 'Failed to log in');
        return done(null, null, { message: 'Failed to log in' });
    }
    let did_authenticate = user.authenticate(password);
    if (!did_authenticate) {
        req.flash('error', 'Failed to log in');
        return done(null, null, { message: 'Failed to log in' });
    }
    req.flash('info', `Logged in as ${user.name}`);
    return done(null, user);
}));
function process_login() {
    return passport_1.default.authenticate('local', { successRedirect: '/dashboard',
        failureRedirect: '/' });
}
exports.process_login = process_login;
function log_out() {
    return (req, resp) => {
        req.session = null;
        resp.redirect('/');
        return;
    };
}
exports.log_out = log_out;
function require_logged_in() {
    return (req, resp, next) => {
        if (!req.isAuthenticated()) {
            req.flash('error', 'please log in');
            resp.redirect('/');
            return;
        }
        next();
        return;
    };
}
exports.require_logged_in = require_logged_in;
function prohibit_logged_in() {
    return (req, resp, next) => {
        if (req.isAuthenticated()) {
            req.flash('info', 'already logged in');
            resp.redirect('/dashboard');
            return;
        }
        next();
        return;
    };
}
exports.prohibit_logged_in = prohibit_logged_in;
