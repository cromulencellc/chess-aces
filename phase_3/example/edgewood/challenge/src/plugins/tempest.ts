
var db2 = require('../models/database');
var dgram = require('dgram');
const server = dgram.createSocket('udp4');

server.on('error', (err: Error) => {
  console.log(`server error:\n${err.stack}`);
  server.close();
});

server.on('message', (msg: Buffer, _rinfo: any) => {


  var data;


  try {

    data = JSON.parse(msg.toString());

  }
  catch {

    console.log("Bad msg received and ignored");
    console.log(msg);
    return;

  }

  // console.log(data);

  switch(data.type) {

    case 'hub_status':

      // console.log("got a hub status message");
      process_hub_status(data);
      break;

    case 'obs_st':

      // console.log("got an observation message");
      process_observation(data);
      break;

    case 'device_status':

      // console.log("got a device status message");
      process_device_status(data);
      break;

    case 'rapid_wind':

      // console.log("got a rapid wind message");
      process_wind(data);
      break;

    case 'evt_precip':

      // console.log("got a rain starting message");
      process_precip(data);
      break;

    case 'evt_strike':

      // console.log("got a lightning strike message");
      process_lightning(data);
      break;

    default:

      // console.log("****************************");
      // console.log(data.type);
      db2.saveCustomWeatherRecord(msg);

  }

});

server.on('listening', () => {
  const address = server.address();
  console.log(`weather collector listening ${address.address}:${address.port}/udp`);

});

server.bind(parseInt(process.env.WEATHER_PORT || '50222'));

function process_observation(data: any) {

  if ( data.type != 'obs_st') {

    console.log("Bad message type for observation");
    return;

  }

  const data2 = data.obs[0];

  const timestamp = data2[0];

  const temp_C = data2[7];
  const temp_F = temp_C * 9 / 5 + 32;

  const rain_mm = data2[12];
  const rain_in = rain_mm * 0.0393700787;

  db2.save_temperature(timestamp, temp_F);
  db2.save_avg_wind( timestamp, data2[2], data2[1], data2[3], data2[4] );
  db2.save_instant_rain(timestamp, rain_in);
  db2.save_air_stats(timestamp, temp_F, data2[8], data2[6]);

}

function process_wind(data: any) {

  if ( data.type != 'rapid_wind') {

    console.log("Bad message type for wind reading");
    return;

  }

  db2.save_instant_wind(data.ob[0], data.ob[1], data.ob[2]);

}

function process_device_status(data: any) {

  if ( data.type != 'device_status') {

    console.log("Bad message type for device status");
    return;

  }

  db2.save_device_status(data.timestamp, data.firmware_revision, data.uptime, data.serial_number, data.sensor_status)

}

function process_hub_status(data: any) {

  if ( data.type != 'hub_status') {

    console.log("Bad message type for hub status");
    return;

  }

}

function process_precip(data: any) {

  if ( data.type != 'evt_precip') {

    console.log("Bad message type for rain starting");
    return;

  }

}

function process_lightning(data: any) {

  if ( data.type != 'evt_strike') {

    console.log("Bad message type for lightning strike");
    return;

  }

}

