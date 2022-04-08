"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Message = void 0;
const topic_1 = require("./topic");
class Message {
    constructor(attrs) {
        this.id = attrs.id;
        this.topic_id = attrs.topic_id;
        this.content = attrs.content;
        this.created_at = new Date(attrs.created_at);
        if (attrs.topic instanceof topic_1.Topic) {
            this.topic_id = attrs.topic.id;
            this._topic = attrs.topic;
        }
        else if (attrs._topic_name && attrs._topic_created_at) {
            this._topic = new topic_1.Topic({
                id: this.topic_id,
                name: attrs._topic_name,
                created_at: attrs._topic_created_at
            });
        }
    }
    save() {
        if (this.id)
            return this;
        let stmt = global.iroquois.db.prepare("INSERT INTO messages (topic_id, content) \
VALUES (CAST(:topic_id AS INT), :content)");
        let _got = stmt.run({ topic_id: this.topic_id, content: this.content });
        Message.prune();
        return this;
    }
    static findByTopicIdNewerThan(topic_id, newer_than) {
        let stmt = global.iroquois.db.prepare("SELECT * FROM messages \
WHERE topic_id = :topic_id AND created_at >= datetime(:newer_than) \
ORDER BY created_at DESC");
        let found = stmt.all({ topic_id: topic_id,
            newer_than: newer_than.toISOString() });
        return found.map((row) => new Message(row));
    }
    static prune() {
        let stmt = global.iroquois.db.exec("\
    WITH recent_messages AS ( \
      SELECT id, \
        row_number() \
          OVER(PARTITION BY topic_id ORDER BY id DESC) AS most_recent \
        FROM messages \
      ), \
    to_delete AS ( \
      SELECT id FROM recent_messages WHERE most_recent > 100) \
    DELETE FROM messages WHERE id IN to_delete; \
");
    }
    static create(attrs) {
        let message = new Message(attrs);
        return message.save();
    }
}
exports.Message = Message;