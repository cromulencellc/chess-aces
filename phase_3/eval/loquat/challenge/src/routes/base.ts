import util from 'util'

import express, { RequestHandler, response } from 'express'
import csurf from 'csurf'

import logger from '../logger'
import User from '../models/user'
import Search from '../models/search'

let r = express.Router()
let csrf_middleware = csurf()

r.get('/',
csrf_middleware,
        (req, resp) => {

          resp.render('index',
                      {flash: req.flash()})
        })

r.get('/ws.json', (_req, resp) => {
  resp.json({
    'ping': 'ws://_/ping',
    'firehose': 'ws://_/firehose',
    'per-satellite': 'ws://_/satellite/:id'
  })
})

r.post('/user', (req, resp) => {
  //#ifndef PATCH_USER_CREATION_PARAMETERS
  let u_params = req.body
  //#else
  /*
  let u_params = {
    name: req.body.name,
    password: req.body.password
  }
  */
  //#endif
  var u
  try {
    u = new User(u_params)
  } catch (e) {
    logger.ERROR(e)
    logger.ERROR(req.body)

    if (e instanceof TypeError) {
      resp.status(400).send("Type error in user parameters")
      return
    }
    return resp.sendStatus(500)
  }

  u.save().
    then(user_id => {
      resp.redirect(201, `/user/${user_id}`)
    }).
    catch(e => {
      logger.ERROR(e)
      logger.ERROR(req.body)
      resp.sendStatus(400)
    })
})

r.post('/filter', (req, resp) => {
  let params = req.body

  var f
  try {
    f = new Search(params)
  } catch (e) {
    logger.ERROR(e)
    logger.ERROR(req.body)

    if (e instanceof TypeError) {
      resp.status(400).send("Type error in user parameters")
      return
    }

    return resp.sendStatus(500)
  }

  f.save().
    then(filter_id => {
      resp.redirect(201, `/filter/${filter_id}`)
    }).
    catch(e => {
      logger.ERROR(e)
      logger.ERROR(req.body)
      resp.sendStatus(400)
    })

})

export default r