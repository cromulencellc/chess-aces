"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const type_predicates_1 = require("@tool-belt/type-predicates");
const logger_1 = __importDefault(require("../logger"));
const sql_1 = __importDefault(require("../sql"));
class Satellite {
    constructor(attrs) {
        let maybe_id = attrs.id;
        (0, type_predicates_1.assertIsString)(maybe_id);
        this.id = maybe_id;
    }
    async save() {
        let got = await (0, sql_1.default) `
      INSERT INTO satellites
      ${(0, sql_1.default)(this.attrs())}
      ON CONFLICT DO NOTHING
      RETURNING id
    `;
        logger_1.default.DEBUG(got);
    }
    attrs() {
        return { id: this.id };
    }
    static async findOrCreateById(candidate_id) {
        if (undefined == candidate_id)
            return undefined;
        let s = new Satellite({ id: candidate_id });
        await s.save();
        return s;
    }
}
exports.default = Satellite;
