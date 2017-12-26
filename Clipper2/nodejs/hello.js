// hello.js
const clipper = require('./build/Release/nativeclipper');
let c = new clipper.Clipper(1000);
console.log(`Precision ${c.precision()}`);
c.addPath([1, 100], "subject");
c.addPath([{x:1, y:100}], "clip");
c.addPath(Float64Array.of(1, 100), "subject");
c.addPath(Int32Array.of(1, 100), "invalid", (e) => {console.log(`e is ${e}`)});
