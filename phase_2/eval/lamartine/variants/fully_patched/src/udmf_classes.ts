import util from 'util'

import {Vertex, Sector, Sidedef, Linedef} from './map_classes'
import { DoomMap } from './doom_map'

import { parse, SyntaxError } from './udmf_parser'

import Logger from './logger'
import { exit } from 'process'

export type Value = number | Keyword | string

export class UnknownBlockEntryError extends Error {
    constructor(attribute: string, type: string, block: Block) {
        super(`wanted ${attribute}:${type} in ${util.inspect(block)}`)
    }
}

export class Identifier {
    text: string
    constructor(text: string) {
        this.text = text
    }

    [util.inspect.custom]() {
        return `Identifier(${this.text})`
    }
}

export class QuotedString {
    text: string
    constructor(text: string) {
        this.text = JSON.parse(text)
    }

    [util.inspect.custom]() {
        return `QuotedString(${this.text})`
    }
}

export class Keyword {
    text: string

    constructor(text: string) {
        this.text = text
    }

    [util.inspect.custom]() {
        return `Keyword(${this.text})`
    }
}

export class Block {
    name: Identifier
    content: Assignment[]

    constructor(name: Identifier, content: Assignment[]) {
        this.name = name
        this.content = content
    }

    [util.inspect.custom]() {
        return `Block(${this.name.text}): ${util.inspect(this.content)}`
    }

    get(key: string): Value | undefined {
        for (let assn of this.content) {
            if (key == assn.name.text) {
                return assn.value
            }
        }

        return undefined
    }

    getNum(key: string, default_number?: number): number {
        let got = this.get(key)
        if ((undefined == got) && (undefined != default_number)) {
            return default_number
        }
        if ('number' == typeof got) return got

        throw new UnknownBlockEntryError(key, 'number | default', this)
    }

    getNumU(key: string): number | undefined {
        let got = this.get(key)
        if (undefined == got) return undefined
        if ('number' == typeof got) return got

        throw new UnknownBlockEntryError(key, 'number | undefined', this)
    }

    getStr(key: string): string {
        let got = this.get(key)
        if ('string' == typeof got) return got

        throw new UnknownBlockEntryError(key, 'string', this)
    }
}

export class Assignment {
    name: Identifier
    value: Value
    
    constructor(name: any, value: any) {
        this.name = name
        this.value = value

        if ((value instanceof Keyword) && ('0' == value.text)) {
            this.value = 0
        } else if (value instanceof QuotedString) {
            this.value = value.text
        }
    }

    [util.inspect.custom]() {
        return `Assignment(${this.name.text} := ${util.inspect(this.value)})`
    }
}

export class UdmfMap extends DoomMap {
    constructor(udmf_content: string) {
        try {
            var parsed = parse(udmf_content)
        } catch (e: any) {
            if (! (e instanceof SyntaxError)) throw e

            Logger.ERROR(`syntax error at ${util.inspect(e.location)}`)
            Logger.ERROR(e.message)
            throw e
        }

        let got_vertexes: Vertex[] = new Array()
        let got_linedefs: Linedef[] = new Array()
        let got_sidedefs: Sidedef[] = new Array()
        let got_sectors: Sector[] = new Array()

        for (let element of parsed) {
            if (element instanceof Assignment) {
            } else if (element instanceof Block) {
        
                switch (element.name.text) {
                    case 'vertex':
                        got_vertexes.push(Vertex.fromBlock(element))
                        break
                    case 'linedef':
                        got_linedefs.push(Linedef.fromBlock(element))
                        break
                    case 'sidedef':
                        got_sidedefs.push(Sidedef.fromBlock(element))
                        break
                    case 'sector':
                        got_sectors.push(Sector.fromBlock(element))
                        break
                    default:
                        Logger.NOOP(`unhandled element ${element.name.text}`)
                }
            } else {
                Logger.INFO(`unknown type ${typeof element}`)
            }
        }

        super({
            linedefs: got_linedefs,
            sidedefs: got_sidedefs,
            vertexes: got_vertexes,
            sectors: got_sectors
        })
    }
}