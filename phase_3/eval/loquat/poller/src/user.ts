import {randomUUID} from 'crypto'
import {Random} from 'random'

export default class User {
  id?: string
  name: string
  password: string
  navstar_ids: string[] = []

  constructor() {
    this.name = randomUUID()
    this.password = randomUUID()
  }

  params() {
    return {
      name: this.name,
      password: this.password
    }
  }

  has_navstars() {
    return this.navstar_ids.length > 0
  }

  random_navstar_id(rng: Random) {
    let idx = rng.int(0, this.navstar_ids.length - 1)
    return this.navstar_ids[idx]
  }

  remove_random_navstar_id(rng: Random) {
    let idx = rng.int(0, this.navstar_ids.length - 1)
    let id = this.navstar_ids[idx]
    this.navstar_ids.splice(idx, 1)
    return id
  }
}