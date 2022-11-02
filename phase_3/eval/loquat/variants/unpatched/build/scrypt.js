"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const crypto_1 = __importDefault(require("crypto"));
const SCRYPT_KEY_LEN = 32;
const SCRYPT_COST = 0x4000;
const SCRYPT_BLOCK_SIZE = 8;
const SCRYPT_PARALLELISM = 3;
const SCRYPT_SALT_SIZE = 32;
function authenticate(digest, candidate) {
    let n_s, r_s, p_s, salt, hash;
    [n_s, r_s, p_s, salt, hash] = digest.split('$');
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
        return true;
    return false;
}
function create(new_password) {
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
    return pack;
}
exports.default = { authenticate, create };
