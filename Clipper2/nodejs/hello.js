// hello.js
const clipper = require('./build/Release/nativeclipper');
let c = new clipper.Clipper(1000);
console.log(`Precision ${c.precision()}`);
c.addPath([1, 100], "subject");
c.addPath([{x:1, y:100}], "clip");
c.addPath(Float64Array.of(1, 100), "subject");
c.addPath(Int32Array.of(1, 100), "invalid", (e) => {console.log(`e is ${e}`)});

c.addPaths([Int32Array.of(1, 100), Float64Array.of(1, 100), [55, 77, 33, 11]], "clip");
console.log('complete');
