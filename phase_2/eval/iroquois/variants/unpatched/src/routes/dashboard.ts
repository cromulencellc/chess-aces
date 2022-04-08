import express from 'express'

import { require_logged_in } from '../authentication'
import Logger from '../logger'
import { User } from '../models/user'

let dashboard = express.Router()
dashboard.use(require_logged_in())

dashboard.get('/', (req, resp) => {
  resp.render('dashboard',
              {csrf: req.csrfToken(),
               flash: req.flash(),
               user: req.user})
})

export default dashboard
