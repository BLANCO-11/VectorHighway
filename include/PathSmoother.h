#pragma once

#include "Geo.h"
#include <vector>

// Catmull-Rom spline interpolation for path smoothing
namespace PathSmoother {

// Interpolate a Catmull-Rom curve through control points
// Returns (subdivisions * (n-1) + 1) points forming a C1-continuous curve 
std::vector<Coordinate> catmullRomInterpolate(
    const std::vector<Coordinate>& controlPoints,
    int subdivisions = 5
);

// Single segment interpolation between p1->p2 with tangents from p0 and p3
Coordinate catmullRomPoint(
    const Coordinate& p0, const Coordinate& p1,
    const Coordinate& p2, const Coordinate& p3,
    double t
);

} // namespace PathSmoother