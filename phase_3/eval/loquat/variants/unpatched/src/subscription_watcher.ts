import User from "./models/user"
import sql from "./sql"

import { WebSocket } from 'ws'
import { assertIsDefined, assertIsString, isString, isUndefined } from "@tool-belt/type-predicates"
import { inspect, promisify } from "util"
import logger from "./logger"

class Subscription {
  user: User
  sock: WebSocket
  satellite_ids: Set<string> = new Set()
  only?: string

  constructor(u: User, s: WebSocket, o?: string) {
    this.user = u
    this.sock = s
    this.only = o

    this.user.satellites().
      then(sxs => {
        for (let sx of sxs) {
          this.satellite_ids.add(sx.id)
        }
      }).
      then(() => {
        var sids = Array.from(this.satellite_ids)
        if (this.only) sids = [this.only]
        logger.DEBUG(sids)
        this.sock.send(JSON.stringify(sids))
      })
  }

  has_satellite_id(id: string) {
    if (this.only && (this.only != id)) return false

    return this.satellite_ids.has(id)
  }

}

export default class SubscriptionWatcher {
  subscriptions: Subscription[] = []

  async start_watching() {
    await sql.listen('telemetry_inserts',
      this.got_telemetry_notification.bind(this))

  }

  subscribe(user: User, sock: WebSocket, only?: string) {
    let sub = new Subscription(user, sock, only)
    this.subscriptions.push(sub)
    sock.on('close', this.unsubscribe.bind(this, sub))
    sock.on('error', this.unsubscribe.bind(this, sub))
    logger.DEBUG(`subscribed ${user.id} to ${inspect(sub.satellite_ids)}`)
  }

  unsubscribe(sub: Subscription) {
    if (sub.sock.readyState == sub.sock.OPEN) {
      sub.sock.close()
    }
    let idx = this.subscriptions.indexOf(sub)
    this.subscriptions.splice(idx, 1)
  }

  got_telemetry_notification(content?: string) {
    assertIsString(content)
    let [row_id, satellite_id] = content.split("\x1f")

    if ((! isString(row_id)) || (! isString(satellite_id))) return

    let matching_subs =
      this.subscriptions.
      filter(sub => sub.has_satellite_id(satellite_id))

    logger.DEBUG(`found ${matching_subs.length} matching subs`)

    sql`SELECT * FROM telemetry WHERE id = ${row_id}`.
      then(([row]) => {
        let row_j = JSON.stringify(row)
        logger.DEBUG(`sending tlm ${row_j}...`)
        let pxs = matching_subs.map(sub => {
          logger.DEBUG(sub)
          let send_fn = sub.sock.send.bind(sub.sock,
            row_j, {})
          let send_px = promisify(send_fn)

          return send_px().
            catch(e => {
              logger.ERROR(`got ${e} sending telemetry to ${sub.user.id}, disconnecting`)
              this.unsubscribe(sub)
            })
        })
        return Promise.all(pxs)
      })
  }

}
