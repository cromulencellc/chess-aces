#!/usr/bin/env ts-node-script

import fs from 'fs'
import util from 'util'

import connect_flash from 'connect-flash'
import cookie_session from 'cookie-session'
import express, { RequestHandler } from 'express'
import expressWs from 'express-ws'
import { v4 as uuidv4 } from 'uuid'

import Logger from './logger'
import TlmListener from './tlm_listener'
import { URL } from 'url'

import User from './models/user'
import admin_routes from './routes/admin'
import base_routes from './routes/base'
import user_routes from './routes/user'
import ws_routes from './routes/ws'
import logger from './logger'

async function main(_argv: string[]) {
  if (! process.env.CHESS) {
    Logger.FATAL("This application is for research purposes only")
    process.exit(1)
  }

  if (process.env.CLOBBER_ADMIN) {
    await User.clobber_default_admin()
  } else {
    await User.make_default_admin()
  }

  let { app } = expressWs(express())

  app.set('view engine', 'pug')
  app.set('views',
          process.env.VIEW_DIR || '/static/view')

  app.use(express.urlencoded({extended: true}) as RequestHandler)
  app.use((req, resp, next) => {
    Logger.INFO(`${req.method} ${req.path} ${util.inspect(req.body)}`)
    next()
  })
  app.use(cookie_session({secret: process.env.SESSION_SECRET ||
    'ha9nohrohvahx6Ke2qui'}))
  app.use(connect_flash())


  app.use('/static',
          express.static(process.env.STATIC_DIR || '/static/static'))

  app.use('/favicon.ico', express.static('/static/static/favicon.ico'))

  app.use('/', base_routes)
  app.use('/', user_routes)
  app.use('/admin', admin_routes)
  app.use('/', await ws_routes(express.Router()))

  let port = parseInt(process.env.PORT || '3080')


  let tlm_listener = new TlmListener(
    parseInt(process.env.TLM_PORT || '3081')
  )

  await tlm_listener.listen()

  app.listen(port, () => {
    Logger.INFO(`http listening on port ${port}`)
  })
}

export default main(process.argv).
  catch(err => {
    Logger.FATAL(err)
    process.exit(1)
  })
