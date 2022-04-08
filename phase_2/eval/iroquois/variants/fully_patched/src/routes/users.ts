import express from 'express'

import { prohibit_logged_in } from '../authentication'
import Logger from '../logger'
import { User } from '../models/user'

let ROUTE_PREFIX = '/users'

let users = express.Router()

users.post('/',
           prohibit_logged_in(),
           (req, resp) => {
  if (req.body.user.password_confirmation !=
    req.body.user.password) {
    req.flash('error', "password and password confirmation didn't match")
    resp.redirect('/')
    return
  }

  try {
    let new_user = User.create({
      name: req.body.user.name,
      password: req.body.user.password
    })
  } catch (err) {
    req.flash('error', `couldn't save user: ${err}`)
    resp.redirect('/')
    return
  }

  req.flash('info', "created user, please log in")
  resp.redirect('/')
  return
})

export default users
