#!/usr/bin/env ts-node-script

import fs from 'fs'
import util from 'util'

import connect_flash from 'connect-flash'
import cookie_session from 'cookie-session'
import csurf from 'csurf'
import express from 'express'
import multer from 'multer'
import { v4 as uuidv4 } from 'uuid'

import Logger from './logger'
import { Watcher } from './watcher'
import path from 'path'

async function main(_argv: string[]) {
  if (! process.env.CHESS) {
    Logger.FATAL("This application is for research purposes only")
    process.exit(1)
  }

  let watcher = new Watcher()
  watcher.start()

  let upload = multer({storage: multer.diskStorage({
    destination: '/data/incoming',
    filename: (req, file, cb) => {
      cb(null, uuidv4() + '.map')
    }
  }),
  limits: {
    fileSize: 5 * 1024 * 1024
  }})

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
    'OvertoneLatherSurfaceSpreeSetbackBoil'}))
  app.use(connect_flash())


  app.use('/static',
          express.static(process.env.STATIC_DIR || '/static/static'))

  app.use('/favicon.ico', express.static('/static/static/favicon.ico'))

  app.get('/',
    csrf_middleware,  
            (req, resp) => {

      fs.promises.readdir('/data/done').
      then(files => {
        let already_converted = files.map(fn => path.basename(fn, '.svg'))

        resp.render('index',
                {csrf: req.csrfToken(),
                 flash: req.flash(),
                already_converted: already_converted})
      })
  })

  app.post('/upload',
    upload.single('map'), 
    csrf_middleware,
    (req, resp) => {
      let uploaded_path = path.parse(req.file.path)
      fs.promises.rename(path.format(uploaded_path), path.format({
        dir: '/data/enqueued',
        name: uploaded_path.name
      })).
      then(() => {
        resp.redirect(303, `/maps/${uploaded_path.name}.html`)
      })
  })

app.get(/^\/maps\/(.+)\.svg$/,
(req, resp) => {
  Logger.INFO(path.join('/data/done', req.params[0]))
  fs.promises.readFile(path.join('/data/done', req.params[0])).
    then((svg_data) => {
        resp.
          contentType('image/svg+xml').
          send(svg_data)
      }).
    catch((err) => {
      fs.promises.access(path.join('/data/processing', req.params[0])).
      then(() => {
        resp.
          status(425).
          contentType('text/html').
          header('Refresh', '5').
          send("still processing, will refresh automatically&hellip;")
      }).
      catch(() => {
        fs.promises.access(path.join('/data/enqueued', req.params[0])).
        then(() => {
          resp.
            status(425).
            contentType('text/html').
            header('Refresh', '5').
            send("still enqueued, will refresh automatically&hellip;")
        }).
        catch((e) => {
          Logger.ERROR(e)
          resp.
            status(404).
            contentType('text/html').
            send("missing, or race condition'd")
        })
      })
    })

  })

  app.get(/^\/maps\/(.+)\.html$/,
  (req, resp) => {
    Logger.INFO(path.join('/data/done', req.params[0]))
    fs.promises.readFile(path.join('/data/done', req.params[0])).
    then((svg_data) => {
        resp.
          render('show', {
            flash: req.flash(),
            id: req.params[0],
            svg_data: svg_data})
      }).
    catch((err) => {
      Logger.ERROR(err)
      fs.promises.access(path.join('/data/processing', req.params[0])).
      then(() => {
        resp.
          status(425).
          header('Refresh', '5').
          send("still processing, will refresh automatically&hellip;")
      }).
      catch(() => {
        fs.promises.access(path.join('/data/enqueued', req.params[0])).
        then(() => {
          resp.
            status(425).
            header('Refresh', '5').
            send("still enqueued, will refresh automatically&hellip;")
        }).
        catch((e) => {
          Logger.ERROR(e)
          resp.status(404).send("missing, or race condition'd")
        })
      })
    })

  })
// endif

  let port = parseInt(process.env.PORT || '3038')

  app.listen(port, () => {
    Logger.INFO(`lamartine listening on port ${port}`)
  })
}

export default main(process.argv).
  catch(err => Logger.FATAL(err))
