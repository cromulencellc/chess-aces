"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = __importDefault(require("express"));
const csurf_1 = __importDefault(require("csurf"));
const logger_1 = __importDefault(require("../logger"));
const user_1 = __importDefault(require("../models/user"));
const search_1 = __importDefault(require("../models/search"));
let r = express_1.default.Router();
let csrf_middleware = (0, csurf_1.default)();
r.get('/', csrf_middleware, (req, resp) => {
    resp.render('index', { flash: req.flash() });
});
r.get('/ws.json', (_req, resp) => {
    resp.json({
        'ping': 'ws://_/ping',
        'firehose': 'ws://_/firehose',
        'per-satellite': 'ws://_/satellite/:id'
    });
});
r.post('/user', (req, resp) => {
    let u_params = req.body;
    var u;
    try {
        u = new user_1.default(u_params);
    }
    catch (e) {
        logger_1.default.ERROR(e);
        logger_1.default.ERROR(req.body);
        if (e instanceof TypeError) {
            resp.status(400).send("Type error in user parameters");
            return;
        }
        return resp.sendStatus(500);
    }
    u.save().
        then(user_id => {
        resp.redirect(201, `/user/${user_id}`);
    }).
        catch(e => {
        logger_1.default.ERROR(e);
        logger_1.default.ERROR(req.body);
        resp.sendStatus(400);
    });
});
r.post('/filter', (req, resp) => {
    let params = req.body;
    var f;
    try {
        f = new search_1.default(params);
    }
    catch (e) {
        logger_1.default.ERROR(e);
        logger_1.default.ERROR(req.body);
        if (e instanceof TypeError) {
            resp.status(400).send("Type error in user parameters");
            return;
        }
        return resp.sendStatus(500);
    }
    f.save().
        then(filter_id => {
        resp.redirect(201, `/filter/${filter_id}`);
    }).
        catch(e => {
        logger_1.default.ERROR(e);
        logger_1.default.ERROR(req.body);
        resp.sendStatus(400);
    });
});
exports.default = r;
