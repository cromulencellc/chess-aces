import { PwadWriter } from "./pwad_writer"
import { Pwad } from "./pwad"

import crypto from 'crypto'

let original_pwad = process.argv[2]
let new_pwad = process.argv[3]

let pwad = new Pwad(original_pwad)

// only delete one of these, and definitely at least one of these, 
// since we don't want
// to accidentally make a still-valid map
let sectors_to_delete = crypto.randomInt(0, 8)
let linedefs_to_delete = crypto.randomInt(0, 8)
let sidedefs_to_delete = crypto.randomInt(0, 8)
let vertexes_to_delete = crypto.randomInt(1, 8)

if (sectors_to_delete > 0) {
    for (let i = 0; i < sectors_to_delete; i++) {
        pwad.map.sectors.pop()
    }
} else if (linedefs_to_delete > 0) {
    for (let i = 0; i < linedefs_to_delete; i++) {
        pwad.map.linedefs.pop()
    }
} else if (sidedefs_to_delete > 0) {
    for (let i = 0; i < sidedefs_to_delete; i++) {
        pwad.map.sidedefs.pop()
    }
} else if (vertexes_to_delete > 0) {
    for (let i = 0; i < vertexes_to_delete; i++) {
        pwad.map.vertexes.pop()
    }
}


let writer = new PwadWriter(pwad.map)
writer.save(new_pwad)