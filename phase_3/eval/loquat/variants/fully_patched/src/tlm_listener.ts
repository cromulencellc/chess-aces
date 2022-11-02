import dgram from 'dgram'
import util from 'util'

import sql from './sql'

import logger from './logger'

import { Telemetry, PositionVelocity, Battery, TelemetryObject } from './tlm_proto'


export default class TlmListener {
  port: number
  server: dgram.Socket

  constructor(port: number) {
    this.port = port
    this.server = dgram.createSocket('udp4')

    this.server.on('message', this.didReceive.bind(this))
  }

  async listen() {
    await new Promise((resolve, reject) => {
      this.server.on('listening', () => {
        let addr = this.server.address()
        logger.INFO(`tlm listening on ${addr.address}:${addr.port}`)
        resolve(true)
      })
      this.server.bind(this.port)
    })
  }

  didReceive(message: Buffer, rinfo: dgram.RemoteInfo) {
    logger.DEBUG(`received ${message.byteLength} byte message from ${util.inspect(rinfo)}`)

    let got_tlm = Telemetry.decode(message)
    let got_o = Telemetry.toObject(got_tlm) as TelemetryObject
    let metric = {
      'satellite_id': got_o.satelliteId || null,
      'tx_at': new Date(got_o.txTimeMs) || null,
      'pv_x': got_o.pv.x || null,
      'pv_dx': got_o.pv.dx || null,
      'pv_y': got_o.pv.y || null,
      'pv_dy': got_o.pv.dy || null,
      'pv_z': got_o.pv.z || null,
      'pv_dz': got_o.pv.dz || null,
      'bt_level': got_o.bt.level || null,
      'bt_charging_time': got_o.bt.chargingTime || null,
      'bt_discharging_time': got_o.bt.dischargingTime || null
  }

    sql`INSERT INTO TELEMETRY ${sql(metric)}`.
      catch(err => {
        logger.ERROR(`rejected tlm from ${util.inspect(rinfo)} because ${err}`)
      })
  }
}