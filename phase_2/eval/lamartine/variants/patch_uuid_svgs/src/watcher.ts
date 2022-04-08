import fs from 'fs'
import path from 'path'
import util from 'util'
import { exit } from 'process'


import logger from './logger'

import { Pwad } from './pwad'
import { SvgWriter } from './svg_writer'

const DEFAULT_INTERVAL = 500 // milliseconds

export class Watcher {
    interval: number
    
    constructor() {
        fs.mkdirSync('/data/incoming', {recursive: true})
        fs.mkdirSync('/data/enqueued', {recursive: true})
        fs.mkdirSync('/data/processing', {recursive: true})
        fs.mkdirSync('/data/finalizing', {recursive: true})
        fs.mkdirSync('/data/done', {recursive: true})

        this.interval = Number(process.env.WATCHER_INTERVAL || DEFAULT_INTERVAL)
    }

    did_start = false
    keep_going = false

    start(): void {
        if (this.did_start) return

        setTimeout(this._running.bind(this), this.interval)
        this.keep_going = true
        this.did_start = true
    }

    _running(): void {
        let entries = fs.readdirSync('/data/enqueued')

        let runner_pxss = entries.map(this._process)

        try {
            Promise.all(runner_pxss).then(this._maybe_requeue.bind(this))
        } catch (e: any) {
            logger.FATAL("unhandled watcher error!")
            logger.FATAL(e)
            logger.FATAL("you probably need to clean out the work dirs")
            exit(-1)
        }
    }

    _maybe_requeue(): void {
        if (! this.keep_going) return

        setTimeout(this._running.bind(this), this.interval)
    }

    _process(file: string): Promise<void> {
        let parsed_path = path.parse(file)
        let queue_path = path.join('/data/enqueued', parsed_path.base)
        let inflight_path = path.join('/data/processing', parsed_path.base)
        let inflight_svg_path = path.join('/data/finalizing', parsed_path.base)
        let done_path = path.join('/data/done', parsed_path.name)

        return fs.promises.rename(queue_path, inflight_path).
            then((): Promise<Pwad> => {
                return new Promise((resolve, _reject) => {
                    return resolve(new Pwad(inflight_path))
            })}).
            then((wad: Pwad): Promise<void> => {
                return new Promise((resolve, _reject) => {
                    let writer = new SvgWriter(inflight_svg_path, wad.map)
                    return resolve(writer.write())
                })
            }).
            then((): Promise<void> => fs.promises.rename(inflight_svg_path,
                done_path)).
            then((): Promise<void> => fs.promises.unlink(inflight_path)).
            catch((reason): Promise<void> => {
               logger.ERROR(`barfed processing ${inflight_path} for ${util.inspect(reason)}`) 
               return fs.promises.copyFile('/static/lamartine-failed.svg', done_path)
            })
    }
}