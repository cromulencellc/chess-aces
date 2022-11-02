"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.pb_root = exports.Battery = exports.PositionVelocity = exports.Telemetry = void 0;
const protobufjs_1 = __importDefault(require("protobufjs"));
let pb_root = protobufjs_1.default.loadSync('/static/tlm.proto');
exports.pb_root = pb_root;
let Telemetry = pb_root.lookupType('Telemetry');
exports.Telemetry = Telemetry;
let PositionVelocity = pb_root.lookupType('PositionVelocity');
exports.PositionVelocity = PositionVelocity;
let Battery = pb_root.lookupType('Battery');
exports.Battery = Battery;
