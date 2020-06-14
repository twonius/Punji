const http = require('http')
const express = require('express');
const app = express();
const path = require('path');
const router = express.Router();


router.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/index.html'));
  //__dirname : It will resolve to your project folder.
});

app.use('/', router);
app.listen(process.env.PORT || 3000);

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
