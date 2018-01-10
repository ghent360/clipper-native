/*******************************************************************************
* Author    :  Venelin Efremov                                                 *
* Version   :  10.0 (beta)                                                     *
* Date      :  8 Noveber 2017                                                  *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Venelin Efremov 2017                                            *
* Purpose   :  Javascript interface for base clipping module                   *
* License   : http://www.boost.org/LICENSE_1_0.txt                             *
*******************************************************************************/

/*
    Note: when making changes to clipper_js.cpp these have to be reflected here as well.

    Javascript interface definitions:

      - Point: any type with properties x and y of type number;

      - Path: a collection of points describing the polygon. These can be open 
              or closed.

      - Paths: a collection of polygons describing a shape. Shapes have a fill-rule.
               The fill-rule describes which parts of the shape are to be colored.
               For examples see: https://en.wikipedia.org/wiki/Nonzero-rule
               or https://en.wikipedia.org/wiki/Even%E2%80%93odd_rule

      - PathType: when adding polygons to the Clipper you have to tell which set the
                  polygon belongs to. The final clipping operation is performed on the
                  polygon sets. For example if you have shape A and want to cut shape B
                  from it, you would put all polygons from shape A as ptSubject and all
                  polygons from shape B as ptClip.

                  Consider the clipping operation as something like:
                  ptSubject (op) ptClip

                  Where (op) is the operation code from ClipType.

      - PoinType: the last parameter of the execute method is a type of the objects we
                  should construct when computing the solution. The provided type has to
                  implement a constructor which takes two numbers (x and y coordinate).
                  If the PointType is not specified, we would create  tuples {x:number, y:number}.

      - Precision: the Clipper library operates internaly in integer coordinates. To convert
                   from JavaScript numbers, which are floating point, we provide a 
                   precisionMultiplier in the constructor. This means all input coordinates
                   would be multiplied by that number and all output coordinates would be
                   divided by that number when converted to floating point.
 */
export interface Point {
    readonly x: number;
    readonly y: number;
}

// These are C++ enums, but they appear as classes in JavaScriot
export declare class ClipType {
    static None:ClipType;
    static Intersection:ClipType;
    static Union:ClipType;
    static Difference:ClipType;
    static Xor:ClipType;
}

export declare class PathType {
    static Clip:PathType;
    static Subject:PathType;
}

export declare class FillRule {
    static EvenOdd:FillRule;
    static NonZero:FillRule;
    static Positive:FillRule;
    static Negative:FillRule;
}

export declare class JoinType {
    static Square:JoinType;
    static Round:JoinType;
    static Miter:JoinType;
}

export declare class EndType {
    static Polygon:EndType;
    static OpenJoined:EndType;
    static OpenButt:EndType;
    static OpenSquare:EndType;
    static OpenRound:EndType;
}

export declare interface Bounds {
    readonly minx:number;
    readonly miny:number;
    readonly maxx:number;
    readonly maxy:number;
}

export declare interface Result<T> {
    success:boolean;
    // The following two properties are defined if success is true.
    solution_closed?:Array<Array<T>>;
    solution_open?:Array<Array<T>>;
    bounds_closed?:Bounds;
    bounds_open?:Bounds;
}

export declare interface ResultArrays {
    success:boolean;
    // The following two properties are defined if success is true.
    solution_closed?:Array<PolygonArray>;
    solution_open?:Array<PolygonArray>;
    bounds_closed?:Bounds;
    bounds_open?:Bounds;
}

export type PolygonArray = Float64Array;

export declare class Clipper<T> {
    constructor(precisionMultiplier:number);

    addPath(polygon:Array<Point>, pathType:PathType, isOpen:boolean):void;
    addPaths(shape:Array<Array<Point>>, pathType:PathType, isOpen:boolean):void;
    addPathArray(polygon:PolygonArray, pathType:PathType, isOpen:boolean):void;
    addPathArrays(shape:Array<PolygonArray>, pathType:PathType, isOpen:boolean):void;
    executeOpenClosedToPoints(
        clipType:ClipType,
        fillRule:FillRule,
        pointType?:new(x:number, y:number) => T):Result<T>;
    executeClosedToPoints(
        clipType:ClipType,
        fillRule:FillRule,
        pointType?:new(x:number, y:number) => T):Result<T>;
    executeClosedToArrays(
        clipType:ClipType,
        fillRule:FillRule):ResultArrays;
            
    clear():void;
    delete():void;
}

export function OffsetPaths(
    input:Array<PolygonArray>,
    precisionMultiplier:number,
    offset:number,
    jointType:JoinType,
    endType:EndType):{solution:Array<PolygonArray>};

export interface ClipperSubModule {
    Clipper: typeof Clipper;
    FillRule: typeof FillRule;
    PathType: typeof PathType;
    ClipType: typeof ClipType;
    JoinType: typeof JoinType;
    EndType: typeof EndType;
    OffsetPaths: typeof OffsetPaths;
}
