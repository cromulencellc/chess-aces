#!/usr/bin/env ts-node-script

import fs from 'fs'
import util from 'util'

import connect_flash from 'connect-flash'
import cookie_session from 'cookie-session'
import csurf from 'csurf'
import express from 'express'
import sqlite3 from 'better-sqlite3'

import * as Authentication from './authentication'
import { Broker } from './broker'
import Logger from './logger'

import Dashboard from './routes/dashboard'
import Subscriptions from './routes/subscriptions'
import Users from './routes/users'

declare global {
  namespace NodeJS {
    interface Global {
      iroquois: {
        db: sqlite3.Database,
        broker: Broker
      }
    }
  }
}

async function main(_argv: string[]) {
  if (! process.env.CHESS) {
    Logger.FATAL("This application is for research purposes only")
    process.exit(1)
  }

  let database_path = process.env.DB_PATH || '/data/iroquois.sqlite3'
  let database_options = {fileMustExist: true,
                          verbose: Logger.DEBUG }
  let db = sqlite3(database_path, database_options)

  if (! process.env.DONT_DELETE_CHEATSHEET) {
    db.exec('DROP TABLE IF EXISTS users_cheatsheet;')
  }

  db.pragma('foreign_keys = ON')

  let broker = new Broker(process.env.MQTT_URL || 'tcp://mosquitto:1883')

  let csrf_middleware = csurf()

  let app = express()

  app.set('view engine', 'pug')
  app.set('views',
          process.env.VIEW_DIR || '/static/view')

  app.use(express.urlencoded({extended: true}))
  app.use((req, resp, next) => {
    Logger.INFO(`${req.method} ${req.path} ${util.inspect(req.params)}`)
    next()
  })
  app.use(cookie_session({secret: process.env.SESSION_SECRET ||
    'AngelfishLanguageCeremonyRemissionMammogramTipping'}))
  app.use(csrf_middleware)
  app.use(connect_flash())
  app.use(Authentication.passport.initialize())
  app.use(Authentication.passport.session())


  global['iroquois'] = {
    broker: broker,
    db: db
  }

  app.use('/static',
          express.static(process.env.STATIC_DIR || '/static/static'))

  app.use('/dashboard', Dashboard)
  app.use('/subscriptions', Subscriptions)
  app.use('/users', Users)

  app.post('/session', Authentication.process_login())
  app.post('/logout', Authentication.log_out())

  app.get('/',
          Authentication.prohibit_logged_in(),
          (req, resp) => {
    resp.render('index',
                {csrf: req.csrfToken(),
                 flash: req.flash()})
  })

  let port = parseInt(process.env.PORT || '3035')

  app.listen(port, () => {
    Logger.INFO(`iroquois listening on port ${port}`)
  })
}

export default main(process.argv).
  catch(err => Logger.FATAL(err))
