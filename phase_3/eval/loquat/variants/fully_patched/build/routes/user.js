"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = __importDefault(require("express"));
const express_basic_auth_1 = __importDefault(require("express-basic-auth"));
const csurf_1 = __importDefault(require("csurf"));
const logger_1 = __importDefault(require("../logger"));
const user_1 = __importDefault(require("../models/user"));
const search_1 = __importDefault(require("../models/search"));
const type_predicates_1 = require("@tool-belt/type-predicates");
let r = express_1.default.Router();
r.use((0, express_basic_auth_1.default)({
    authorizer: user_1.default.authorizeUser,
    authorizeAsync: true
}));
let csrf_middleware = (0, csurf_1.default)();
r.get('/filter/:filter_id', async (req, resp) => {
    let f = await search_1.default.findById(req.params['filter_id']);
    let u = await user_1.default.findByUsername(req.auth.user);
    (0, type_predicates_1.assertIsDefined)(u);
    (0, type_predicates_1.assertIsDefined)(u.id);
    let got = await f.perform_search(u.id);
    logger_1.default.DEBUG(JSON.stringify(got));
    resp.send(JSON.stringify(got));
});
exports.default = r;
