# Research: Military-Grade UAV Engine

## 1. Pathfinding Strategy: Smooth Obstacle-Aware Replanning

### Decision
Implement a **hybrid approach**: (a) `GlobalPathfinder` (existing A*) for long-range pre-planning, (b) **new trajectory re-planner** using a **Dubins-path-based local replanner** with **potential-field obstacle repulsion** for smooth local adjustments, and (c) **RRT*-inspired sampling** for complex obstacle clusters.

### Rationale
- Pure A* on a discrete grid produces jerky 8-directional paths — unsuitable for smooth UAV flight
- Potential fields alone get stuck in local minima (concave obstacles, U-shaped walls)
- RRT* (Rapidly-exploring Random Tree star) is well-suited for continuous configuration spaces with obstacles but can be slow for real-time
- **Hybrid**: Use the existing A* `GlobalPathfinder` for coarse route, then use a **spline-based local planner** that pushes waypoints away from obstacle surfaces using a continuous repulsion function. For complex obstacle fields, fall back to RRT* sampling with smoothing
- Dubins curves (minimum-turn-radius arcs) produce flyable paths inherently

### Implementation approach:
1. Keep `GlobalPathfinder::findPath()` for initial A* route
2. After A* produces waypoints, run a **path smoother** that iteratively adjusts waypoints using obstacle repulsion vectors + path curvature minimization
3. For dynamic obstacles: run a lightweight **local replanner** each tick that checks if any waypoint segment intersects an obstacle. If so, insert intermediate avoidance waypoints using RRT* sampling in the local problem space (not full global replan)
4. Tune repulsion strength so paths arc smoothly rather than zigzag

### Alternatives considered
- Pure potential fields — local minima problem with concave obstacles
- Pure RRT* — too slow for 20Hz replanning with 100 drones; better as a background thread
- Pure A* grid — jerky paths, needs expensive post-smoothing
- Hybrid A* (continuous steering A*) — good but complex to implement; our hybrid approach achieves same effect more simply

---

## 2. Military-Grade UI Design System

### Decision
Adopt a **dark tactical theme** inspired by military C2 (Command & Control) systems with:
- Background: `#0a0a0f` (near-black with blue undertone)
- Surface: `#1a1a2e` with `rgba(255,255,255,0.05)` border
- Accent: `#00d4ff` (cyan) for primary actions / active state
- Danger: `#ff3355` (red) for alerts / low battery
- Success: `#00ff88` (green) for nominal status
- Warning: `#ffaa00` (amber) for caution
- Text: `#e0e0e0` primary, `#888899` secondary
- Glassmorphism: `backdrop-blur-xl bg-black/40` for panels
- Typography: JetBrains Mono (monospace) for telemetry data, Inter for UI labels

### Rationale
- Military C2 systems universally use dark themes to reduce eye strain during extended operations
- High-contrast accent colors ensure critical information is immediately visible
- Glassmorphism provides visual depth without distracting from data
- Monospace telemetry data conveys precision and professionalism

### Implementation
- CSS custom properties in `globals.css` for all theme tokens
- TailwindCSS v4 extension using `@theme` directive
- Framer Motion for panel transitions (slide-in, fade)
- Radar sweep animation using CSS `conic-gradient` + `@keyframes`

### Alternatives considered
- Material Design 3 — too generic, not tactical
- Cyberpunk theme — too stylized, not professional
- Flat minimal — lacks visual hierarchy for complex data

---

## 3. Real-Drone Integration Protocol

### Decision
Extend the existing WebSocket pub/sub system with standardized telemetry topics:
- `telemetry/external/{drone_id}` — incoming real-drone telemetry (same JSON format as internal)
- `cmd/mission/{drone_id}/upload` — upload mission plan to drone
- `cmd/mission/{drone_id}/start` — command drone to begin mission
- `cmd/mission/{drone_id}/abort` — abort and return to home
- `state/drone/{drone_id}` — drone-reported state (ARMED, FLYING, LANDED, EMERGENCY, MISSION)

### Rationale
- Reuses the existing WebSocket infrastructure without introducing a new transport layer
- Same pub/sub routing already implemented in `websockets.cpp`
- JSON format is universally supported and human-readable for debugging
- Topic-based routing allows selective subscription (frontend can subscribe only to relevant drones)

### Implementation
- `ExternalTelemetry` struct already exists in `CommandEvent.h` — extend with `altitude`, `status` fields
- Add `MissionCommand` struct for mission upload/start/abort
- Add `SIMULATION_TICK_HZ` rate-limit for external telemetry to prevent flooding

### Alternatives considered
- MAVLink protocol — standard for drone communication but requires C MAVLink library + complex message parsing. Can add as v2 feature
- gRPC streaming — more structured but adds protobuf dependency and complexity
- MQTT — better for edge/cloud scenarios but another service to deploy

---

## 4. Mission Planning & Autonomous Execution

### Decision
Add a `Mission` system with:
- **MissionPlan**: ordered list of waypoints, home base, calculated ETA/battery
- **MissionState**: IDLE, UPLOADED, IN_PROGRESS, PAUSED, COMPLETED, FAILED, RTH
- **MissionExecutor**: runs in the UAV's update loop, advances waypoint on arrival (within 100m radius), triggers RTH on completion or fault
- **Mission upload**: WebSocket topic `cmd/mission/{id}/upload` sends JSON mission payload

### Rationale
- Missions are the core of autonomous operation
- State machine makes execution predictable and testable
- Decoupling mission plan from execution allows pre-planning and upload
- RTH on completion or fault is a fail-safe requirement

### Alternatives considered
- Waypoint list only (no mission abstraction) — lacks state tracking, failover
- External mission planner (e.g., QGC) — coupling to external tool; we want self-contained

---

## 5. Health Monitoring

### Decision
Add a `HealthMonitor` class that runs as a periodic check (1Hz):
- Engine tick rate (warn if <15Hz)
- WebSocket connection count and last message timestamp
- Per-drone: battery level, last telemetry timestamp, mission state
- Alert levels: NOMINAL, CAUTION, WARNING, CRITICAL
- Broadcasts health status on `state/health` topic

### Rationale
- Health monitoring is essential for military-grade reliability
- Separate from simulation logic so it doesn't affect tick timing
- Alert levels allow graduated response (CAUTION → log, WARNING → notify operator, CRITICAL → auto-RTH)

### Alternatives considered
- Integrated into exec loop — would pollute simulation logic
- External monitoring service — over-engineering for v1

---

## 6. Flight Data Recorder

### Decision
Add a `FlightRecorder` class that:
- Writes JSON-lines (`{timestamp, uav_id, position, heading, speed, battery, altitude}\n`) to a file
- Rotates files every 1 hour or 100MB
- Can be toggled on/off at runtime
- Filename format: `flightlog_{YYYYMMDD_HHMMSS}.jsonl`

### Rationale
- JSON-lines is append-only, streaming-friendly, and parseable line-by-line
- No database dependency
- Simple replay: read lines back, emit as `uav_update` messages at original timestamps
- File rotation prevents unbounded disk usage

### Alternatives considered
- SQLite — heavier dependency, overkill for sequential time-series
- Protobuf binary — more compact but harder to debug and replay
- CSV — less expressive for nested telemetry

---

## 7. Zustand State Management

### Decision
Adopt **zustand** for frontend global state (already in package.json but unused):
- `useSimulationStore`: UAVs, obstacles, charging stations, selected entity, connection status, health status
- WebSocket hook updates the store
- Panels and map components subscribe to relevant slices

### Rationale
- Existing dependency already in `package.json` — no new installs
- Lightweight (~1KB), no boilerplate, works with React 19
- Components only re-render when their subscribed slice changes
- Avoids prop-drilling through deeply nested component tree

### Alternatives considered
- React Context — causes unnecessary re-renders for all consumers
- Redux — too much boilerplate for this scale
- Jotai/Recoil — similar to zustand but not already installed

---

## 8. Testing Strategy

### Decision
- **C++ backend**: Extend existing Catch2 v3 suite. Add tests for: Planner (smooth avoidance), Mission (state transitions), FlightRecorder (write/read/replay), HealthMonitor (detection logic), enhanced UAV tests for RTH behavior
- **Frontend**: Vitest + React Testing Library for component tests, MSW for WebSocket mocking

### Rationale
- Catch2 already integrated via FetchContent — no new deps
- Vitest is Next.js-compatible and faster than Jest
- WebSocket hook tests need a mock server — MSW v2 supports WebSocket mocking

### Alternatives considered
- Google Test — not already in project; Catch2 is lighter
- Cypress/Playwright — E2E tests are valuable but separate from unit/component testing
