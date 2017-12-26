// hello.js
const clipper = require('./build/Release/nativeclipper');
let c = new clipper.Clipper(1000);
console.log(`Precision ${c.precision()}`);
c.addPath([0, 0, 10, 0, 10, 10, 0, 10, 0, 0], "subject");
c.addPath([0, 0, 5, 0, 5, 5, 0, 5, 0, 0], "clip");
let r = c.execute("diff");
console.log(`Result ${r.success}, solution ${r.solution}`);
