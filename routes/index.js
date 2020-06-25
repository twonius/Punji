var express = require("express");
var router = express.Router();

//root route
router.get("/",function(req,res){
  res.render("index");
});

router.post("/",function(req,res){
  console.log('req.params.weight')
  res.send('POST request to the homepage');
});

module.exports = router;
