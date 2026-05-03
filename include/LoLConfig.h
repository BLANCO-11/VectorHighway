#pragma once

#include "Types.h"
#include "UAV.h"

struct LoLConfig {
    std::string lolProcedure = "LOITER_AND_RTH";
    double lolTimer = 0.0;
    bool lolActive = false;
    double lastTelemetryTimestamp = 0.0;

    void reset() {
        lolTimer = 0.0;
        lolActive = false;
    }
};
