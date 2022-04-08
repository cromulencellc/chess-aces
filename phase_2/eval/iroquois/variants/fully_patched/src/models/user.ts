import crypto from 'crypto'

import { Subscription } from './subscription'

const SCRYPT_KEY_LEN = 32
const SCRYPT_COST = 0x4000
const SCRYPT_BLOCK_SIZE = 8
const SCRYPT_PARALLELISM = 3
const SCRYPT_SALT_SIZE = 32

declare global {
  namespace Express {
    interface User {
      id: any
    }
  }
}

export class User implements Express.User {
  id: number
  name: string
  password_digest: string
  created_at: Date
  _subscriptions: Array<Subscription> | undefined

  constructor(attrs: any) {
    this.id = attrs.id
    this.name = attrs.name
    this.password_digest = attrs.password_digest
    this.created_at = new Date(attrs.created_at)
    if (attrs.password) {
      this.setPassword(attrs.password)
    }
  }

  authenticate(candidate: string | Buffer): this | boolean {
    let n_s, r_s, p_s, salt, hash
    [n_s, r_s, p_s, salt, hash] = this.password_digest.split('$')
    let n = parseInt(n_s, 16)
    let r = parseInt(r_s, 16)
    let p = parseInt(p_s, 16)
    let salt_buf = Buffer.from(salt, 'hex')
    let hash_buf = Buffer.from(hash, 'hex')

    let candidate_hash = crypto.scryptSync(candidate,
                                           salt_buf, hash_buf.length,
                                           {
                                             cost: n,
                                             blockSize: r,
                                             parallelization: p})

    let eq = crypto.timingSafeEqual(hash_buf, candidate_hash)

    if (eq) return this

    return false
  }

  setPassword(new_password: string | Buffer): void {
    let salt_buf = crypto.randomBytes(SCRYPT_SALT_SIZE)
    let hash = crypto.scryptSync(new_password,
                                 salt_buf,
                                 SCRYPT_KEY_LEN,
                                 {
                                   cost: SCRYPT_COST,
                                   blockSize: SCRYPT_BLOCK_SIZE,
                                   parallelization: SCRYPT_PARALLELISM})
    let pack = [
      SCRYPT_COST.toString(16),
      SCRYPT_BLOCK_SIZE.toString(16),
      SCRYPT_PARALLELISM.toString(16),
      salt_buf.toString('hex'),
      hash.toString('hex')
    ].join('$')

    this.password_digest = pack
  }

  get subscriptions(): Array<Subscription> {
    if (this._subscriptions) return this.subscriptions

    return this._subscriptions = Subscription.findByUserId(this.id)
  }

  findSubscription(sub_id: number): Subscription | undefined {
    return Subscription.findByIdAndUserId(sub_id, this.id)
  }

  save(): this {
    if (this.id) return this.update()
    return this.create()
  }
  update(): this {
    let stmt = global.iroquois.db.prepare("UPDATE users SET \
name = :name, password_digest = :password_digest \
WHERE id = :id")
    stmt.run({name: this.name,
              password_digest: this.password_digest,
              id: this.id})

    return this
  }
  create(): this {
    let stmt = global.iroquois.db.prepare(
      "INSERT INTO users (name, password_digest) \
VALUES (:name, :password_digest)")
    let got = stmt.run({name: this.name,
                        password_digest: this.password_digest})
    this.id = Number(got.lastInsertRowid)

    return this
  }

  static find(id: number): User | undefined {
    const stmt = global.iroquois.db.prepare('SELECT * FROM users WHERE id = ?')
    const found = stmt.get(id)

    if (undefined == found) return undefined
    return new User(found)
  }
  static findByName(name: string): User | undefined {
    const stmt = global.iroquois.db.prepare(
      'SELECT * FROM users WHERE name = ?')
    const found = stmt.get(name)

    if (undefined == found) return undefined
    return new User(found)
  }
  static create(attrs: any): User {
    let user = new User(attrs)
    user.save()

    return user
  }
}
