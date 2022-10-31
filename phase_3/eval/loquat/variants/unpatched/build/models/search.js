"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const util_1 = __importDefault(require("util"));
const sql_1 = __importDefault(require("../sql"));
const logger_1 = __importDefault(require("../logger"));
const type_predicates_1 = require("@tool-belt/type-predicates");
class Search {
    constructor(attrs) {
        try {
            this._satellite_ids = attrs.satellite_ids;
            this._tx_at_gt = attrs.tx_at_gt;
            this._tx_at_lt = attrs.tx_at_lt;
            this._bt_level_gt = attrs.bt_level_gt;
            this._bt_level_lt = attrs.bt_level_lt;
        }
        catch (e) {
            logger_1.default.ERROR(util_1.default.inspect(attrs));
            throw e;
        }
        if (attrs.id) {
            this.id = attrs.id;
        }
        else {
            this.id = undefined;
        }
    }
    async save() {
        if (undefined == this.id)
            return await this.insert();
        throw ("can't edit a search");
    }
    async insert() {
        let attrs = this.attrs();
        let [inserted] = await (0, sql_1.default) `
      INSERT INTO searches
      ${(0, sql_1.default)(this.attrs())}
      RETURNING id
    `;
        logger_1.default.DEBUG(inserted);
        let got_id = inserted.id;
        if (undefined == got_id) {
            throw `didn't get id back from search insertion: ${inserted}`;
        }
        this.id = got_id;
        return this.id;
    }
    async perform_search() {
        let c = "select * from telemetry where ";
        c += this.where_clause();
        return await sql_1.default.unsafe(c, []);
    }
    attrs() {
        let attrs = {
            // 2950 is the OID for uuid, hopefully it's stable
            // select oid, typname from pg_type where typname = 'uuid';
            satellite_ids: sql_1.default.array(this._satellite_ids, 2950)
        };
        if (this._tx_at_gt)
            attrs['tx_at_gt'] = this._tx_at_gt;
        if (this._tx_at_lt)
            attrs['tx_at_lt'] = this._tx_at_lt;
        if (this._bt_level_gt)
            attrs['bt_level_gt'] = this._bt_level_gt;
        if (this._bt_level_lt)
            attrs['bt_level_lt'] = this._bt_level_lt;
        return attrs;
    }
    where_clause() {
        var clauses = [];
        if ((0, type_predicates_1.isDate)(this._tx_at_gt))
            clauses.push(`tx_at >= ${this._tx_at_gt}`);
        if ((0, type_predicates_1.isDate)(this._tx_at_lt))
            clauses.push(`tx_at < ${this._tx_at_lt}`);
        if ((0, type_predicates_1.isNumber)(this._bt_level_gt))
            clauses.push(`bt_level >= ${this._bt_level_gt}`);
        if ((0, type_predicates_1.isNumber)(this._bt_level_lt))
            clauses.push(`bt_level < ${this._bt_level_lt}`);
        let sat_id = this.satellite_ids_clause();
        if (sat_id) {
            clauses.push(this.satellite_ids_clause());
        }
        return clauses.
            map(c => `(${c})`).
            join(' and ');
    }
    satellite_ids_clause() {
        if ((0, type_predicates_1.isUndefined)(this._satellite_ids))
            return null;
        let id_body = this._satellite_ids.
            map(i => `'${i}'`).
            join(', ');
        return `satellite_id in (${id_body})`;
    }
    static async findById(candidate_id) {
        if (!(0, type_predicates_1.isString)(candidate_id)) {
            return undefined;
        }
        let got = await (0, sql_1.default) `
      select * from searches where id = ${candidate_id}
    `;
        if (0 == got.count) {
            return undefined;
        }
        if (1 != got.count) {
            throw `wanted to find one search, got ${got}`;
        }
        return new Search(got[0]);
    }
}
exports.default = Search;
