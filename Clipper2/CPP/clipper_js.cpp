#include "clipper.h"
#include <emscripten/bind.h>

using emscripten::val;

class ClipperJS {
public:
    ClipperJS(double precision_multiplier)
        : precision_multiplier_(precision_multiplier) {}

    void AddPath(val v_path, val v_polytype);
    void AddPaths(val v_paths, val v_polytype);

    bool Execute(
        val clipType,
        val solution_closed,
        val fr);

    void Clear() { clipper_.Clear(); }

private:
    clipperlib::Clipper clipper_;
    double precision_multiplier_;
};

void ClipperJS::AddPath(val v_path, val v_polytype) {
    unsigned length = v_path["length"].as<unsigned>();
    std::vector<clipperlib::Point64> path(length);
    for (unsigned idx = 0; idx < length; idx++) {
        int64_t x = (int64_t) (v_path[idx]["x"].as<double>() * precision_multiplier_);
        int64_t y = (int64_t) (v_path[idx]["y"].as<double>() * precision_multiplier_);
        path[idx].x = x;
        path[idx].y = y;
        printf("P(%lld, %lld) ", x, y);
    }
    printf("\n");
    clipper_.AddPath(path, clipperlib::PathType(v_polytype.as<int>()));
}

void ClipperJS::AddPaths(val v_paths, val v_polytype) {
    unsigned length = v_paths["length"].as<unsigned>();
    for (unsigned idx = 0; idx < length; idx++) {
        AddPath(v_paths[idx], v_polytype);
    }
}

bool ClipperJS::Execute(
        val v_clipType,
        val v_solution,
        val v_fr) {
    clipperlib::Paths solution;
    bool result = clipper_.Execute(
        clipperlib::ClipType(v_clipType.as<int>()),
        solution,
        clipperlib::FillRule(v_fr.as<int>()));
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
        v_solution[path_idx++] = v_path;
    }
    return result;
}

EMSCRIPTEN_BINDINGS(clipper_js)
{
    emscripten::class_<ClipperJS>("ClipperJS")
    .constructor<double>()
    .function("AddPath", &ClipperJS::AddPath)
    .function("AddPaths", &ClipperJS::AddPaths)
    .function("Execute", &ClipperJS::Execute)
    .function("Clear", &ClipperJS::Clear)
    ;
}