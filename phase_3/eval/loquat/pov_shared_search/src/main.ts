import { randomInt, randomUUID } from 'crypto'
import dgram from 'dgram'
import http from 'http'
import process from 'process'
import { URL } from 'url'
import util from 'util'

import WebSocket from 'ws'
import needle, { NeedleResponse } from 'needle'

import {assert, assert_array_match, assert_equal, refute} from './assertions'
import logger from './logger'
import Navstar from './navstar'
import User from './user'
import { isArray } from '@tool-belt/type-predicates'

class Pov {
  timeout: number

  host: string
  http_port: number
  tlm_port: number

  base_url: URL
  ws_urls: Map<string, URL> = new Map()

  navstars: Map<string, Navstar> = new Map()
  users: User[] = []

  current_time: Date
  time_step = 1000 * 60 * 60 // 1000ms * 60s * 60m; hour steps

  navstar_promise: Promise<any>

  udp_sock: dgram.Socket
  assertion_count: number = 0

  admin_username: string
  admin_password: string

  constructor() {
    this.host = process.env['HOST'] || 'ta3_loquat'
    this.http_port = parseInt(process.env['HTTP_PORT'] || '3080')
    this.tlm_port = parseInt(process.env['TLM_PORT'] || '3081')

    this.timeout = 1000 * parseInt(process.env['TIMEOUT'] || '5')

    this.current_time = new Date(Date.now())

    this.base_url = new URL(`http://${this.host}:${this.http_port}/`)

    this.navstar_promise = Navstar.load_navstars().then(ns => this.navstars = ns)

    this.admin_username = process.env['ADMIN_USERNAME']
    this.admin_password = process.env['ADMIN_PASSWORD']
    if (!this.admin_username || !this.admin_password) {
      throw "expected ADMIN_USERNAME and ADMIN_PASSWORD environment variables"
    }

    this.udp_sock = dgram.createSocket('udp4')
  }

  async run() {
    await this.navstar_promise
    if (0 == this.navstars.size) {
      throw "expected navstars to be loaded"
    }

    await this.load_ws_urls()
    await this.check_admin_login()

    let timeout = setTimeout(() => {
        logger.FATAL(`${this.timeout}ms timeout expired`)
        process.exit(0)
      }, this.timeout)

    // set up a user and a navstar they have access to
    let user = await this.random_user_with_navstar()
    let navstar_id = user.random_navstar_id()
    let navstar = this.navstars.get(navstar_id)

    let filter = {satellite_ids: [navstar.uuid]}
    let filter_post_got = await this.post('/filter', filter)
    assert_equal(201, filter_post_got.statusCode)
    assert(filter_post_got.headers['location'])

    let filter_path = filter_post_got.headers['location']
      
    // admin removes the user's access
    await this.remove_user_satellite(user, navstar)

    // send data to the ingest port that the user shouldn't get
    await this.send_privileged_data(navstar)

    let search_got = await this.user_get(filter_path, user)
    assert_equal(200, search_got.statusCode)
    var got_data = JSON.parse(search_got.read())    

    refute(0 == got_data.length)

    let heredoc = randomUUID()
    console.log(`PRIVDATA_HERE=${heredoc}`)
    console.log(util.inspect(got_data))
    console.log(heredoc)
  }


  //
  // ACTIVITIES
  //

  async ping_ws() {
    let ping_ws = new WebSocket(this.ws_urls.get('ping'))
    await new Promise((resolve, _reject) => {
      ping_ws.on('open', () => resolve(ping_ws))
    })
    let got_px = new Promise((resolve, _reject) => {
      ping_ws.on('message', data => resolve(data))
    })
    ping_ws.send('ping')
    let got = await got_px
    assert_equal("pong", got.toString())
  }

  async start_firehose_subscription(user: User, navstar: Navstar) {

    let mesg = navstar.telemetryAt(this.next_timestamp())

    let firehose_url = this.ws_urls.get('firehose')
    firehose_url.username = user.name
    firehose_url.password = user.password

    let firehose_ws = new WebSocket(firehose_url)
    await new Promise((resolve, _reject) => {
      firehose_ws.on('open', () => resolve(firehose_ws))
    })
    return firehose_ws
  }



  async send_privileged_data(navstar: Navstar) {
    let mesg = navstar.telemetryAt(this.next_timestamp())

    await new Promise((resolve, reject) => {
      this.udp_sock.send(mesg, this.tlm_port, this.host,
        (err: Error, sent_bytes: number) => {
          if (err) return reject(err)
          resolve(sent_bytes)
        })
    })
  }


  got_message_sym = Symbol('got_message_sym')
  timed_out_sym = Symbol('timed_out_sym')


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

  async remove_user_satellite(u: User, navstar: Navstar) {
    let n_id = navstar.uuid

    let got = await this.admin_delete(`/admin/user/${u.name}/satellites/${n_id}`)
    assert_equal(204, got.statusCode)

    let check_got = await this.admin_get(`/admin/user/${u.name}`)
    assert_equal(200, check_got.statusCode)

    let body = JSON.parse(check_got.read())
    assert_array_match([],
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

    let navstar_key = navstar_keys[randomInt(navstar_keys.length - 1)]

    return this.navstars.get(navstar_key)
  }

  async random_user() {
    if (0 == this.users.length) {
      logger.INFO("no known users, adding one")
      await this.create_user()
    }

    if (1 == this.users.length) {
      return this.users[0]
    }

    let idx = randomInt(this.users.length - 1)
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

    if (1 == users_with_navstars.length) {
      return users_with_navstars[0]
    }

    let idx = randomInt(users_with_navstars.length - 1)
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
  let p = new Pov()
  await p.run()
  console.log("pov exited successfully")
  process.exit(0)
})()
