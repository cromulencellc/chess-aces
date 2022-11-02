import { assertIsString, isString } from '@tool-belt/type-predicates'
import logger from '../logger'
import sql from '../sql'

export type maybeSatellite = Satellite | undefined

export default class Satellite {
  id: string

  constructor(attrs: any) {
    let maybe_id = attrs.id
    assertIsString(maybe_id)
    this.id = maybe_id
  }

  async save() {
    let got = await sql`
      INSERT INTO satellites
      ${sql(this.attrs())}
      ON CONFLICT DO NOTHING
      RETURNING id
    `
    logger.DEBUG(got)
  }

  attrs() {
    return {id: this.id}
  }

  static async findOrCreateById(candidate_id?: string): Promise<maybeSatellite> {
    if (undefined == candidate_id) return undefined

    let s = new Satellite({id: candidate_id})
    await s.save()
    return s
  }
}