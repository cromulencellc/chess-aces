import util from 'util'

import express, { RequestHandler, response } from 'express'
import basicAuth from 'express-basic-auth'
import csurf from 'csurf'

import logger from '../logger'
import User from '../models/user'
import Search from '../models/search'
import { assertIsDefined } from '@tool-belt/type-predicates'

let r = express.Router()
r.use(basicAuth({
  authorizer: User.authorizeUser,
  authorizeAsync: true
}))
let csrf_middleware = csurf()

r.get('/filter/:filter_id', async (req: basicAuth.IBasicAuthedRequest, resp) => {
  let f = await Search.findById(req.params['filter_id'])
  //#ifdef PATCH_WHOS_SEARCHING
  let u = await User.findByUsername(req.auth.user)
  
  assertIsDefined(u)
  assertIsDefined(u.id)

  let got = await f.perform_search(u.id)

  //# else
  //let got = f.perform_search()
  //#endif

  logger.DEBUG(JSON.stringify(got))

  resp.send(JSON.stringify(got))
})

export default r