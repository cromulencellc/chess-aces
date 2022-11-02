"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const sql_1 = __importDefault(require("./sql"));
const type_predicates_1 = require("@tool-belt/type-predicates");
const util_1 = require("util");
const logger_1 = __importDefault(require("./logger"));
class Subscription {
    constructor(u, s, o) {
        this.satellite_ids = new Set();
        this.user = u;
        this.sock = s;
        this.only = o;
        this.user.satellites().
            then(sxs => {
            for (let sx of sxs) {
                this.satellite_ids.add(sx.id);
            }
        }).
            then(() => {
            var sids = Array.from(this.satellite_ids);
            if (this.only)
                sids = [this.only];
            logger_1.default.DEBUG(sids);
            this.sock.send(JSON.stringify(sids));
        });
    }
    has_satellite_id(id) {
        if (this.only && (this.only != id))
            return false;
        return this.satellite_ids.has(id);
    }
}
class SubscriptionWatcher {
    constructor() {
        this.subscriptions = [];
    }
    async start_watching() {
        await sql_1.default.listen('telemetry_inserts', this.got_telemetry_notification.bind(this));
    }
    subscribe(user, sock, only) {
        let sub = new Subscription(user, sock, only);
        this.subscriptions.push(sub);
        sock.on('close', this.unsubscribe.bind(this, sub));
        sock.on('error', this.unsubscribe.bind(this, sub));
        logger_1.default.DEBUG(`subscribed ${user.id} to ${(0, util_1.inspect)(sub.satellite_ids)}`);
    }
    unsubscribe(sub) {
        if (sub.sock.readyState == sub.sock.OPEN) {
            sub.sock.close();
        }
        let idx = this.subscriptions.indexOf(sub);
        this.subscriptions.splice(idx, 1);
    }
    got_telemetry_notification(content) {
        (0, type_predicates_1.assertIsString)(content);
        let [row_id, satellite_id] = content.split("\x1f");
        if ((!(0, type_predicates_1.isString)(row_id)) || (!(0, type_predicates_1.isString)(satellite_id)))
            return;
        let matching_subs = this.subscriptions.
            filter(sub => sub.has_satellite_id(satellite_id));
        logger_1.default.DEBUG(`found ${matching_subs.length} matching subs`);
        (0, sql_1.default) `SELECT * FROM telemetry WHERE id = ${row_id}`.
            then(([row]) => {
            let row_j = JSON.stringify(row);
            logger_1.default.DEBUG(`sending tlm ${row_j}...`);
            let pxs = matching_subs.map(sub => {
                logger_1.default.DEBUG(sub);
                let send_fn = sub.sock.send.bind(sub.sock, row_j, {});
                let send_px = (0, util_1.promisify)(send_fn);
                return send_px().
                    catch(e => {
                    logger_1.default.ERROR(`got ${e} sending telemetry to ${sub.user.id}, disconnecting`);
                    this.unsubscribe(sub);
                });
            });
            return Promise.all(pxs);
        });
    }
}
exports.default = SubscriptionWatcher;
