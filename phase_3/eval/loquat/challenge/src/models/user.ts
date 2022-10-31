import crypto from 'crypto'
import util from 'util'

import scrypt from '../scrypt'
import sql from '../sql'
import logger from '../logger'

import {assertIsString, assertIsBoolean, assertIsBuffer, isString, isBuffer} 
  from '@tool-belt/type-predicates'
import { AsyncAuthorizerCallback } from 'express-basic-auth'
import { threadId } from 'worker_threads'
import Satellite from './satellite'

export type maybeUser = User | undefined

function digest_password(attrs: any): string {
  if (isString(attrs.password_digest)) {
    return attrs.password_digest
  }
  if (isBuffer(attrs.password_digest)) {
    return attrs.password_digest.toString()
  }

  if (isString(attrs.password)) {
    return scrypt.create(attrs.password)
  }

  throw `couldn't digest password from ${util.inspect(attrs)}`
}

export default class User {
  id?: string
  
  _name?: string
  _password_digest?: string
  _is_admin?: boolean

  constructor(attrs: any) {
    try {
      this.name = attrs.name
      this.password_digest = digest_password(attrs)
      this.is_admin = !!attrs.is_admin
    } catch (e) {
      logger.ERROR(util.inspect(attrs))
      throw e
    }

    if (attrs.id) {
      this.id = attrs.id
    } else {
      this.id = undefined
    }
  }

  async save() {
    if (undefined == this.id) return await this.insert()

    return await this.update()
  }

  async insert() {
    let [inserted] = await sql`
      INSERT INTO users 
      ${sql(this.attrs())}
      RETURNING id
    `

    let got_id = inserted.id

    if (undefined == got_id) {
      throw `didn't get id back from user insertion: ${inserted}`
    }

    this.id = got_id
    return this.id
  }

  async update() {
    assertIsString(this.id)

    await sql`UPDATE USERS 
      SET ${sql(this.attrs())} 
      WHERE id = ${this.id}`

    return this.id
  }

  set name(n) {
    assertIsString(n)
    this._name = n
  }
  get name() { 
    assertIsString(this._name)
    return this._name 
  }

  set password_digest(d: string) {
    assertIsString(d)
    this._password_digest = d
  }
  get password_digest(): string {
    assertIsString(this._password_digest)
    return this._password_digest 
  }

  set is_admin(a) {
    assertIsBoolean(a)
    this._is_admin = a
  }
  get is_admin() { 
    assertIsBoolean(this._is_admin)  
    return this._is_admin 
  }

  attrs() {
    return {
      name: this.name,
      password_digest: this.password_digest,
      is_admin: this.is_admin
    }
  }

  async satellites() {
    assertIsString(this.id)

    let ids = await sql`
      SELECT satellite_id AS id
      FROM satellites_users
      WHERE user_id = ${this.id}
    `

    let sxs = ids.map(row => {
      logger.DEBUG(row)
      let s = new Satellite(row)
      s.save()
      return s
    })

    return sxs
  }

  async add_satellite(satellite_id: string) {
    let attrs = {
      user_id: this.id,
      satellite_id: satellite_id
    }

    let got_sat = await Satellite.findOrCreateById(satellite_id)
    if (undefined == got_sat) {
      throw `couldn't add satellite ${satellite_id} to user ${this.name}`
    }

    await sql`
      INSERT INTO satellites_users
      ${sql(attrs)}
      ON CONFLICT DO NOTHING
    `
  }

  async remove_satellite(satellite_id: string) {
    assertIsString(this.id)
    await sql`
      DELETE FROM satellites_users
      WHERE user_id = ${this.id} AND
        satellite_id = ${satellite_id}
    `
  }

  static async clobber_default_admin() {
    let _got = await sql`update users set is_admin = false`

    return this.make_default_admin()
  }

  static async make_default_admin() {
    let got = await sql`select count(id) from users where is_admin;`
    let user_count = got[0].count
    if (typeof user_count != 'bigint') {
      throw `expected numeric user count, got (${typeof user_count}) ${user_count}`
    }
    logger.DEBUG(`found ${user_count} admins`)
    if (0n != user_count) return
      
    let password = crypto.randomUUID()

    let user_attrs = {
      name: `admin ${crypto.randomInt(100000)}`,
      password_digest: scrypt.create(password),
      is_admin: true
    }

    let did_create = await sql`
      insert into users
        ${sql(user_attrs)}
    `
    if (1 != did_create.count) {
      throw `wanted one row back when creating user, got ${did_create}`
    }

    let admin_env = `ADMIN_USERNAME='${user_attrs.name}' ADMIN_PASSWORD='${password}'`
    logger.WARN(`created new admin user ${admin_env}`)
  }

  static async findByUsername(candidate_username?: string): Promise<maybeUser> {
    if (!isString(candidate_username)) {
      return undefined
    }

    let got = await sql`
      select * from users where name = ${candidate_username}
    `

    if (0 == got.count) return undefined

    if (1 != got.count) {
      throw `wanted to find one user, got ${got}`
    }
    return new User(got[0])
  }

  static async findByUsernameAndPassword(candidate_username?: string,
    candidate_password?: string): Promise<maybeUser> {
    if (!isString(candidate_username) || !isString(candidate_password)) {
      return undefined
    }

    let u = await this.findByUsername(candidate_username)
    if (undefined == u) return undefined

    if (!scrypt.authenticate(u.password_digest, candidate_password)) {
      return undefined
    }

    return u
  }

  static authorizeUser(username: string, password: string,
    callback: AsyncAuthorizerCallback) {
    User.findByUsername(username).
      then(u => {
        if (undefined == u) return callback(null, false)
        if (! scrypt.authenticate(u.password_digest, password)) {
          return callback(null, false)
        }

        return callback(null, true)
      }).
      catch(e => {
        return callback(e, false)
      })
  }

  static authorizeAdmin(username: string, password: string, 
    callback: AsyncAuthorizerCallback) {
    User.findByUsername(username).
      then(u => {
        if (undefined == u) return callback(null, false)
        if (! scrypt.authenticate(u.password_digest, password)) {
          return callback(null, false)
        }
        
        if (u.is_admin) return callback(null, true)

        return callback(null, false)
      }).
      catch(e => {
        return callback(e, false)
      })
    
  }
}