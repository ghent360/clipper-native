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

    void AddPath2(const val& v_path, clipperlib::PathType polytype) {
        AddPath(v_path, polytype, false);
    }
    void AddPath(const val& v_path, clipperlib::PathType polytype, bool is_open);

    void AddPaths2(const val& v_paths, clipperlib::PathType polytype) {
        AddPaths(v_paths, polytype, false);
    }
    void AddPaths(const val& v_paths, clipperlib::PathType polytype, bool is_open);

    val Execute1(
        clipperlib::ClipType clip_type) {
        return Execute(clip_type, clipperlib::frEvenOdd, val::undefined());
    }

    val Execute(
        clipperlib::ClipType clip_type,
        clipperlib::FillRule fr,
        const val& point_type);

    void Clear() { clipper_.Clear(); }

private:
    void PathsToJsArray(const clipperlib::Paths& solution, val& v_solution, const val& point_type);

    clipperlib::Clipper clipper_;
    double precision_multiplier_;
};

void ClipperJS::AddPath(const val& v_path, clipperlib::PathType polytype, bool is_open) {
    unsigned length = v_path["length"].as<unsigned>();
    std::vector<clipperlib::Point64> path(length);
    for (unsigned idx = 0; idx < length; idx++) {
        int64_t x = (int64_t) (v_path[idx]["x"].as<double>() * precision_multiplier_);
        int64_t y = (int64_t) (v_path[idx]["y"].as<double>() * precision_multiplier_);
        path[idx].x = x;
        path[idx].y = y;
        //printf("[%lld, %lld] ", x, y);
    }
    //printf("\n");
    clipper_.AddPath(path, polytype, is_open);
}

void ClipperJS::AddPaths(const val& v_paths, clipperlib::PathType polytype, bool is_open) {
    unsigned length = v_paths["length"].as<unsigned>();
    //printf("Addin %d paths\n", length);
    for (unsigned idx = 0; idx < length; idx++) {
        const val& path(v_paths[idx]);
        if (!path.isUndefined()) {
            AddPath(path, polytype, is_open);
        }
    }
}

void ClipperJS::PathsToJsArray(
    const clipperlib::Paths& solution, val& v_solution, const val& point_type) {
    size_t path_idx = 0;
    for (auto path : solution) {
        val v_path(val::array());
        size_t idx = 0;
        for (auto point : path) {
            double x = point.x / precision_multiplier_;
            double y = point.y / precision_multiplier_;
            if (point_type.isUndefined()) {
                val v_point(val::object());
                v_point.set("x", x);
                v_point.set("y", y);
                v_path.set(idx++, v_point);
            } else {
                val v_point(point_type.new_(x, y));
                v_path.set(idx++, v_point);
            }
        }
        v_solution.set(path_idx++, v_path);
    }
}

val ClipperJS::Execute(
        clipperlib::ClipType clip_type,
        clipperlib::FillRule fr,
        const val& point_type) {
    clipperlib::Paths solution_closed;
    clipperlib::Paths solution_open;
    bool result = clipper_.Execute(clip_type, solution_closed, solution_open, fr);
    val v_solution_closed(val::array());
    val v_solution_open(val::array());
    PathsToJsArray(solution_closed, v_solution_closed, point_type);
    PathsToJsArray(solution_open, v_solution_open, point_type);
    val v_result(val::object());
    v_result.set("success", result);
    v_result.set("solution_closed", v_solution_closed);
    v_result.set("solution_open", v_solution_open);

    return v_result;
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
        .function("Execute1", &ClipperJS::Execute1)
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
