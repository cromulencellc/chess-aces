"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const process_1 = __importDefault(require("process"));
const NS_PER_SEC = 1e9;
class Timing {
    constructor() {
        this.count = 0;
        this.total = 0.0;
        this.min = Number.MAX_VALUE;
        this.max = Number.MIN_VALUE;
    }
    async time(fn) {
        let before_time = process_1.default.hrtime();
        await fn();
        let elapsed = process_1.default.hrtime(before_time);
        let elapsed_s = elapsed[0] + (elapsed[1] / NS_PER_SEC);
        this.add(elapsed_s);
    }
    add(elapsed) {
        this.count++;
        this.total += elapsed;
        if (this.min > elapsed)
            this.min = elapsed;
        if (this.max < elapsed)
            this.max = elapsed;
    }
    avg() {
        if (0 == this.count)
            return -1;
        return this.total / this.count;
    }
}
exports.default = Timing;
