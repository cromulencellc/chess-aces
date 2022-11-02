"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const basic_auth_1 = __importDefault(require("basic-auth"));
const logger_1 = __importDefault(require("../logger"));
const user_1 = __importDefault(require("../models/user"));
const subscription_watcher_1 = __importDefault(require("../subscription_watcher"));
let subscription_watcher = new subscription_watcher_1.default();
let rf = async (r) => {
    await subscription_watcher.start_watching();
    r.ws('/ping', async (ws, _req) => {
        logger_1.default.DEBUG('got ws ping request');
        ws.send('');
        ws.on('message', (mesg) => {
            logger_1.default.DEBUG(mesg.toString());
            if ('ping' != mesg.toString().toLowerCase())
                return;
            logger_1.default.DEBUG('pong');
            ws.send('pong');
        });
        logger_1.default.DEBUG('finished instrumenting ws ping request');
    });
    r.ws('/firehose', (ws, req) => {
        let auth_result = (0, basic_auth_1.default)(req);
        if (undefined == auth_result) {
            return ws.close();
        }
        user_1.default.findByUsernameAndPassword(auth_result.name, auth_result.pass).
            then(u => {
            if (undefined == u) {
                return ws.close();
            }
            logger_1.default.DEBUG(u);
            subscription_watcher.subscribe(u, ws);
        });
    });
    r.ws('/satellite/:id', (ws, req) => {
        let auth_result = (0, basic_auth_1.default)(req);
        if (undefined == auth_result) {
            return ws.close();
        }
        user_1.default.findByUsernameAndPassword(auth_result.name, auth_result.pass).
            then(u => {
            if (undefined == u) {
                return ws.close();
            }
            logger_1.default.DEBUG(u);
            subscription_watcher.subscribe(u, ws, req.params.id);
        }).
            catch(e => {
            logger_1.default.ERROR(`got ${e} on ws:///satellite/${req.params.id}`);
            return ws.terminate();
        });
    });
    return r;
};
exports.default = rf;
