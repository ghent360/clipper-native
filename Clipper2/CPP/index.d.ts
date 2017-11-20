export interface Point {
    readonly x: number;
    readonly y: number;
}

export declare class ClipType {
    static ctNone:ClipType;
    static ctIntersection:ClipType;
    static ctUnion:ClipType;
    static ctDifference:ClipType;
    static ctXor:ClipType;
}

export declare class PathType {
    static ptClip:PathType;
    static ptSubject:PathType;
}

export declare class FillRule {
    static frEvenOdd:FillRule;
    static frNonZero:FillRule;
    static frPositive:FillRule;
    static frNegative:FillRule;
}

export declare interface Result<T> {
    success:boolean;
    solution_closed?:Array<Array<T>>;
    solution_open?:Array<Array<T>>;
}

export declare class Clipper<T> {
    constructor(precisionMultiplier:number);

    addPath(path:Array<Point>, pathtype:PathType, is_open:boolean):void;
    addPaths(paths:Array<Array<Point>>, pathtype:PathType, is_open:boolean):void;
    execute(
        clipType:ClipType,
        fillRule:FillRule,
        pointType:new(x:number, y:number) => T):Result<T>;
    
    clear():void;
    delete():void;
}
