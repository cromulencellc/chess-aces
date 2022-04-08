"use strict";

if (! process.env.CHESS) {
  Logger.FATAL("This application is for research purposes only")
  process.exit(1)
}

const fs = require('fs');
const ws = require('ws');
const express = require('express')
const path = require('path');
const timestamp = require('./dateutils');


var port = process.env.PORT;

if (!port) {

  port = 3000;

}

console.log(`Listening on port ${port}`);

// get the list of planet models available at startup
const directoryPath = path.join(__dirname, 'Planets');

var planets = [];

fs.readdirSync(directoryPath).forEach(file => {

  planets.push(file.split(".")[0]); 

});

// str = '/$PATH/alpha/$PATH';
// let replaced = str.replace(/\$([A-Z_]+)/g, (_,n) => process.env[n])
// console.log(replaced);

const spacecraft = require('./spacecraft')

var selected_planet = './Planets/mars';

let planet_model = require(selected_planet)

let planet = new planet_model();

//set a bunch of initial defaults that will always work
var starting_altitude = 9000;
var starting_velocity = -500.0;
var starting_fuel_grams = 65000.0;
var fuel_consumption_rate = 131.0*12.0;
var maximum_thrust = 268*12;
var beta1 = 41.0;
var beta2 = 10.0;
var Kp = 0.8;
var Ki = 0.1;
var Kd = 0.5;
var pid_starting_altitude = 1600;
var pid_starting_velocity = -81.0;
var pid_final_velocity = -10.0;
var pid_final_altitude = 40.0;
var sim_logging = false;


const lander = new spacecraft(starting_altitude, starting_velocity, maximum_thrust, starting_fuel_grams, 
                                  fuel_consumption_rate, beta1, beta2, Kp, Ki, Kd, pid_starting_altitude, 
                                  pid_starting_velocity, pid_final_altitude, pid_final_velocity, planet);

var app = express();

app.use(express.static('html'));

const ws_server = new ws.Server({noServer: true });

console.log("Starting up");

ws_server.on('connection', function(socket) {

  socket.on('message', function(msg) {

    let message;
    try {

      message = JSON.parse(msg);

    }
    catch {

      return;
    }

    let type = message['cmd'];


    if (type == 'start') {


      lander.reset();

      runSimulation(socket);

    }
    else if (type == 'stop') {

      clearInterval(simTimer);

    }
    else if (type == "setconfig") {

      lander.pid_Kp = parseFloat(message['Kp']);
      lander.pid_Ki = parseFloat(message['Ki']);
      lander.pid_Kd = parseFloat(message['Kd']);
      lander.pid_final_alt = parseFloat(message['faltitude']);
      lander.pid_final_vel = parseFloat(message['fvelocity']);
      lander.pid_starting_alt = parseFloat(message['ialtitude']);
      lander.pid_starting_vel = parseFloat(message['ivelocity']);
      lander.starting_fuel_weight = parseFloat(message['fuel_max']);
      lander.max_thrust = parseFloat(message['thrust_max']);
      lander.fuel_consumption_rate = parseFloat(message['mpg']);
 
    }

    else if (type == 'getconfig') {

      let config = {
        'type': 'config',
        'Kp': lander.pid_Kp,
        'Ki': lander.pid_Ki,
        'Kd': lander.pid_Kd,
        'faltitude': lander.pid_final_alt,
        'fvelocity': lander.pid_final_vel,
        'ialtitude': lander.pid_starting_alt,
        'ivelocity': lander.pid_starting_vel,
        'fuel_max': lander.starting_fuel_weight,
        'thrust_max': lander.max_thrust,
        'mpg': lander.max_fuel_consumption,

      };

      let message = JSON.stringify(config);

      socket.send(message);

    }

    else if (type == 'setsimconfig') {

      lander.starting_velocity = parseFloat(message['sim_vel']);
      lander.starting_altitude = parseFloat(message['sim_alt']);

      let selected_planet = './Planets/' + message['planet'];

      sim_logging = message['logging'];

      let planet2;

      try {

        let planet_model2 = require(selected_planet)
        planet2 = new planet_model2();

      }
      catch {

        console.log("Unable to load the selected planet model");
        return;

      }

      lander.planet = planet2;

    }
    else if (type == 'getsimconfig') {

      let config = {
        'type': 'simconfig',
        'sim_vel': lander.starting_velocity,
        'sim_alt': lander.starting_altitude,
        'planets': planets,
        'selected_planet': lander.planet.name,
        'logging': sim_logging
      };

      let message = JSON.stringify(config);

      socket.send(message);

    }
    else if (type == 'listlogs') {

      listLogs(socket);

    }
    else if (type == 'sendlog') {

      let filename = message['name'];

      sendLog(socket, filename);
    }
  
  });

  socket.on('close', function() {

    console.log("connection closed");

  });

});

const server = app.listen(port);

console.log("Listening");

server.on('upgrade', (request, socket, head) => {

  ws_server.handleUpgrade(request, socket, head, socket => {

    ws_server.emit('connection', socket, request);

  });
});

var logList;

function listLogs(socket) {

// get the list of files currently in the logs directory
const directoryPath = path.join(__dirname, './Logs');

  let logs = [];

  logList = fs.readdirSync(directoryPath);
  
  logList.forEach(file => {
    logs.push(file); 
  });

  let response = {
    'type': 'loglist',
    'logs': logs
  };

  let message = JSON.stringify(response);

  socket.send(message);

}

function sendLog(socket, logname) {

  if (!logList || logList.length == 0)  {

    console.log("No log data available");
    return;
  }

  let filepath = './Logs/' + logname;

  let data;

  try {

    data = fs.readFileSync(filepath, 'utf8');

  }
  catch {

    data = 'File not found';

  }

  let message = {'type':'logdata',
                  'data': data };

  socket.send(JSON.stringify(message));

}

function runSimulation(socket) {

  let iterationCount = 0;
  let interval = .01;
  let terminalCount = 0;
  let data_logging = undefined;

  if (sim_logging) {

    let timestampstring = timestamp.getTimestamp();

    let logname = `${timestampstring}-simulation.csv`;
    
    data_logging = fs.createWriteStream('./Logs/'+logname);

    data_logging.write('# time_delta, altitude, velocity, fuel%, thrust%\n');

  }

  while (true) {

    let result = lander.update(interval);

    if (!result) {

      let message = {'type':'status',
      'status': 'fail',
      'message': 'Simulation stopped with math errors'};

      socket.send(JSON.stringify(message));

      break;

    }

    let stats = lander.get_current_stats();

    let time = Number.parseFloat(stats['sim_time_delta']).toFixed(2);
    let altitude = Number.parseFloat(stats['altitude']).toFixed(4);
    let velocity = Number.parseFloat(stats['velocity']).toFixed(4);
    let thrust = Number.parseFloat(stats['thrust%']).toFixed(2);
    let fuel = Number.parseFloat(stats['fuel%']).toFixed(2);


    if (sim_logging) {

      let csv_data = `${time}, ${altitude}, ${velocity}, ${fuel}, ${thrust}\n`
      data_logging.write(csv_data);

    }

    if (iterationCount % 10 == 0 ) {

      let data = JSON.stringify(stats);
      socket.send(data);

    }

    iterationCount += 1;

    if ( iterationCount > 40000 ) {

      let message = {'type':'status',
      'status': 'fail',
      'message': 'Simulation stopped after too many iterations'};

      socket.send(JSON.stringify(message));

      break;

    }
    // once the sim get to the ground, run for 10 more iterations to give the controller 
    // a chance to do its thing after touchdown
    if (lander.current_altitude <= 0.0 ) {
  
      terminalCount += 1;
    }
  
    if (terminalCount > 10) {
  
        console.log(`Simulation complete.`);

        if (sim_logging) {

          data_logging.end();

        }

        let status;
        let status_message;

        if (lander.spacecraft_touchdown == 1) {

          status = 'success';
          status_message = 'Simulation completed successfully';

        }
        else {

          status = 'fail';
          status_message = 'Simulation completed with a failure';

        }

        let message = {'type':'status',
                      'status': status,
                      'message': status_message};

        socket.send(JSON.stringify(message));

        break;
  
    }

  }

}
