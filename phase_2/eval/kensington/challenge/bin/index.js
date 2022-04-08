'use strict'

const crypto = require('crypto')
const process = require('process')
const sqlite3 = require('better-sqlite3')

const FtpSrv = require('../src')
const FileSystem = require('../src/fs.js')
const AnonymousFiles = require('../src/afs.js')
const Logger = require("../src/logger.js")

function db() {
    return sqlite3(process.env.DB_PATH || '/data/kensington.sqlite3')
}

async function main(_argv) {
    if (! process.env.CHESS) {
        Logger.FATAL("This application is for research purposes only")
        process.exit(1)
    }

    let port = parseInt(process.env.PORT || '3037')

    let root_dir = process.env.ROOT_DIR || '/data/ftproot'

    db().exec('DROP TABLE IF EXISTS users_cheatsheet;')

    let server = new FtpSrv({
        url: `ftp://0:${port}`,
        whitelist: ('PWD LIST NLST STAT SIZE RNFR MDTM ' +
                    'CWD CDUP STOR APPE RETR DELE RNTO STOU ' +
                    'USER PASS PORT PASV QUIT TYPE NOOP ').split(' ')
    })

    server.on('login', ({connection, username, password}, resolve, reject) => {
        if ('anonymous' == username) {
            return resolve({
                fs: new AnonymousFiles(connection, {root: root_dir,
                    cwd: '/'
                }),
                blacklist: ('RNFR DELE RNTO RETR').split(' ')           
            })
        }

        let stmt = db().prepare('SELECT * FROM users WHERE name = ?')
        let got = stmt.get(username)

        if (undefined == got) return reject('nah')

        let n_s, r_s, p_s, salt, hash
        [n_s, r_s, p_s, salt, hash] = got.password_digest.split('$')
        let n = parseInt(n_s, 16)
        let r = parseInt(r_s, 16)
        let p = parseInt(p_s, 16)
        let salt_buf = Buffer.from(salt, 'hex')
        let hash_buf = Buffer.from(hash, 'hex')
        let candidate_hash = 
            crypto.scryptSync(password,
                salt_buf, hash_buf.length,
                {
                    cost: n,
                    blockSize: r,
                    parallelization: p})
        let eq = crypto.timingSafeEqual(hash_buf, candidate_hash)
              
        if (eq) {
            return resolve({
                fs: new FileSystem(connection, {root: root_dir})
            })
        }

        return reject('nah')
    })

    server.listen()
}

main(process.argv).catch(err => Logger.FATAL(err))
