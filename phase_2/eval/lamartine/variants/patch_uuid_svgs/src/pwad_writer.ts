import { BinaryWriter, File } from "csbinary";
import { pwad_magic } from "./pwad";
import { DoomMap } from "./doom_map";

import fs from 'fs'
import logger from "./logger";

export class PwadWriter {
    map: DoomMap
    constructor(map: DoomMap) {
        this.map = map
    }

    save(filename: string) {
        let textmap = this.map.to_textmap()
        let textmap_start = 4 + 4 + 4 + // header
        4 + 4 + 8 + // startmap
        4 + 4 + 8 + // textmap
        4 + 4 + 8 + // endmap
        0
        
        let writer = new BinaryWriter(File(fs.openSync(filename, 'w')))
        writer.writeBuffer(pwad_magic)
        writer.writeUInt32(3) // lump count
        writer.writeUInt32(4+4+4)
        
        logger.INFO(`file header 0x${writer.file.tell().toString(16)}`)

        writer.writeUInt32(textmap_start)
        writer.writeUInt32(0)
        writer.writeRawString("E1M1\0\0\0\0")
        logger.INFO(`E1M1 0x${writer.file.tell().toString(16)}`)

        writer.writeUInt32(textmap_start)
        writer.writeUInt32(textmap.length)
        writer.writeRawString("TEXTMAP\0")
        logger.INFO(`TEXTMAP 0x${writer.file.tell().toString(16)}`)

        writer.writeUInt32(textmap_start + textmap.length)
        writer.writeUInt32(0)
        writer.writeRawString("ENDMAP\0\0")
        logger.INFO(`ENDMAP 0x${writer.file.tell().toString(16)}`)


        if (textmap_start != writer.file.tell()) {
            logger.FATAL(`calculated 0x${textmap_start.toString(16)}`)
            logger.FATAL(`got 0x${writer.file.tell().toString(16)}`)
            throw new Error("wad toc longer than expected")
        }

        writer.writeRawString(textmap)

        writer.close()
    }
}