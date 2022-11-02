"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.refute = exports.assert = exports.assert_equal = exports.assert_array_match = exports.assertion_count = void 0;
const type_predicates_1 = require("@tool-belt/type-predicates");
const logger_1 = __importDefault(require("./logger"));
const util_1 = __importDefault(require("util"));
exports.assertion_count = 0;
function assert_array_match(expected, got) {
    exports.assertion_count++;
    if (!(0, type_predicates_1.isArray)(expected)) {
        throw `assert_array_match first arg (expected) should be array, got ${util_1.default.inspect(expected)}`;
    }
    if (!(0, type_predicates_1.isArray)(got)) {
        logger_1.default.ERROR("expected:");
        console.log(expected);
        logger_1.default.ERROR("got non-array:");
        console.log(got);
        throw "failed assert_array_match (got non-array)";
    }
    if (expected.length != got.length) {
        logger_1.default.ERROR("expected:");
        console.log(expected);
        logger_1.default.ERROR("got:");
        console.log(got);
        throw "failed assert_array_match (length mismatch)";
    }
    for (var i = 0; i < expected.length; i++) {
        if (expected[i] != got[i]) {
            logger_1.default.ERROR("expected:");
            console.log(expected);
            logger_1.default.ERROR("got:");
            console.log(got);
            throw `failed assert_array_match (index ${i})`;
        }
    }
}
exports.assert_array_match = assert_array_match;
function assert_equal(expected, got) {
    if ((0, type_predicates_1.isArray)(expected)) {
        return assert_array_match(expected, got);
    }
    exports.assertion_count++;
    if (expected == got)
        return;
    logger_1.default.ERROR("expected:");
    console.log(expected);
    logger_1.default.ERROR("got:");
    console.log(got);
    throw "failed assert_equal";
}
exports.assert_equal = assert_equal;
function assert(expr) {
    exports.assertion_count++;
    if (expr)
        return;
    throw "failed assert";
}
exports.assert = assert;
function refute(expr) {
    exports.assertion_count++;
    if (!expr)
        return;
    throw "failed refute";
}
exports.refute = refute;
