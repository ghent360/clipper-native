'use strict';

var path1 = [0, 0, 50, 0, 50, 25, 0, 25, 0, 0];
var path2 = [0, 0, 20, 0, 20, 50, 0, 50, 0, 0];
async function main() {
    let cl = await Promise.resolve().then(() => require("./clipper_js.js"));
    let c = new cl.Clipper(1000);
    c.addPathArrays([path1, path2], cl.PathType.Subject, false);
    let result = c.executeClosedToArrays(cl.ClipType.Union, cl.FillRule.NonZero);
    console.log('success');
    console.log(`result = ${result.solution_closed}`);
    result = cl.OffsetPaths([path1], 1000, 5, cl.JoinType.Miter, cl.EndType.Polygon);
    console.log('success offset');
    console.log(`result = ${result.solution}`);
}
main()
    .then(() => console.log('complete'))
    .catch(err => console.error(`Error:${err}`));