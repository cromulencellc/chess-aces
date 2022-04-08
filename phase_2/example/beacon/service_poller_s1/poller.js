'use strict'

const assert = require('assert').strict
const crypto = require('crypto')
const fs = require('fs/promises')
const path = require('path')
const process = require('process')

const leveldown = require('leveldown')
const levelup = require('levelup')
const playwright = require('playwright')
const random = require('random')
const seedrandom = require('seedrandom')
const sprintf = require('sprintf-js').sprintf

// seconds to wait per activity
const DEFAULT_TIME_LIMIT = 10

const DEFAULT_HOST = 'challenge'
const DEFAULT_PORT = '3028'

const MIN_LENGTH = 50
const MAX_LENGTH = 100

const WORDS =
      ("alpha bravo charlie delta echo foxtrot golf hotel india " +
       "juliet kilo lima mike november oscar papa quebec romeo " +
       "sierra tango uniform victor whiskey xray yankee zulu"
      ).split(/\s+/)

const NS_PER_SEC = 1e9

class Timing {
  constructor() {
    this.count = 0
    this.total = 0.0
    this.min = Number.MAX_VALUE
    this.max = Number.MIN_VALUE
  }

  add(time) {
    this.count ++
    this.total += time
    if (this.min > time) this.min = time
    if (this.max < time) this.max = time
  }

  mean() {
    if (0 == this.count) return -1
    return (this.total / this.count)
  }
}

const ACTIVITIES =
      ("homepage " +
       "index_tweets get_tweet post_tweet post_photo_tweet " +
       "get_noexist_tweet " +
       "index_orders filter_orders " +
       "get_noexist_order get_unauthenticated_order " +
       "get_wrong_auth_order get_authenticated_order " +
       "login_unprivileged login_admin create_user"
      ).split(/\s+/)

class Poller {
  constructor() {
    this.host = process.env['HOST'] || DEFAULT_HOST
    this.port = process.env['PORT'] || DEFAULT_PORT


    this.base = new URL(`http://${this.host}:${this.port}`);

    this._collection = new Map()

    this.report = new Map()
    for (const a of ACTIVITIES) {
      this.report.set(a, new Timing())
    }

    this.seed_rng()
    this.pick_length()
    this.pick_timeout()
  }

  async run() {
    this.browser = await playwright['webkit'].launch()
    this.context = await this.browser.newContext()

    for (let i = 0; i < this.length; i++) {
      let activity_index = this.rng.int(0, ACTIVITIES.length - 1)
      let activity_name = ACTIVITIES[activity_index]

      let activity_fn = this[activity_name]

      if (! activity_fn) {
        console.log(`couldn't find activity named ${activity_name}`)
        continue
      }

      console.log(`trying ${activity_name}`)

      this.page = await this.context.newPage()

      let before_time = process.hrtime()
      let timeout_id = setTimeout(this.timed_out, this.timeout * 1000)
      try {
        await activity_fn.apply(this)
      } catch (e) {
        clearTimeout(timeout_id)
        console.log(`caught ${e}`)
        console.log(e.stack)
        console.log('screenshotting...')
        let screenshot_name =
            `tmp/${Math.floor(Date.now() / 1000)}-${i}.png`
        await this.page.screenshot({path: screenshot_name})
        console.log(`screenshot in ${screenshot_name}`)
        throw e
      }
      clearTimeout(timeout_id)
      let elapsed_time = process.hrtime(before_time)

      let elapsed_s = elapsed_time[0] + (elapsed_time[1] / NS_PER_SEC)
      this.report.get(activity_name).add(elapsed_s)

      await this.page.close()
    }

    await this.browser.close()
  }

  timed_out() {
    console.log('TIMED OUT')
    process.exit(2)
  }

  print_report() {
    console.log(sprintf('%20s\t%s\t%s\t%s\t%s',
                        'name', 'count', 'min', 'mean', 'max'))
    for (const [name, entry] of this.report) {
      if (0 >= entry.count) continue
      console.log(sprintf('%20s\t%d\t%1.4f\t%1.4f\t%1.4f',
                          name,
                          entry.count, entry.min,
                          entry.mean(), entry.max))
    }
  }

  seed_rng() {
    let seed = process.env['SEED'] || crypto.randomInt(2**48 - 1).toString()

    console.log(`SEED=${seed}`)

    this.rng = random.clone(seedrandom(seed))
  }

  pick_length() {
    var length = parseInt(process.env['LENGTH'] || -1)
    if (0 >= length) {
      length = this.rng.int(MIN_LENGTH, MAX_LENGTH)
    }

    console.log(`LENGTH=${length}`)
    this.length = length
  }

  pick_timeout() {
    this.timeout = parseInt(process.env['TIMEOUT'] || DEFAULT_TIME_LIMIT)
    console.log(`TIMEOUT=${this.timeout}`)
  }

  async homepage() {
    let page = await this.visit('/')
    let header = await page.innerText('h1')
    assert.equal(header, "Beacon")
  }

  async index_tweets() {
    let page = await this.visit('/tweets/')
    let header = await page.innerText('h1')
    assert.equal(header, "Tweets")
    assert.equal(await page.innerText('a#more_link'), 'more')
    let more_path = await page.getAttribute('a#more_link', 'href')


    let tweets_coll = await this.collection('tweets')
    let tweets_stream = tweets_coll.createValueStream({limit: 25})

    await new Promise((resolve, _rej) => tweets_stream.on('readable', resolve))

    while (tweets_stream.readable) {
      let data = tweets_stream.read()
      if (null == data) continue

      var tw = data
      if ('string' == typeof data) {
        tw = JSON.parse(data)
      } else if (Buffer.isBuffer(data)) {
        tw = JSON.parse(data.toString())
      }

      // console.log(tw)

      let check_pxs = []

      await page.$(`#tweet-${tw.id}`).
        then(el => {
          if (null == el) {
            assert.fail(`couldn't find tweet id ${tw.id}`)
          }

          check_pxs.push(
            new Promise((resolve, _reject) => {
              el.$('.body').
                then(bd => bd.innerText()).
                then(it => {
                  assert.equal(it, tw.body)
                  //console.log('good body')
                  resolve()
                }).
                catch(err => assert.fail(`poller error ${err}`))
            }),
            new Promise((resolve, _reject) => {
              el.$('time').
                then(tm => tm.getAttribute('data-epoch')).
                then(ep => {
                  assert.equal(ep,
                               Math.floor(tw.created_at / 1000).toString())
                  //console.log('good timestamp')
                  resolve()
                }).
                catch(err => assert.fail(`poller error ${err}`))
            }))

          if (tw.mime_type) {
            check_pxs.push(new Promise((resolve, _reject) => {
              el.$('.has_image').
                then(hi => {
                  if (null == hi) assert.fail(`expected ${tw.id} to has_image`)
                  //console.log('good image')
                  resolve()
                }).
                catch(err => assert.fail(`poller error ${err}`))
            }))
          }
        }).
        catch(err => assert.fail(`poller error ${err}`))

      //console.log(`waiting on ${check_pxs.length}`)

      await Promise.all(check_pxs)


      //console.log('did wait')
    }

    tweets_stream.destroy()

  }

  async get_tweet() {
    let keys = await this.list_ids('tweets')
    let key = keys[this.rng.int(0, keys.length - 1)]
    console.log(key)
    let tweet = await this.read('tweets', key)

    let page = await this.visit(`/tweets/${key}`)

    let header = await page.innerText('h1')
    assert.equal(header, `Tweet ${key}`)

    let body = await page.innerText('p')
    assert.equal(body.replace(/\s+$/, ''), tweet.body.replace(/\s+$/, ''))

    if (tweet.mime_type) {
      let photo = await page.getAttribute('img', 'src')
      assert.equal(photo, `/tweets/${key}/photo`)
    }
  }

  async get_noexist_tweet() {
    let page = await this.visit('/tweets/noexist')
    let message = await page.innerText('pre')
    assert.equal(message, "Bad Request")
  }

  async post_tweet() {
    let body = []
    let word_count = this.rng.int(5, 15)
    for (var i = 0; i < word_count; i++) {
      body.push(WORDS[this.rng.int(0, WORDS.length - 1)])
    }

    let attrs = {body: body.join(' ')}

    let page = await this.visit('/tweets/')
    await page.fill('input[type=text]', body.join(' '))

    await Promise.all([page.waitForNavigation({url: /tweets\/[^\/]+$/}),
                       page.click('input[type=submit]')])

    let got_body = await page.innerText('p')
    assert.equal(got_body.replace(/\s+$/, ''), body.join(' '))

    let got_id = (await page.innerText('h1')).
        replace(/^Tweet /, '').
        replace(/\s+$/, '')

    attrs['id'] = got_id
    attrs['created_at'] =
      (await page.getAttribute('time', 'data-epoch')) * 1000

    console.log(attrs['created_at'])

    let coll = await this.collection('tweets')
    await coll.put(got_id, JSON.stringify(attrs))
  }

  async post_photo_tweet() {
    let body = []
    let word_count = this.rng.int(5, 15)
    for (var i = 0; i < word_count; i++) {
      body.push(WORDS[this.rng.int(0, WORDS.length - 1)])
    }

    let attrs = {body: body.join(' ')}

    let page = await this.visit('/tweets/')
    await page.fill('input[type=text]', body.join(' '))

    let photos =
        (await fs.readdir(path.join(__dirname, 'pics'),
                          {withFileTypes: true})).
        filter(de => de.isFile())
    let photo = photos[this.rng.int(0, photos.length - 1)]

    attrs['mime_type'] = true // only checked for presence, not content

    await page.setInputFiles('input[type=file]',
                             path.join(__dirname, 'pics', photo.name))

    await Promise.all([page.waitForNavigation({url: /tweets\/[^\/]+$/}),
                       page.click('input[type=submit]')])

    let got_body = await page.innerText('p')
    assert.equal(got_body.replace(/\s+$/, ''), body.join(' '))

    let got_id = (await page.innerText('h1')).
        replace(/^Tweet /, '').
        replace(/\s+$/, '')

    attrs['id'] = got_id
    attrs['created_at'] =
      (await page.getAttribute('time', 'data-epoch')) * 1000

    console.log(attrs['created_at'])

    let coll = await this.collection('tweets')
    await coll.put(got_id, JSON.stringify(attrs))
  }

  async index_orders() {
    let page = await this.visit('/orders/')
    let header = await page.innerText('h1')
    assert.equal(header, 'Orders')

    let orders_coll = await this.collection('orders')
    let orders_stream = orders_coll.createValueStream({limit: 25})

    await new Promise((resolve, _rej) => orders_stream.on('readable', resolve))

    while (orders_stream.readable) {
      let data = orders_stream.read()
      if (null == data) continue

      var ord = data
      if ('string' == typeof data) {
        ord = JSON.parse(data)
      } else if (Buffer.isBuffer(data)) {
        ord = JSON.parse(data.toString())
      }

      let check_pxs = []

      await page.$(`#order-${ord.id}`).
        then(async (el) => {
          if (null == el) {
            assert.fail(`couldn't find order id ${ord.id}`)
          }

          await el.scrollIntoViewIfNeeded()

          check_pxs.push(
            new Promise((resolve, _reject) => {
              el.$('.id').
                then(id => id.innerText()).
                then(it => {
                  assert.equal(it, ord.id)
                  resolve()
                }).
                catch(err => assert.fail(`poller error ${err}`))
                  }),
            new Promise((resolve, _reject) => {
              el.$('.postcode').
                then(pc => pc.innerText()).
                then(it => {
                  assert.equal(it, ord.postcode)
                  resolve()
                })
            }),
            new Promise((resolve, _reject) => {
              el.$('.total').
                then(tot => tot.innerText()).
                then(it => {
                  assert.equal(it,
                               (ord.total_cents / 100).toString())
                  resolve()
                })
            }))
        }).
        catch(err => assert.fail(`poller error ${err}`))


          await Promise.all(check_pxs)
    }

    orders_stream.destroy()
  }

  async filter_orders() {
    let boundary_price = this.rng.int(0, 100000)
    let my_filter = (ord) => ord.total_cents >= boundary_price
    let their_filter = 'filter[total_cents:gteq]'

    if (this.rng.boolean()) {
      my_filter = (ord) => ord.total_cents <= boundary_price
      their_filter = 'filter[total_cents:lteq]'
    }

    let page = await this.visit('/orders/',
                                `?${their_filter}=${boundary_price}`)
    let header = await page.innerText('h1')
    assert.equal(header, 'Orders')

    let orders_coll = await this.collection('orders')
    let orders_stream = orders_coll.createValueStream({limit: 25})

    await new Promise((resolve, _rej) => orders_stream.on('readable', resolve))

    while (orders_stream.readable) {
      let data = orders_stream.read()
      if (null == data) continue

      var ord = data
      if ('string' == typeof data) {
        ord = JSON.parse(data)
      } else if (Buffer.isBuffer(data)) {
        ord = JSON.parse(data.toString())
      }

      let check_pxs = []

      if (! my_filter(ord)) {
        await page.$(`#order-${ord.id}`).
          then(async (el) => {
            if (el) {
              assert.fail(`found order ${ord.id} when it was filtered out`)
            }
          }).
          catch(err => assert.fail(`poller error ${err}`))
            }
      else {
        await page.$(`#order-${ord.id}`).
          then(async (el) => {
            if (null == el) {
              assert.fail(`couldn't find order id ${ord.id}`)
            }

          await el.scrollIntoViewIfNeeded()

          check_pxs.push(
            new Promise((resolve, _reject) => {
              el.$('.id').
                then(id => id.innerText()).
                then(it => {
                  assert.equal(it, ord.id)
                  resolve()
                }).
                catch(err => assert.fail(`poller error ${err}`))
                  }),
            new Promise((resolve, _reject) => {
              el.$('.postcode').
                then(pc => pc.innerText()).
                then(it => {
                  assert.equal(it, ord.postcode)
                  resolve()
                })
            }),
            new Promise((resolve, _reject) => {
              el.$('.total').
                then(tot => tot.innerText()).
                then(it => {
                  assert.equal(it,
                               (ord.total_cents / 100).toString())
                  resolve()
                })
            }))
        }).
        catch(err => assert.fail(`poller error ${err}`))


          await Promise.all(check_pxs)
      }
    }

    orders_stream.destroy()
  }

  async get_noexist_order() {
    let page = await this.visit('/orders/00000000-0000-0000-0000-000000000000')
    let message = await page.innerText('pre')
    assert.equal(message, "Not Found")
  }

  async get_unauthenticated_order() {
    let keys = await this.list_ids('orders')
    let key = keys[this.rng.int(0, keys.length - 1)]
    let order = await this.read('orders', key)

    let page = await this.visit(`/orders/${key}`)

    let header = await page.innerText('h1')
    assert.equal(header, `Order ${key}`)

    let content = await page.innerText('body')

    assert.doesNotMatch(content, new RegExp(order.pin, 'i'))
    assert.doesNotMatch(content, new RegExp(order.pan, 'i'))

    assert.match(content, new RegExp(order.total_cents, 'i'))
    assert.match(content, new RegExp(order.postcode, 'i'))
  }

  async get_wrong_auth_order() {
    let keys = await this.list_ids('orders')
    let key = keys[this.rng.int(0, keys.length - 1)]
    let order = await this.read('orders', key)

    let wrong_pin = (parseInt(order.pin, 16) + 1).toString(16)

    let page = await this.visit(`/orders/${key}`, `pin=${wrong_pin}`)

    let header = await page.innerText('h1')
    assert.equal(header, `Order ${key}`)

    let content = await page.innerText('body')

    assert.doesNotMatch(content, new RegExp(order.pin, 'i'))
    assert.doesNotMatch(content, new RegExp(order.pan, 'i'))

    assert.match(content, new RegExp(order.total_cents, 'i'))
    assert.match(content, new RegExp(order.postcode, 'i'))
  }

  async get_authenticated_order() {
    let keys = await this.list_ids('orders')
    let key = keys[this.rng.int(0, keys.length - 1)]
    let order = await this.read('orders', key)

    // console.log(`order ${key} with pin ${order.pin}`)

    let page = await this.visit(`/orders/${key}`, `pin=${order.pin}`)

    // console.log("did get page")

    let header = await page.innerText('h1')
    assert.equal(header, `Order ${key}`)

    // console.log("validated h1")

    let content = await page.innerText('body')

    // console.log(`got ${content.length} bytes of body`)

    assert.match(content, new RegExp(order.pin, 'i'))
    assert.match(content, new RegExp(order.pan, 'i'))

    assert.match(content, new RegExp(order.total_cents, 'i'))
    assert.match(content, new RegExp(order.postcode, 'i'))
  }

  async login_unprivileged() {
    let usernames = await this.list_ids('users')
    console.log(`from ${usernames.length} users...`)
    var user
    do {
      let username_idx = this.rng.int(0, usernames.length - 1)
      let username = usernames[username_idx]
      console.log(`picking username idx ${username_idx} ${username}`)
      user = await this.read('users', username)
    } while (user.is_admin)

    let password = (await this.read_raw('users_cheatsheet', user.name)).toString()

    let page = await this.visit(`/login`)

    await page.fill('#login_username', user.name)
    await page.fill('#login_password', password)

    console.log(`logging in ${user.name} / ${password} ...`)

    await Promise.all([page.waitForNavigation({url: /login\/did_login/}),
                       page.click('#login_form button')])

    let page_title = await page.innerText('h1')
    assert.equal(page_title, 'beacon logged in')

    await Promise.all([page.waitForNavigation({url: /login/}),
                       page.click('button')])
  }

  async login_admin() {
    let usernames = await this.list_ids('users')
    var admin
    do {
      let username = usernames[this.rng.int(0, usernames.length - 1)]
      admin = await this.read('users', username)
    } while (! admin.is_admin)

    let password = (await this.read_raw('users_cheatsheet', admin.name)).toString()

    let page = await this.visit(`/login`)

    await page.fill('#login_username', admin.name)
    await page.fill('#login_password', password)

    await Promise.all([page.waitForNavigation({url: /login\/did_login/}),
                       page.click('#login_form button')])

    let page_title = await page.innerText('h1')
    assert.equal(page_title, 'beacon admin')

    let maybe_expected_token = process.env['TOKEN'] || false
    if (maybe_expected_token) {
      let actual_token = await page.innerText('#token')
      assert.equal(actual_token, maybe_expected_token)
    }

    await Promise.all([page.waitForNavigation({url: /login/}),
                       page.click('button')])
  }

  async create_user() {
    let username_word = WORDS[this.rng.int(0, WORDS.length - 1)]
    let username_number = this.rng.int(0, 1000000000)
    let username = `${username_word}_${username_number}`

    let password = `${username} ${username_number}`

    let page = await this.visit('/login')

    console.log('filling out')

    await page.fill('#signup_username', username)
    await page.fill('#signup_password', password)
    await page.fill('#signup_password_confirmation', password)

    console.log(`filled out ${username} and ${password}`)

    await Promise.all([page.waitForNavigation({url: /login\/did_login/}),
                       page.click('#signup_form button')])

    let page_title = await page.innerText('h1')
    assert.equal(page_title, 'beacon logged in')

    await Promise.all([page.waitForNavigation({url: /login/}),
                       page.click('button')])
  }

  async visit(path, search = {}) {
    let url = this.base
    url.pathname = path
    url.search = search
    await this.page.goto(url.toString())
    return this.page
  }

  async list_ids(collection_name) {
    let keys = []
    let coll = await this.collection(collection_name)
    let stream = coll.createKeyStream()

    return new Promise((resolve, reject) => {
      let did_already_close = false
      let maybe_resolve = () => {
        if (did_already_close) return

        did_already_close = true
        stream.destroy()
        resolve(keys)
      }

      stream.on('error', err => reject(err))
      stream.on('data', data => keys.push(data.toString()))
      stream.on('close', maybe_resolve)
      stream.on('end', maybe_resolve)

    })
  }

  async read(collection_name, id) {
    let coll = this.collection(collection_name)
    let obj = await coll.get(id)

    var content = obj

    if ('string' == typeof obj) {
      content = JSON.parse(obj)
    } else if (Buffer.isBuffer(obj)) {
      content = JSON.parse(obj.toString())
    }
    return new Promise(resolve => resolve(content))
  }

  async read_raw(collection_name, id) {
    let coll = this.collection(collection_name)
    let obj = await coll.get(id)

    return obj
  }

  collection(name) {
    if (this._collection.has(name)) {
      return this._collection.get(name)
    }

    let db = levelup(leveldown(`/data/${name}`))

    this._collection.set(name, db)

    return db
  }
}



(async () => {
  let poller = new Poller()
  try {
    await poller.run()
  } catch (e) {
    console.log(e)
    process.exit(1)
  }
  poller.print_report()
/*
  const browser = await playwright['webkit'].launch();
  const context = await browser.newContext();
  const page = await context.newPage();
  await page.goto(base.toString());
  await page.screenshot({ path: `example.png` });
  await browser.close();
*/
})();
