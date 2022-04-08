import { Topic } from './topic'

import Logger from '../logger'

export class Message {
  id: number
  topic_id: number
  content: string | Buffer
  created_at: Date
  _topic: Topic | undefined

  constructor(attrs: any) {
    this.id = attrs.id
    this.topic_id = attrs.topic_id
    this.content = attrs.content
    this.created_at = new Date(attrs.created_at)

    if (attrs.topic instanceof Topic) {
      this.topic_id = attrs.topic.id
      this._topic = attrs.topic
    }
    else if (attrs._topic_name && attrs._topic_created_at) {
      this._topic = new Topic({
        id: this.topic_id,
        name: attrs._topic_name,
        created_at: attrs._topic_created_at
      })
    }
  }

  save(): this {
    if (this.id) return this

    // ifdef PATCH_USE_PREPARED_STATEMENTS_FOR_MESSAGE_SAVE
//     let stmt = global.iroquois.db.prepare(
//       "INSERT INTO messages (topic_id, content) \
// VALUES (CAST(:topic_id AS INT), :content)")
//     let _got = stmt.run({topic_id: this.topic_id, content: this.content})
    // else
    global.iroquois.db.exec(
      `INSERT INTO messages (topic_id, content) \
VALUES (${this.topic_id}, \
'${this.content}');`)
    // endif

    Message.prune()

    return this
  }

  static findByTopicIdNewerThan(topic_id: number,
                                newer_than: Date): Array<Message> {
    let stmt = global.iroquois.db.prepare(
      "SELECT * FROM messages \
WHERE topic_id = :topic_id AND created_at >= datetime(:newer_than) \
ORDER BY created_at DESC")
    let found = stmt.all({topic_id: topic_id,
                          newer_than: newer_than.toISOString()})

    return found.map((row: any) => new Message(row))
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
")

  }

  static create(attrs: any): Message {
    let message = new Message(attrs)
    return message.save()
  }
}
