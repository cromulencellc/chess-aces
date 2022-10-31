// var url = "mongodb://localhost:27017/weatherdb";

import { CallbackError, Models } from "mongoose";
var mongooseDrv = require('mongoose'); //Get the Mongoose Driver

mongooseDrv.connect(
    (process.env.MONGO_URL || "mongodb://mongodb:27017/weatherdb"), 
    {w: "majority"});

var db = mongooseDrv.Connection; //The Connection
var schema = mongooseDrv.Schema;

if (db == 'undefined') {

    console.log("The Connecion issues");
}


var models = require('./weather');
var adminModel = require('./admin')

exports.create_admin_user = function (userid: string, password_digest: string) {
    let newAdmin = new adminModel.AdminAccessModel({
        "userid": userid,
        "password": password_digest
    })
    return adminModel.AdminAccessModel.create(newAdmin) // returns promise
}

exports.save_device_status = 
    async function(timestamp: number, 
        firmware_version: number,
        uptime: number,
        serial_number: string,
        sensor_status: number) {

    var newReading = new models.DeviceModel({
        timestamp: timestamp,
        firmware_version:firmware_version,
        uptime:uptime,
        serial_number:serial_number,
        sensor_status:sensor_status
    });

    await models.DeviceModel.create(newReading);


}

exports.get_last_device_status = async function() {

    var query = models.DeviceModel.findOne({},{"_id": 0, "timestamp":1, "serial_number":1, "firmware_version":1, "uptime":1, "sensor_status":1}).sort([['timestamp',-1]]);

    var results = await query.exec();

    return results;

}


exports.save_instant_wind = 
    async function(timestamp: number, 
        speed: number, 
        direction: number) {

    var newReading = new models.InstantWindModel({
                                                timestamp: timestamp,
                                                speed:speed,
                                                direction:direction
                                            });

    await models.InstantWindModel.create(newReading);

};

exports.get_instant_wind = async function() {

    var result = await models.InstantWindModel.find().exec();

    return result;

}

exports.save_avg_wind = async function(timestamp: number, 
    avg: number, lull: number, gust: number, direction: number) {

    var newReading = new models.AvgWindModel( {
                                                timestamp:timestamp,
                                                avg:avg,
                                                lull:lull,
                                                gust:gust,
                                                direction:direction
                                            });

    await models.AvgWindModel.create(newReading);

};



exports.save_air_stats = function(timestamp: number,
    temperature: number,
    humidity: number,
    pressure: number) {

    var newReading = new models.AirModel({
                                            timestamp:timestamp,
                                            temperature:temperature,
                                            humidity:humidity,
                                            pressure:pressure
                                        });

    models.AirModel.create(newReading, 
        function(addError: CallbackError | undefined, _addedRecord: any) {

        if (addError) {

            console.log("Error");
        }
        else {

            // console.log(addedRecord);
            // console.log("good");
        }
    });

}


exports.get_air_stats = async function() {

    var result = await models.AirModel.find().exec();

    return result;

}


exports.save_temperature = function(timestamp: number, temperature_f: number) {

    var newReading = new models.TempModel({
                                            timestamp:timestamp,
                                            temperature:temperature_f
                                        });

    models.TempModel.create(newReading, 
        function(addError: CallbackError | undefined, _addedRecord: any) {

        if (addError) {

            return("Error");
        }
        else {

            // console.log(addedRecord);
            // return("good");
        }
    });

}

exports.get_temperature = async function() {

    var result = await models.TempModel.find().exec();

    return result;

}

exports.get_temperature_since = async function(start_timestamp: number, end_timestamp: number) {

    var query = models.TempModel.find({},{"_id": 0, "timestamp":1, "temperature":1}).sort([['timestamp',1]]);

    if (start_timestamp > 0 && end_timestamp > 0) {

        // console.log(`${start_timestamp} ${end_timestamp}`);

        query.where('timestamp').gte(start_timestamp).lte(end_timestamp);

    }
    else if (start_timestamp > 0 ) {

        query.where('timestamp').gte(start_timestamp);

    }
    else if (end_timestamp > 0 ) {

        query.where('timestamp').lte(end_timestamp);

    }

    var results = await query.exec();

    return results;

}

exports.get_wind_since = async function(start_timestamp: number, end_timestamp: number) {


    // avg:avg,
    // lull:lull,
    // gust:gust,
    // direction:direction


    var query = models.AvgWindModel.find({},{"_id": 0, "timestamp":1, "avg":1, "gust":1, "lull":1}).sort([['timestamp',1]]);

    if (start_timestamp > 0 && end_timestamp > 0) {

        // console.log(`${start_timestamp} ${end_timestamp}`);

        query.where('timestamp').gte(start_timestamp).lte(end_timestamp);

    }
    else if (start_timestamp > 0 ) {

        query.where('timestamp').gte(start_timestamp);

    }
    else if (end_timestamp > 0 ) {

        query.where('timestamp').lte(end_timestamp);

    }

    var results = await query.exec();

    return results;

}

exports.get_last_temperature = async function() {

    var query = models.TempModel.findOne({},{"_id": 0, "timestamp":1, "temperature":1}).sort([['timestamp',-1]]);

    var results = await query.exec();

    return results;

}


exports.get_last_wind = async function() {

    var query = models.AvgWindModel.findOne({},{"_id": 0, "timestamp":1, "avg":1, "lull":1, "gust":1, "direction":1}).sort([['timestamp',-1]]);

    var results = await query.exec();

    return results;

}

exports.get_last_air = async function() {

    var query = models.AirModel.findOne({},{"_id": 0, "timestamp":1, "humidity":1, "pressure":1, "temperature":1}).sort([['timestamp',-1]]);

    var results = await query.exec();

    return results;

}

exports.get_max_temperature = async function() {

    var query = models.TempModel.findOne({},{"_id": 0, "timestamp":1, "temperature":1}).sort([['temperature',-1]]);

    var results = await query.exec();

    return results;

}

exports.get_max_wind = async function() {

    var query = models.AvgWindModel.findOne({},{"_id": 0, "timestamp":1, "avg":1, "lull":1, "gust":1, "direction":1}).sort([['avg',-1]]);

    var results = await query.exec();

    return results;

}

exports.delete_rain = async function() {

    models.AvgWindModel.remove({});
    models.TempModel.remove({});
    models.TotalRainModel.remove({});
    models.InstantRainModel.remove({});
    models.InstantWindModel.remove({});

}

exports.save_instant_rain = function(timestamp: number, rain_per_minute: number) {


    var newReading = new models.InstantRainModel({
                                                    timestamp:timestamp,
                                                    rain_per_minute:rain_per_minute
                                                });

    models.InstantRainModel.create(newReading, 
        function(addError: CallbackError | undefined, _addedRecord: any) {

        if (addError) {

            return("Error");
        }
        else {

            // return("good");
        }
    });


}

exports.save_rain_total = function(timestamp: number, rain_total: number) {

    var newReading = new models.TotalRainModel({
                                                timestamp:timestamp,
                                                rain_total:rain_total
                                            });

    models.TotalRainModel.create(newReading, 
        function(addError: CallbackError | undefined, _addedRecord: any) {


        if (addError) {

            return("Error");
        }
        else {

            return("good");
        }
    });

}

exports.saveDynamicSchema = async function(raw_message: string) {

    // walk through JSON message and create a Mongoose schema based on the fields
    var imported_json;
    var message_type;

    console.log(raw_message);

    try {

        // save the fields to the database based upon the message type
        imported_json = JSON.parse(raw_message);
    }
    catch {

        console.log("Unable to import this message as a weather record");
        return;
    }

    if (!imported_json.hasOwnProperty('type')) {

        console.log("Type property not found");
        return;

    }

    if (!imported_json.hasOwnProperty('timestamp')) {

        console.log("Timestamp property not found");
        return;

    }

    message_type = imported_json['type'];

    var new_schema = "{ ";
    var field_type;

    for (let key in imported_json) {

        field_type = typeof imported_json[key];

        console.log(field_type);

        new_schema += `"${key}":"${field_type}", `;

    }

    new_schema = new_schema.slice(0, new_schema.length - 2);

    new_schema += (" }");

    // console.log("schema: " + new_schema);

    var new_schema_json = JSON.parse(new_schema);

    var dynamicSchema = schema(new_schema_json);

    var dynamicModel = mongooseDrv.model(message_type, dynamicSchema);

    // walk thhrough the message again and assign the values to the model
    for (let key in imported_json) {

        let value = imported_json.key;
    }

    var savedRecord = new dynamicModel(imported_json);

    // save the model to a new collection
    await dynamicModel.create(savedRecord);

}

exports.saveCustomWeatherRecord = async function(msg: string) {

    var imported_json;
    var message_type;

    try {

        // save the fields to the database based upon the message type
        imported_json = JSON.parse(msg);
    }
    catch {

        console.log("Unable to import this message as a weather record");
        return;
    }

    if (!imported_json.hasOwnProperty('type')) {

        console.log("Type property not found");
        return;

    }

    if (!imported_json.hasOwnProperty('timestamp')) {

        console.log("Timestamp property not found");
        return;

    }

    message_type = imported_json['type'];

    const { MongoClient } = require("mongodb");

    const client = new MongoClient((process.env.MONGO_URL || "mongodb://mongodb:27017/weatherdb"), 
        {w: "majority"});

  try {

    await client.connect();

    const database = client.db('weatherdb');
    const collection = database.collection(message_type);

    await collection.insertOne(imported_json);

  } finally {
    await client.close();
  }
}
