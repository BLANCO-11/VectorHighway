# Tasks: Military-Grade UAV Engine & Professional UI Overhaul

**Branch**: `002-military-grade-engine`
**Input**: Design documents from `specs/002-military-grade-engine/`
**Prerequisites**: plan.md, spec.md (16 user stories), research.md, data-model.md, contracts/

**Organization**: Tasks follow the 5-phase delivery roadmap (A→E) from the plan. Phase A tasks cover the MVP scope (UI, pathfinding, memory safety, WebSocket health).

---

## Phase A.1: Setup (Shared Infrastructure)

**Purpose**: Project initialization, build system, and directory structure.

- [X] T001 Create `include/Planner.h` with hybrid pathfinding interface (smooth local replanning, RRT* fallback, obstacle repulsion)
- [X] T002 Create `include/Mission.h` with mission plan, state machine, waypoint sequence entities
- [X] T003 Create `include/FlightRecorder.h` with JSON-lines logging interface
- [X] T004 Create `include/HealthMonitor.h` with health check and alert engine interface
- [X] T005 Create `include/GeoUtils.h` with polygon containment, elevation helpers
- [X] T006 Create `src/Planner.cpp` — stub implementation matching Planner.h
- [X] T007 Create `src/Mission.cpp` — stub implementation matching Mission.h
- [X] T008 Create `src/FlightRecorder.cpp` — stub implementation matching FlightRecorder.h
- [X] T009 Create `src/HealthMonitor.cpp` — stub implementation matching HealthMonitor.h
- [X] T010 Update `CMakeLists.txt` — add Planner.cpp, Mission.cpp, FlightRecorder.cpp, HealthMonitor.cpp to uav_sim and uav_tests targets
- [X] T011 Update `include/CommandEvent.h` — add missionJson field, ExternalTelemetry speed/status fields
- [X] T012 Update `include/Types.h` — add MissionState enum, AlertLevel enum, ConnectionStatus enum
- [X] T013 [P] Create `web/src/lib/types.ts` — TypeScript types mirroring C++ data model (UAV, Obstacle, Mission, Waypoint, HealthStatus, ExternalTelemetry)
- [X] T014 [P] Create `web/src/lib/constants.ts` — group colors, theme token constants, default config values
- [X] T015 [P] Create `web/src/lib/geo.ts` — client-side Haversine, bearing, coordinate conversion utilities

---

## Phase A.2: Foundational — Backend Refactor & WebSocket Robustness

**Purpose**: Memory safety audit, global state elimination, WebSocket rate limiting and reconnect.

**⚠️ No user story tasks can begin until this phase is complete**

### Backend Memory Safety

- [X] T016 Refactor `src/exec.cpp` — replace all raw `new`/`delete` with `std::unique_ptr` and `std::make_unique`
- [X] T017 Refactor `src/exec.cpp` — move `DroneContext` struct into `SimulationContext.h` as a named type
- [X] T018 Refactor `src/websockets.cpp` — wrap `WSServerImpl` raw pointer in `std::unique_ptr`

### WebSocket Robustness

- [X] T019 Add rate limiting to `src/websockets.cpp` — max 100 messages/sec per client, drop excess with console warning
- [X] T020 Add `connection_status` broadcast to `src/websockets.cpp` — emit on connect/disconnect
- [X] T021 Add WebSocket reconnect logic to `web/src/useSimulationWebSocket.ts` — exponential backoff (1s, 2s, 4s, max 30s), auto-reconnect on close
- [X] T022 Add `connection_status` message handler to `web/src/useSimulationWebSocket.ts` — expose `isConnected` state

**Checkpoint**: Backend is memory-safe, WebSocket is rate-limited, frontend auto-reconnects. User story implementation can begin.

---

## Phase A.3: User Story 1 — Professional Military Dashboard (P1)

**Goal**: Dark tactical theme, 3D globe, glassmorphism panels, zustand store, proper layout, radar overlay.

**Independent Test**: Load the web UI and verify all panels render correctly, real-time data flows, controls respond without errors.

### Implementation for User Story 1

- [X] T023 [P] [US1] Create `web/src/styles/theme.ts` — export theme tokens (colors, spacing, typography) matching research.md military-grade palette
- [X] T024 [P] [US1] Update `web/src/app/globals.css` — TailwindCSS v4 `@theme` with dark tactical colors (`#0a0a0f`, `#1a1a2e`, `#00d4ff`, `#ff3355`, `#00ff88`, `#ffaa00`), glassmorphism utilities, custom scrollbar, range slider
- [X] T025 [P] [US1] Update `web/src/app/layout.tsx` — proper metadata title "VectorHighway — UAV Command Center", dark background, Geist font
- [X] T026 [US1] Create `web/src/store/useSimulationStore.ts` — Zustand store with slices: uavs, obstacles, chargingStations, selectedEntity, connectionStatus, clickMode, activeGroup, groups, params, alerts, health
- [X] T027 [US1] Refactor `web/src/app/page.tsx` — replace raw `useState` with zustand store selectors, delegate rendering to sub-components, add top status bar, left sidebar, globe area, right detail panel layout
- [X] T028 [P] [US1] Create `web/src/components/ui/Toggle.tsx` — toggle switch component with accent color prop
- [X] T029 [P] [US1] Create `web/src/components/ui/Alert.tsx` — alert badge with level-based color (CAUTION=amber, WARNING=orange, CRITICAL=red)
- [X] T030 [P] [US1] Create `web/src/components/ui/ProgressBar.tsx` — horizontal progress bar with label and percentage
- [X] T031 [P] [US1] Create `web/src/components/ui/Badge.tsx` — status badge (CONNECTED=green, STALE=amber, DISCONNECTED=red, IDLE=grey)
- [X] T032 [P] [US1] Create `web/src/components/ui/Timeline.tsx` — scrollable event log component with filter by level
- [X] T033 [US1] Create `web/src/components/map/RadarOverlay.tsx` — CSS conic-gradient radar sweep animation on globe, toggleable via store
- [X] T034 [US1] Update `web/src/components/map/Earth.tsx` — add atmosphere glow shader, click handler pushes lat/lon to store
- [X] T035 [US1] Update `web/src/components/map/UAVMarker.tsx` — add altitude indicator, selection ring, group-color-coded glow, smooth position interpolation via useFrame
- [X] T036 [US1] Update `web/src/components/panels/SwarmPanel.tsx` — read groups/activeGroup from store, color-coded tabs, new group input
- [X] T037 [US1] Update `web/src/components/panels/ControlPanel.tsx` — read params from store, write changes via WebSocket sendMessage
- [X] T038 [US1] Update `web/src/components/panels/TelemetryPanel.tsx` — read selectedEntity from store, display lat/lon/heading/battery/alt/speed/missionState
- [X] T039 [US1] Update `web/src/components/panels/BatteryChart.tsx` — read activeGroup battery history from store, recharts LineChart

**Checkpoint**: Professional dashboard renders with dark theme, 3D globe, all panels, zustand state flow. Controls update drones. Independently testable.

---

## Phase A.4: User Story 2 — Smooth Obstacle-Aware Path Recalculation (P1)

**Goal**: Replace basic 90-degree evasion with continuous smooth path recalculation around static and dynamic obstacles.

**Independent Test**: Place an obstacle in a drone's flight path and verify the drone smoothly arcs around it.

### Implementation for User Story 2

- [X] T040 [P] [US2] Implement `Planner::smoothPath()` in `src/Planner.cpp` — take A* waypoints, apply obstacle repulsion vectors + curvature minimization, return smooth waypoint sequence
- [X] T041 [P] [US2] Implement `Planner::localReplan()` in `src/Planner.cpp` — per-tick check if any path segment intersects an obstacle, insert RRT*-sampled avoidance waypoints in local space
- [X] T042 [US2] Implement `Planner::buildRepulsionField()` in `src/Planner.cpp` — compute obstacle repulsion vectors for each waypoint based on distance and radius
- [X] T043 [US2] Integrate `Planner` into `src/exec.cpp` — after A* produces initial bearing, call `Planner::smoothPath()`; each tick call `Planner::localReplan()` for dynamic adjustments
- [X] T044 [P] [US2] Create `web/src/components/map/PathLine.tsx` — render smoothed planned path as dashed great-circle arc on globe, update on mission/target change
- [X] T045 [P] [US2] Update `web/src/components/map/ConnectionLines.tsx` — add projected path display alongside current connection line

**Checkpoint**: Drones smoothly avoid obstacles (static and dynamic). PathLine overlay shows planned route on globe. Independently testable.

---

## Phase A.5: User Story 16 — Emergency Procedures Engine (P1)

**Goal**: Configurable emergency rules (if-this-then-that), pre-flight checklists, auto-ditching.

**Independent Test**: Trigger GPS-loss at high altitude and verify drone auto-descends to 200m, loiters 60s, then lands.

### Implementation for User Story 16

- [X] T046 [P] [US16] Implement `EmergencyEngine` in `include/HealthMonitor.h` and `src/HealthMonitor.cpp` — rule evaluation engine matching EmergencyRule data model, runs at 1Hz, triggers actions
- [X] T047 [P] [US16] Implement `Checklist` system in `include/Mission.h` and `src/Mission.cpp` — pre-flight checklist items with auto-check and manual override
- [X] T048 [US16] Update `src/exec.cpp` — hook EmergencyEngine into main loop, execute auto-actions (RTH, DESCEND, LAND, NOTIFY)
- [X] T049 [P] [US16] Create `web/src/components/panels/EmergencyPanel.tsx` — emergency rule editor, enable/disable rules, view triggered alerts
- [X] T050 [P] [US16] Create `web/src/components/panels/ChecklistPanel.tsx` — pre-flight checklist display with auto-check results, manual override buttons, status indicators
- [X] T051 [US16] Update `src/websockets.cpp` — add `cmd/system/checklist` topic for checklist overrides, `state/alert` topic for emergency broadcasts

**Checkpoint**: EmergencyEngine runs rules, executes actions. Pre-flight checklists block launch on failure. Independently testable.

---

## Phase B.1: User Story 4 — Mission Planning & Autonomous Execution (P2)

**Goal**: Multi-waypoint mission planner UI, upload/execute/abort flow, autonomous waypoint following, RTH.

**Independent Test**: Design a 5-waypoint mission, upload to simulated drone, verify it executes the complete route and returns to base.

### Implementation for User Story 4

- [X] T052 [P] [US4] Implement `MissionExecutor` in `src/Mission.cpp` — advance waypoint on arrival (100m radius), track missionState transitions, trigger RTH on completion/fault
- [X] T053 [US4] Update `include/UAV.h` — add missionState (MissionState) and currentWaypointIndex fields
- [X] T054 [US4] Update `src/exec.cpp` — per-tick call MissionExecutor for each drone with active mission, update UAV missionState, broadcast mission status
- [X] T055 [US4] Update `src/websockets.cpp` — add `cmd/mission/{id}/upload`, `cmd/mission/{id}/start`, `cmd/mission/{id}/rth` topic handlers; broadcast `state/mission` updates
- [X] T056 [P] [US4] Create `web/src/components/panels/MissionPanel.tsx` — waypoint placement on globe (click to add, drag to reorder), ETA/battery estimation display, Upload/Start/Abort buttons
- [X] T057 [US4] Update `web/src/store/useSimulationStore.ts` — add missions slice, currentMission, missionStatus
- [X] T058 [US4] Update `web/src/app/page.tsx` — integrate MissionPanel into left sidebar below SwarmPanel
- [X] T059 [P] [US4] Update `web/src/useSimulationWebSocket.ts` — handle `state/mission` message type, update store

**Checkpoint**: Mission plans uploaded, executed, and completed with RTH. Waypoint progress visible in MissionPanel. Independently testable.

---

## Phase B.2: User Story 5 — System Health & Failover Monitoring (P3)

**Goal**: 1Hz health checks, alert engine, per-drone connection status, WebSocket reconnect on server drop.

**Independent Test**: Kill WebSocket server and verify health dashboard reports failure within 5 seconds.

### Implementation for User Story 5

- [X] T060 [P] [US5] Implement `HealthMonitor::check()` in `src/HealthMonitor.cpp` — measure tick rate, count WS connections, check per-drone battery/lastSeen/missionState, generate alerts by level
- [X] T061 [US5] Integrate `HealthMonitor` into `src/exec.cpp` — tick callback every 50 iterations (1Hz at 20Hz sim rate), broadcast `state/health` JSON
- [X] T062 [US5] Update `src/websockets.cpp` — broadcast `state/health` and `state/alert` messages; handle `cmd/system/recording` topic for flight recorder toggle
- [X] T063 [P] [US5] Create `web/src/components/panels/HealthPanel.tsx` — engine tick rate gauge, WS connection indicator, per-drone status badges, alert list with auto-dismiss
- [X] T064 [P] [US5] Create `web/src/hooks/useHealthMonitor.ts` — hook subscribing to `state/health` via WebSocket, pushes to store health slice
- [X] T065 [US5] Update `web/src/app/page.tsx` — render HealthPanel in right sidebar area, style alerts with framer-motion slide-in
- [X] T066 [US5] Update `web/src/store/useSimulationStore.ts` — add health slice (engineTickHz, wsConnectionCount, alerts, droneHealth map)

**Checkpoint**: Health monitor reports system status. Alerts trigger on low battery, stale drones, engine slowdown. Independently testable.

---

## Phase B.3: User Story 6 — Flight Data Recorder & Replay (P3)

**Goal**: JSON-lines recording, replay with scrub/play/pause/speed, file rotation.

**Independent Test**: Run 60s mission, stop recording, load replay, verify exact path renders.

### Implementation for User Story 6

- [X] T067 [P] [US6] Implement `FlightRecorder` in `src/FlightRecorder.cpp` — append JSON-lines per telemetry tick, rotate at 100MB or 1hr, filename `flightlog_{timestamp}.jsonl`
- [X] T068 [US6] Integrate `FlightRecorder` into `src/exec.cpp` — toggle via `cmd/system/recording`, write per-tick UAV state line
- [X] T069 [US6] Implement replay in `src/exec.cpp` — `cmd/system/replay` loads jsonl file, emits `uav_update` at original timestamps with speed multiplier
- [X] T070 [P] [US6] Create `web/src/components/panels/FlightLogPanel.tsx` — file selector, play/pause/scrub timeline, speed control (0.5x–4x), time display
- [X] T071 [US6] Update `web/src/store/useSimulationStore.ts` — add replay slice (isReplaying, replaySpeed, replayTime, availableLogs)
- [X] T072 [US6] Update `web/src/useSimulationWebSocket.ts` — handle replay state messages, distinguish live vs replay uav_updates

**Checkpoint**: Flight logs recorded, replay shows exact paths with speed control. Independently testable.

---

## Phase B.4: User Story 11 — No-Fly Zones (P2)

**Goal**: Geofenced areas (circular/polygonal) enforced by pathfinder.

**Independent Test**: Create a no-fly zone and verify drone path avoids it.

### Implementation for User Story 11

- [X] T073 [P] [US11] Update `include/Environment.h` — add NoFlyZone entity with shape enum (CIRCLE, POLYGON), center/radius, vertices array, active flag
- [X] T074 [US11] Update `include/Pathfinder.h` — add no-fly zone intersection checks in A* edge cost, penalize edges crossing zones (infinite cost if active)
- [X] T075 [US11] Update `src/websockets.cpp` — add `cmd/environment/noflyzone` topic for create/delete/toggle
- [X] T076 [P] [US11] Create `web/src/components/map/NoFlyZoneMarker.tsx` — render circular (semi-transparent disc) and polygonal (outline + fill) no-fly zones on globe
- [X] T077 [US11] Update `web/src/components/panels/ControlPanel.tsx` — add no-fly zone click mode, zone type selector (circle/polygon), toggle active/inactive

**Checkpoint**: No-fly zones visible on globe, pathfinder avoids them. Independently testable.

---

## Phase C.1: User Story 3 — Real Drone Telemetry Integration (P1-P2)

**Goal**: External telemetry ingestion, stale drone detection, external drone badges in UI.

**Independent Test**: Send synthetic telemetry via WebSocket, verify drone appears on globe with correct position.

### Implementation for User Story 3

- [X] T078 [US3] Update `include/CommandEvent.h` — extend ExternalTelemetry with speed (double), status (string) fields
- [X] T079 [US3] Update `src/websockets.cpp` — parse speed/status from `telemetry/external/{id}` payload
- [X] T080 [US3] Update `src/exec.cpp` — mark external drones as `isExternal=true` in broadcast, include speed/status in uav_update JSON
- [X] T081 [US3] Update `web/src/store/useSimulationStore.ts` — differentiate external drones in uavs slice
- [X] T082 [US3] Update `web/src/components/map/UAVMarker.tsx` — render external drone badge (different icon/ring style, e.g. dashed ring)
- [X] T083 [US3] Update `web/src/components/panels/TelemetryPanel.tsx` — show external status badge, connection timestamp

**Checkpoint**: External drones appear on globe with distinct styling, stale detection works. Independently testable.

---

## Phase C.2: User Story 8 — Digital Twin / Mirror Mode (P1)

**Goal**: Clone live state to sandbox, simulate missions in twin, promote to live, conflict detection.

**Independent Test**: Clone live state, modify mission in sandbox, promote, verify live drone executes updated plan.

### Implementation for User Story 8

- [X] T084 [P] [US8] Implement `MirrorContext` in `include/MirrorContext.h` and `src/MirrorContext.cpp` — deep-copy UAV/obstacle/mission state from SimulationContext, run independent sim loop in sandbox
- [X] T085 [US8] Update `src/websockets.cpp` — add `cmd/mirror/clone`, `cmd/mirror/simulate`, `cmd/mirror/promote`, `cmd/mirror/diff` topics
- [X] T086 [US8] Implement conflict detection in `src/MirrorContext.cpp` — compare sandbox outcome vs live state, detect path conflicts, battery insufficiency, no-fly zone violations
- [X] T087 [P] [US8] Create `web/src/components/panels/MirrorPanel.tsx` — side-by-side live/twin view, diff highlighting (green=added, red=removed, amber=changed), Promote button with conflict warning
- [X] T088 [US8] Update `web/src/store/useSimulationStore.ts` — add mirror slice (isMirrorActive, liveState, twinState, conflicts, isPromotable)
- [X] T089 [US8] Update `web/src/app/page.tsx` — add Mirror Mode toggle in top bar, swap to MirrorPanel layout when active
- [X] T090 [US8] Update `web/src/useSimulationWebSocket.ts` — handle mirror state messages, dual WebSocket channels (live + twin)

**Checkpoint**: Live state clones to sandbox, sandbox sim runs independently, promote pushes plan to live. Conflict detection blocks unsafe promotion. Independently testable.

---

## Phase C.3: User Story 7 — Geospatial Intelligence Layer (P2)

**Goal**: Weather overlay (wind arrows, precipitation), terrain elevation (auto-altitude), airspace (color-coded zones).

**Independent Test**: Enable weather overlay and verify wind arrows, precipitation cells appear on globe.

### Implementation for User Story 7

- [X] T091 [P] [US7] Create `include/WeatherAPI.h` and `src/WeatherAPI.cpp` — fetch NOAA/OpenWeatherMap data for bounding box, parse wind/precip/visibility
- [X] T092 [P] [US7] Create `include/TerrainLoader.h` and `src/TerrainLoader.cpp` — load SRTM elevation data, return elevation at lat/lon coordinate
- [X] T093 [P] [US7] Create `include/AirspaceParser.h` and `src/AirspaceParser.cpp` — parse airspace classes from OpenAir format or TFR feed, return polygon/circle zones with class type
- [X] T094 [US7] Integrate geospatial data into `src/exec.cpp` — terrain elevation feeds into waypoint altitude adjustment, airspace zones treated as soft no-fly zones with operator approval prompt
- [X] T095 [P] [US7] Create `web/src/components/map/WeatherOverlay.tsx` — Three.js InstancedMesh for wind arrows, custom shader for precipitation cells, visibility contour rings
- [X] T096 [P] [US7] Create `web/src/components/map/AirspaceLayer.tsx` — color-coded airspace zone rendering (Class A=red, B=blue, C=green, restricted=amber, TFR=orange)
- [X] T097 [US7] Update `web/src/store/useSimulationStore.ts` — add geospatial slice (weatherData, terrainData, airspaceZones, enabledLayers)
- [X] T098 [US7] Update `web/src/app/page.tsx` — add layer toggle controls (weather, terrain, airspace) in a floating toolbar above the globe
- [X] T099 [US7] Update `include/Pathfinder.h` — integrate terrain elevation into A* edge cost (penalize segments that don't clear terrain), flag airspace crossings

**Checkpoint**: Weather/terrain/airspace overlays render on globe. Pathfinder accounts for terrain clearance. Airspace crossings are flagged. Independently testable.

---

## Phase C.4: Loss-of-Link Procedures (P2)

**Goal**: Configurable per-drone loss-of-link behavior (loiter → RTH → land).

**Independent Test**: Disconnect a drone's link and verify it executes the configured LoL procedure.

- [X] T100 [US3] Add per-drone LoL config to `include/UAV.h` — lolProcedure enum (LOITER_AND_RTH, IMMEDIATE_RTH, LAND), lolTimer (seconds), lolActive bool
- [X] T101 [US3] Update `src/exec.cpp` — track last telemetry timestamp per drone, tick down LoL timer, execute procedure on expiry
- [X] T102 [US3] Update `web/src/useSimulationWebSocket.ts` — track last message timestamp per drone, expose per-drone connectionStatus in store

---

## Phase D.1: User Story 9 — Multi-Operator C2 Hierarchy (P2)

**Goal**: Role-based access (Commander, Sensor Operator, Supervisor, Viewer), permission filtering, emergency override.

**Independent Test**: Connect two browser tabs with different roles. Verify Viewer cannot send commands.

### Implementation for User Story 9

- [X] T103 [P] [US9] Create `include/AuthManager.h` and `src/AuthManager.cpp` — role enum, token validation, permission matrix
- [X] T104 [US9] Update `src/websockets.cpp` — authenticate on connect via `auth` message, filter commands by role, Supervisor override bypasses all filters
- [X] T105 [P] [US9] Create `web/src/components/panels/C2Panel.tsx` — role selector on connect, permission display, operator list with roles
- [X] T106 [US9] Update `web/src/store/useSimulationStore.ts` — add operator slice (currentRole, permissions, operatorList)

**Checkpoint**: Roles enforced, commands filtered, override works. Independently testable.

---

## Phase D.2: User Story 13 — Dynamic Mission Adaptation (P2)

**Goal**: Live waypoint dragging, conditional branching, priority override.

**Independent Test**: While a drone executes a mission, drag a waypoint and verify the drone recalculates course.

- [X] T107 [P] [US13] Add `MissionCondition` to `include/Mission.h` — condition expression parser (battery/position/time triggers), skip/reroute actions
- [X] T108 [US13] Update `src/Mission.cpp` — evaluate conditions on each waypoint arrival, execute skip/reroute
- [X] T109 [US13] Update `src/websockets.cpp` — add `cmd/mission/{id}/edit` topic for live waypoint mutation, `cmd/mission/{id}/priority` for priority target
- [X] T110 [P] [US13] Create `web/src/components/panels/MissionEditor.tsx` — drag-and-drop waypoints on globe, add/remove/insert waypoints, real-time ETA recalc
- [X] T111 [P] [US13] Create `web/src/components/panels/ConditionEditor.tsx` — visual flow-chart editor for conditional branching (if battery, if position, if time → skip to WP, RTH, hold)

**Checkpoint**: Live mission editing works. Conditional branching executes autonomously. Independently testable.

---

## Phase D.3: User Story 11 — Multi-Drone Collision Avoidance / UTM (P2)

**Goal**: Drone-vs-drone deconfliction, separation bubble, UTM corridor enforcement.

**Independent Test**: Send two drones on intersecting paths and verify they auto-adjust to maintain separation.

- [X] T112 [P] [US11] Implement drone-vs-drone deconfliction in `include/Pathfinder.h` and `src/Planner.cpp` — compute separation bubbles (2km radius), negotiate altitude/speed offsets for intersecting paths
- [X] T113 [US11] Update `src/exec.cpp` — run deconfliction per tick for all active drone pairs, apply negotiated adjustments
- [X] T114 [US11] Add UTM corridor to `include/Environment.h` — corridor defined by centerline waypoints + width, pathfinder penalizes exits
- [X] T115 [P] [US11] Create `web/src/components/map/UTMCorridor.tsx` — render flight corridor as extruded polygon on globe
- [X] T116 [US11] Update `web/src/components/map/UAVMarker.tsx` — show separation bubble as transparent sphere

**Checkpoint**: Drones deconflict automatically. UTM corridors rendered and enforced. Independently testable.

---

## Phase E.1: User Story 12 — Payload & Sensor Management (P3)

**Goal**: Payload types, gimbal control (pan/tilt/zoom), picture-in-picture synthetic camera feed.

**Independent Test**: Select a payload, operate gimbal, verify synthetic view updates.

- [X] T117 [P] [US12] Create `include/Payload.h` and `src/Payload.cpp` — payload entity with type, weight, powerDraw, gimbal state
- [X] T118 [US12] Update `src/exec.cpp` — per-tick payload power draw affects battery, gimbal commands processed
- [X] T119 [US12] Update `src/websockets.cpp` — add `cmd/payload/{id}/gimbal`, `cmd/payload/{id}/zoom` topics
- [X] T120 [P] [US12] Create `web/src/components/panels/PayloadPanel.tsx` — payload selector, gimbal pan/tilt joystick, zoom slider, payload status
- [X] T121 [P] [US12] Create `web/src/components/map/SensorView.tsx` — synthetic camera feed (render scene from drone POV using separate Three.js camera), picture-in-picture overlay
- [X] T122 [P] [US12] Create `web/src/components/ui/PIP.tsx` — picture-in-picture container, draggable, resizable

**Checkpoint**: Payloads selectable, gimbal controllable, synthetic camera feed in PIP. Independently testable.

---

## Phase E.2: User Story 10 — Communications Link Management (P3)

**Goal**: Link quality model, auto-failover, loss-of-link procedure.

**Independent Test**: Degrade primary link and verify auto-failover to backup link triggers.

- [X] T123 [P] [US10] Create `include/LinkManager.h` and `src/LinkManager.cpp` — link quality model (latency/jitter/packet loss per link type), failover threshold, heartbeat tracking
- [X] T124 [US10] Update `src/exec.cpp` — per-drone LinkManager runs per tick, triggers failover on threshold breach
- [X] T125 [P] [US10] Create `web/src/components/panels/LinkPanel.tsx` — signal strength bars per drone (LOS/SATCOM/CELLULAR), latency chart, packet loss %, failover event log

**Checkpoint**: Link quality visible, auto-failover works. Independently testable.

---

## Phase E.3: User Story 14 — Protocol Adapter Layer (P3)

**Goal**: Abstract ProtocolAdapter interface, MAVLink translator.

**Independent Test**: Send MAVLink heartbeat message, verify drone appears in UI within 5 seconds.

- [X] T126 [P] [US14] Create `include/ProtocolAdapter.h` — virtual interface with `translateTelemetry()`, `translateCommand()`, `connect()`, `disconnect()`
- [X] T127 [P] [US14] Create `src/MAVLinkAdapter.cpp` — implement ProtocolAdapter for MAVLink v2, parse HEARTBEAT, GLOBAL_POSITION_INT, SYS_STATUS, BATTERY_STATUS messages, emit internal ExternalTelemetry
- [X] T128 [US14] Update `src/websockets.cpp` — register protocol adapters, route incoming adapter telemetry to SimulationContext

**Checkpoint**: MAVLink drone connects and appears in UI. New drone types added via adapter. Independently testable.

---

## Phase E.4: User Story 15 — Multi-Tenant Fleet Operations (P3)

**Goal**: Tenant isolation, fleet-wide analytics, global admin view.

**Independent Test**: Two tenants each see only their own drones and missions.

- [X] T129 [P] [US15] Create `include/TenantManager.h` and `src/TenantManager.cpp` — tenant entity, per-tenant state isolation, admin view aggregator
- [X] T130 [US15] Update `src/websockets.cpp` — associate connections with tenant on auth, filter state broadcasts by tenant
- [X] T131 [P] [US15] Create `web/src/components/panels/TenantPanel.tsx` — tenant selector (admin only), per-tenant drone/mission/operator list
- [X] T132 [P] [US15] Create `web/src/components/panels/AnalyticsPanel.tsx` — aggregate charts: total flight hours across tenants, battery cycles, incident reports, active drone count

**Checkpoint**: Tenants isolated. Admin sees all tenants. Fleet analytics render. Independently testable.

---

## Phase F: Polish & Cross-Cutting Concerns

**Purpose**: Documentation, tests, performance, hardening

- [X] T133 Update `README.md` — architecture overview, all 5 phases, commands, WebSocket protocol reference
- [X] T134 [P] Add Catch2 tests for `Planner` in `src/tests.cpp` — smoothPath obstacle repulsion, localReplan dynamic obstacle, no-fly zone avoidance
- [X] T135 [P] Add Catch2 tests for `Mission` in `src/tests.cpp` — state transitions, waypoint advancement, RTH on completion, conditional branching
- [X] T136 [P] Add Catch2 tests for `HealthMonitor` in `src/tests.cpp` — alert generation by level, stale detection
- [X] T137 [P] Add Catch2 tests for `FlightRecorder` in `src/tests.cpp` — write/read/replay roundtrip, file rotation
- [X] T138 [P] Add Catch2 tests for `EmergencyEngine` in `src/tests.cpp` — rule evaluation, action execution, trigger priority
- [X] T139 [P] Add Catch2 tests for `MirrorContext` in `src/tests.cpp` — deep-copy fidelity, sandbox isolation, conflict detection
- [X] T140 [P] Add Catch2 tests for `Payload` in `src/tests.cpp` — gimbal commands, power draw calculation
- [X] T141 [P] Add Catch2 tests for `LinkManager` in `src/tests.cpp` — failover threshold, heartbeat expiry, LoL procedure execution
- [X] T142 AddressSanitizer run — verify zero memory leaks across all test cases
- [X] T143 Performance profiling — verify 20Hz tick rate with 100 drones, <100ms obstacle replan
- [X] T144 Update `quickstart.md` — reflect all implemented phases and commands

---

## Dependencies & Execution Order

### Phase Dependencies

| Phase | Depends On | Description |
|-------|-----------|-------------|
| A.1 Setup | — | Project initialization, no dependencies |
| A.2 Foundational | A.1 | Memory safety, WS robustness — BLOCKS all stories |
| A.3 US1 Dashboard | A.1, A.2 | UI overhaul, zustand store |
| A.4 US2 Smooth Pathfinding | A.1, A.2 | Backend Planner + frontend PathLine |
| A.5 US16 Emergency Engine | A.1, A.2 | Rule engine, checklists |
| B.1 US4 Mission | A.1–A.5 | Mission plan + executor |
| B.2 US5 Health Monitor | A.1–A.5 | Health checks + alert dashboard |
| B.3 US6 Flight Recorder | A.1–A.5 | JSON-lines recorder + replay |
| B.4 US11 No-Fly Zones | A.1–A.5, B.1 | Geofenced zones |
| C.1 US3 Real Telemetry | A.1–A.5, B.1 | External drone ingestion |
| C.2 US8 Digital Twin | A.1–A.5, B.1, C.1 | Mirror mode |
| C.3 US7 Geospatial | A.1–A.5, B.1, B.4 | Weather/terrain/airspace |
| D.1 US9 C2 Hierarchy | A.1–A.5, C.1 | Roles + permissions |
| D.2 US13 Dynamic Missions | A.1–A.5, B.1 | Live editing + branching |
| D.3 US11 Collision Avoidance | A.1–A.5, B.1, D.2 | Deconfliction + UTM |
| E.1 US12 Payload | A.1–A.5, C.1 | Sensors + gimbal |
| E.2 US10 Link Mgmt | A.1–A.5, C.1 | Link quality + failover |
| E.3 US14 Protocol | A.1–A.5, C.1 | MAVLink adapter |
| E.4 US15 Multi-Tenant | A.1–A.5, D.1 | Tenant isolation |
| F Polish | All phases | Tests, docs, perf |

### Within Each User Story
- Model/header first → Implementation → WebSocket integration → UI component → Store update

### Parallel Opportunities
- A.1 Setup tasks T001–T015: all [P] tasks run in parallel (independent files)
- A.3 US1 Dashboard: T023–T025 independent component tasks run in parallel
- A.4 US2 Pathfinding: T040–T041 backend tasks parallel with T044 frontend task
- All B-tier stories are independent of each other (B.1, B.2, B.3, B.4 can run in parallel after A-phase)
- All C-tier stories are independent of each other
- D.1 C2 runs in parallel with D.2 Dynamic Missions and D.3 Collision Avoidance
- E.1 Payload, E.2 Link, E.3 Protocol, E.4 Multi-Tenant are all independent

---

## Implementation Strategy

### Recommended MVP (Phase A: T001–T051)

```
1. Setup (T001–T015)        →  Foundation ready
2. Foundational (T016–T022) →  Memory-safe, WS robust
3. US1 Dashboard (T023–T039) →  Professional UI complete  ✓ MVP
4. US2 Pathfinding (T040–T045) →  Smooth avoidance       ✓ MVP+
5. US16 Emergency (T046–T051)  →  Safety engine           ✓ MVP+
```

### Incremental Delivery

| Milestone | Tasks | What Works |
|-----------|-------|------------|
| M1: Foundation | T001–T022 | Buildable project, safe backend |
| M2: Dashboard | T023–T039 | Professional UI, dark theme, 3D globe, controls |
| M3: Smart Drones | T040–T051 | Smooth avoidance, emergency rules |
| M4: Operations | T052–T077 | Missions, health, recorder, no-fly zones |
| M5: Real Drones | T078–T102 | External telemetry, digital twin, geospatial |
| M6: Fleet C2 | T103–T116 | Multi-operator, collision avoidance, live editing |
| M7: Complete | T117–T144 | Payloads, links, protocols, multi-tenant, polish |

### Parallel Example: Phase A Kickoff

```bash
# All Setup [P] tasks launch together:
# T013 (types.ts), T014 (constants.ts), T015 (geo.ts) — frontend lib
# T001 (Planner.h), T002 (Mission.h), T005 (GeoUtils.h) — backend headers

# After Setup, A.2 Foundational (T016–T022) runs sequentially

# After Foundational, US1 Dashboard tasks launch:
# T023 (theme.ts) + T024 (globals.css) + T025 (layout.tsx) — theme, parallel
# T028 (Toggle.tsx) + T029 (Alert.tsx) + T030 (ProgressBar.tsx) — components, parallel
```

---

## Task Summary

| Phase | Tasks | Count | Priority | Independently Testable |
|-------|-------|-------|----------|----------------------|
| A.1 Setup | T001–T015 | 15 | P0 | No (infrastructure) |
| A.2 Foundational | T016–T022 | 7 | P0 | No (blocking) |
| A.3 US1 Dashboard | T023–T039 | 17 | P1 | Yes — load UI, see panels |
| A.4 US2 Pathfinding | T040–T045 | 6 | P1 | Yes — obstacle avoidance test |
| A.5 US16 Emergency | T046–T051 | 6 | P1 | Yes — trigger GPS loss scenario |
| B.1 US4 Mission | T052–T059 | 8 | P2 | Yes — 5-WP mission loop |
| B.2 US5 Health | T060–T066 | 7 | P2 | Yes — kill WS, see failure |
| B.3 US6 Recorder | T067–T072 | 6 | P2 | Yes — record then replay |
| B.4 US11 No-Fly | T073–T077 | 5 | P2 | Yes — create zone, avoid |
| C.1 US3 Telemetry | T078–T083 | 6 | P1 | Yes — send synthetic telemetry |
| C.2 US8 Twin | T084–T090 | 7 | P1 | Yes — clone → sandbox → promote |
| C.3 US7 Geospatial | T091–T099 | 9 | P2 | Yes — enable weather overlay |
| C.4 LoL | T100–T102 | 3 | P2 | Yes — link loss → execute procedure |
| D.1 US9 C2 | T103–T106 | 4 | P2 | Yes — two roles, verify restriction |
| D.2 US13 Dynamic | T107–T111 | 5 | P2 | Yes — drag WP mid-mission |
| D.3 US11 Collision | T112–T116 | 5 | P2 | Yes — intersecting paths auto-adjust |
| E.1 US12 Payload | T117–T122 | 6 | P3 | Yes — gimbal control, camera feed |
| E.2 US10 Link | T123–T125 | 3 | P3 | Yes — degrade link, auto-failover |
| E.3 US14 Protocol | T126–T128 | 3 | P3 | Yes — MAVLink heartbeat → UI |
| E.4 US15 Tenant | T129–T132 | 4 | P3 | Yes — two tenants, isolated views |
| F Polish | T133–T144 | 12 | P3 | No (testing/docs) |

**Total**: 144 tasks (51 MVP Phase A, 26 Phase B, 25 Phase C, 14 Phase D, 16 Phase E, 12 polish)
