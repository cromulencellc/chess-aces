import util from 'util'

import express, { Router } from 'express'
import basic_auth from 'basic-auth'

import logger from '../logger'
import User from '../models/user'
import expressWs from 'express-ws'
import SubscriptionWatcher from '../subscription_watcher'
import Search from 'models/search'

let subscription_watcher = new SubscriptionWatcher()

let rf = async (r: expressWs.Router): Promise<expressWs.Router> => {
  await subscription_watcher.start_watching()

  r.ws('/ping', async (ws, _req) => {
    logger.DEBUG('got ws ping request')
    ws.send('')
    ws.on('message', (mesg) => {
      logger.DEBUG(mesg.toString())
      if ('ping' != mesg.toString().toLowerCase()) return
      logger.DEBUG('pong')
      ws.send('pong')
    })
    logger.DEBUG('finished instrumenting ws ping request')
  })
  
  r.ws('/firehose', (ws, req) => {
    let auth_result = basic_auth(req)
    if (undefined == auth_result) { return ws.close() }
    User.findByUsernameAndPassword(auth_result.name, auth_result.pass).
    then(u => {
      if (undefined == u) { return ws.close() }

      logger.DEBUG(u)
      subscription_watcher.subscribe(u, ws)
    })
  })

  r.ws('/satellite/:id', (ws, req) => {
    let auth_result = basic_auth(req)
    if (undefined == auth_result) { return ws.close() }
    User.findByUsernameAndPassword(auth_result.name, auth_result.pass).
    then(u => {
      if (undefined == u) { return ws.close() }

      logger.DEBUG(u)
      subscription_watcher.subscribe(u, ws, req.params.id)
    }).
    catch(e => {
      logger.ERROR(`got ${e} on ws:///satellite/${req.params.id}`)
      return ws.terminate()
    })
  })

  return r
}

export default rf