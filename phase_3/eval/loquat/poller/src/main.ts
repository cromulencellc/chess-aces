import crypto from 'crypto'
import dgram from 'dgram'
import http from 'http'
import process from 'process'
import { URL } from 'url'
import util from 'util'

import random, { Random } from 'random'
import seedrandom from 'seedrandom'
import {sprintf} from 'sprintf'
import WebSocket from 'ws'
import needle, { NeedleResponse } from 'needle'

import {
  assert, assert_equal,
  assertion_count,
  assert_array_match,
  refute } from './assertions'
import logger from './logger'
import Navstar from './navstar'
import User from './user'
import Timing from './timing'
import IncomingPost from './incoming_post'
import { assertIsArray, isArray, isUndefined } from '@tool-belt/type-predicates'
import { Telemetry } from './tlm_proto'

class Poller {
  length: number
  seed: number
  timeout: number

  rng: Random

  host: string
  http_port: number
  tlm_port: number

  base_url: URL
  ws_urls: Map<string, URL> = new Map()

  navstars: Map<string, Navstar> = new Map()
  users: User[] = []

  current_time: Date
  time_step = 1000 * 60 * 60 // 1000ms * 60s * 60m; hour steps

  timings: Map<string, Timing> = new Map()

  navstar_promise: Promise<any>

  udp_sock: dgram.Socket

  assertion_count: number = 0

  admin_username: string
  admin_password: string

  expected_token?: string

  ACTIVITIES = ("homepage " + 
  //"ping_ws " + 
  "submit_tlm_per submit_tlm_firehose " +
  "submit_tlm_firehose_nada " +
  // "check_tlm " +
  // "check_noexist " +
  "filter_tlm " +
  "create_user add_user_satellite remove_user_satellite " +
  // "user_watch_allow user_watch_forbid " +
  "check_admin_login check_user_refuse_admin "
  ).split(/\s+/)

  rejected_activities: string[] = []

  constructor() {
    this.host = process.env['HOST'] || 'ta3_loquat'
    this.http_port = parseInt(process.env['HTTP_PORT'] || '3080')
    this.tlm_port = parseInt(process.env['TLM_PORT'] || '3081')

    this.expected_token = process.env['EXPECTED_TOKEN']

    var seed = process.env['SEED'] ||
      crypto.randomInt(2**48 - 1).toString()
    console.log(`SEED=${seed}`)
    this.rng = random.clone(seedrandom(seed))
    this.rng.patch()

    var len = process.env['LENGTH'] ||
      this.rng.int(50, 100).toString()
    console.log(`LENGTH=${len}`)
    this.length = parseInt(len)

    this.timeout = 1000 * parseInt(process.env['TIMEOUT'] || '5')

    this.current_time = new Date(Date.now() - (this.length * this.time_step))

    this.base_url = new URL(`http://${this.host}:${this.http_port}/`)

    this.navstar_promise = Navstar.load_navstars().then(ns => this.navstars = ns)

    this.admin_username = process.env['ADMIN_USERNAME']
    this.admin_password = process.env['ADMIN_PASSWORD']
    if (!this.admin_username || !this.admin_password) {
      throw "expected ADMIN_USERNAME and ADMIN_PASSWORD environment variables"
    }


    for (let a of this.ACTIVITIES) {
      if (this[a] instanceof Function) {
        this.timings.set(a, new Timing())
      } else {
        this.rejected_activities.push(a)
        logger.INFO(`rejected activity ${a}`)
      }
    }

    this.udp_sock = dgram.createSocket('udp4')
  }

  async run() {
    await this.navstar_promise
    if (0 == this.navstars.size) {
      throw "expected navstars to be loaded"
    }

    if (0 == this.timings.size) {
      logger.ERROR("no activities, that was easy")
      return
    }

    let known_activity_names = Array.from(this.timings.keys())

    await this.load_ws_urls()
    await this.check_admin_login()

    for (let i = 0; i < this.length; i++) {
      let acti_idx = this.rng.int(0, this.timings.size - 1)
      let acti_name = known_activity_names[acti_idx]

      let acti_fn = this[acti_name].bind(this)
      let acti_time = this.timings.get(acti_name)

      if (! (acti_fn instanceof Function)) {
        throw `couldn't get activity named ${acti_name} (got a ${typeof acti_fn})`
      }
      logger.INFO(`trying ${acti_name}`)
      let timeout = setTimeout(() => {
        logger.FATAL(`${this.timeout}ms timeout expired`)
        process.exit(1)
      }, this.timeout)
      await acti_time.time(() => acti_fn())
      clearTimeout(timeout)
    }
  }

  print_report() {
    if (0 != this.rejected_activities.length) {
      console.log('rejected activities: (not implemented?)')
      console.log(this.rejected_activities.join("\n"))
    }

    console.log(sprintf("%20s\t%s\t%s\t%s\t%s",
      'activity', 'count', 'min', 'avg', 'max'
    ))

    this.timings.forEach((tmg, name) => {
      console.log(sprintf("%20s\t%d\t%1.4f\t%1.4f\t%1.4f",
        name, tmg.count, tmg.min, tmg.avg(), tmg.max
      ))
    })

    console.log(`${assertion_count} assertions`)
  }

  //
  // ACTIVITIES
  //

  async homepage() {
    let got = await this.get('/')

    assert_equal(200, got.statusCode)
  }

  async ping_ws() {
    let url = this.ws_urls.get('ping')
    let ping_ws = new WebSocket(url)
    let ping_ws_open = await new Promise((resolve, _reject) => {
      ping_ws.on('open', () => resolve(ping_ws))
    })
    logger.DEBUG(ping_ws_open)
    logger.DEBUG(`did ${url} open`)

    await new Promise((resolve, _reject) => {
      ping_ws.on('message', data => {
        logger.DEBUG(data)
        resolve(null)
      })
    })
    logger.DEBUG(`websocket opened`)
    let got_px = new Promise((resolve, _reject) => {
      ping_ws.on('message', data => resolve(data))
    })
    logger.DEBUG('sending ping')
    ping_ws.send('ping')
    logger.DEBUG('waiting for pong')
    let got = await got_px
    assert_equal("pong", got.toString())
  }

  async submit_tlm_firehose() {
    let user = await this.random_user_with_navstar()
    let navstar = this.navstars.get(user.random_navstar_id(this.rng))

    let mesg = navstar.telemetryAt(this.next_timestamp())

    let firehose_url = this.ws_urls.get('firehose')
    firehose_url.username = user.name
    firehose_url.password = user.password

    logger.DEBUG(firehose_url)

    let firehose_ws = new WebSocket(firehose_url)
    await new Promise((resolve, _reject) => {
      firehose_ws.on('open', () => resolve(firehose_ws))
    })

    let sids: string = await new Promise((resolve, _reject) => {
      firehose_ws.on('message', data => resolve(data.toString()))
    })
    let sida = JSON.parse(sids)
    assertIsArray(sida)

    assert_equal(user.navstar_ids.sort(),
      sida.sort()
    )


    logger.DEBUG('sending tlm')
    await new Promise((resolve, reject) => {
      this.udp_sock.send(mesg, this.tlm_port, this.host,
        (err: Error, sent_bytes: number) => {
          if (err) return reject(err)
          resolve(sent_bytes)
        })
    })

    logger.DEBUG('waiting for message')
    let got = await new Promise((resolve, _reject) => {
      firehose_ws.on('message', data => resolve(data))
    })

    let recv_blob = JSON.parse(got.toString())
    assert_equal(navstar.uuid, recv_blob['satellite_id'])
  }

  async submit_tlm_per() {
    let user = await this.random_user_with_navstar()
    let navstar = this.navstars.get(user.random_navstar_id(this.rng))

    logger.DEBUG(user.navstar_ids)
    logger.DEBUG(navstar)

    let mesg = navstar.telemetryAt(this.next_timestamp())

    let per_url = new URL(this.ws_urls.get('per-satellite').toString())
    per_url.username = user.name
    per_url.password = user.password
    per_url.pathname =
      per_url.pathname.replace(':id', navstar.uuid)
    logger.DEBUG(per_url)

    let per_ws = new WebSocket(per_url)
    await new Promise((resolve, _reject) => {
      per_ws.on('open', () => resolve(per_ws))
    })

    let sids: string = await new Promise((resolve, _reject) => {
      per_ws.on('message', data => resolve(data.toString()))
    })
    let sida = JSON.parse(sids)
    assertIsArray(sida)

    assert_equal([navstar.uuid], sida)
    logger.DEBUG('subscribed to expected satellite')

    logger.DEBUG(user.id)

    logger.DEBUG('sending tlm')
    await new Promise((resolve, reject) => {
      this.udp_sock.send(mesg, this.tlm_port, this.host,
        (err: Error, sent_bytes: number) => {
          if (err) return reject(err)
          resolve(sent_bytes)
        })
    })

    logger.DEBUG('waiting for message')
    let got = await new Promise((resolve, _reject) => {
      per_ws.on('message', data => {
        // logger.DEBUG(util.inspect(data))
        resolve(data)
      })
    })
    // logger.DEBUG(`got ${got}`)

    let recv_blob = JSON.parse(got.toString())
    assert_equal(navstar.uuid, recv_blob['satellite_id'])
  }


  got_message_sym = Symbol('got_message_sym')
  timed_out_sym = Symbol('timed_out_sym')

  async submit_tlm_firehose_nada() {
    await this.create_user()
    let user = this.users[this.users.length - 1]
    let navstar = this.random_navstar()

    let mesg = navstar.telemetryAt(this.next_timestamp())

    let firehose_url = this.ws_urls.get('firehose')
    firehose_url.username = user.name
    firehose_url.password = user.password

    let firehose_ws = new WebSocket(firehose_url)
    await new Promise((resolve, _reject) => {
      firehose_ws.on('open', () => resolve(firehose_ws))
    })

    let sids: string = await new Promise((resolve, _reject) => {
      firehose_ws.on('message', data => resolve(data.toString()))
    })
    let sida = JSON.parse(sids)
    assertIsArray(sida)

    assert_equal(user.navstar_ids.sort(),
      sida.sort()
    )

    let maybe_got_message = new Promise((resolve, _reject) => {
      firehose_ws.on('message', data => resolve(this.got_message_sym))
    })
    let maybe_timeout = new Promise((resolve, _reject) => {
      setTimeout(() => {
        resolve(this.timed_out_sym)
      }, this.timeout / 2)
    })

    logger.DEBUG('sending tlm')
    await new Promise((resolve, reject) => {
      this.udp_sock.send(mesg, this.tlm_port, this.host,
        (err: Error, sent_bytes: number) => {
          if (err) return reject(err)
          resolve(sent_bytes)
        })
    })

    logger.DEBUG('waiting for no message')
    await Promise.
      any([maybe_got_message, maybe_timeout]).
      then(result => {
        assert_equal(this.timed_out_sym, result)
      })
  }

  async filter_tlm() {
    let user = await this.random_user_with_navstar()
    let navstar = this.navstars.get(user.random_navstar_id(this.rng))

    // create filter
    let filter_battery_gteq = this.rng.boolean()
    let battery_level = 0.25 + (this.rng.normal()() / 2)

    let ts = this.next_timestamp()

    let metric = navstar.telemetryContentAt(ts)
    var will_match = false
    
    var filter: any = {}
    if (filter_battery_gteq) {
      filter['bt_level_gt'] = battery_level
      will_match = metric.bt.level >= battery_level
    } else {
      filter['bt_level_lt'] = battery_level
      will_match = metric.bt.level < battery_level
    }

    filter['satellite_ids'] = [navstar.uuid]

    let filter_post_got = await this.post('/filter', filter)
    assert_equal(201, filter_post_got.statusCode)
    assert(filter_post_got.headers['location'])

    let filter_path = filter_post_got.headers['location']

    // submit tlm
    logger.DEBUG('sending tlm')
    await new Promise((resolve, reject) => {
      this.udp_sock.send(Telemetry.encode(metric).finish(), this.tlm_port, this.host,
        (err: Error, sent_bytes: number) => {
          if (err) return reject(err)
          resolve(sent_bytes)
        })
    })
    logger.DEBUG(`expecting match: ${will_match}`)

    await new Promise((resolve, _reject) => {
      setTimeout(resolve, 1000)
    })

    // witness me
    let search_got = await this.user_get(filter_path, user)
    assert_equal(200, search_got.statusCode)
    var got_data = JSON.parse(search_got.read())
    var did_find = false

    if (! isArray(got_data)) got_data = [got_data]

    logger.DEBUG(metric)
    for (let datum of got_data) {
      if (Date.parse(datum['tx_at']) == ts.valueOf()) {
        logger.DEBUG(datum)
        did_find = true
        break
      }
    }

    assert_equal(will_match, did_find)
  }

  async create_user() {
    let u = new User()
    let got = await this.post('/user', u.params())
    assert_equal(201, got.statusCode)
    u.id = got.headers.location.split('/').pop()
    this.users.push(u)
  }

  async add_user_satellite() {
    let u = await this.random_user()
    let navstar = this.random_navstar()

    let got = await this.admin_post(`/admin/user/${u.name}/satellites`, {
      satellite_id: navstar.uuid
    })

    assert_equal(204, got.statusCode)

    if (! u.navstar_ids.includes(navstar.uuid)) {
      u.navstar_ids.push(navstar.uuid)
    }

    let check_got = await this.admin_get(`/admin/user/${u.name}`)
    assert_equal(200, check_got.statusCode)

    let body = JSON.parse(check_got.read())

    assert_equal(u.navstar_ids.sort(),
      body['satellite_ids'].sort())
  }

  async remove_user_satellite() {
    let u = await this.random_user_with_navstar()

    let n_id = u.remove_random_navstar_id(this.rng)

    let got = await this.admin_delete(`/admin/user/${u.name}/satellites/${n_id}`)
    assert_equal(204, got.statusCode)

    let check_got = await this.admin_get(`/admin/user/${u.name}`)
    assert_equal(200, check_got.statusCode)

    let body = JSON.parse(check_got.read())
    assert_array_match(u.navstar_ids.sort(),
      body['satellite_ids'].sort())
  }

  //
  // ACTIVITY HELPERS
  //

  async load_ws_urls() {
    let got = await this.get('/ws.json')
    assert_equal(200, got.statusCode)
    let ws_urls = JSON.parse(got.read()) as object

    for (const url_name in ws_urls) {
      let n = new URL(this.base_url.toString())
      let b = new URL(ws_urls[url_name])
      n.protocol = b.protocol
      n.pathname = b.pathname
      this.ws_urls.set(url_name, n)
    }
    // logger.DEBUG(this.ws_urls)
  }

  async check_admin_login() {
    let got = await this.admin_get('/admin')
    assert_equal(200, got.statusCode)

    if (isUndefined(this.expected_token)) return

    let got_token = got.
      read().
      toString().
      replace(/\s+$/, '')

    logger.DEBUG(util.inspect(this.expected_token))
    logger.DEBUG(util.inspect(got_token))

    assert_equal(this.expected_token, got_token)
  }

  async check_user_refuse_admin() {
    let got = await this.get('/admin')
    assert_equal(401, got.statusCode)

    if (isUndefined(this.expected_token)) return

    let got_content = got.read()
    if (! got_content) {
      assert(true)
      return
    }

    logger.DEBUG(`kinda wanted ${got_content} to be empty :/`)

    refute(got_content.toString().match(this.expected_token))
  }

  async get(path: string): Promise<http.IncomingMessage> {
    let url = new URL(path, this.base_url)

    logger.DEBUG(`GET ${url}`)

    return new Promise((resolve, reject) => {
      http.get(url, resp => {
        resolve(resp)
      }).
        on("error", e => {
          logger.FATAL(`GET ${url} got ${e}`)
          reject(e)
        })

    })
  }

  async admin_get(path: string): Promise<http.IncomingMessage> {
    let url = new URL(path, this.base_url)

    logger.DEBUG(`GET admin ${url}`)

    return new Promise((resolve, reject) => {
      http.get(url,
        {auth: `${this.admin_username}:${this.admin_password}`},
        resp => {
          resolve(resp)
        }).
          on('error', e => {
            logger.FATAL(`admin GET ${url} got ${e}`)
            reject(e)
          })
    })
  }

  async user_get(path: string, user: User): Promise<http.IncomingMessage> {
    let url = new URL(path, this.base_url)
    logger.DEBUG(`GET user ${user.name} ${url}`)

    return new Promise((resolve, reject) => {
      http.get(url,
        {auth: `${user.name}:${user.password}`},
        resp => {
          resolve(resp)
        }).
          on('error', e => {
            logger.FATAL(`user GET ${url} got ${e}`)
            reject(e)
          })
    })
  }

  async post(path: string, body_params: object): Promise<NeedleResponse> {
    let url = new URL(path, this.base_url)

    logger.DEBUG(`POST ${url} ${util.inspect(body_params)}`)

    return await needle('post', url.toString(), body_params)
  }

  async admin_post(path: string, body_params?: object): Promise<NeedleResponse> {
    let url = new URL(path, this.base_url)

    logger.DEBUG(`POST admin ${url} ${util.inspect(body_params)}`)

    return await needle('post', url.toString(), body_params,
      {username: this.admin_username, password: this.admin_password})
  }

  async admin_delete(path: string, body_params?: object): Promise<NeedleResponse> {
    let url = new URL(path, this.base_url)

    logger.DEBUG(`DELETE admin ${url} ${util.inspect(body_params)}`)

    return await needle('delete', url.toString(), body_params,
      {username: this.admin_username, password: this.admin_password})
  }

  random_navstar() {
    let navstar_keys = Array.from(this.navstars.keys())

    let navstar_key = navstar_keys[this.rng.integer(0,
      navstar_keys.length - 1)]

    return this.navstars.get(navstar_key)
  }

  async random_user() {
    if (0 == this.users.length) {
      logger.INFO("no known users, adding one")
      await this.create_user()
    }

    let idx = this.rng.integer(0, this.users.length - 1)
    return this.users[idx]
  }

  async random_user_with_navstar() {
    let users_with_navstars = this.users.filter(u => u.has_navstars())
    if (0 == users_with_navstars.length) {
      logger.INFO("no users with navstars, adding one")
      let u = await this.random_user()
      let s = this.random_navstar()
      let got = await this.admin_post(`/admin/user/${u.name}/satellites`, {
        satellite_id: s.uuid
      })

      assert_equal(204, got.statusCode)
      u.navstar_ids.push(s.uuid)


      logger.DEBUG(u)

      return u
    }

    let idx = this.rng.integer(0, users_with_navstars.length - 1)
    return users_with_navstars[idx]
  }

  next_timestamp() {
    let cur = this.current_time
    this.current_time =
      new Date(this.current_time.valueOf() + this.time_step)

    return cur
  }
}

(async () => {
  let p = new Poller()
  await p.run()
  p.print_report()
  console.log("Poller exited successfully")
  process.exit(0)
})()
