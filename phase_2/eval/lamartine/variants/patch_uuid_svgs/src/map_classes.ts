import {Block} from './udmf_classes'

import util from 'util'

export class NoSuchObjectError extends Error {
    constructor(index: number, type: string) {
        super(`the ${type} collection has no index ${index}`)
    }
}

export class LinedefWithoutFrontSidedefError extends Error {
    constructor(linedef: Linedef) {
        super(`linedef ${util.inspect(linedef)} is missing a front sidedef`)
    }
}

export class UndrawableLinedefError extends Error {
    constructor(linedef: Linedef) {
        super(`couldn't figure out how to draw ${util.inspect(linedef)}`)
    }
}

export class MalformedPolygonsError extends Error {
    constructor(polys: Polygon[]) {
        super(`couldn't make sensible polygons out of ${util.inspect(polys)}`)
    }
}

function check_get<Type extends MapClass>(index: number, collection: Array<Type>): Type {
    if (index >= collection.length) {
        throw new NoSuchObjectError(index, collection[0].constructor.name)
    }
    return collection[index]
}

class MapClass {
}

export class Vertex extends MapClass {
    x: number
    y: number

    static fromBlock(block: Block) {
        return new Vertex(block.getNum('x'), block.getNum('y'))
    }

    constructor(x: number, y: number) {
        super()
        this.x = x
        this.y = y
    }
}

export type Polygon = Vertex[]

export class Sector extends MapClass {
    texturefloor: string
    heightceiling: number
    heightfloor: number
    linedefs: Set<Linedef> = new Set()

    #polygons: Polygon[] = new Array()
    
    static fromBlock(block: Block) {
        let texturefloor = block.getStr('texturefloor')
        let heightceiling = block.getNum('heightceiling', 0)
        let heightfloor = block.getNum('heightfloor', 0)

        return new Sector(texturefloor, heightceiling, heightfloor)
    }

    constructor(texturefloor: string, 
        heightceiling: number, 
        heightfloor: number) {
        super()
        this.texturefloor = texturefloor
        this.heightceiling = heightceiling
        this.heightfloor = heightfloor
    }

    polygons(): Polygon[] {
        if (0 < this.#polygons.length) return this.#polygons

        let valid_polygons: Polygon[] = []

        var unmatched_lines: Linedef[] = new Array()
        for (let [linedef, _ld] of this.linedefs.entries()) {
            unmatched_lines.push(linedef)
        }

        var current_polygon: Polygon = []
        let first_ld = unmatched_lines.pop()!
        current_polygon.unshift(first_ld.v1!)
        current_polygon.unshift(first_ld.v2!)

        while (0 < unmatched_lines.length) {
            let want_vertex = current_polygon[0]
            let matching_line = unmatched_lines.find(line => {
                if (line.v1 == want_vertex) return true
                if (line.v2 == want_vertex) return true
                return false
            })

            if (undefined == matching_line) {
                // start new polygon
                valid_polygons.push(current_polygon)
                let new_line = unmatched_lines.pop()!

                current_polygon = []
                current_polygon.unshift(new_line.v1!)
                current_polygon.unshift(new_line.v2!)
                continue
            }

            let rm = unmatched_lines.indexOf(matching_line)
            unmatched_lines.splice(rm, 1)

            if (matching_line.v1 == want_vertex) {
                current_polygon.unshift(matching_line.v2!)
            } else {
                current_polygon.unshift(matching_line.v1!)
            }
        }

        valid_polygons.push(current_polygon)


        for (let p of valid_polygons) {
            if (3 <= p.length) {
                this.#polygons.push(p)
            }
        }

        if (0 == this.#polygons.length) {
            throw new MalformedPolygonsError(this.#polygons)
        }

        return this.#polygons
    }
}

export class Sidedef extends MapClass {
    si: number
    sector?: Sector

    static fromBlock(block: Block) {
        return new Sidedef(block.getNum('sector'))
    }

    constructor(si: number) {
        super()
        this.si = si
    }

    resolve(sectors: Sector[]) {
        this.sector = check_get(this.si, sectors)
    }
}

export enum DrawStyle {
    no_draw,
    one_sided,
    different_ceilings,
    different_floors,
    secret
}

export class Linedef extends MapClass {
    v1i: number
    v2i: number
    v1?: Vertex
    v2?: Vertex

    fsi: number
    bsi?: number
    fs?: Sidedef
    bs?: Sidedef

    secret: boolean

    static fromBlock(block: Block) {
        return new Linedef(block.getNum('v1'),
        block.getNum('v2'),
        block.getNum('sidefront'),
        block.getNumU('sideback'),
        !! block.get('secret'))
    }

    constructor(v1i: number, v2i: number, 
        fsi: number, bsi: (number | undefined), 
        secret: boolean) {
        super()
        this.v1i = v1i
        this.v2i = v2i

        this.fsi = fsi
        this.bsi = bsi

        this.secret = secret
    }

    resolve(vertexes: Vertex[], sidedefs: Sidedef[], sectors: Sector[]) {
        this.v1 = check_get(this.v1i, vertexes)
        this.v2 = check_get(this.v2i, vertexes)

        this.fs = check_get(this.fsi, sidedefs)
        this.fs.resolve(sectors)
        this.fs.sector!.linedefs.add(this)
        if (undefined != this.bsi) {
            this.bs = check_get(this.bsi, sidedefs)
            this.bs.resolve(sectors)
            this.bs.sector!.linedefs.add(this)
        }
    }

    drawStyle(): DrawStyle {
        if (undefined == this.fs) {
            throw new LinedefWithoutFrontSidedefError(this)
        }
        if (this.secret) return DrawStyle.secret
        if (undefined == this.bs) return DrawStyle.one_sided

        if (this.bs.sector!.heightfloor != this.fs.sector!.heightfloor) {
            return DrawStyle.different_floors
        }

        if (this.bs.sector!.heightceiling != this.fs.sector!.heightceiling) {
            return DrawStyle.different_ceilings
        }

        return DrawStyle.no_draw
    }
}