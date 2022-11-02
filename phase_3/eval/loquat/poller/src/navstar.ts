import fs from 'fs/promises'

import {SatRec, twoline2satrec, propagate} from 'satellite.js'
import { v4 as uuidv4 } from 'uuid'

import TleBlob from './tle_blob'

import { Telemetry, PositionVelocity, Battery } from './tlm_proto'

import { Random } from 'random'
import logger from './logger'

export type Metric = {
    satelliteId: string,
    txTimeMs: number,

    pv: {
      x: number,
      y: number,
      z: number,

      dx: number,
      dy: number,
      dz: number
    },

    bt: {
      level: number
    }
  }

export default class Navstar {
  name: string
  satrec: SatRec
  blob: TleBlob
  uuid: string
  battery_period: number
  battery_offset: number

  constructor(json_blob: TleBlob) {
    this.blob = json_blob
    this.name = json_blob['ON']
    this.satrec = twoline2satrec(json_blob['TLE1'], json_blob['TLE2'])
    this.uuid = uuidv4()

    this.battery_offset = Math.random()
    this.battery_period = Math.random()
  }

  telemetryContentAt(date: Date): Metric {
    let pv = propagate(this.satrec, date)

    if (typeof pv.position == 'boolean') throw 'got non-position'
    if (typeof pv.velocity == 'boolean') throw 'got non-velocity'
    
    let bt = 0.2 + (0.5 * Math.sin((this.battery_offset + (
      this.battery_period * date.getTime())) / (1000 * 60 * 60 * Math.PI)))

    return {
      satelliteId: this.uuid,
      txTimeMs: date.getTime(),

      pv: {
        x: pv.position.x,
        y: pv.position.y,
        z: pv.position.z,

        dx: pv.velocity.x,
        dy: pv.velocity.y,
        dz: pv.velocity.z
      },

      bt: {
        level: bt
      }
    }
  }

  telemetryAt(date: Date) {
    let content = this.telemetryContentAt(date)

    logger.DEBUG(content)

    return Telemetry.encode(content).finish()
  }

  static async load_navstars(path: string = "/static/navstar_tle.json"): Promise<NavstarMap> {
    const contents = await fs.readFile(path, { encoding: 'utf8' })
    let l: [TleBlob] = JSON.parse(contents)
    let navstars: Map<string, Navstar> = new Map()
    for (let n of l) {
      let ns = new Navstar(n)
      navstars.set(ns.uuid, ns)
    }
    return navstars
  }
}

export type NavstarMap = Map<string, Navstar>