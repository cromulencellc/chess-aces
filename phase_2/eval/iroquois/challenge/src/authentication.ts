import { ServerResponse} from 'http'

import express from 'express'
import passport from 'passport'
import passport_local from 'passport-local'

import { User } from './models/user'

export { passport }

const LocalStrategy = passport_local.Strategy

passport.serializeUser<User>((user, done) => {
  done(undefined, user.id)
})

passport.deserializeUser((id: number, done) => {
  let maybe_user = User.find(id)
  if (undefined == maybe_user) {
    return done('not found', undefined)
  }

  return done(null, maybe_user)
})

passport.use(new LocalStrategy(
  {passReqToCallback: true},
  (req, username, password, done) => {
    let user = User.findByName(username)
    if (undefined == user) {
      req.flash('error', 'Failed to log in')
      return done(null, null, {message: 'Failed to log in'})
    }

    let did_authenticate = user.authenticate(password)

    if (! did_authenticate) {
      req.flash('error', 'Failed to log in')
      return done(null, null, {message: 'Failed to log in'})
    }

    req.flash('info', `Logged in as ${user.name}`)
    return done(null, user)
    }))

export function process_login() {
  return passport.authenticate('local',
                               { successRedirect: '/dashboard',
                                 failureRedirect: '/' })
}

export function log_out() {
  return (req: express.Request, resp: express.Response) => {
    req.session = null
    resp.redirect('/')
    return
  }
}

export function require_logged_in() {
  return (req: express.Request,
          resp: express.Response,
          next: Function) => {
            if (! req.isAuthenticated()) {
              req.flash('error', 'please log in')
              resp.redirect('/')
              return
            }

            next()
            return
          }
}

export function prohibit_logged_in() {
  return (req: express.Request,
          resp: express.Response,
          next: Function) => {
            if (req.isAuthenticated()) {
              req.flash('info', 'already logged in')
              resp.redirect('/dashboard')
              return
            }

            next()
            return
          }
}
