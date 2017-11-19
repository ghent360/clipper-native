/*******************************************************************************
* Author    :  Venelin Efremov                                                 *
* Version   :  10.0 (beta)                                                     *
* Date      :  8 Noveber 2017                                                  *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Venelin Efremov 2017                                            *
* Purpose   :  Javascript interface for base clipping module                   *
* License   : http://www.boost.org/LICENSE_1_0.txt                             *
*******************************************************************************/

#include "clipper.h"
#include <emscripten/bind.h>

using emscripten::class_;
using emscripten::enum_;
using emscripten::select_overload;
using emscripten::val;

class ClipperJS {
public:
    ClipperJS(double precision_multiplier)
        : precision_multiplier_(precision_multiplier) {}

    void AddPath2(val v_path, clipperlib::PathType polytype) {
        AddPath(v_path, polytype, false);
    }
    void AddPath(val v_path, clipperlib::PathType polytype, bool is_open);

    void AddPaths2(val v_paths, clipperlib::PathType polytype) {
        AddPaths(v_paths, polytype, false);
    }
    void AddPaths(val v_paths, clipperlib::PathType polytype, bool is_open);

    bool Execute2(
        clipperlib::ClipType clip_type,
        val solution_closed) {
        return Execute(clip_type, solution_closed, clipperlib::frEvenOdd);
    }

    bool Execute(
        clipperlib::ClipType clip_type,
        val solution_closed,
        clipperlib::FillRule fr);

    bool ExecuteClosedOpen3(
        clipperlib::ClipType clip_type,
        val solution_closed,
        val solution_open) {
        return ExecuteClosedOpen(clip_type, solution_closed, solution_open, clipperlib::frEvenOdd);
    }

    bool ExecuteClosedOpen(
        clipperlib::ClipType clip_type,
        val solution_closed,
        val solution_open,
        clipperlib::FillRule fr);

    void Clear() { clipper_.Clear(); }

private:
    void PathsToJsArray(const clipperlib::Paths& solution, val* v_solution);

    clipperlib::Clipper clipper_;
    double precision_multiplier_;
};

void ClipperJS::AddPath(val v_path, clipperlib::PathType polytype, bool is_open) {
    unsigned length = v_path["length"].as<unsigned>();
    std::vector<clipperlib::Point64> path(length);
    for (unsigned idx = 0; idx < length; idx++) {
        int64_t x = (int64_t) (v_path[idx]["x"].as<double>() * precision_multiplier_);
        int64_t y = (int64_t) (v_path[idx]["y"].as<double>() * precision_multiplier_);
        path[idx].x = x;
        path[idx].y = y;
    }
    clipper_.AddPath(path, polytype, is_open);
}

void ClipperJS::AddPaths(val v_paths, clipperlib::PathType polytype, bool is_open) {
    unsigned length = v_paths["length"].as<unsigned>();
    for (unsigned idx = 0; idx < length; idx++) {
        AddPath(v_paths[idx], polytype, is_open);
    }
}

void ClipperJS::PathsToJsArray(
    const clipperlib::Paths& solution, val* v_solution) {
    size_t path_idx = 0;
    for (auto path : solution) {
        val v_path(val::array());
        size_t idx = 0;
        for (auto point : path) {
            val v_point(val::object());
            v_point.set("x", val(point.x / precision_multiplier_));
            v_point.set("y", val(point.x / precision_multiplier_));
            v_path[idx++] = v_point;
        }
        (*v_solution)[path_idx++] = v_path;
    }
}

bool ClipperJS::Execute(
        clipperlib::ClipType clip_type,
        val v_solution,
        clipperlib::FillRule fr) {
    clipperlib::Paths solution;
    bool result = clipper_.Execute(clip_type, solution, fr);
    PathsToJsArray(solution, &v_solution);
    return result;
}

bool ClipperJS::ExecuteClosedOpen(
        clipperlib::ClipType clip_type,
        val v_solution_closed,
        val v_solution_open,
        clipperlib::FillRule fr) {
    clipperlib::Paths solution_closed;
    clipperlib::Paths solution_open;
    bool result = clipper_.Execute(clip_type, solution_closed, solution_open, fr);
    PathsToJsArray(solution_closed, &v_solution_closed);
    PathsToJsArray(solution_open, &v_solution_open);
    return result;
}

EMSCRIPTEN_BINDINGS(clipper_js)
{
    class_<ClipperJS>("Clipper")
        .constructor<double>()
        .function("AddPath2",  &ClipperJS::AddPath2)
        .function("AddPath",  &ClipperJS::AddPath)
        .function("AddPaths2", &ClipperJS::AddPaths2)
        .function("AddPaths", &ClipperJS::AddPaths)
        .function("Execute", &ClipperJS::Execute)
        .function("Execute2", &ClipperJS::Execute2)
        .function("ExecuteClosedOpen", &ClipperJS::ExecuteClosedOpen)
        .function("ExecuteClosedOpen3", &ClipperJS::ExecuteClosedOpen3)
        .function("Clear", &ClipperJS::Clear)
        ;
    enum_<clipperlib::ClipType>("ClipType")
        .value("ctNone", clipperlib::ctNone)
        .value("ctIntersection", clipperlib::ctIntersection)
        .value("ctUnion", clipperlib::ctUnion)
        .value("ctDifference", clipperlib::ctDifference)
        .value("ctXor", clipperlib::ctXor)
        ;
    enum_<clipperlib::PathType>("PathType")
        .value("ptClip", clipperlib::ptClip)
        .value("ptSubject", clipperlib::ptSubject)
        ;
    enum_<clipperlib::FillRule>("FillRule")
        .value("frEvenOdd", clipperlib::frEvenOdd)
        .value("frNonZero", clipperlib::frNonZero)
        .value("frPositive", clipperlib::frPositive)
        .value("frNegative", clipperlib::frNegative)
        ;
}
