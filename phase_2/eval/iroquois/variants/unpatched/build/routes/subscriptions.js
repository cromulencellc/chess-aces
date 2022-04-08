"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const express_1 = __importDefault(require("express"));
const authentication_1 = require("../authentication");
const logger_1 = __importDefault(require("../logger"));
const user_1 = require("../models/user");
const subscription_1 = require("../models/subscription");
const topic_1 = require("../models/topic");
let router = express_1.default.Router();
router.use(authentication_1.require_logged_in());
router.post('', (req, resp) => {
    let current_user = req.user;
    if ((undefined == current_user) ||
        !(current_user instanceof user_1.User)) {
        resp.sendStatus(401);
        return;
    }
    let topic_name = req.body.subscription.topic.name;
    if ('string' != typeof topic_name) {
        resp.sendStatus(400);
        return;
    }
    let maybe_topic = topic_1.Topic.findByName(topic_name);
    if (undefined == maybe_topic) {
        let new_topic = topic_1.Topic.create({ name: topic_name });
        global.iroquois.broker.subscribe(topic_name);
        let _subscription = subscription_1.Subscription.create({ user_id: current_user.id,
            topic_id: new_topic.id });
        resp.redirect('/dashboard');
        return;
    }
    let _subscription = subscription_1.Subscription.create({ user_id: current_user.id,
        topic_id: maybe_topic.id });
    resp.redirect('/dashboard');
});
router.get(/^\/(\d+)$/, (req, resp) => {
    let current_user = req.user;
    if ((undefined == current_user) ||
        !(current_user instanceof user_1.User)) {
        resp.sendStatus(401);
        return;
    }
    let sub_id = Number(req.params[0]);
    let sub = subscription_1.Subscription.find(sub_id);
    if (undefined == sub) {
        resp.sendStatus(404);
        return;
    }
    resp.render('subscriptions/show', { csrf: req.csrfToken(),
        flash: req.flash(),
        user: current_user,
        subscription: sub });
});
router.post(/^\/(\d+)\/message$/, (req, resp) => {
    let current_user = req.user;
    if ((undefined == current_user) ||
        !(current_user instanceof user_1.User)) {
        resp.sendStatus(401);
        return;
    }
    let sub_id = Number(req.params[0]);
    let sub = current_user.findSubscription(sub_id);
    if (undefined == sub) {
        resp.sendStatus(404);
        return;
    }
    global.iroquois.broker.publish(sub.topic, req.body.message.content).
        then(() => resp.redirect(`/subscriptions/${sub.id}`)).
        catch(err => {
        logger_1.default.ERROR(err);
        resp.sendStatus(500);
    });
});
router.post(/^\/(\d+)\/delete$/, (req, resp) => {
    let current_user = req.user;
    if ((undefined == current_user) ||
        !(current_user instanceof user_1.User)) {
        resp.sendStatus(401);
        return;
    }
    let sub_id = Number(req.params[0]);
    let sub = current_user.findSubscription(sub_id);
    if (undefined == sub) {
        resp.sendStatus(404);
        return;
    }
    sub.destroy();
    resp.redirect('/dashboard');
});
exports.default = router;
