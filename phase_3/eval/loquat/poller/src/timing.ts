import process from 'process'

const NS_PER_SEC = 1e9

export default class Timing {
  count: number
  total: number
  min: number
  max: number

  constructor() {
    this.count = 0
    this.total = 0.0
    this.min = Number.MAX_VALUE
    this.max = Number.MIN_VALUE
  }

  async time(fn: Function) {
    let before_time = process.hrtime()
    await fn()
    let elapsed = process.hrtime(before_time)
    let elapsed_s = elapsed[0] + (elapsed[1] / NS_PER_SEC)
    this.add(elapsed_s)
  }

  add(elapsed: number) {
    this.count ++
    this.total += elapsed
    if (this.min > elapsed) this.min = elapsed
    if (this.max < elapsed) this.max = elapsed
  }

  avg() {
    if (0 == this.count) return -1
    return this.total / this.count
  }
}