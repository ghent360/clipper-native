const __build = require('./build');
console.log(`ClipperJS build ${__build.build}`);
var Module;
if (!Module) Module = (typeof Module !== 'undefined' ? Module : null) || {};
module.exports = new Promise(function(resolve, reject) {
    Module['onRuntimeInitialized'] = function() {
        const exports = {
            Clipper: Module.Clipper,
            Result: Module.Result,
            FillRule: Module.FillRule,
            PathType: Module.PathType,
            ClipType: Module.ClipType,
            Point: Module.Point,
            JoinType: Module.JoinType,
            EndType: Module.EndType,
            OffsetPaths: Module.OffsetPaths,
        };
        resolve(exports);
    }
});
