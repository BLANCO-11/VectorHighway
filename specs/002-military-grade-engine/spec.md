# Feature Specification: Military-Grade UAV Engine & Professional UI Overhaul

**Feature Branch**: `002-military-grade-engine`
**Created**: 2026-05-02
**Status**: Draft
**Input**: "Complete UI overhaul, improved path recalculation around obstructions smoothly, military-grade UAV engine with real-drone integration, virtual monitoring, simulated path execution"

## User Scenarios & Testing

### User Story 1 - Professional Military Dashboard (Priority: P1)

As a UAV operator, I want a polished, military-grade dark-theme dashboard with a 3D globe, real-time telemetry, and mission control panels so that I can command and monitor drone operations in a professional environment.

**Why this priority**: The UI is the primary interface. Without a professional appearance and reliable operation, the system cannot be used for real-world operations.

**Independent Test**: Load the web UI and verify all panels render correctly, real-time data flows, and controls respond without errors.

**Acceptance Scenarios**:

1. **Given** the engine is running, **When** I load the dashboard, **Then** I see a dark-theme 3D globe with drone markers, a mission control panel, telemetry panel, and system health indicators
2. **Given** drones are airborne, **When** I select a drone, **Then** detailed telemetry (position, heading, speed, battery, altitude) animates into view
3. **Given** the control panel is active, **When** I adjust speed or heading, **Then** the drone responds within 1 simulation tick
4. **Given** multiple swarm groups exist, **When** I switch groups, **Then** all panels update to show the selected group's metrics

---

### User Story 2 - Smooth Obstacle-Aware Path Recalculation (Priority: P1)

As a mission planner, I want the pathfinding engine to smoothly recalculate routes around static and dynamic obstacles so that drones autonomously avoid hazards without jarring course changes.

**Why this priority**: Obstacle avoidance is critical for autonomous operations. Current LocalAvoidance is basic (90-degree evasion), not smooth.

**Independent Test**: Place an obstacle in a drone's flight path and verify the drone smoothly reroutes around it and continues to the target.

**Acceptance Scenarios**:

1. **Given** a drone is en route to a waypoint, **When** a static obstacle appears in its path, **Then** the drone recalculates a path that smoothly arcs around the obstacle
2. **Given** a drone is following a planned route, **When** a dynamic obstacle moves into its path, **Then** the drone continuously replans and adjusts course smoothly
3. **Given** obstacle clearance is complete, **When** the path is clear, **Then** the drone resumes optimal heading to target
4. **Given** an obstacle is too large to go around (within operating bounds), **When** pathfinding fails, **Then** the drone halts and reports "unreachable target"

---

### User Story 3 - Real Drone Telemetry Integration (Priority: P2)

As a UAV operator, I want to connect real drones to the engine so that I can see their live status on the virtual globe alongside simulated assets.

**Why this priority**: Real-drone integration differentiates this from pure simulators and enables real-world mission support.

**Independent Test**: Send a synthetic MAVLink-style telemetry message via WebSocket and verify the drone appears on the globe with correct position, heading, and battery.

**Acceptance Scenarios**:

1. **Given** the engine is running, **When** external telemetry is received via WebSocket, **Then** the drone appears on the virtual globe at the correct position
2. **Given** external telemetry is streaming, **When** telemetry stops for 30 seconds, **Then** the drone is marked as stale/disconnected
3. **Given** a real drone connects, **When** its telemetry includes waypoint progress, **Then** the mission panel shows the drone's current mission phase

---

### User Story 4 - Mission Planning & Autonomous Execution (Priority: P2)

As a mission commander, I want to design multi-waypoint missions that the drone can download and execute autonomously, even when disconnected from the engine.

**Why this priority**: Autonomous offline execution is the defining feature of a military-grade UAV system.

**Independent Test**: Design a mission with 5 waypoints, upload to a simulated drone, and verify it executes the complete route and returns to base.

**Acceptance Scenarios**:

1. **Given** the mission planner UI, **When** I place waypoints on the globe, **Then** a flight plan is generated with estimated time and fuel/battery
2. **Given** a mission plan is ready, **When** I command "execute mission", **Then** the drone follows the waypoint sequence
3. **Given** the drone is executing a mission, **When** it completes all waypoints, **Then** it returns to home base autonomously
4. **Given** the drone loses connection mid-mission, **When** it has a pre-loaded plan, **Then** it continues executing autonomously

---

### User Story 5 - System Health & Failover Monitoring (Priority: P3)

As a system administrator, I want real-time health monitoring of all system components (engine, WebSocket, drones) so that I can detect and respond to failures immediately.

**Why this priority**: Reliability monitoring is essential for military-grade operations but can be built incrementally after core functionality.

**Independent Test**: Kill the WebSocket server and verify the health dashboard reports the failure within 5 seconds.

**Acceptance Scenarios**:

1. **Given** the system is running, **When** I open the health panel, **Then** I see status indicators for engine, WebSocket, and each drone
2. **Given** a drone's battery is critically low, **When** the health monitor detects it, **Then** an alert is raised and the drone is commanded to return to base
3. **Given** the WebSocket server drops connections, **When** reconnection logic triggers, **Then** clients reconnect within 3 seconds

---

### User Story 6 - Flight Data Recorder & Replay (Priority: P3)

As an operations analyst, I want to record all flight data and replay missions so that I can analyze performance and investigate incidents.

**Why this priority**: Post-mission analysis is valuable but not critical for initial deployment.

**Independent Test**: Run a 60-second mission, stop recording, then replay and verify the exact path is rendered.

**Acceptance Scenarios**:

1. **Given** the engine is running, **When** recording is enabled, **Then** all telemetry is logged to a JSON-lines file
2. **Given** a recorded flight log exists, **When** I load it into the replay viewer, **Then** the mission replays with correct position, heading, and timing

---

### Edge Cases

- What happens when all drones have 0% battery simultaneously?
- How does the pathfinder handle concave obstacle clusters (U-shaped walls)?
- What happens when WebSocket receives a command for a non-existent drone?
- How does the system behave when real-drone telemetry rate exceeds 100Hz?
- What happens when two obstacles are placed directly on top of each other?
- How does the globe renderer handle 500+ simultaneous drone markers?
- What happens if the mission planner creates a path that crosses no-fly zones?
- How does the system handle chronologically disordered telemetry packets?

## Requirements

### Functional Requirements

- **FR-001**: System MUST provide a military-grade dark-theme web UI with 3D globe visualization, radar overlay, and glassmorphism panels
- **FR-002**: System MUST support smooth obstacle-aware path recalculation using hybrid A*/RRT* with continuous replanning for both static and dynamic obstacles
- **FR-003**: System MUST support real-time WebSocket communication between C++ engine and web UI
- **FR-004**: System MUST support external real-drone telemetry ingestion via WebSocket pub/sub topics
- **FR-005**: System MUST provide a mission planning interface with multi-waypoint route design, ETA, and battery estimation
- **FR-006**: System MUST support mission upload to drones for autonomous offline execution
- **FR-007**: System MUST provide system health monitoring: engine status, WebSocket status, per-drone connection status
- **FR-008**: System MUST include a flight data recorder logging all telemetry to JSON-lines format
- **FR-009**: System MUST support mission replay from recorded flight logs with scrubbing and speed control
- **FR-010**: System MUST manage multiple UAV swarm groups with independent control and color coding
- **FR-011**: System MUST support no-fly zone creation and enforcement in pathfinding
- **FR-012**: System MUST eliminate all raw pointer usage in favor of smart pointers or RAII containers
- **FR-013**: System MUST provide a test suite covering Pathfinder (smooth avoidance), Geo calculations, UAV physics, and WebSocket messaging
- **FR-014**: System MUST provide environment-based configuration for WebSocket URL, port, and recording paths
- **FR-015**: System MUST support return-to-home (RTH) on low battery, connection loss, or manual command

### Key Entities

- **UAV**: A drone agent with position, heading, speed, battery, altitude, group membership, physics-based movement, mission state
- **Obstacle**: Static or dynamic hazard with position, radius, shape (circle/polygon), no-fly zone flag
- **Waypoint**: Named geographic coordinate with altitude, expected arrival time, action (fly-over/fly-to/loiter)
- **Mission**: Ordered list of waypoints with home base, total distance, estimated duration, battery budget
- **ChargingStation**: POI with position, name, and charging rate for battery replenishment
- **SwarmGroup**: Named collection of UAVs sharing target assignments, color, and settings
- **NoFlyZone**: Geofenced area (circular or polygonal) that pathfinding must avoid
- **FlightLog**: Recorded sequence of telemetry snapshots with timestamps for replay
- **HealthStatus**: System component health (engine, WebSocket, per-drone) with alert level

---

### User Story 7 - Geospatial Intelligence Layer (Priority: P2)

As a mission planner, I want real weather, terrain elevation, and airspace data overlaid on the globe so that I can plan safe, informed missions accounting for environmental conditions.

**Why this priority**: Real-world flight depends on environmental awareness. Without weather/terrain/airspace, the platform is a toy.

**Independent Test**: Enable the weather overlay and verify wind arrows, precipitation cells, and visibility data appear on the globe.

**Acceptance Scenarios**:

1. **Given** the geospatial layer is active, **When** I enable weather overlay, **Then** I see wind speed/direction arrows, precipitation zones, and visibility contours on the globe
2. **Given** terrain elevation data is loaded, **When** I plan a mission through mountainous area, **Then** the pathfinder automatically adjusts waypoint altitudes to clear terrain by configurable margin
3. **Given** airspace data is loaded, **When** a drone's path crosses restricted airspace, **Then** the path is flagged and the operator is prompted to approve

---

### User Story 8 - Digital Twin / Mirror Mode (Priority: P1)

As a UAV operator, I want every command previewed in simulation before execution on the real drone so that I can validate mission plans without risk to hardware.

**Why this priority**: This is the core differentiator — simulate before you execute. The entire platform centers on this capability.

**Independent Test**: Load the live mirror view while the sim is running. Clone the live state into a sandbox, modify the mission, then promote the plan back to live.

**Acceptance Scenarios**:

1. **Given** a live simulation session, **When** I switch to Mirror Mode, **Then** I see a side-by-side view of "Live State" and "Simulated Twin" with identical positions
2. **Given** the digital twin is active, **When** I design a new mission in the sandbox, **Then** the twin simulates the entire mission before any command touches the live drone
3. **Given** a sandbox mission completes successfully, **When** I promote it, **Then** the mission plan is uploaded to the live drone for execution
4. **Given** the twin detects a conflict (obstacle, battery insufficient, no-fly zone), **When** the user tries to promote, **Then** the system warns with specific reasons and blocks promotion

---

### User Story 9 - Multi-Operator C2 Hierarchy (Priority: P2)

As a mission commander, I want to assign operator roles with different permission levels so that large-scale fleet operations have proper command structure and safety oversight.

**Why this priority**: Real operations require role separation — one operator cannot have sole control over a fleet.

**Independent Test**: Connect two browser tabs with different roles (Commander vs Viewer). Verify the Viewer cannot send commands.

**Acceptance Scenarios**:

1. **Given** the C2 system, **When** an operator connects, **Then** they authenticate with a role: Commander, Sensor Operator, Supervisor, or Viewer
2. **Given** a Supervisor is connected, **When** they issue an emergency override, **Then** all drones execute the override regardless of current mission
3. **Given** a Viewer is connected, **When** they attempt to send a command, **Then** the command is rejected

---

### User Story 10 - Communications Link Management (Priority: P3)

As a network operator, I want to see link quality (latency, signal strength) per drone and configure automatic link failover so that drones maintain connectivity across long-range operations.

**Why this priority**: Link management is essential for beyond-line-of-sight operations but can be layered after core C2 is stable.

**Acceptance Scenarios**:

1. **Given** drones are operational, **When** I open the link panel, **Then** I see per-drone signal strength, latency, packet loss, and active link type (SATCOM/4G/LOS)
2. **Given** a drone's primary link degrades below threshold, **When** a backup link is available, **Then** the drone auto-switches and a link-failover event is logged
3. **Given** a drone loses all links, **When** the loss-of-link timer expires, **Then** the drone executes its configured loss-of-link procedure (loiter → RTH → land)

---

### User Story 11 - Multi-Drone Collision Avoidance / UTM (Priority: P2)

As a fleet operator, I want drones to automatically deconflict paths when missions overlap so that mid-air collisions are impossible.

**Why this priority**: Fleet safety is non-negotiable for any multi-drone platform.

**Independent Test**: Send two drones on intersecting paths. Verify they automatically adjust to maintain separation.

**Acceptance Scenarios**:

1. **Given** two drones have intersecting paths, **When** their separation bubble would overlap, **Then** the system negotiates altitude or speed changes to maintain minimum separation
2. **Given** a UTM corridor is defined, **When** a drone's path leaves the corridor, **Then** the system warns and auto-corrects the path
3. **Given** deconfliction is active, **When** paths are recalculated, **Then** separation is maintained throughout the new routes

---

### User Story 12 - Payload & Sensor Management (Priority: P3)

As a sensor operator, I want to control drone payloads (cameras, SAR, LIDAR) from the dashboard and see live synthetic sensor feeds so that I can conduct remote surveillance operations.

**Why this priority**: Payload control transforms the platform from a tracking tool to an operational C2 system.

**Acceptance Scenarios**:

1. **Given** a drone has a camera payload, **When** I select the payload tab, **Then** I see gimbal pan/tilt/zoom controls and a picture-in-picture synthetic view from the drone's perspective
2. **Given** a drone has multiple payload types, **When** I switch payloads, **Then** the UI updates to show payload-specific controls and telemetry

---

### User Story 13 - Dynamic Mission Adaptation (Priority: P2)

As a mission commander, I want to edit missions in real-time while drones are airborne so that I can respond to changing operational requirements without landing.

**Why this priority**: Static missions fail in dynamic environments. Live editing is essential for real ops.

**Acceptance Scenarios**:

1. **Given** a drone is executing a mission, **When** I drag a waypoint to a new position on the globe, **Then** the drone immediately recalculates and adjusts course
2. **Given** a mission is in progress, **When** I insert a conditional branch ("if battery < 30% at WP3, skip to WP6"), **Then** the drone autonomously follows the condition
3. **Given** a high-priority target appears, **When** I trigger priority override, **Then** the system proposes an updated mission plan inserting the new target

---

### User Story 14 - Protocol Adapter Layer (Priority: P3)

As a system integrator, I want to connect different drone types (MAVLink, PX4, ArduPilot, DJI) so that the platform is drone-agnostic.

**Why this priority**: Real-world drone ops use diverse hardware. This enables broad compatibility.

**Acceptance Scenarios**:

1. **Given** a MAVLink drone connects, **When** the adapter translates MAVLink telemetry to internal format, **Then** the drone appears in the UI within 5 seconds
2. **Given** a new drone type needs integration, **When** I implement the adapter interface, **Then** no engine code changes are needed

---

### User Story 15 - Multi-Tenant Fleet Operations (Priority: P3)

As a platform operator, I want to manage multiple organizations, each with their own drones, missions, and operators, so that the platform can serve as a hosted service.

**Why this priority**: Multi-tenancy enables SaaS/hosted deployment but is feature-complete icing on the cake.

**Acceptance Scenarios**:

1. **Given** the platform has multiple tenants, **When** Tenant A views their dashboard, **Then** they see only their drones and missions
2. **Given** a global admin view, **When** I switch to fleet-wide analytics, **Then** I see aggregate metrics across all tenants

---

### User Story 16 - Emergency Procedures Engine (Priority: P1)

As a safety officer, I want configurable emergency procedures (if-this-then-that rules) and pre-flight checklists so that the platform enforces safety standards automatically.

**Why this priority**: Safety automation is critical for military-grade operations and prevents human error in emergencies.

**Acceptance Scenarios**:

1. **Given** the emergency engine is active, **When** GPS is lost and altitude exceeds 400m, **Then** the drone auto-descends to 200m, loiters 60s, then lands
2. **Given** a pre-flight checklist exists, **When** I initiate a mission, **Then** the system runs the checklist and blocks launch if any item fails
3. **Given** auto-ditching triggers, **When** no safe landing zone is reachable, **Then** the drone diverts to the safest available zone based on population density overlay

---

## Phased Delivery Roadmap

```
Phase A (Foundation)    — Professional UI, smooth pathfinding, memory safety, WebSocket robustness
Phase B (Operations)    — Mission planning, flight recorder, health monitoring, no-fly zones
Phase C (Real Ops)      — Real-drone telemetry, digital twin/mirror mode, weather/terrain/airspace, emergency engine
Phase D (Fleet C2)      — Multi-operator roles, collision avoidance/UTM, dynamic mission editing
Phase E (Advanced)      — Payload/sensor management, link management, protocol adapters, multi-tenancy
```

### Measurable Outcomes

- **SC-001**: UI page load completes in under 3 seconds on a modern browser with 50 drones
- **SC-002**: Simulation tick rate maintains at least 20Hz with 100 concurrent drones
- **SC-003**: Obstacle avoidance recalculates in under 100ms for paths with 10+ obstacles
- **SC-004**: Zero memory leaks detected by AddressSanitizer in a 1-hour simulation run
- **SC-005**: WebSocket command latency stays under 50ms p99 for command to UI update
- **SC-006**: Flight data recorder uses less than 1MB/minute at 20Hz with 50 drones
- **SC-007**: Mission replay accurately reproduces positions within 1m of recorded data
- **SC-008**: Real-drone telemetry ingestion handles 50 simultaneous external drones
- **SC-009**: Test suite achieves >80% line coverage on core modules (Geo, Pathfinder, UAV)

## Assumptions

- Users will run the C++ engine and web frontend on the same machine during development
- Real-drone telemetry arrives via WebSocket in the same format as internal telemetry
- The simulation runs in flat geographic space (WGS84 ellipsoid accuracy not required)
- Node.js 18+ is available for the web frontend
- The project targets Linux development primarily (Windows support is secondary)
- External drones support at minimum position, heading, battery, and status telemetry fields
- Flight logs are stored locally; cloud sync is out of scope for v1
- Military-grade implies: deterministic behavior, fail-safe defaults, comprehensive logging, no single points of failure
