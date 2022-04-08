"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.User = void 0;
const crypto_1 = __importDefault(require("crypto"));
const subscription_1 = require("./subscription");
const SCRYPT_KEY_LEN = 32;
const SCRYPT_COST = 0x4000;
const SCRYPT_BLOCK_SIZE = 8;
const SCRYPT_PARALLELISM = 3;
const SCRYPT_SALT_SIZE = 32;
class User {
    constructor(attrs) {
        this.id = attrs.id;
        this.name = attrs.name;
        this.password_digest = attrs.password_digest;
        this.created_at = new Date(attrs.created_at);
        if (attrs.password) {
            this.setPassword(attrs.password);
        }
    }
    authenticate(candidate) {
        let n_s, r_s, p_s, salt, hash;
        [n_s, r_s, p_s, salt, hash] = this.password_digest.split('$');
        let n = parseInt(n_s, 16);
        let r = parseInt(r_s, 16);
        let p = parseInt(p_s, 16);
        let salt_buf = Buffer.from(salt, 'hex');
        let hash_buf = Buffer.from(hash, 'hex');
        let candidate_hash = crypto_1.default.scryptSync(candidate, salt_buf, hash_buf.length, {
            cost: n,
            blockSize: r,
            parallelization: p
        });
        let eq = crypto_1.default.timingSafeEqual(hash_buf, candidate_hash);
        if (eq)
            return this;
        return false;
    }
    setPassword(new_password) {
        let salt_buf = crypto_1.default.randomBytes(SCRYPT_SALT_SIZE);
        let hash = crypto_1.default.scryptSync(new_password, salt_buf, SCRYPT_KEY_LEN, {
            cost: SCRYPT_COST,
            blockSize: SCRYPT_BLOCK_SIZE,
            parallelization: SCRYPT_PARALLELISM
        });
        let pack = [
            SCRYPT_COST.toString(16),
            SCRYPT_BLOCK_SIZE.toString(16),
            SCRYPT_PARALLELISM.toString(16),
            salt_buf.toString('hex'),
            hash.toString('hex')
        ].join('$');
        this.password_digest = pack;
    }
    get subscriptions() {
        if (this._subscriptions)
            return this.subscriptions;
        return this._subscriptions = subscription_1.Subscription.findByUserId(this.id);
    }
    findSubscription(sub_id) {
        return subscription_1.Subscription.findByIdAndUserId(sub_id, this.id);
    }
    save() {
        if (this.id)
            return this.update();
        return this.create();
    }
    update() {
        let stmt = global.iroquois.db.prepare("UPDATE users SET \
name = :name, password_digest = :password_digest \
WHERE id = :id");
        stmt.run({ name: this.name,
            password_digest: this.password_digest,
            id: this.id });
        return this;
    }
    create() {
        let stmt = global.iroquois.db.prepare("INSERT INTO users (name, password_digest) \
VALUES (:name, :password_digest)");
        let got = stmt.run({ name: this.name,
            password_digest: this.password_digest });
        this.id = Number(got.lastInsertRowid);
        return this;
    }
    static find(id) {
        const stmt = global.iroquois.db.prepare('SELECT * FROM users WHERE id = ?');
        const found = stmt.get(id);
        if (undefined == found)
            return undefined;
        return new User(found);
    }
    static findByName(name) {
        const stmt = global.iroquois.db.prepare('SELECT * FROM users WHERE name = ?');
        const found = stmt.get(name);
        if (undefined == found)
            return undefined;
        return new User(found);
    }
    static create(attrs) {
        let user = new User(attrs);
        user.save();
        return user;
    }
}
exports.User = User;
