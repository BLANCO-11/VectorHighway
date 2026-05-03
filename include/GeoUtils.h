#pragma once

#include "Geo.h"
#include "Types.h"
#include <vector>
#include <cmath>
#include <algorithm>

double toRadians(double degrees);
double toDegrees(double radians);

bool pointInPolygon(const Coordinate& point, const std::vector<Coordinate>& polygon);
bool pointInCircle(const Coordinate& point, const Coordinate& center, double radiusKm);

double polygonArea(const std::vector<Coordinate>& polygon);
double haversineDistance(double lat1, double lon1, double lat2, double lon2);

double crossTrackDistance(const Coordinate& point, const Coordinate& segmentStart,
                          const Coordinate& segmentEnd);

Coordinate midpoint(const Coordinate& a, const Coordinate& b);

double elevationAt(const Coordinate& point);
double interpolateBearing(double bearing1, double bearing2, double fraction);

bool segmentsIntersect(const Coordinate& a1, const Coordinate& a2,
                       const Coordinate& b1, const Coordinate& b2);
