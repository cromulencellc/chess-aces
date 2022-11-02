import {randomInt, randomUUID} from 'crypto'

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

  random_navstar_id() {
    if (1 == this.navstar_ids.length) return this.navstar_ids[0]
    let idx = randomInt(0, this.navstar_ids.length - 1)
    return this.navstar_ids[idx]
  }

}