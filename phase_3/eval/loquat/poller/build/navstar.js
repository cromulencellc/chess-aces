"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const promises_1 = __importDefault(require("fs/promises"));
const satellite_js_1 = require("satellite.js");
const uuid_1 = require("uuid");
const tlm_proto_1 = require("./tlm_proto");
const logger_1 = __importDefault(require("./logger"));
class Navstar {
    constructor(json_blob) {
        this.blob = json_blob;
        this.name = json_blob['ON'];
        this.satrec = (0, satellite_js_1.twoline2satrec)(json_blob['TLE1'], json_blob['TLE2']);
        this.uuid = (0, uuid_1.v4)();
        this.battery_offset = Math.random();
        this.battery_period = Math.random();
    }
    telemetryContentAt(date) {
        let pv = (0, satellite_js_1.propagate)(this.satrec, date);
        if (typeof pv.position == 'boolean')
            throw 'got non-position';
        if (typeof pv.velocity == 'boolean')
            throw 'got non-velocity';
        let bt = 0.2 + (0.5 * Math.sin((this.battery_offset + (this.battery_period * date.getTime())) / (1000 * 60 * 60 * Math.PI)));
        return {
            satelliteId: this.uuid,
            txTimeMs: date.getTime(),
            pv: {
                x: pv.position.x,
                y: pv.position.y,
                z: pv.position.z,
                dx: pv.velocity.x,
                dy: pv.velocity.y,
                dz: pv.velocity.z
            },
            bt: {
                level: bt
            }
        };
    }
    telemetryAt(date) {
        let content = this.telemetryContentAt(date);
        logger_1.default.DEBUG(content);
        return tlm_proto_1.Telemetry.encode(content).finish();
    }
    static async load_navstars(path = "/static/navstar_tle.json") {
        const contents = await promises_1.default.readFile(path, { encoding: 'utf8' });
        let l = JSON.parse(contents);
        let navstars = new Map();
        for (let n of l) {
            let ns = new Navstar(n);
            navstars.set(ns.uuid, ns);
        }
        return navstars;
    }
}
exports.default = Navstar;
