#!/usr/bin/env ts-node-script
"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
    if (mod && mod.__esModule) return mod;
    var result = {};
    if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
    __setModuleDefault(result, mod);
    return result;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const util_1 = __importDefault(require("util"));
const connect_flash_1 = __importDefault(require("connect-flash"));
const cookie_session_1 = __importDefault(require("cookie-session"));
const csurf_1 = __importDefault(require("csurf"));
const express_1 = __importDefault(require("express"));
const better_sqlite3_1 = __importDefault(require("better-sqlite3"));
const Authentication = __importStar(require("./authentication"));
const broker_1 = require("./broker");
const logger_1 = __importDefault(require("./logger"));
const dashboard_1 = __importDefault(require("./routes/dashboard"));
const subscriptions_1 = __importDefault(require("./routes/subscriptions"));
const users_1 = __importDefault(require("./routes/users"));
async function main(_argv) {
    if (!process.env.CHESS) {
        logger_1.default.FATAL("This application is for research purposes only");
        process.exit(1);
    }
    let database_path = process.env.DB_PATH || '/data/iroquois.sqlite3';
    let database_options = { fileMustExist: true,
        verbose: logger_1.default.DEBUG };
    let db = better_sqlite3_1.default(database_path, database_options);
    if (!process.env.DONT_DELETE_CHEATSHEET) {
        db.exec('DROP TABLE IF EXISTS users_cheatsheet;');
    }
    db.pragma('foreign_keys = ON');
    let broker = new broker_1.Broker(process.env.MQTT_URL || 'tcp://mosquitto:1883');
    let csrf_middleware = csurf_1.default();
    let app = express_1.default();
    app.set('view engine', 'pug');
    app.set('views', process.env.VIEW_DIR || '/static/view');
    app.use(express_1.default.urlencoded({ extended: true }));
    app.use((req, resp, next) => {
        logger_1.default.INFO(`${req.method} ${req.path} ${util_1.default.inspect(req.params)}`);
        next();
    });
    app.use(cookie_session_1.default({ secret: process.env.SESSION_SECRET ||
            'AngelfishLanguageCeremonyRemissionMammogramTipping' }));
    app.use(csrf_middleware);
    app.use(connect_flash_1.default());
    app.use(Authentication.passport.initialize());
    app.use(Authentication.passport.session());
    global['iroquois'] = {
        broker: broker,
        db: db
    };
    app.use('/static', express_1.default.static(process.env.STATIC_DIR || '/static/static'));
    app.use('/dashboard', dashboard_1.default);
    app.use('/subscriptions', subscriptions_1.default);
    app.use('/users', users_1.default);
    app.post('/session', Authentication.process_login());
    app.post('/logout', Authentication.log_out());
    app.get('/', Authentication.prohibit_logged_in(), (req, resp) => {
        resp.render('index', { csrf: req.csrfToken(),
            flash: req.flash() });
    });
    let port = parseInt(process.env.PORT || '3035');
    app.listen(port, () => {
        logger_1.default.INFO(`iroquois listening on port ${port}`);
    });
}
exports.default = main(process.argv).
    catch(err => logger_1.default.FATAL(err));
