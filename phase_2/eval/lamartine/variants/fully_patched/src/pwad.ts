
import { BinaryReader, File, SeekOrigin } from 'csbinary'
import fs from 'fs'
import { Linedefs, Vertexes, Sectors, Sidedefs } from './pwad_lumps'
import util from 'util'
import { DoomMap } from './doom_map'
import { UdmfMap } from './udmf_classes'

export const pwad_magic = Buffer.from('PWAD', 'binary')

export class NotPwadError extends Error {
    constructor(wad_kind: Buffer) {
        super(`expected PWAD but got ${wad_kind}`)
    }
}

export class TooManyMapsError extends Error {
    constructor(telltale_name: string) {
        super(`multiple ${telltale_name} lumps, expected only one`)
    }
}

export class MissingLumpError extends Error {
    constructor(telltale_name: string) {
        super(`needed but couldn't find ${telltale_name} lump`)
    }
}

export class Pwad {
    filename: string
    header: Header
    directory: Directory

    map: DoomMap

    constructor(filename: string) {
        this.filename = filename
        let reader = new BinaryReader(File(fs.openSync(filename, 'r')))

        this.header = new Header(reader)
        this.directory = new Directory(reader,
            this.header.directory_location,
            this.header.lump_count)


        if (this.directory.has('TEXTMAP')) {
            let udmf_dir = this.directory.get('TEXTMAP')
            reader.file.seek(udmf_dir.start, SeekOrigin.Begin)
            let udmf_lump = reader.readBytes(udmf_dir.size).toString()
            this.map = new UdmfMap(udmf_lump)
        } else {
            this.map = new PwadMap(reader, this.directory)
        }
    }

    [util.inspect.custom]() {
        return `Pwad(${this.filename}, directory=${util.inspect(this.directory)})`
    }
}

export class PwadMap extends DoomMap {
    constructor(reader: BinaryReader,
        directory: Directory) {
            let linedefs_dir = directory.get('LINEDEFS')

            reader.file.seek(linedefs_dir.start, SeekOrigin.Begin)
            let linedefs_lump = new Linedefs(reader, linedefs_dir.size)

            let sidedefs_dir = directory.get('SIDEDEFS')
            reader.file.seek(sidedefs_dir.start, SeekOrigin.Begin)
            let sidedefs_lump = new Sidedefs(reader, sidedefs_dir.size)

            let vertexes_dir = directory.get('VERTEXES')
            reader.file.seek(vertexes_dir.start, SeekOrigin.Begin)
            let vertexes_lump = new Vertexes(reader, vertexes_dir.size)

            let sectors_dir = directory.get('SECTORS')
            reader.file.seek(sectors_dir.start, SeekOrigin.Begin)
            let sectors_lump = new Sectors(reader, sectors_dir.size)

        super({linedefs: linedefs_lump.linedefs,
            sidedefs: sidedefs_lump.sidedefs,
            vertexes: vertexes_lump.vertexes,
            sectors: sectors_lump.sectors})
    }
}

class Header {
    wad_kind: Buffer
    lump_count: number
    directory_location: number

    constructor(reader: BinaryReader) {
        reader.file.seek(0, SeekOrigin.Begin)
        this.wad_kind = reader.readBytes(4)
        this.lump_count = reader.readUInt32()
        this.directory_location = reader.readUInt32()
    }
}

class Directory {
    entries: Map<string, DirectoryEntry> = new Map()

    constructor(reader: BinaryReader, 
        directory_location: number,
        lump_count: number) {
        reader.file.seek(directory_location, SeekOrigin.Begin)
        for (let i = 0; i < lump_count; i++) {
            let ent = new DirectoryEntry(reader)
            if (this.entries.has(ent.name)) {
                throw new TooManyMapsError(ent.name)
            }

            this.entries.set(ent.name, ent)
        }
    }

    has(lump_name: string) {
        return this.entries.has(lump_name)
    }

    get(lump_name: string): DirectoryEntry {
        let got = this.entries.get(lump_name)
        if (undefined == got) throw new MissingLumpError(lump_name)

        return got
    }
}

class DirectoryEntry {
    start: number
    size: number
    name: string

    constructor(reader: BinaryReader) {
        this.start = reader.readUInt32()
        this.size = reader.readUInt32()
        this.name = reader.readBytes(8).toString().replace(/\0+$/, '')
    }

    [util.inspect.custom]() {
        return `{${this.name} @ 0x${this.start.toString(16)}+${this.size.toString(16)}}`
    }
}