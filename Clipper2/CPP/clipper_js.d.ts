import * as cl from "./clipper.js";

interface SubModule {
    Clipper: typeof cl.Clipper;
    FillRule: typeof cl.FillRule;
    PathType: typeof cl.PathType;
    ClipType: typeof cl.ClipType;
}

declare const onLoad:Promise<SubModule>;

export = onLoad;