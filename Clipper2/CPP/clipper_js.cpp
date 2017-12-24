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
#include <vector>

using emscripten::class_;
using emscripten::enum_;
using emscripten::select_overload;
using emscripten::val;


static bool slopesAreEqual(
    const clipperlib::Point64& pt1,
    const clipperlib::Point64& pt2,
    const clipperlib::Point64& pt3) {
    int64_t dx12 = pt1.x - pt2.x;
    int64_t dy12 = pt1.y - pt2.y;
    int64_t dx23 = pt2.x - pt3.x;
    int64_t dy23 = pt2.y - pt3.y;
    return (dy12 * dx23) == (dx12 * dy23);
}

static void removeDuplicatePoints(
    const clipperlib::Path& path,
    clipperlib::Path* result) {
    if (path.size() < 2) {
        *result = path;
        return;
    }
    const clipperlib::Point64* last = &path[0];
    result->clear();
    result->push_back(*last);
    for (size_t idx = 1; idx < path.size(); idx++) {
        if (path[idx] == *last) {
            continue;
        }
        last = &path[idx];
        result->push_back(*last);
    }
}

static void removeMidPoints(
    const clipperlib::Path& path,
    clipperlib::Path* result) {
    if (path.size() < 3) {
        *result = path;
        return;
    }
    result->clear();
    size_t lastIdx = path.size() - 1;
    if (!slopesAreEqual(path[lastIdx], path[0], path[1])) {
        result->push_back(path[0]);
    }
    for (size_t idx = 1; idx < lastIdx; idx++) {
        if (!slopesAreEqual(path[idx - 1], path[idx], path[idx+1])) {
            result->push_back(path[idx]);
        }
    }
    if (!slopesAreEqual(path[lastIdx - 1], path[lastIdx], path[0])) {
        result->push_back(path[lastIdx]);
    }
}

static void simplifyPolygon(
    const clipperlib::Path& path,
    clipperlib::Path* result) {
    clipperlib::Path tmp1;
    removeDuplicatePoints(path, &tmp1);
    clipperlib::Path tmp2;
    removeMidPoints(tmp1, &tmp2);
    removeDuplicatePoints(tmp2, result);
}

static void simplifyPolygonSet(
    const clipperlib::Paths& paths,
    clipperlib::Paths* result) {
    result->clear();
    for (auto path : paths) {
        clipperlib::Path simplified;
        simplifyPolygon(path, &simplified);
        result->push_back(simplified);
    }
}

static void connectWires(
    const clipperlib::Paths& polygonSet,
    clipperlib::Paths* result) {
    if (polygonSet.size() < 1) {
        *result = polygonSet;
        return;
    }
    const clipperlib::Point64* lastPt = &polygonSet[0].back();
    result->push_back(polygonSet[0]);
    for (size_t idx = 1; idx < polygonSet.size(); idx++) {
        if (polygonSet[idx].size() > 0) {
            const auto& polygon = polygonSet[idx];
            if (*lastPt == polygon[0]) {
                auto& last = result->back();
                last.insert(last.end(), polygon.begin() + 1, polygon.end());
                lastPt = &last.back();
            } else {
                result->push_back(polygon);
                lastPt = &polygon.back();
            }
        }
    }
}

class ClipperJS {
public:
    ClipperJS(double precision_multiplier)
        : precision_multiplier_(precision_multiplier) {}

    void AddPath(const val& v_path, clipperlib::PathType polytype, bool is_open);
    void AddPathArray(const val& v_path, clipperlib::PathType polytype, bool is_open);
    void AddPaths(const val& v_paths, clipperlib::PathType polytype, bool is_open);
    void AddPathArrays(const val& v_path, clipperlib::PathType polytype, bool is_open);
    val ExecuteOpenClosedToPoints(
        clipperlib::ClipType clip_type,
        clipperlib::FillRule fr,
        const val& point_type);
    val ExecuteClosedToPoints(
        clipperlib::ClipType clip_type,
        clipperlib::FillRule fr,
        const val& point_type);
    val ExecuteClosedToArrays(
        clipperlib::ClipType clip_type,
        clipperlib::FillRule fr);

    void Clear() { clipper_.Clear(); }

private:
    void PathsToJsArrayPoints(
        const clipperlib::Paths& solution,
        const val& point_type,
        val* v_solution);
    void copyToJSArrays(const clipperlib::Paths& solution, val* result);

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

static void copyToVector(const val &arr, std::vector<double> &vec)
{
    unsigned length = arr["length"].as<unsigned>();
    val memory = val::module_property("buffer");
    vec.resize(length);
    val memoryView = val::global("Float64Array").new_(
        memory, reinterpret_cast<uintptr_t>(vec.data()), length);
    memoryView.call<void>("set", arr);
}

void ClipperJS::AddPathArray(const val& v_path, clipperlib::PathType polytype, bool is_open) {
    std::vector<double> inputValues;
    copyToVector(v_path, inputValues);
    unsigned length = inputValues.size() / 2;
    std::vector<clipperlib::Point64> path(length);
    for (unsigned idx = 0; idx < length; idx++) {
        int64_t x = (int64_t)(inputValues[idx * 2] * precision_multiplier_);
        int64_t y = (int64_t)(inputValues[idx * 2 + 1] * precision_multiplier_);
        path[idx].x = x;
        path[idx].y = y;
    }
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

void ClipperJS::AddPathArrays(const val& v_paths, clipperlib::PathType polytype, bool is_open) {
    unsigned length = v_paths["length"].as<unsigned>();
    //printf("Addin %d paths\n", length);
    for (unsigned idx = 0; idx < length; idx++) {
        const val& path(v_paths[idx]);
        if (!path.isUndefined()) {
            AddPathArray(path, polytype, is_open);
        }
    }
}

void ClipperJS::PathsToJsArrayPoints(
    const clipperlib::Paths& solution, const val& point_type, val* v_solution) {
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
        v_solution->set(path_idx++, v_path);
    }
}

void ClipperJS::copyToJSArrays(const clipperlib::Paths& solution, val* result) {
    val memory = val::module_property("buffer");
    size_t path_idx = 0;
    for (auto path : solution) {
        std::vector<double> vec;
        vec.reserve(path.size() * 2);
        for (size_t idx = 0; idx < path.size(); idx++) {
            double x = (double)path[idx].x / precision_multiplier_;
            double y = (double)path[idx].y / precision_multiplier_;
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

val ClipperJS::ExecuteOpenClosedToPoints(
        clipperlib::ClipType clip_type,
        clipperlib::FillRule fr,
        const val& point_type) {
    clipperlib::Paths solution_closed;
    clipperlib::Paths solution_open;
    bool result = clipper_.Execute(clip_type, solution_closed, solution_open, fr);
    clipperlib::Paths closed_simplified;
    simplifyPolygonSet(solution_closed, &closed_simplified);
    clipperlib::Paths open_simplified;
    simplifyPolygonSet(solution_open, &open_simplified);
    val v_solution_closed(val::array());
    val v_solution_open(val::array());
    PathsToJsArrayPoints(closed_simplified, point_type, &v_solution_closed);
    PathsToJsArrayPoints(open_simplified, point_type, &v_solution_open);
    val v_result(val::object());
    v_result.set("success", result);
    v_result.set("solution_closed", v_solution_closed);
    v_result.set("solution_open", v_solution_open);

    return v_result;
}

val ClipperJS::ExecuteClosedToPoints(
        clipperlib::ClipType clip_type,
        clipperlib::FillRule fr,
        const val& point_type) {
    clipperlib::Paths solution_closed;
    bool result = clipper_.Execute(clip_type, solution_closed, fr);
    clipperlib::Paths closed_simplified;
    simplifyPolygonSet(solution_closed, &closed_simplified);
    val v_solution_closed(val::array());
    PathsToJsArrayPoints(closed_simplified, point_type, &v_solution_closed);
    val v_result(val::object());
    v_result.set("success", result);
    v_result.set("solution_closed", v_solution_closed);

    return v_result;
}

val ClipperJS::ExecuteClosedToArrays(
        clipperlib::ClipType clip_type,
        clipperlib::FillRule fr) {
    clipperlib::Paths solution_closed;
    bool result = clipper_.Execute(clip_type, solution_closed, fr);
    clipperlib::Paths closed_simplified;
    simplifyPolygonSet(solution_closed, &closed_simplified);
    val v_solution_closed(val::array());
    copyToJSArrays(closed_simplified, &v_solution_closed);
    val v_result(val::object());
    v_result.set("success", result);
    v_result.set("solution_closed", v_solution_closed);

    return v_result;
}

EMSCRIPTEN_BINDINGS(clipper_js)
{
    class_<ClipperJS>("Clipper")
        .constructor<double>()
        .function("addPath",  &ClipperJS::AddPath)
        .function("addPaths", &ClipperJS::AddPaths)
        .function("addPathArray",  &ClipperJS::AddPathArray)
        .function("addPathArrays", &ClipperJS::AddPathArrays)
        .function("executeOpenClosedToPoints", &ClipperJS::ExecuteOpenClosedToPoints)
        .function("executeClosedToPoints", &ClipperJS::ExecuteClosedToPoints)
        .function("executeClosedToArrays", &ClipperJS::ExecuteClosedToArrays)
        .function("clear", &ClipperJS::Clear)
        ;
    enum_<clipperlib::ClipType>("ClipType")
        .value("None", clipperlib::ctNone)
        .value("Intersection", clipperlib::ctIntersection)
        .value("Union", clipperlib::ctUnion)
        .value("Difference", clipperlib::ctDifference)
        .value("Xor", clipperlib::ctXor)
        ;
    enum_<clipperlib::PathType>("PathType")
        .value("Clip", clipperlib::ptClip)
        .value("Subject", clipperlib::ptSubject)
        ;
    enum_<clipperlib::FillRule>("FillRule")
        .value("EvenOdd", clipperlib::frEvenOdd)
        .value("NonZero", clipperlib::frNonZero)
        .value("Positive", clipperlib::frPositive)
        .value("Negative", clipperlib::frNegative)
        ;
}
