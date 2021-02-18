const http = require('http')
const express = require('express');
const mongoose = require("mongoose");
const passport = require("passport");
var bodyParser = require("body-parser")
var app = express();
var methodOverride = require("method-override");
var cookieParser = require("cookie-parser")
const path = require('path');
const router = express.Router();
const sslRedirect = require('heroku-ssl-redirect');
const WebSocketServer = require('websocket').server;
var sensorData = require("./models/sensorData");






var indexRoutes = require("./routes/index");

//assign mongoose a promise library and connect to a database
mongoose.promise = global.Promises;
const databaseUri = process.env.MONGODB_URI || 'mongodb+srv://user:yqpAl6Q4oHLliI00@beta.d501j.mongodb.net/spot?retryWrites=true&w=majority';

mongoose.connect(databaseUri, {useNewUrlParser: true})
  .then(() => console.log('database connected on: ', databaseUri))
  .catch(err => console.log('Database conection error : ${err.message}') );


app.use(bodyParser.urlencoded({extended: true}));
app.set("view engine","ejs");
app.use(express.static(__dirname+"/public"));
app.use(methodOverride('_method'));
app.use(cookieParser('secret'));

//app.use(sslRedirect());

app.locals.moment = require('moment');

const port = process.env.PORT || 3000;
const ip = process.env.IP;

app.use('/', indexRoutes);
app.listen(port,ip,function(){
  console.log(`server has started on ${ip} : ${port}`);
});



//setup websockeet
const server = http.createServer();
server.listen(9898);
const wsServer = new WebSocketServer({
    httpServer: server
});
wsServer.on('request', function(request) {
    const connection = request.accept(null, request.origin);
    connection.on('message', function(message) {
      msg = JSON.parse(message.utf8Data);
      console.log('Received Message:', msg);

      reading = new sensorData(msg);
      reading.save(function (err, point) {
      if (err) return console.error(err);
    });
      connection.sendUTF('Hi this is WebSocket server!');
    });
    connection.on('close', function(reasonCode, description) {
        console.log('Client has disconnected.');
    });
});

function onButtonClick(){
  let filters = [];
  let options = {};

  let filterName = 'Bluefruit52 HRM';
  let filterService = parseInt(0x180D);

  filters.push({name: filterName});
  filters.push({services: [filterService]});

  options.filters = filters;


  console.log('Requesting Bluetooth Device...');
  console.log('with ' + JSON.stringify(options));
  navigator.bluetooth.requestDevice(options)
  .then(device => {
    console.log('> Name:             ' + device.name);
    console.log('> Id:               ' + device.id);
    console.log('> Connected:        ' + device.gatt.connected);
  })
  .catch(error => {
    console.log('Argh! ' + error);
  });
}
