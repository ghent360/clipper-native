'use strict';
const cl_ = require("./clipper_js").then(cl => {
  let clipper = new cl.Clipper(1000);
  console.log('success');
});
