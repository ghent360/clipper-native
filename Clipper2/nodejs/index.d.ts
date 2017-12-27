/*******************************************************************************
* Author    :  Venelin Efremov                                                 *
* Version   :  10.0 (beta)                                                     *
* Date      :  26 December 2017                                                  *
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
export type ClipType = "none" | "intersection" | "int" | "union" | "difference" | "diff" | "xor";
export type PathType = "subject" | "clip";
export type FillRule = "evenodd" | "even-odd" | "nonzero" | "non-zero" | "positive" | "negative";

/*
    A Path is a collection of points. This can be specified in the following ways:
    Array<Point> - an array of objects with properties x and y of type number;
    Array<number> - an array of numbers where x coordinates are folowed by y coordinates. Length must be even.
    Float64Array, Float32Array and Int32Array have the same representation as Array<number> but could be 
    more efficient
 */
export type Path = Array<Point>| Array<number> | Float64Array | Float32Array | Int32Array;

/*
    The result of the operation is given as an array of Path in the Float64Array format.
 */
export declare interface Result {
    success:boolean;
    // The following property is defined if success is true.
    solution_closed?:Array<Float64Array>;
}

export declare class Clipper<T> {
    constructor(precisionMultiplier:number);

    addPath(polygon:Path, pathType:PathType, isOpen?:boolean = false):void;
    addPaths(shape:Array<Path>, pathType:PathType, isOpen?:boolean = false):void;
    execute(clipType:ClipType, fillRule?:FillRule = "even-odd"):Result;
    clear():void;
    precision():number;
}
