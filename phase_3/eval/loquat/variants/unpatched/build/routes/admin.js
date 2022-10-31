"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = __importDefault(require("express"));
const express_basic_auth_1 = __importDefault(require("express-basic-auth"));
const promises_1 = require("fs/promises");
const user_1 = __importDefault(require("../models/user"));
const type_predicates_1 = require("@tool-belt/type-predicates");
const logger_1 = __importDefault(require("../logger"));
let r = express_1.default.Router();
r.use((0, express_basic_auth_1.default)({
    authorizer: user_1.default.authorizeAdmin,
    authorizeAsync: true
}));
r.get('/', (_req, resp) => {
    (0, promises_1.readFile)('/token').
        then(tokenContent => resp.status(200).send(tokenContent)).
        catch(e => {
        resp.status(500).send(`error reading token: ${e}`);
    });
});
r.get('/user/:user_name', async (req, resp) => {
    let u = await user_1.default.findByUsername(req.params.user_name);
    if (undefined == u) {
        resp.sendStatus(404);
        return;
    }
    let sats = await u.satellites();
    let ret = {
        name: u.name,
        is_admin: u.is_admin,
        satellite_ids: sats.map(s => s.id)
    };
    resp.send(JSON.stringify(ret));
});
r.post('/user/:user_name/satellites', async (req, resp) => {
    let u = await user_1.default.findByUsername(req.params.user_name);
    if (undefined == u) {
        logger_1.default.ERROR(`couldn't find user ${req.params.user_name}`);
        resp.sendStatus(404);
        return;
    }
    let s_id = req.body.satellite_id;
    if (!(0, type_predicates_1.isString)(s_id)) {
        logger_1.default.ERROR(`couldn't find satellite id ${req.body.satellite_id}`);
        resp.sendStatus(400);
        return;
    }
    let redirect_url = `/user/${u.name}/satellites/${s_id}`;
    u.add_satellite(s_id).
        then(() => resp.sendStatus(204)).
        catch(e => {
        logger_1.default.ERROR(e);
        resp.sendStatus(500);
    });
});
r.delete('/user/:user_name/satellites/:satellite_id', async (req, resp) => {
    let u = await user_1.default.findByUsername(req.params.user_name);
    if (undefined == u) {
        logger_1.default.ERROR(`couldn't find user ${req.params.user_name}`);
        resp.sendStatus(404);
        return;
    }
    let s_id = req.params.satellite_id;
    if (!(0, type_predicates_1.isString)(s_id)) {
        logger_1.default.ERROR(`couldn't find satellite id ${req.params.satellite_id}`);
        resp.sendStatus(400);
        return;
    }
    u.remove_satellite(s_id).
        then(() => resp.sendStatus(204)).
        catch(e => {
        logger_1.default.ERROR(e);
        resp.sendStatus(500);
    });
});
exports.default = r;
