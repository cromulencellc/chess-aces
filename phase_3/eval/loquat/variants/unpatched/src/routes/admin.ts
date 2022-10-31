import express from 'express'
import basicAuth from 'express-basic-auth'
import csurf from 'csurf'
import { readFile } from 'fs/promises'

import User from '../models/user'
import { isString } from '@tool-belt/type-predicates'
import logger from '../logger'

let r = express.Router()
r.use(basicAuth({
  authorizer: User.authorizeAdmin,
  authorizeAsync: true
}))

r.get('/', (_req, resp) => {
  readFile('/token').
    then( tokenContent => resp.status(200).send(tokenContent)).
  catch(e => {
    resp.status(500).send(
      `error reading token: ${e}`
    )
  })
})

r.get('/user/:user_name', async (req, resp) => {
  let u = await User.findByUsername(req.params.user_name)
  if (undefined == u) {
    resp.sendStatus(404)
    return
  }

  let sats = await u.satellites()

  let ret: object = {
    name: u.name,
    is_admin: u.is_admin,
    satellite_ids: sats.map(s => s.id)
  }

  resp.send(JSON.stringify(ret))
})

r.post('/user/:user_name/satellites', async (req, resp) => {
  let u = await User.findByUsername(req.params.user_name)
  if (undefined == u) {
    logger.ERROR(`couldn't find user ${req.params.user_name}`)
    resp.sendStatus(404)
    return
  }

  let s_id = req.body.satellite_id
  if (!isString(s_id)) {
    logger.ERROR(`couldn't find satellite id ${req.body.satellite_id}`)
    resp.sendStatus(400)
    return
  }

  let redirect_url = `/user/${u.name}/satellites/${s_id}`

  u.add_satellite(s_id).
    then(() => resp.sendStatus(204)).
    catch(e => {
      logger.ERROR(e)
      resp.sendStatus(500)
    })
})

r.delete('/user/:user_name/satellites/:satellite_id', async (req, resp) => {
  let u = await User.findByUsername(req.params.user_name)
  if (undefined == u) {
    logger.ERROR(`couldn't find user ${req.params.user_name}`)
    resp.sendStatus(404)
    return
  }

  let s_id = req.params.satellite_id
  if (!isString(s_id)) {
    logger.ERROR(`couldn't find satellite id ${req.params.satellite_id}`)
    resp.sendStatus(400)
    return
  }

  u.remove_satellite(s_id).
    then(() => resp.sendStatus(204)).
    catch(e => {
      logger.ERROR(e)
      resp.sendStatus(500)
    })
})

export default r