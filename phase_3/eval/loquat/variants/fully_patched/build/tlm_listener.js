"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const dgram_1 = __importDefault(require("dgram"));
const util_1 = __importDefault(require("util"));
const sql_1 = __importDefault(require("./sql"));
const logger_1 = __importDefault(require("./logger"));
const tlm_proto_1 = require("./tlm_proto");
class TlmListener {
    constructor(port) {
        this.port = port;
        this.server = dgram_1.default.createSocket('udp4');
        this.server.on('message', this.didReceive.bind(this));
    }
    async listen() {
        await new Promise((resolve, reject) => {
            this.server.on('listening', () => {
                let addr = this.server.address();
                logger_1.default.INFO(`tlm listening on ${addr.address}:${addr.port}`);
                resolve(true);
            });
            this.server.bind(this.port);
        });
    }
    didReceive(message, rinfo) {
        logger_1.default.DEBUG(`received ${message.byteLength} byte message from ${util_1.default.inspect(rinfo)}`);
        let got_tlm = tlm_proto_1.Telemetry.decode(message);
        let got_o = tlm_proto_1.Telemetry.toObject(got_tlm);
        let metric = {
            'satellite_id': got_o.satelliteId || null,
            'tx_at': new Date(got_o.txTimeMs) || null,
            'pv_x': got_o.pv.x || null,
            'pv_dx': got_o.pv.dx || null,
            'pv_y': got_o.pv.y || null,
            'pv_dy': got_o.pv.dy || null,
            'pv_z': got_o.pv.z || null,
            'pv_dz': got_o.pv.dz || null,
            'bt_level': got_o.bt.level || null,
            'bt_charging_time': got_o.bt.chargingTime || null,
            'bt_discharging_time': got_o.bt.dischargingTime || null
        };
        (0, sql_1.default) `INSERT INTO TELEMETRY ${(0, sql_1.default)(metric)}`.
            catch(err => {
            logger_1.default.ERROR(`rejected tlm from ${util_1.default.inspect(rinfo)} because ${err}`);
        });
    }
}
exports.default = TlmListener;
