var mongoose = require("mongoose");

var sensorDataSchema = new mongoose.Schema({
    deviceID: {type:Number,default: 0},
    userID: {type: Number, default: 0},
    timestamp: Number,
    weight: Number,
    battery: Number
});

module.exports = mongoose.model("sensorData",sensorDataSchema);
