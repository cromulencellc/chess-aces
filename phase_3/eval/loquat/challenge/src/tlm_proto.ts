import protobuf from 'protobufjs'

let pb_root = protobuf.loadSync('/static/tlm.proto')
let Telemetry = pb_root.lookupType('Telemetry')
let PositionVelocity = pb_root.lookupType('PositionVelocity')
let Battery = pb_root.lookupType('Battery')

type PositionVelocityObject = {
  x: number,
  y: number,
  z: number,
  dx: number,
  dy: number,
  dz: number
}

type BatteryObject = {
  level: number,
  chargingTime: number,
  dischargingTime: number
}

type TelemetryObject = {
  satelliteId: string,
  txTimeMs: number,
  pv: PositionVelocityObject,
  bt: BatteryObject
}


export {TelemetryObject,
  Telemetry, PositionVelocity, Battery, pb_root}
