import express from 'express'

import { require_logged_in } from '../authentication'
import Logger from '../logger'

import { User } from '../models/user'
import { Subscription } from '../models/subscription'
import { Topic } from '../models/topic'

let router = express.Router()
router.use(require_logged_in())

router.post('', (req, resp) => {
  let current_user = req.user

  if ((undefined == current_user) ||
    !(current_user instanceof User)) {
    resp.sendStatus(401)
    return
  }

  let topic_name = req.body.subscription.topic.name
  if ('string' != typeof topic_name) {
    resp.sendStatus(400)
    return
  }

  let maybe_topic = Topic.findByName(topic_name)
  if (undefined == maybe_topic) {
    let new_topic = Topic.create({name: topic_name})
    global.iroquois.broker.subscribe(topic_name)
    let _subscription = Subscription.create({user_id: current_user.id,
                                             topic_id: new_topic.id})

    resp.redirect('/dashboard')
    return
  }

  let _subscription = Subscription.create({user_id: current_user.id,
                                           topic_id: maybe_topic.id})
  resp.redirect('/dashboard')
})

router.get(/^\/(\d+)$/, (req, resp) => {
  let current_user = req.user

  if ((undefined == current_user) ||
      !(current_user instanceof User)) {
    resp.sendStatus(401)
    return
  }

  let sub_id = Number(req.params[0])

  let sub = Subscription.find(sub_id)

  if (undefined == sub) {
    resp.sendStatus(404)
    return
  }

  resp.render('subscriptions/show',
              {csrf: req.csrfToken(),
               flash: req.flash(),
               user: current_user,
               subscription: sub})
})

router.post(/^\/(\d+)\/message$/, (req, resp) => {
  let current_user = req.user
  if ((undefined == current_user) ||
    !(current_user instanceof User)) {
    resp.sendStatus(401)
    return
  }

  let sub_id = Number(req.params[0])
  let sub = current_user.findSubscription(sub_id)

  if (undefined == sub) {
    resp.sendStatus(404)
    return
  }

  global.iroquois.broker.publish(sub.topic, req.body.message.content).
    then(() => resp.redirect(`/subscriptions/${sub!.id}`)).
    catch(err => {
      Logger.ERROR(err)
      resp.sendStatus(500)
    })
})

router.post(/^\/(\d+)\/delete$/, (req, resp) => {
  let current_user = req.user
  if ((undefined == current_user) ||
    !(current_user instanceof User)) {
    resp.sendStatus(401)
    return
  }

  let sub_id = Number(req.params[0])
  let sub = current_user.findSubscription(sub_id)

  if (undefined == sub) {
    resp.sendStatus(404)
    return
  }

  sub.destroy()

  resp.redirect('/dashboard')
})

export default router
