#include <catch2/catch_test_macros.hpp>
#include "../include/Geo.h"
#include "../include/Environment.h"
#include "../include/Pathfinder.h"
#include "../include/UAV.h"
#include "../include/Types.h"
#include "../include/CommandEvent.h"
#include "../include/SimulationContext.h"
#include <cmath>
#include <limits>

// --- Geo Utility Tests (T025) ---

TEST_CASE("Coordinate validation: latitude range") {
    Coordinate c(45.0, 10.0, 0.0);
    REQUIRE(c.latitude == 45.0);
    REQUIRE(c.longitude == 10.0);

    Coordinate c2(-90.0, -180.0, 0.0);
    REQUIRE(c2.latitude == -90.0);
    REQUIRE(c2.longitude == -180.0);
}

TEST_CASE("Coordinate distance calculation") {
    Coordinate paris(48.8566, 2.3522, 0.0);
    Coordinate london(51.5074, -0.1278, 0.0);
    double dist = paris.distanceTo(london);
    REQUIRE(dist > 300.0);
    REQUIRE(dist < 400.0);
}

TEST_CASE("Coordinate distance to self is zero") {
    Coordinate c(10.0, 20.0, 0.0);
    REQUIRE(c.distanceTo(c) == 0.0);
}

TEST_CASE("Coordinate bearing calculation") {
    Coordinate from(0.0, 0.0, 0.0);
    Coordinate north(10.0, 0.0, 0.0);
    double bearing = from.bearingTo(north);
    REQUIRE(bearing == 0.0);
}

TEST_CASE("Vector3 operations") {
    Vector3 a(1.0, 2.0, 3.0);
    Vector3 b(4.0, 5.0, 6.0);
    Vector3 sum = a + b;
    REQUIRE(sum.x == 5.0);
    REQUIRE(sum.y == 7.0);
    REQUIRE(sum.z == 9.0);

    Vector3 diff = b - a;
    REQUIRE(diff.x == 3.0);
    REQUIRE(diff.y == 3.0);
    REQUIRE(diff.z == 3.0);

    Vector3 scaled = a * 2.0;
    REQUIRE(scaled.x == 2.0);
    REQUIRE(scaled.y == 4.0);
    REQUIRE(scaled.z == 6.0);
}

TEST_CASE("Vector3 magnitude and normalization") {
    Vector3 v(3.0, 4.0, 0.0);
    REQUIRE(v.magnitude() == 5.0);

    Vector3 n = v.normalized();
    REQUIRE(std::abs(n.magnitude() - 1.0) < 0.0001);
}

// --- Pathfinder Tests (T026) ---

TEST_CASE("A* pathfinding finds direct path") {
    GlobalPathfinder pf(2000.0);
    int start = pf.addNode(Coordinate(48.8566, 2.3522, 0.0), "Paris");
    int goal = pf.addNode(Coordinate(51.5074, -0.1278, 0.0), "London");
    pf.buildGraph();

    auto path = pf.findPathAStar(start, goal);
    REQUIRE(path.size() == 2);
    REQUIRE(path[0].name == "Paris");
    REQUIRE(path[1].name == "London");
}

TEST_CASE("A* pathfinding returns empty for unreachable") {
    GlobalPathfinder pf(1.0);
    int a = pf.addNode(Coordinate(0.0, 0.0, 0.0), "A");
    int b = pf.addNode(Coordinate(50.0, 50.0, 0.0), "B");
    pf.buildGraph();

    auto path = pf.findPathAStar(a, b);
    REQUIRE(path.empty());
}

TEST_CASE("Local avoidance detects obstacle in path") {
    LocalAvoidance avoidance(10.0, 60.0);
    Coordinate hostPos(48.0, 2.0, 0.0);
    StaticObstacle obs(Coordinate(48.03, 2.02, 0.0), 1.0);

    bool detected = avoidance.isObstacleInPath(hostPos, 45.0, &obs);
    REQUIRE(detected == true);
}

TEST_CASE("Local avoidance ignores obstacle out of range") {
    LocalAvoidance avoidance(10.0, 60.0);
    Coordinate hostPos(48.0, 2.0, 0.0);
    StaticObstacle obs(Coordinate(55.0, 10.0, 0.0), 1.0);

    bool detected = avoidance.isObstacleInPath(hostPos, 45.0, &obs);
    REQUIRE(detected == false);
}

// --- UAV Physics Tests (T027) ---

TEST_CASE("UAV initial state") {
    UAV uav(Coordinate(48.0, 2.0, 0.0), 90.0, 0.25, 90.0, "alpha");
    REQUIRE(uav.groupId == "alpha");
    REQUIRE(uav.heading == 90.0);
    REQUIRE(uav.batteryLevel == 100.0);
    REQUIRE(uav.currentSpeed == 0.0);
}

TEST_CASE("UAV heading adjustment: small diff") {
    UAV uav(Coordinate(48.0, 2.0, 0.0), 90.0, 0.25, 90.0, "alpha");
    uav.adjustHeading(95.0, 1.0);
    REQUIRE(uav.heading == 95.0);
}

TEST_CASE("UAV heading adjustment: limited by turn rate") {
    UAV uav(Coordinate(48.0, 2.0, 0.0), 0.0, 0.25, 90.0, "alpha");
    uav.adjustHeading(180.0, 1.0);
    REQUIRE(uav.heading == 90.0);
}

TEST_CASE("UAV heading wraps correctly") {
    UAV uav(Coordinate(48.0, 2.0, 0.0), 350.0, 0.25, 90.0, "alpha");
    uav.adjustHeading(10.0, 1.0);
    REQUIRE(uav.heading == 10.0);
}

TEST_CASE("UAV battery drains over time") {
    UAV uav(Coordinate(48.0, 2.0, 0.0), 0.0, 0.25, 90.0, "alpha");
    uav.update(10.0, 0.0);
    REQUIRE(uav.batteryLevel < 100.0);
    REQUIRE(uav.batteryLevel >= 0.0);
}

TEST_CASE("UAV movement changes position") {
    UAV uav(Coordinate(48.0, 2.0, 0.0), 45.0, 0.25, 90.0, "alpha");
    Coordinate before = uav.position;
    uav.targetSpeed = 0.25;
    uav.update(10.0, 0.0);
    Coordinate after = uav.position;
    double dist = before.distanceTo(after);
    REQUIRE(dist > 0.0);
}

TEST_CASE("CommandEvent struct construction") {
    CommandEvent cmd;
    cmd.type = "target";
    cmd.targetId = "all";
    cmd.lat = 48.8566;
    cmd.lon = 2.3522;
    REQUIRE(cmd.type == "target");
    REQUIRE(cmd.targetId == "all");
    REQUIRE(cmd.lat == 48.8566);
}

TEST_CASE("SimulationContext command queue") {
    SimulationContext ctx;
    CommandEvent cmd;
    cmd.type = "spawn";
    cmd.groupId = "alpha";
    ctx.pushCommand(cmd);

    auto cmds = ctx.drainCommands();
    REQUIRE(cmds.size() == 1);
    REQUIRE(cmds[0].type == "spawn");

    auto emptyCmds = ctx.drainCommands();
    REQUIRE(emptyCmds.empty());
}

TEST_CASE("SimulationContext external telemetry") {
    SimulationContext ctx;
    ExternalTelemetry ext;
    ext.id = "ext_1";
    ext.lat = 51.5;
    ext.lon = -0.12;
    ctx.pushExternalTelemetry(ext);

    auto telemetry = ctx.drainExternalTelemetry();
    REQUIRE(telemetry.size() == 1);
    REQUIRE(telemetry[0].id == "ext_1");

    auto empty = ctx.drainExternalTelemetry();
    REQUIRE(empty.empty());
}

TEST_CASE("CoordinateConversion spherical to cartesian") {
    Coordinate coord(0.0, 0.0, 0.0);
    Vector3 cart = CoordinateConversion::sphericalToCartesian(coord);
    REQUIRE(std::abs(cart.x) > 0.0);
}

TEST_CASE("CoordinateConversion cartesian to spherical") {
    Vector3 cart(6371.0, 0.0, 0.0);
    Coordinate coord = CoordinateConversion::cartesianToSpherical(cart);
    REQUIRE(std::abs(coord.latitude) < 0.001);
    REQUIRE(std::abs(coord.longitude) < 0.001);
}

TEST_CASE("Types.h constants") {
    REQUIRE(EARTH_RADIUS_KM == 6371.0);
    REQUIRE(SIMULATION_TICK_HZ == 20.0);
    REQUIRE(DEFAULT_PORT == 7200);
    REQUIRE(STALE_DRONE_TIMEOUT_S == 30.0);
}