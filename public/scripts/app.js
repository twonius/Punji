

var deviceName = 'Spot WS'
var wsService = 'weight_scale'
var wsCharacteristic = 'weight_measurement'
var battService = 'battery_service'
var battCharacteristic = 'battery_level'
var bluetoothDeviceDetected
var gattCharacteristic
var weightCharacteristic
var battCharacteristic
var battStatus

var weightData = new Array()

const ws = new WebSocket('ws://localhost:9898/');
ws.onopen = function() {
    console.log('WebSocket Client Connected');
    ws.send(9999);
};
ws.onmessage = function(e) {
  //console.log("Received: '" + e.data + "'");
};


document.querySelector('#read').addEventListener('click', function() {
  if (isWebBluetoothEnabled()) { read() }
})

document.querySelector('#start').addEventListener('click', function(event) {
  if (isWebBluetoothEnabled()) { start() }
})

document.querySelector('#stop').addEventListener('click', function(event) {
  if (isWebBluetoothEnabled()) { stop() }
})

Plotly.plot('chart',[{
  y:[],
  type:'line'
}]);

function isWebBluetoothEnabled() {
  if (!navigator.bluetooth) {
    console.log('Web Bluetooth API is not available in this browser!')
    return false
  }

  return true
}

function getDeviceInfo() {
  let options = {

    filters: [
     { "name": deviceName,
     services: ['weight_scale','battery_service']
  }]
  }

  console.log('Requesting any Bluetooth Device...')
  return navigator.bluetooth.requestDevice(options).then(device => {
    bluetoothDeviceDetected = device
  }).catch(error => {
    console.log('Argh! ' + error)
  })
}

function read() {
  return (bluetoothDeviceDetected ? Promise.resolve() : getDeviceInfo())
  .then(connectGATTAsync)
  .then(_ => {
    //console.log('Reading Weight...')
    return weightCharacteristic.startNotifications()
  })
  .catch(error => {
    console.log('Waiting to start reading: ' + error)
  })
}

// function connectGATT() {
//   if (bluetoothDeviceDetected.gatt.connected && gattCharacteristic) {
//     return Promise.resolve()
//   }
//
//   return bluetoothDeviceDetected.gatt.connect()
//   .then(server => {
//     console.log('Getting GATT Service...')
//     return server.getPrimaryService(wsService)
//     console.log(wsService)
//   })
//   .then(service => {
//     console.log('Getting GATT Characteristic...')
//     return service.getCharacteristic(wsCharacteristic)
//     console.log(wsCharacteristic)
//   })
//   .then(characteristic => {
//     gattCharacteristic = characteristic
//     gattCharacteristic.addEventListener('characteristicvaluechanged',
//         handleNotifications)
//     document.querySelector('#start').disabled = false
//     document.querySelector('#stop').disabled = true
//   })
// }

async function connectGATTAsync() {
  try {


    const server = await bluetoothDeviceDetected.gatt.connect();
    console.log('Connecting to GATT Server...');

    console.log('Getting Battery Service...');
    const batteryService = await server.getPrimaryService(battService);

    console.log('Getting Battery Level Characteristic...');
    batteryCharacteristic = await batteryService.getCharacteristic(battCharacteristic);

    console.log('Getting Weight Service...');
    const weightService = await server.getPrimaryService(wsService);

    console.log('Getting Weight Measurement Characteristic...');
    weightCharacteristic = await weightService.getCharacteristic(wsCharacteristic);



    weightCharacteristic.addEventListener('characteristicvaluechanged',handleNotifications);

    //batteryCharacteristic.addEventListener('characteristicvaluechanged',handleBattery);


}catch(error) {
    console.log('Argh! ' + error);
  }
}

async function readBattery(){
  const value = await batteryCharacteristic.readValue();

  console.log('> Battery Level is ' + value.getUint8(0) + '%');

}

function handleBattery(event) {
  battStatus = event.target.value.getUint8(0)
  console.log('Battery Percent Updated: ' + battStatus.toString() + '%');
}

function timeConverter(UNIX_timestamp){
  var a = new Date(UNIX_timestamp * 1000);
  var months = ['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec'];
  var year = a.getFullYear();
  var month = months[a.getMonth()];
  var date = a.getDate();
  var hour = a.getHours();
  var min = a.getMinutes();
  var sec = a.getSeconds();
  var time = date + ' ' + month + ' ' + year + ' ' + hour + ':' + min + ':' + sec ;
  return time;
}

//swapped in code from https://googlechrome.github.io/samples/web-bluetooth/notifications.html
function handleNotifications(event) {

//console.log('notification Received')
let value = event.target.value;

// Convert raw data bytes to hex values just for the sake of showing something.
// In the "real" world, you'd use data.getUint8, data.getUint16 or even
// TextDecoder to process raw data bytes.
var weightReading = value.getUint16(1)
var byte1 = value.getUint8(3)
var byte2 = value.getUint8(4)
var byte3 = value.getUint8(5)
var byte4 = value.getUint8(6)
var byte5 = value.getUint8(7)
var byte6 = value.getUint8(8)
console.log(weightReading.toString(16)+byte1.toString(16)+ byte2.toString(16)+ byte3.toString(16) + byte4.toString(16) + byte5.toString(16) + byte6.toString(16))
//console.log(timeConverter(unix_timestamp));


// build array to plot
weightData.push(weightReading);

ws.send(weightReading); //send over websocket

var weightDisp = document.getElementById("weightDisplay");
var unit = "";
var weightReading_str = weightReading.toString();

//console.log('weight: ' + weightReading_str);
weightDisp.textContent = weightReading_str.concat(unit);


Plotly.extendTraces('chart', { y: [[weightReading]] }, [0]);

cnt = weightData.length;

if(cnt>500) {
  Plotly.relayout('chart',{
    xaxis: {
    range: [cnt-500,cnt]}
  });
}
readBattery()

}


function start() {
  weightCharacteristic.startNotifications()
  .then(_ => {
    console.log('Start reading...')
    document.querySelector('#start').disabled = true
    document.querySelector('#stop').disabled = false
  })
  .catch(error => {
    console.log('[ERROR] Start: ' + error)
  })
}

function stop() {
  weightCharacteristic.stopNotifications()
  .then(_ => {
    console.log('Stop reading...')
    document.querySelector('#start').disabled = false
    document.querySelector('#stop').disabled = true
  })
  .catch(error => {
    console.log('[ERROR] Stop: ' + error)
  })
}
