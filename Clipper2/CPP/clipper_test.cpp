#include <stdio.h>
#include "clipper.h"

int main() {
    clipperlib::Clipper c;
    clipperlib::Path p1;
    p1.push_back(clipperlib::Point64(0, 0));
    p1.push_back(clipperlib::Point64(10, 0));
    p1.push_back(clipperlib::Point64(10, 20));
    p1.push_back(clipperlib::Point64(0, 20));
    p1.push_back(clipperlib::Point64(0, 0));
    clipperlib::Path p2;
    p1.push_back(clipperlib::Point64(0, 0));
    p1.push_back(clipperlib::Point64(20, 0));
    p1.push_back(clipperlib::Point64(20, 10));
    p1.push_back(clipperlib::Point64(0, 10));
    p1.push_back(clipperlib::Point64(0, 0));

    std::vector<clipperlib::Path> ps1;
    ps1.push_back(p1);
    std::vector<clipperlib::Path> ps2;
    ps2.push_back(p2);
    c.AddPaths(ps1, clipperlib::ptSubject);
    c.AddPaths(ps2, clipperlib::ptClip);
    clipperlib::Paths solution;
    bool result = c.Execute(clipperlib::ctUnion, solution, clipperlib::frNonZero);
    printf("result = %d\n", result);
    printf("size = %ld\n", solution[0].size());
    for (auto poly : solution) {
        for (auto pt : poly) {
            printf("(%ld, %ld),", (long)pt.x, (long)pt.y);
        }
        printf("\n");
    }
    return 0;
}
