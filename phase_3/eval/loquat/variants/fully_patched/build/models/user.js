"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const crypto_1 = __importDefault(require("crypto"));
const util_1 = __importDefault(require("util"));
const scrypt_1 = __importDefault(require("../scrypt"));
const sql_1 = __importDefault(require("../sql"));
const logger_1 = __importDefault(require("../logger"));
const type_predicates_1 = require("@tool-belt/type-predicates");
const satellite_1 = __importDefault(require("./satellite"));
function digest_password(attrs) {
    if ((0, type_predicates_1.isString)(attrs.password_digest)) {
        return attrs.password_digest;
    }
    if ((0, type_predicates_1.isBuffer)(attrs.password_digest)) {
        return attrs.password_digest.toString();
    }
    if ((0, type_predicates_1.isString)(attrs.password)) {
        return scrypt_1.default.create(attrs.password);
    }
    throw `couldn't digest password from ${util_1.default.inspect(attrs)}`;
}
class User {
    constructor(attrs) {
        try {
            this.name = attrs.name;
            this.password_digest = digest_password(attrs);
            this.is_admin = !!attrs.is_admin;
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
        return await this.update();
    }
    async insert() {
        let [inserted] = await (0, sql_1.default) `
      INSERT INTO users 
      ${(0, sql_1.default)(this.attrs())}
      RETURNING id
    `;
        let got_id = inserted.id;
        if (undefined == got_id) {
            throw `didn't get id back from user insertion: ${inserted}`;
        }
        this.id = got_id;
        return this.id;
    }
    async update() {
        (0, type_predicates_1.assertIsString)(this.id);
        await (0, sql_1.default) `UPDATE USERS 
      SET ${(0, sql_1.default)(this.attrs())} 
      WHERE id = ${this.id}`;
        return this.id;
    }
    set name(n) {
        (0, type_predicates_1.assertIsString)(n);
        this._name = n;
    }
    get name() {
        (0, type_predicates_1.assertIsString)(this._name);
        return this._name;
    }
    set password_digest(d) {
        (0, type_predicates_1.assertIsString)(d);
        this._password_digest = d;
    }
    get password_digest() {
        (0, type_predicates_1.assertIsString)(this._password_digest);
        return this._password_digest;
    }
    set is_admin(a) {
        (0, type_predicates_1.assertIsBoolean)(a);
        this._is_admin = a;
    }
    get is_admin() {
        (0, type_predicates_1.assertIsBoolean)(this._is_admin);
        return this._is_admin;
    }
    attrs() {
        return {
            name: this.name,
            password_digest: this.password_digest,
            is_admin: this.is_admin
        };
    }
    async satellites() {
        (0, type_predicates_1.assertIsString)(this.id);
        let ids = await (0, sql_1.default) `
      SELECT satellite_id AS id
      FROM satellites_users
      WHERE user_id = ${this.id}
    `;
        let sxs = ids.map(row => {
            logger_1.default.DEBUG(row);
            let s = new satellite_1.default(row);
            s.save();
            return s;
        });
        return sxs;
    }
    async add_satellite(satellite_id) {
        let attrs = {
            user_id: this.id,
            satellite_id: satellite_id
        };
        let got_sat = await satellite_1.default.findOrCreateById(satellite_id);
        if (undefined == got_sat) {
            throw `couldn't add satellite ${satellite_id} to user ${this.name}`;
        }
        await (0, sql_1.default) `
      INSERT INTO satellites_users
      ${(0, sql_1.default)(attrs)}
      ON CONFLICT DO NOTHING
    `;
    }
    async remove_satellite(satellite_id) {
        (0, type_predicates_1.assertIsString)(this.id);
        await (0, sql_1.default) `
      DELETE FROM satellites_users
      WHERE user_id = ${this.id} AND
        satellite_id = ${satellite_id}
    `;
    }
    static async clobber_default_admin() {
        let _got = await (0, sql_1.default) `update users set is_admin = false`;
        return this.make_default_admin();
    }
    static async make_default_admin() {
        let got = await (0, sql_1.default) `select count(id) from users where is_admin;`;
        let user_count = got[0].count;
        if (typeof user_count != 'bigint') {
            throw `expected numeric user count, got (${typeof user_count}) ${user_count}`;
        }
        logger_1.default.DEBUG(`found ${user_count} admins`);
        if (0n != user_count)
            return;
        let password = crypto_1.default.randomUUID();
        let user_attrs = {
            name: `admin ${crypto_1.default.randomInt(100000)}`,
            password_digest: scrypt_1.default.create(password),
            is_admin: true
        };
        let did_create = await (0, sql_1.default) `
      insert into users
        ${(0, sql_1.default)(user_attrs)}
    `;
        if (1 != did_create.count) {
            throw `wanted one row back when creating user, got ${did_create}`;
        }
        let admin_env = `ADMIN_USERNAME='${user_attrs.name}' ADMIN_PASSWORD='${password}'`;
        logger_1.default.WARN(`created new admin user ${admin_env}`);
    }
    static async findByUsername(candidate_username) {
        if (!(0, type_predicates_1.isString)(candidate_username)) {
            return undefined;
        }
        let got = await (0, sql_1.default) `
      select * from users where name = ${candidate_username}
    `;
        if (0 == got.count)
            return undefined;
        if (1 != got.count) {
            throw `wanted to find one user, got ${got}`;
        }
        return new User(got[0]);
    }
    static async findByUsernameAndPassword(candidate_username, candidate_password) {
        if (!(0, type_predicates_1.isString)(candidate_username) || !(0, type_predicates_1.isString)(candidate_password)) {
            return undefined;
        }
        let u = await this.findByUsername(candidate_username);
        if (undefined == u)
            return undefined;
        if (!scrypt_1.default.authenticate(u.password_digest, candidate_password)) {
            return undefined;
        }
        return u;
    }
    static authorizeUser(username, password, callback) {
        User.findByUsername(username).
            then(u => {
            if (undefined == u)
                return callback(null, false);
            if (!scrypt_1.default.authenticate(u.password_digest, password)) {
                return callback(null, false);
            }
            return callback(null, true);
        }).
            catch(e => {
            return callback(e, false);
        });
    }
    static authorizeAdmin(username, password, callback) {
        User.findByUsername(username).
            then(u => {
            if (undefined == u)
                return callback(null, false);
            if (!scrypt_1.default.authenticate(u.password_digest, password)) {
                return callback(null, false);
            }
            if (u.is_admin)
                return callback(null, true);
            return callback(null, false);
        }).
            catch(e => {
            return callback(e, false);
        });
    }
}
exports.default = User;
