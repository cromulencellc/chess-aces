import { PwadWriter } from "./pwad_writer"
import { Pwad } from "./pwad"

let original_pwad = process.argv[2]
let new_pwad = process.argv[3]

let pwad = new Pwad(original_pwad)

for (let s of pwad.map.sectors) {
    s.texturefloor = '../../token'
}

let writer = new PwadWriter(pwad.map)
writer.save(new_pwad)