"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const crypto_1 = require("crypto");
class User {
    constructor() {
        this.navstar_ids = [];
        this.name = (0, crypto_1.randomUUID)();
        this.password = (0, crypto_1.randomUUID)();
    }
    params() {
        return {
            name: this.name,
            password: this.password
        };
    }
    has_navstars() {
        return this.navstar_ids.length > 0;
    }
    random_navstar_id() {
        if (1 == this.navstar_ids.length)
            return this.navstar_ids[0];
        let idx = (0, crypto_1.randomInt)(0, this.navstar_ids.length - 1);
        return this.navstar_ids[idx];
    }
}
exports.default = User;
