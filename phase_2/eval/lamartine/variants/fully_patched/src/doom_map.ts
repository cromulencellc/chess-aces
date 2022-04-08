import { DefaultMap } from './default_map'
import {Linedef, Sidedef, Sector, Vertex, DrawStyle} from './map_classes'

export class DoomMap {
    linedefs: Linedef[]
    sidedefs: Sidedef[]
    vertexes: Vertex[]
    sectors: Sector[]

    sectors_by_floor: DefaultMap<string, Set<Sector>>
    linedefs_by_drawstyle: DefaultMap<DrawStyle, Set<Linedef>>

    min_x = Number.MAX_VALUE
    min_y = Number.MAX_VALUE
    max_x = Number.MIN_VALUE
    max_y = Number.MIN_VALUE

    constructor({ linedefs, sidedefs, vertexes, sectors }: 
        { linedefs: Linedef[]; 
            sidedefs: Sidedef[]; 
            vertexes: Vertex[]; 
            sectors: Sector[] }) {
        this.linedefs = linedefs
        this.sidedefs = sidedefs
        this.vertexes = vertexes
        this.sectors = sectors

        this.linedefs.forEach(ld => ld.resolve(this.vertexes,
            this.sidedefs, 
            this.sectors))

        this.sectors_by_floor = new DefaultMap(() => new Set())

        this.sectors.forEach(sector => {
            this.sectors_by_floor.get(sector.texturefloor).add(sector)
        })

        this.linedefs_by_drawstyle = new DefaultMap(() => new Set())
        this.linedefs.forEach(ld => {
            this.linedefs_by_drawstyle.get(ld.drawStyle()).add(ld)
        })

        this.vertexes.forEach(vx => {
            if (vx.x > this.max_x) this.max_x = vx.x
            if (vx.x < this.min_x) this.min_x = vx.x
            if (vx.y > this.max_y) this.max_y = vx.y
            if (vx.y < this.min_y) this.min_y = vx.y
        })
        
    }

    to_textmap(): string {
        let lines: string[] = []

        this.vertexes.forEach((vtx) => {
            lines.push('vertex')
            lines.push('{')
            lines.push(`x = ${vtx.x};`)
            lines.push(`y = ${vtx.y};`)
            lines.push('}')
        })

        this.sidedefs.forEach((sd) => {
           lines.push('sidedef')
           lines.push('{')
           lines.push(`sector = ${sd.si};`)
           lines.push('}')
        })

        this.sectors.forEach((st) => {
            lines.push('sector')
            lines.push('{')
            lines.push(`texturefloor = "${st.texturefloor}";`)
            lines.push(`heightceiling = ${st.heightceiling};`)
            lines.push(`heightfloor = ${st.heightfloor};`)
            lines.push('}')
        })

        this.linedefs.forEach((ld) => {
            lines.push('linedef')
            lines.push('{')
            lines.push(`v1 = ${ld.v1i};`)
            lines.push(`v2 = ${ld.v2i};`)
            lines.push(`sidefront = ${ld.fsi};`)
            if (undefined != ld.bsi) {
                lines.push(`sideback = ${ld.bsi};`)
            }
            if (ld.secret) {
                lines.push('secret = true;')
            }
            lines.push('}')
        })

        return lines.join("\n")
    }


}