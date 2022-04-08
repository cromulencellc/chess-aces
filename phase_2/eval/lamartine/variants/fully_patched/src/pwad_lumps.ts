import { raw } from 'express'
import {Linedef, Sidedef, Sector, Vertex} from './map_classes'

import {TooBigError} from './too_big_error'

import { BinaryReader } from 'csbinary'


import util from 'util'

function cleanBuf(b: Buffer): string {
    return b.toString().replace(/\0+$/, '')
}

export class Lump {
    name: string
    length: number
    expected_count: number

    constructor(name: string, expected_size: number, length: number) {
        this.name = name
        this.length = length

        if (0 != (length % expected_size)) {
            throw new LumpSizeError(name, expected_size)
        }

        this.expected_count = length / expected_size
        if (this.expected_count > Number.MAX_SAFE_INTEGER) {
            throw new TooBigError()
        }
    }
}

export class LumpSizeError extends Error {
    constructor(name: string, expectation: number) {
        super(`${name} content wasn't divisible by ${expectation}`)
    }
}

export class Linedefs extends Lump {
    linedefs: Linedef[]

    static expected_size = 14

    constructor(reader: BinaryReader, length: number) {
        super('LINEDEFS', Linedefs.expected_size, length)

        this.linedefs = []

        for (let i = 0; i < this.expected_count; i++) {
            let v1 = reader.readUInt16()
            let v2 = reader.readUInt16()
            let flags = reader.readUInt16()
            let _special_type = reader.readUInt16()
            let _sector_tag = reader.readUInt16()
            let fsi = reader.readUInt16()
            var bsi_maybe: (number | undefined) = reader.readUInt16()

            if (0xffff == bsi_maybe) bsi_maybe = undefined

            let secret: boolean = (0x20 & flags) > 0

            this.linedefs.push(new Linedef(
                v1, v2, fsi, bsi_maybe, secret
            ))
        }
    }

    [util.inspect.custom]() {
        return `Linedefs(${this.linedefs.length})`
    }
}

export class Sidedefs extends Lump {
    sidedefs: Sidedef[]

    static expected_size = 30

    constructor(reader: BinaryReader, length: number) {
        super('SIDEDEFS', Sidedefs.expected_size, length)

        this.sidedefs = []

        for (let i = 0; i < this.expected_count; i++) {
            let _xo = reader.readInt16()
            let _yo = reader.readInt16()
            let _upper = reader.readBytes(8)
            let _lower = reader.readBytes(8)
            let _middle = reader.readBytes(8)
            let si = reader.readUInt16()

            this.sidedefs.push(new Sidedef(si))
        }
    }
}

export class Vertexes extends Lump {
    vertexes: Vertex[]

    static expected_size = 4

    constructor(reader: BinaryReader, length: number) {
        super('VERTEXES', Vertexes.expected_size, length)

        this.vertexes = []

        for (let i = 0; i < this.expected_count; i++) {
            let x = reader.readInt16()
            let y = reader.readInt16()

            this.vertexes.push(new Vertex(x, y))
        }
    }
}

export class Sectors extends Lump {
    sectors: Sector[]

    static expected_size = 26

    constructor(reader: BinaryReader, length: number) {
        super('SECTORS', Sectors.expected_size, length)

        this.sectors = []

        for (let i = 0; i < this.expected_count; i++) {
            let heightfloor = reader.readInt16()
            let heightceiling = reader.readInt16()
            let texturefloor = cleanBuf(reader.readBytes(8))
            let _textureceiling = reader.readBytes(8)
            let _lightlevel = reader.readInt16()
            let _special = reader.readInt16()
            let _tag = reader.readInt16()

            this.sectors.push(new Sector(texturefloor, heightceiling, heightfloor))
        }
    }
}