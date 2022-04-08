"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = __importDefault(require("express"));
const authentication_1 = require("../authentication");
let dashboard = express_1.default.Router();
dashboard.use(authentication_1.require_logged_in());
dashboard.get('/', (req, resp) => {
    resp.render('dashboard', { csrf: req.csrfToken(),
        flash: req.flash(),
        user: req.user });
});
exports.default = dashboard;
