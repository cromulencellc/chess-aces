// this is the weather controller functions
const { models } = require('mongoose');
var db = require('../models/database');
let get_current_temperature = async function (_req, res) {
    var results = await db.get_last_temperature();
    res.send(results);
};
let get_max_temperature = async function (_req, res) {
    var results = await db.get_max_temperature();
    res.send(results);
};
let get_temperature_from = async function (req, res) {
    var results = await db.get_temperature_since(req.params['start'], req.params['end']);
    res.send(results);
};
let get_current_wind = async function (_req, res) {
    var results = await db.get_last_wind();
    res.send(results);
};
let get_max_wind = async function (_req, res) {
    var results = await db.get_max_wind();
    res.send(results);
};
let current_weather = async function (_req, _res) {
    // read the current temperature, humidity, pressure, and wind data from the database
    var airStats = db.get_last_air();
    var windStats = db.get_last_wind();
    const epoch_timestamp = Date.now();
    if (epoch_timestamp - 5 * 1000 > airStats.timestamp) {
        airStats = undefined;
    }
    if (epoch_timestamp - 5 * 1000 > windStats.timestamp) {
        windStats = undefined;
    }
};
let get_summary_data = async function (req, res) {
    let datestring = req.query.date;
    if (typeof datestring != 'string') {
        console.log({ result: "Bad query format--date is required" });
        res.send({ result: "Bad query format--date is required" });
        return;
    }
    if (datestring.length < 8) {
        res.send({ result: "bad date format" });
        return;
    }
    let year_str = datestring.slice(0, 4);
    let mon_str = datestring.slice(4, 6);
    let day_str = datestring.slice(6, 8);
    let date_start = new Date(parseInt(year_str), parseInt(mon_str) - 1, parseInt(day_str), 0, 0, 0);
    let date_end = new Date(parseInt(year_str), parseInt(mon_str) - 1, parseInt(day_str) + 1, 0, 0, 0);
    let timestamp_start = date_start.getTime() / 1000.0;
    let timestamp_end = date_end.getTime() / 1000.0;
    // console.log(timestamp_start);
    // console.log(timestamp_end);
    var get_temperatures = req.query.temperatures;
    var get_wind = req.query.wind;
    if (get_temperatures == undefined && get_wind == undefined) {
        get_temperatures = true;
        get_wind = true;
    }
    else {
        if (get_temperatures == 1 || get_temperatures == 'true') {
            get_temperatures = true;
        }
        else {
            get_temperatures = false;
        }
        if (get_wind == 1 || get_wind == 'true') {
            get_wind = true;
        }
        else {
            get_wind = false;
        }
    }
    if (get_wind == false && get_temperatures == false) {
        res.send({ result: "bad query--no items requested" });
        return;
    }
    // console.log(`get_temperatures: ${get_temperatures}, get_wind: ${get_wind}`);
    var report = [];
    if (get_temperatures) {
        var results = await db.get_temperature_since(timestamp_start, timestamp_end);
        report.push({ type: 'temperature', data: results });
    }
    if (get_wind) {
        var results = await db.get_wind_since(timestamp_start, timestamp_end);
        report.push({ type: 'wind', data: results });
    }
    // console.log(report);
    res.send(report);
};
export default {
    get_current_temperature: get_current_temperature,
    get_max_temperature: get_max_temperature,
    get_temperature_from: get_temperature_from,
    get_current_wind: get_current_wind,
    get_max_wind: get_max_wind,
    current_weather: current_weather,
    get_summary_data: get_summary_data
};
