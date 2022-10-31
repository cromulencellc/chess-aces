#!/usr/bin/env ts-node-script
"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const util_1 = __importDefault(require("util"));
const connect_flash_1 = __importDefault(require("connect-flash"));
const cookie_session_1 = __importDefault(require("cookie-session"));
const express_1 = __importDefault(require("express"));
const express_ws_1 = __importDefault(require("express-ws"));
const logger_1 = __importDefault(require("./logger"));
const tlm_listener_1 = __importDefault(require("./tlm_listener"));
const user_1 = __importDefault(require("./models/user"));
const admin_1 = __importDefault(require("./routes/admin"));
const base_1 = __importDefault(require("./routes/base"));
const user_2 = __importDefault(require("./routes/user"));
const ws_1 = __importDefault(require("./routes/ws"));
async function main(_argv) {
    if (!process.env.CHESS) {
        logger_1.default.FATAL("This application is for research purposes only");
        process.exit(1);
    }
    if (process.env.CLOBBER_ADMIN) {
        await user_1.default.clobber_default_admin();
    }
    else {
        await user_1.default.make_default_admin();
    }
    let { app } = (0, express_ws_1.default)((0, express_1.default)());
    app.set('view engine', 'pug');
    app.set('views', process.env.VIEW_DIR || '/static/view');
    app.use(express_1.default.urlencoded({ extended: true }));
    app.use((req, resp, next) => {
        logger_1.default.INFO(`${req.method} ${req.path} ${util_1.default.inspect(req.body)}`);
        next();
    });
    app.use((0, cookie_session_1.default)({ secret: process.env.SESSION_SECRET ||
            'ha9nohrohvahx6Ke2qui' }));
    app.use((0, connect_flash_1.default)());
    app.use('/static', express_1.default.static(process.env.STATIC_DIR || '/static/static'));
    app.use('/favicon.ico', express_1.default.static('/static/static/favicon.ico'));
    app.use('/', base_1.default);
    app.use('/', user_2.default);
    app.use('/admin', admin_1.default);
    app.use('/', await (0, ws_1.default)(express_1.default.Router()));
    let port = parseInt(process.env.PORT || '3080');
    let tlm_listener = new tlm_listener_1.default(parseInt(process.env.TLM_PORT || '3081'));
    await tlm_listener.listen();
    app.listen(port, () => {
        logger_1.default.INFO(`http listening on port ${port}`);
    });
}
exports.default = main(process.argv).
    catch(err => {
    logger_1.default.FATAL(err);
    process.exit(1);
});
