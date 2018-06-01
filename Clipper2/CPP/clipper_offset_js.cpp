/*******************************************************************************
* Author    :  Venelin Efremov                                                 *
* Version   :  10.0 (beta)                                                     *
* Date      :  8 Noveber 2017                                                  *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Venelin Efremov 2017                                            *
* Purpose   :  Javascript interface for base clipping module                   *
* License   : http://www.boost.org/LICENSE_1_0.txt                             *
*******************************************************************************/

#include "clipper_offset.h"
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <limits>
#include <vector>

using emscripten::class_;
using emscripten::enum_;
using emscripten::select_overload;
using emscripten::val;
using clipperlib::Point64;
using clipperlib::Path;
using clipperlib::Paths;
using clipperlib::EndType;
using clipperlib::JoinType;
using clipperlib::ClipperOffset;

static void CopyToVector(const val &arr, std::vector<double> &vec)
{
    unsigned length = arr["length"].as<unsigned>();
    val memory = val::module_property("buffer");
    vec.resize(length);
    val memoryView = val::global("Float64Array").new_(
        memory, reinterpret_cast<uintptr_t>(vec.data()), length);
    memoryView.call<void>("set", arr);
}

static void CopyToJSArrays(
    const Paths& solution,
    val* result,
    //Bounds* bounds,
    double precision_multiplier) {
    val memory = val::module_property("buffer");
    size_t path_idx = 0;
    for (auto path : solution) {
        std::vector<double> vec;
        vec.reserve(path.size() * 2);
        for (size_t idx = 0; idx < path.size(); idx++) {
            double x = (double)path[idx].x / precision_multiplier;
            double y = (double)path[idx].y / precision_multiplier;
            //bounds->CompareX(x);
            //bounds->CompareY(y);
            vec[2 * idx] = x;
            vec[2 * idx + 1] = y;
        }
        val memoryView = val::global("Float64Array").new_(
            memory, reinterpret_cast<uintptr_t>(vec.data()), path.size() * 2);
        val pathCopy = val::global("Float64Array").new_(path.size() * 2);
        pathCopy.call<void>("set", memoryView);
        result->set(path_idx++, pathCopy);
    }
}

static val OffsetPathsJS(
    const val& v_paths,
    double precision_multiplier,
    double delta,
    JoinType jt,
    EndType et) {
    ClipperOffset co;

    unsigned length = v_paths["length"].as<unsigned>();
    for (unsigned idx = 0; idx < length; idx++) {
        const val& v_path(v_paths[idx]);
        if (!v_path.isUndefined()) {
            std::vector<double> inputValues;
            CopyToVector(v_path, inputValues);
            unsigned length = inputValues.size() / 2;
            std::vector<Point64> path(length);
            for (unsigned idx = 0; idx < length; idx++) {
                int64_t x = (int64_t)(inputValues[idx * 2] * precision_multiplier);
                int64_t y = (int64_t)(inputValues[idx * 2 + 1] * precision_multiplier);
                path[idx].x = x;
                path[idx].y = y;
            }
            co.AddPath(path, jt, et);
        }
    }
    Paths paths_out;
    co.Execute(paths_out, delta * precision_multiplier);
    val v_result(val::object());
    //Paths simplified;
    //simplifyPolygonSet(paths_out, &simplified);
    val v_solution(val::array());
    //Bounds bounds;
    CopyToJSArrays(paths_out, &v_solution, precision_multiplier);
    v_result.set("solution", v_solution);
    //v_result.set("bounds", bounds.ToObject());
    return v_result;
}

EMSCRIPTEN_BINDINGS(clipper_offset_js)
{
    function("OffsetPaths", &OffsetPathsJS);
    enum_<JoinType>("JoinType")
        .value("Square", clipperlib::kSquare)
        .value("Round", clipperlib::kRound)
        .value("Miter", clipperlib::kMiter)
        ;
    enum_<EndType>("EndType")
        .value("Polygon", clipperlib::kPolygon)
        .value("OpenJoined", clipperlib::kOpenJoined)
        .value("OpenButt", clipperlib::kOpenButt)
        .value("OpenSquare", clipperlib::kOpenSquare)
        .value("OpenRound", clipperlib::kOpenRound)
        ;
}
