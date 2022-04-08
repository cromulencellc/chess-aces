'use strict'

const Duplex = require('stream').Duplex

const Expression = require('./expression')
const Token = require('./token')

class ExpressionStream extends Duplex {
    constructor(options) {
        super({
            ...{readableObjectMode: true,
                writableObjectMode: true},
            ...options
        })
        this.stack = []
        this.still_receiving = true
        this.parsed_exprs = []
    }

    toString() {
        return `ExpressionStream(${this.stack.length} frames)`
    }

    _write(chunk, _encoding, callback) {
        try {
            this.consume(chunk)
        } catch (err) {
            return callback(err)
        }

        return callback(null)
    }

    _destroy(err, callback) {
        this.still_receiving = false
        callback(err)
    }

    _final(callback) {
        this.still_receiving = false
        callback()
    }

    _read() {
        while (true) {
            if (0 == this.parsed_exprs.length) {
                if (! this.still_receiving) {
                    this.push(null)
                    return
                }

                setImmediate(this._read.bind(this))
                return
            }

            let cur_expr = this.parsed_exprs.shift()

            let got = this.push(cur_expr)
            if (false == got) return
        }
    }

    consume(chunk) {
        if (chunk instanceof Token.OpenList) {
            return this.stack.push(new Expression())
        }
        if (chunk instanceof Token.CloseList) return this.consumeClose()

        let top_frame = this.stack[this.stack.length - 1]
        return top_frame.push(chunk)
    }

    consumeClose() {
        let finished_expr = this.stack.pop()

        if (0 == this.stack.length) {
            return this.parsed_exprs.push(finished_expr)
        }

        let new_top_frame = this.stack[this.stack.length - 1]
        return new_top_frame.push(finished_expr)
    }
}

module.exports = ExpressionStream
