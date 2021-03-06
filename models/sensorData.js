var mongoose = require("mongoose");

var sensorDataSchema = new mongoose.Schema({
    deviceID: {type:Number,default: 0},
    userID: {type: Number, default: 0},
    timestamp: String,
    weight: {type: Number, default : 9999},
    weight2: {type: Number, default : 9999},
    battery: {type: Number, default: 999},
    WeightInput: Number

}, { strict: false });

module.exports = mongoose.model("sensorData",sensorDataSchema);
