#include <catch2/catch_test_macros.hpp>
#include "../include/Geo.h"
#include "../include/Environment.h"
#include "../include/Pathfinder.h"
#include "../include/UAV.h"
#include "../include/Types.h"
#include "../include/CommandEvent.h"
#include "../include/SimulationContext.h"
#include "../include/Planner.h"
#include "../include/Mission.h"
#include "../include/FlightRecorder.h"
#include "../include/HealthMonitor.h"
#include "../include/GeoUtils.h"
#include "../include/MirrorContext.h"
#include "../include/Payload.h"
#include "../include/LinkManager.h"
#include "../include/AuthManager.h"
#include "../include/TenantManager.h"
#include "../include/WeatherAPI.h"
#include <cmath>
#include <limits>
#include <memory>

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

// --- Mission Tests (T135) ---

TEST_CASE("Mission initial state") {
    Mission m("mission_1", "alpha_1");
    REQUIRE(m.id == "mission_1");
    REQUIRE(m.droneId == "alpha_1");
    REQUIRE(m.state == MissionState::IDLE);
    REQUIRE(m.waypoints.empty());
}

TEST_CASE("Mission state transitions") {
    Mission m("m1", "d1");
    REQUIRE(m.state == MissionState::IDLE);

    m.state = MissionState::UPLOADED;
    m.start();
    REQUIRE(m.state == MissionState::IN_PROGRESS);

    m.abort();
    REQUIRE(m.state == MissionState::ABORTED);
}

TEST_CASE("Mission waypoint advancement") {
    Mission m("m1", "d1");
    MissionWaypoint wp1, wp2;
    wp1.coordinate = Coordinate(48.0, 2.0, 0.0);
    wp2.coordinate = Coordinate(49.0, 3.0, 0.0);
    m.waypoints = {wp1, wp2};
    m.state = MissionState::UPLOADED;
    m.currentWaypointIndex = 0;

    m.start();
    REQUIRE(m.state == MissionState::IN_PROGRESS);
    REQUIRE(m.currentWaypointIndex == 0);

    m.advanceWaypoint();
    REQUIRE(m.currentWaypointIndex == 1);

    m.advanceWaypoint();
    REQUIRE(m.state == MissionState::COMPLETED);
}

// --- Planner Tests (T134) ---

TEST_CASE("Planner smooth path returns input for single segment") {
    Planner planner;
    std::vector<Coordinate> waypoints = {Coordinate(48.0, 2.0, 0.0), Coordinate(49.0, 3.0, 0.0)};
    std::vector<std::shared_ptr<Obstacle>> obstacles;
    auto result = planner.smoothPath(waypoints, obstacles);
    REQUIRE(result.size() == 2);
}

TEST_CASE("Planner detects segment-obstacle intersection") {
    Planner planner;
    Coordinate a(0.0, 0.0, 0.0);
    Coordinate b(1.0, 1.0, 0.0);
    auto obs = std::make_shared<StaticObstacle>(Coordinate(0.5, 0.5, 0.0), 0.3);
    bool intersects = planner.segmentIntersectsObstacle(a, b, obs);
    REQUIRE(intersects);
}

TEST_CASE("Planner ignores distant obstacle") {
    Planner planner;
    Coordinate a(0.0, 0.0, 0.0);
    Coordinate b(1.0, 1.0, 0.0);
    auto obs = std::make_shared<StaticObstacle>(Coordinate(10.0, 10.0, 0.0), 0.3);
    bool intersects = planner.segmentIntersectsObstacle(a, b, obs);
    REQUIRE(!intersects);
}

// --- HealthMonitor Tests (T136) ---

TEST_CASE("HealthMonitor engine tick rate check") {
    HealthMonitor hm;
    std::map<std::string, DroneHealth> drones;
    DroneHealth dh;
    dh.batteryLevel = 50.0;
    dh.connectionStatus = ConnectionStatus::CONNECTED;
    drones["alpha_1"] = dh;

    auto status = hm.check(20.0, 1, 0.0, drones);
    REQUIRE(status.engineTickHz == 20.0);
}

TEST_CASE("HealthMonitor generates alerts for low battery") {
    HealthMonitor hm;
    std::map<std::string, DroneHealth> drones;
    DroneHealth dh;
    dh.batteryLevel = 5.0;
    dh.connectionStatus = ConnectionStatus::CONNECTED;
    drones["alpha_1"] = dh;

    auto status = hm.check(20.0, 1, 0.0, drones);
    REQUIRE(status.alerts.size() > 0);
}

// --- FlightRecorder Tests (T137) ---

TEST_CASE("FlightRecorder start/stop") {
    FlightRecorder fr;
    bool started = fr.startRecording("/tmp");
    REQUIRE(started);
    REQUIRE(fr.isRecording());
    fr.stopRecording();
    REQUIRE_FALSE(fr.isRecording());
}

TEST_CASE("FlightRecorder writes and reads entries") {
    FlightRecorder fr;
    fr.startRecording("/tmp");
    fr.writeEntry(1000.0, "alpha_1", 48.0, 2.0, 0.5, 90.0, 0.25, 95.0, "IN_PROGRESS");
    fr.stopRecording();

    auto filename = fr.getCurrentFilename();
    auto entries = fr.loadLog(filename);
    REQUIRE(entries.size() > 0);
    REQUIRE(entries[0].droneId == "alpha_1");
}

// --- EmergencyEngine Tests (T138) ---

TEST_CASE("EmergencyEngine rule evaluation") {
    EmergencyEngine engine;
    EmergencyRule rule;
    rule.id = "battery_low";
    rule.name = "Low Battery";
    rule.condition = "battery < 20";
    rule.action = "RTH";
    rule.priority = 1;
    rule.enabled = true;
    engine.addRule(rule);

    std::map<std::string, DroneHealth> drones;
    DroneHealth dh;
    dh.batteryLevel = 15.0;
    dh.connectionStatus = ConnectionStatus::CONNECTED;
    drones["alpha_1"] = dh;

    auto results = engine.evaluate(20.0, 1, drones, 1000.0);
    REQUIRE(results.size() > 0);
    REQUIRE(results[0].action == "RTH");
}

// --- GeoUtils Tests ---

TEST_CASE("Point in polygon") {
    std::vector<Coordinate> polygon = {
        Coordinate(0.0, 0.0, 0.0),
        Coordinate(0.0, 1.0, 0.0),
        Coordinate(1.0, 1.0, 0.0),
        Coordinate(1.0, 0.0, 0.0),
    };
    Coordinate inside(0.5, 0.5, 0.0);
    Coordinate outside(2.0, 2.0, 0.0);

    REQUIRE(pointInPolygon(inside, polygon));
    REQUIRE_FALSE(pointInPolygon(outside, polygon));
}

TEST_CASE("Point in circle") {
    Coordinate center(48.0, 2.0, 0.0);
    Coordinate inside(48.01, 2.01, 0.0);
    Coordinate outside(50.0, 5.0, 0.0);

    REQUIRE(pointInCircle(inside, center, 10.0));
    REQUIRE_FALSE(pointInCircle(outside, center, 10.0));
}

// --- MirrorContext Tests (T139) ---

TEST_CASE("MirrorContext clone state") {
    MirrorContext mc;
    std::map<std::string, DroneContext> swarm;
    auto uav = std::make_unique<UAV>(Coordinate(48.0, 2.0, 0.0), 90.0, 0.25, 90.0, "alpha");
    swarm.try_emplace("alpha_1", std::move(uav), Coordinate(48.0, 2.0, 0.0),
                      Coordinate(49.0, 3.0, 0.0), 0.25);
    std::vector<Mission> missions;
    bool cloned = mc.cloneState(swarm, missions);
    REQUIRE(cloned);
    REQUIRE(mc.drones.size() == 1);
}

// --- Payload Tests (T140) ---

TEST_CASE("Payload gimbal commands") {
    Payload p("p1", "alpha_1", "EO_IR");
    p.setGimbal(45.0, -30.0);
    REQUIRE(p.gimbalPan == 45.0);
    REQUIRE(p.gimbalTilt == -30.0);
}

TEST_CASE("Payload zoom is clamped") {
    Payload p("p1", "alpha_1");
    p.setZoom(200.0);
    REQUIRE(p.zoom == 100.0);
    p.setZoom(-1.0);
    REQUIRE(p.zoom == 0.1);
}

// --- LinkManager Tests (T141) ---

TEST_CASE("LinkManager heartbeat tracking") {
    LinkManager lm("alpha_1");
    lm.update(1.0, true);
    REQUIRE(lm.lastHeartbeat < 1.0);

    lm.update(5.0, false);
    REQUIRE(lm.lastHeartbeat >= 5.0);
}

TEST_CASE("LinkManager failover detection") {
    LinkManager lm("alpha_1");
    lm.signalStrength = 0.1;
    REQUIRE(lm.shouldFailover());

    lm.signalStrength = 0.9;
    REQUIRE_FALSE(lm.shouldFailover());
}

// --- NoFlyZone Tests ---

TEST_CASE("NoFlyZone circular containment") {
    NoFlyZone zone("z1", "Test Zone");
    zone.shape = NoFlyZoneShape::CIRCLE;
    zone.center = Coordinate(48.0, 2.0, 0.0);
    zone.radius = 100.0;

    REQUIRE(zone.contains(Coordinate(48.5, 2.5, 0.0)));
    REQUIRE_FALSE(zone.contains(Coordinate(55.0, 10.0, 0.0)));
}

TEST_CASE("NoFlyZone polygon containment") {
    NoFlyZone zone("z2", "Poly Zone");
    zone.shape = NoFlyZoneShape::POLYGON;
    zone.vertices = {
        Coordinate(0.0, 0.0, 0.0),
        Coordinate(0.0, 1.0, 0.0),
        Coordinate(1.0, 1.0, 0.0),
        Coordinate(1.0, 0.0, 0.0),
    };

    REQUIRE(zone.contains(Coordinate(0.5, 0.5, 0.0)));
    REQUIRE_FALSE(zone.contains(Coordinate(2.0, 2.0, 0.0)));
}

// --- AuthManager Tests ---

TEST_CASE("AuthManager role permissions") {
    Operator viewOp;
    viewOp.role = OperatorRole::VIEWER;

    AuthManager am;
    REQUIRE_FALSE(am.canExecuteCommand(viewOp, "cmd/fleet/target"));
    REQUIRE(am.hasPermission(viewOp, "view_telemetry"));

    Operator cmdOp;
    cmdOp.role = OperatorRole::COMMANDER;
    REQUIRE(am.canExecuteCommand(cmdOp, "cmd/fleet/target"));
}

// --- WeatherAPI/Terrain/Airspace Tests ---

TEST_CASE("WeatherAPI returns data") {
    WeatherAPI api;
    auto data = api.fetchAt(48.0, 2.0);
    REQUIRE(data.windSpeed >= 0);
    REQUIRE(data.visibility > 0);
}

TEST_CASE("TerrainLoader returns elevation") {
    TerrainLoader loader;
    auto data = loader.getElevation(48.0, 2.0);
    REQUIRE(data.elevation >= 0);
}

TEST_CASE("AirspaceParser zone finding") {
    AirspaceParser parser;
    parser.loadDefaultZones();
    REQUIRE(parser.getDefaultZones().size() > 0);
}

// --- TenantManager Tests ---

TEST_CASE("TenantManager add and query") {
    TenantManager tm;
    TenantManager::Tenant t;
    t.id = "tenant_1";
    t.name = "Test Org";
    t.drones = {"alpha_1", "alpha_2"};
    tm.addTenant(t);

    REQUIRE(tm.getAllTenants().size() == 1);
    REQUIRE(tm.isDroneOwnedByTenant("alpha_1", "tenant_1"));
    REQUIRE_FALSE(tm.isDroneOwnedByTenant("bravo_1", "tenant_1"));
}