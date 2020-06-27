var mongoose = require("mongoose");

var sensorDataSchema = new mongoose.Schema({
    deviceID: {type:Number,default: 0},
    UserID: {type: Number, default: 0},
    timestamp: {type:Date, default:Date.now},
    reading: Number,
});

module.exports = mongoose.model("sensorData",sensorDataSchema);
