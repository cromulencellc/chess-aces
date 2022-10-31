"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const logger_1 = __importDefault(require("./logger"));
const postgres_1 = __importDefault(require("postgres"));
const util_1 = __importDefault(require("util"));
let sql = (0, postgres_1.default)((process.env.DATABASE_URL ||
    'postgres://postgres:Cae4noh5eghaT2vie5nu@db/postgres'), {
    types: {
        bigint: postgres_1.default.BigInt
    },
    debug: (_conn, query, params) => {
        logger_1.default.DEBUG(`${query} <= ${util_1.default.inspect(params)}`);
    }
});
exports.default = sql;
