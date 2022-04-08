import { DoomMap } from "./doom_map"
import fs from "fs"
import path from 'path'
import xml, { ElementObject } from "xml"
import { DrawStyle } from "./map_classes"

export class SvgWriter {
    map: DoomMap
    out_stream: fs.WriteStream
    svg_root: xml.ElementObject
    svg_stream: NodeJS.ReadableStream

    constructor(filename: string, map: DoomMap) {
        this.map = map
        this.out_stream = fs.createWriteStream(filename)
        this.svg_root = this._mk_root()
        this.svg_stream = xml({ svg: this.svg_root },
            { stream: true })

        this.svg_stream.pipe(this.out_stream)
    }

    write() {
        this._write_patterns()
        this._write_bg()
        this._write_sectors()
        this._write_linedefs()
        this.svg_root.close()
    }

    _mk_root(): xml.ElementObject {
        return xml.element({
            _attr: {
                viewBox: [this.map.min_x - 1,
                this.map.min_y - 1,
                2 + this.map.max_x - this.map.min_x,
                2 + this.map.max_y - this.map.min_y].join(' '),
                transform: 'scale(1, -1)',
                xmlns: "http://www.w3.org/2000/svg",
                fill: 'black'
            }
        })
    }

    _write_patterns() {
        for (let [floor, _sxs] of this.map.sectors_by_floor) {
            let image_path = path.format({
                dir: '/static/flats',
                name: path.basename(floor.toLowerCase())
            })
            let image_data = fs.readFileSync(image_path)
            this.svg_root.push(
                {pattern: [{
                    _attr: {
                        id: floor,
                        viewBox: '0,0,64,64',
                        width: '128px',
                        height: '128px',
                        patternUnits: 'userSpaceOnUse'
                }},
                {image: [{
                    _attr: {
                    href: 'data:image/png;base64,' + image_data.toString('base64'),
                    x: 0, y: 0,
                    height: 64, width: 64
                    }
                }]}
            ]})
        }
    }

    _write_bg() {
        this.svg_root.push({rect: {
            _attr: {
                fill: 'black',
                x: this.map.min_x - 1,
                y: this.map.min_y - 1,
                width: 2 + this.map.max_x - this.map.min_x,
                height: 2 + this.map.max_y - this.map.min_y
            }
        }})
    }

    _write_sectors() {
        for (let [floor, sxs] of this.map.sectors_by_floor) {
            var paths: ElementObject[] = []
            sxs.forEach(sect => {
                let polys = sect.polygons()
        
                var cmds: string[] = []
                polys.forEach(poly => {
                    let first_vtx = poly[0]
                    cmds.push(`M ${first_vtx.x},${first_vtx.y}`)
                    poly.slice(1, -1).forEach(v => {
                        cmds.push(`L ${v.x},${v.y}`)
                    })
                    cmds.push('Z')
                })
                
                let attrs = {
                    d: cmds.join(' '),
                    fill: `url(#${floor})`,
                    'fill-rule': 'evenodd'
                }
                this.svg_root.push({path: {_attr: attrs}})
            })
        
        }
    }

    _write_linedefs() {
        let draw_styles = new Map([    
            [DrawStyle.one_sided, '#f00'],
            [DrawStyle.different_ceilings, '#ff0'],
            [DrawStyle.different_floors, '#940'],
            [DrawStyle.secret, '#ff0']
        ])
        
        for (let [style, lds] of this.map.linedefs_by_drawstyle) {
            if (DrawStyle.no_draw == style) continue
            let color = draw_styles.get(style)
            if (undefined == color) {
                throw new Error("couldn't figuire out how to draw style ${style}")
            }
        
            var cmds: string[] = []
            lds.forEach(ld => {
                cmds.push(`M ${ld.v1!.x},${ld.v1!.y} L ${ld.v2!.x} ${ld.v2!.y}`)
            })
        
            let attrs = {
                d: cmds.join(' '),
                stroke: draw_styles.get(style)!
            }
        
            this.svg_root.push({path: {_attr: attrs}})
        }
    }
}