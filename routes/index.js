var express = require("express");
var router = express.Router();

router.use(express.json());

//root route
router.get("/",function(req,res){
  res.render("index");
});

router.post("/data",function(req,res){
  console.log(req.body)
  res.send('POST request to the homepage');
});

module.exports = router;
