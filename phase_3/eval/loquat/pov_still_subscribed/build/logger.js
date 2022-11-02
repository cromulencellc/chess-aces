"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const process_1 = __importDefault(require("process"));
const util_1 = __importDefault(require("util"));
const LevelNames = ["_INVALID", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"];
let LogLevels = new Map();
LevelNames.forEach((name, index, _arr) => { LogLevels.set(name, index); });
const log_level = LogLevels.get(process_1.default.env['LOG_LEVEL'] || 'DEBUG') || 1;
function maybe_log(candidate_level, message) {
    if (candidate_level < log_level)
        return;
    console.log(`[${LevelNames[candidate_level]}] ${util_1.default.format(message)}`);
}
exports.default = {
    NOOP: (_message) => { },
    DEBUG: (message) => maybe_log(1, message),
    INFO: (message) => maybe_log(2, message),
    WARN: (message) => maybe_log(3, message),
    ERROR: (message) => maybe_log(4, message),
    FATAL: (message) => maybe_log(5, message)
};
