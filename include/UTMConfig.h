#pragma once

#include "Types.h"
#include <string>
#include <vector>

struct UTMConfig {
    struct Corridor {
        std::vector<std::pair<double, double>> centerline;
        double widthKm = 2.0;
        bool active = true;
    };

    std::vector<Corridor> corridors;

    bool isInCorridor(double lat, double lon, const Corridor& corridor) const;
    double distanceToCorridorEdge(double lat, double lon, const Corridor& corridor) const;
};
