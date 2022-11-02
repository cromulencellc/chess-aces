import protobuf from 'protobufjs'

let pb_root = protobuf.loadSync('/static/tlm.proto')
let Telemetry = pb_root.lookupType('Telemetry')
let PositionVelocity = pb_root.lookupType('PositionVelocity')
let Battery = pb_root.lookupType('Battery')

export {Telemetry, PositionVelocity, Battery, pb_root}
