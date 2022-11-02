var mongoose = require('mongoose');
var schema = mongoose.Schema;
var InstantWindSchema = schema({
    timestamp: { type: [Number], index: true },
    speed: Number,
    direction: Number
});
var InstantWindModel = mongoose.model('InstantWindData', InstantWindSchema);
var AvgWindSchema = schema({
    timestamp: { type: [Number], index: true },
    avg: Number,
    lull: Number,
    gust: Number,
    direction: Number
});
var AvgWindModel = mongoose.model('AvgWindData', AvgWindSchema);
var AirSchema = schema({
    timestamp: { type: [Number], index: true },
    humidity: Number,
    pressure: Number,
    temperature: Number
});
var AirModel = mongoose.model('AirData', AirSchema);
var InstantRainSchema = schema({
    timestamp: { type: [Number], index: true },
    rain_per_minute: Number
});
var InstantRainModel = mongoose.model('InstantRainData', InstantRainSchema);
var TotalRainSchema = schema({
    timestamp: { type: [Number], index: true },
    rain_per_minute: Number
});
var TotalRainModel = mongoose.model('TotalRainData', TotalRainSchema);
var TempSchema = schema({
    timestamp: { type: [Number], index: true },
    temperature: Number
});
var TempModel = mongoose.model('TempData', TempSchema);
var HubSchema = schema({
    timestamp: { type: [Number], index: true },
    serial_number: String,
    firmware_version: Number,
    uptime: Number
});
var HubModel = mongoose.model('HubData', HubSchema);
var DeviceSchema = schema({
    timestamp: { type: [Number], index: true },
    serial_number: String,
    firmware_version: Number,
    uptime: Number,
    sensor_status: Number
});
var DeviceModel = mongoose.model('DeviceData', DeviceSchema);
exports.InstantWindModel = InstantWindModel;
exports.AvgWindModel = AvgWindModel;
exports.AirModel = AirModel;
exports.InstantRainModel = InstantRainModel;
exports.TotalRainModel = TotalRainModel;
exports.TempModel = TempModel;
exports.HubModel = HubModel;
exports.DeviceModel = DeviceModel;
