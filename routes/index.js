var express = require("express");
var router = express.Router();
var sensorData = require("./models/sensorData");

router.use(express.json());

//root route
router.get("/",function(req,res){
  res.render("index");
});

router.post("/data",function(req,res){
  console.log(req.body)
  res.send('POST request to the homepage');

  reading = new sensorData(req.body);
  reading.save(function (err, point) {
    if (err) return console.error(err);
    console.log('Post saved to DB')
  });
});

module.exports = router;
