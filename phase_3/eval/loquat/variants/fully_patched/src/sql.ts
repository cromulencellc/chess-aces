import logger from './logger'
import postgres from 'postgres'
import util from 'util'

let sql = 
  postgres((process.env.DATABASE_URL ||
    'postgres://postgres:Cae4noh5eghaT2vie5nu@db/postgres'),
    {
      types: {
        bigint: postgres.BigInt
      },
      debug: (_conn, query, params) => {
        logger.DEBUG(`${query} <= ${util.inspect(params)}`)
      }
    })

export default sql