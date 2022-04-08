"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.Topic = void 0;
const logger_1 = __importDefault(require("../logger"));
class Topic {
    constructor(attrs) {
        this.id = attrs.id;
        this.name = attrs.name;
        this.created_at = attrs.created_at;
    }
    save() {
        if (this.id)
            return this;
        let stmt = global.iroquois.db.prepare("INSERT INTO topics (name) VALUES (:name) \
ON CONFLICT DO NOTHING");
        let got = stmt.run({ name: this.name });
        logger_1.default.DEBUG(got);
        if (got.changes > 0) {
            this.id = Number(got.lastInsertRowid);
        }
        else {
            let found = Topic.findByName(this.name);
            if (undefined == found) {
                throw `topic #{this.name} existed during insert but couldn't find id`;
            }
            this.id = found.id;
        }
        return this;
    }
    static each(cb) {
        const stmt = global.iroquois.db.prepare("SELECT * FROM topics;");
        var idx = 0;
        for (const row of stmt.iterate()) {
            cb(new Topic(row), idx);
            idx++;
        }
    }
    static allNames() {
        const stmt = global.iroquois.db.prepare("SELECT name FROM topics;");
        return stmt.all().map(r => r.name);
    }
    static find(id) {
        const stmt = global.iroquois.db.prepare('SELECT * FROM topics WHERE id = ?');
        const found = stmt.get(id);
        if (undefined == found)
            return undefined;
        return new Topic(found);
    }
    static findByName(name) {
        const stmt = global.iroquois.db.prepare('SELECT * FROM topics WHERE name = ?');
        const found = stmt.get(name);
        if (undefined == found)
            return undefined;
        return new Topic(found);
    }
    static create(attrs) {
        let topic = new Topic(attrs);
        topic.save();
        return topic;
    }
    static prune() {
        let db = global.iroquois.db;
        let tx = db.transaction(() => {
            db.exec('CREATE TEMP TABLE prune_topics AS \
      SELECT id, name FROM topics \
      WHERE id NOT IN (SELECT DISTINCT topic_id FROM subscriptions);');
            db.prepare('SELECT topic_id, count(id) as c FROM subscriptions\
      WHERE topic_id IN (SELECT id FROM prune_topics) GROUP BY topic_id;').
                all().
                forEach((row) => logger_1.default.DEBUG(`${row.topic_id}\t${row.c} `));
            db.prepare('SELECT id, name FROM prune_topics').
                all().
                forEach((row) => {
                global.iroquois.broker.unsubscribe(row.name);
            });
            db.exec('DELETE FROM topics WHERE id IN (SELECT id FROM prune_topics);');
            db.exec('DROP TABLE prune_topics');
        });
        tx.exclusive();
    }
}
exports.Topic = Topic;
