var express = require("express");
var router = express.Router();
var sensorData = require("../models/sensorData");

router.use(express.json());

//root route
router.get("/",function(req,res){
  res.render("index");
});

router.post("/data",function(req,res){
  console.log(req.body)

  reading = new sensorData(req.body);
  reading.save().then(
    ()=>{
      req.status(201).json({message:'post saved successfully'});
    }).catch(
      (error) => {
        res.status(400).json({
          error: error
        });
      }
    );
});

module.exports = router;
