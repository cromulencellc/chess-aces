"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Subscription = void 0;
const message_1 = require("./message");
const topic_1 = require("./topic");
class Subscription {
    constructor(attrs) {
        this.id = attrs.id;
        this.user_id = attrs.user_id;
        this.topic_id = attrs.topic_id;
        this.created_at = new Date(attrs.created_at);
        if (attrs._topic_name && attrs._topic_created_at) {
            this._topic = new topic_1.Topic({
                id: this.topic_id,
                name: attrs._topic_name,
                created_at: attrs._topic_created_at
            });
        }
    }
    get topic() {
        if (this._topic)
            return this._topic;
        let found = topic_1.Topic.find(this.topic_id);
        if (undefined == found) {
            throw `couldn't find topic for subscription ${this.id}`;
        }
        return this._topic = found;
    }
    get messages() {
        if (this._messages)
            return this._messages;
        return this._messages =
            message_1.Message.findByTopicIdNewerThan(this.topic_id, this.created_at);
    }
    save() {
        if (this.id)
            return this;
        let stmt = global.iroquois.db.prepare("INSERT INTO subscriptions \
(user_id, topic_id) VALUES (:user_id, :topic_id) \
ON CONFLICT DO NOTHING");
        let got = stmt.run({ user_id: this.user_id,
            topic_id: this.topic_id });
        if (got.lastInsertRowid) {
            this.id = Number(got.lastInsertRowid);
        }
        else {
            let found = Subscription.findByUserIdAndTopicId(this.user_id, this.topic_id);
            if (undefined == found) {
                throw `subscription for user ${this.user_id} \
and topic ${this.topic_id} existed during insert but couldn't find id`;
            }
            this.id = found.id;
        }
        return this;
    }
    destroy() {
        let old_topic = this.topic;
        let stmt = global.iroquois.db.prepare("DELETE FROM subscriptions WHERE id=?");
        let got = stmt.run(this.id);
        if (1 != got.changes) {
            throw `deleting subscription ${this.id} to topic ${old_topic.name} \
deleted ${got.changes} rows unexpectedly`;
        }
    }
    static findByUserIdAndTopicId(user_id, topic_id) {
        const stmt = global.iroquois.db.prepare('SELECT subscriptions.*, \
topics.name as _topic_name, topics.created_at as _topic_created_at \
FROM subscriptions \
JOIN topics ON topics.id = subscriptions.topic_id \
WHERE user_id = :user_id AND topic_id = :topic_id');
        const found = stmt.get({ user_id: user_id, topic_id: topic_id });
        if (undefined == found)
            return undefined;
        return new Subscription(found);
    }
    static findByIdAndUserId(id, user_id) {
        const stmt = global.iroquois.db.prepare('SELECT * FROM subscriptions WHERE id = :id AND user_id = :user_id');
        const found = stmt.get({ id: id, user_id: user_id });
        if (undefined == found)
            return undefined;
        return new Subscription(found);
    }
    static findByUserId(user_id) {
        const stmt = global.iroquois.db.prepare('SELECT subscriptions.*, \
topics.name as _topic_name, topics.created_at AS _topic_created_at \
FROM subscriptions \
JOIN topics ON topics.id = subscriptions.topic_id \
WHERE user_id = ? \
ORDER BY _topic_name ASC');
        const found = stmt.all(user_id);
        return found.map((row) => new Subscription(row));
    }
    static create(attrs) {
        let subscription = new Subscription(attrs);
        return subscription.save();
    }
}
exports.Subscription = Subscription;