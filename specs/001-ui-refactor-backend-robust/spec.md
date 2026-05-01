# Feature Specification: UI Refactor & Backend Robustness

**Feature Branch**: `001-ui-refactor-backend-robust`
**Created**: 2026-05-01
**Status**: Draft
**Input**: "Create a thorough plan based on your analysis. I need to refactor the UI aswell, need to make it look professional and elegant. At the same time the backend sim should be robust."

## User Scenarios & Testing

### User Story 1 - Professional Simulation Dashboard (Priority: P1)

As a UAV operator, I want a polished, elegant control dashboard with a dark-theme UI, intuitive controls, and smooth animations so that I can monitor and command drone swarms effectively.

**Why this priority**: The UI is the primary user-facing interface. Without a professional appearance, the project lacks credibility and usability.

**Independent Test**: Can be fully tested by loading the web UI and verifying all panels render correctly, controls respond, and the globe displays real-time data.

**Acceptance Scenarios**:

1. **Given** the simulation is running, **When** I load the web UI, **Then** I see a dark-themed dashboard with a 3D globe, telemetry panel, and control panel
2. **Given** the simulation is running, **When** I click on a UAV marker, **Then** a telemetry detail panel appears with lat/lon/heading/battery info
3. **Given** the control panel is visible, **When** I adjust the velocity slider, **Then** the UAV responds within 2 ticks
4. **Given** multiple swarm groups exist, **When** I click a group tab, **Then** the telemetry panel updates to show that group's metrics

---

### User Story 2 - Robust Simulation Engine (Priority: P1)

As a developer, I want the C++ simulation engine to be memory-safe, testable, and free of undefined behavior so that the simulation can run reliably for extended periods.

**Why this priority**: The backend is the foundation. Crashes or memory leaks undermine the entire project.

**Independent Test**: Can be verified by running the test suite (zero leaks, all tests pass) and running the sim for 24h without crash.

**Acceptance Scenarios**:

1. **Given** the simulation runs, **When** I spawn 100 drones across multiple groups, **Then** the simulation continues running without memory growth (no leaks)
2. **Given** the simulation runs, **When** I send 1000 rapid WebSocket commands, **Then** all commands are processed without data races
3. **Given** the simulation is compiled with AddressSanitizer, **When** I run the full test suite, **Then** zero memory errors are reported

---

### User Story 3 - Simplified Setup & Documentation (Priority: P2)

As a new contributor, I want clear build instructions and a working test suite so that I can get started in under 5 minutes.

**Why this priority**: Onboarding friction reduces the chance of community contributions.

**Independent Test**: A fresh clone followed by the README instructions produces a working build and passing tests.

**Acceptance Scenarios**:

1. **Given** a fresh clone, **When** I follow the build instructions, **Then** `uav_sim` compiles without errors
2. **Given** a successful build, **When** I run `ctest`, **Then** all tests pass
3. **Given** the web directory, **When** I run `npm install && npm run build`, **Then** the web app builds without errors

---

### User Story 4 - External Telemetry Integration (Priority: P3)

As an operator, I want to see real external drone telemetry alongside simulated drones so that I can monitor both real and simulated assets in one view.

**Why this priority**: This differentiates the project from basic simulators, but existing stubs already partially implement it.

**Independent Test**: Send a mock MAVLink-style telemetry message via WebSocket and verify the new drone appears in the UI.

**Acceptance Scenarios**:

1. **Given** the simulation is running, **When** an external telemetry message is received via WebSocket, **Then** the drone appears on the globe with the correct position
2. **Given** external telemetry stops, **When** 30 seconds pass without updates, **Then** the drone is marked as stale

### Edge Cases

- What happens when the WebSocket server port is already in use?
- How does the system handle malformed JSON from WebSocket clients?
- What happens when battery reaches 0%?
- How does the simulation behave with zero drones?
- What happens when a target waypoint is unreachable (over ocean, no charging stations)?
- How does the UI handle 100+ simultaneous UAV markers?

## Requirements

### Functional Requirements

- **FR-001**: System MUST provide a polished dark-theme web UI with 3D globe visualization
- **FR-002**: System MUST support real-time WebSocket communication between C++ engine and web UI
- **FR-003**: System MUST manage multiple UAV swarm groups with independent control
- **FR-004**: System MUST handle obstacle generation (static and dynamic) with local avoidance
- **FR-005**: System MUST support A* global pathfinding with charging station waypoints
- **FR-006**: System MUST eliminate all raw pointer usage in favor of smart pointers or RAII containers
- **FR-007**: System MUST provide a test suite covering Geo calculations, Pathfinder, UAV physics, and WebSocket messaging
- **FR-008**: System MUST support spawning and controlling drones via WebSocket commands
- **FR-009**: System MUST display real-time telemetry (position, heading, battery, speed) in the UI
- **FR-010**: System MUST support external drone telemetry ingestion via WebSocket pub/sub topics
- **FR-011**: System MUST eliminate global mutable state for simulation-WebSocket IPC
- **FR-012**: System MUST pin all third-party dependency versions for reproducible builds
- **FR-013**: System MUST provide environment-based configuration for WebSocket URL and port

### Key Entities

- **UAV**: A drone agent with position, heading, speed, battery, group membership, physics-based movement
- **Obstacle**: Static or dynamic hazard with position, radius, optional group scoping
- **Waypoint**: Named geographic coordinate used in pathfinding
- **ChargingStation**: POI with position, name, and charging rate for battery replenishment
- **SwarmGroup**: Named collection of UAVs sharing target assignments and settings
- **SimulationServer**: WebSocket server broadcasting state and accepting commands via pub/sub topics

## Success Criteria

### Measurable Outcomes

- **SC-001**: UI page load completes in under 3 seconds on a modern browser
- **SC-002**: Simulation tick rate maintains at least 20Hz with 50 concurrent drones
- **SC-003**: Test suite achieves >80% line coverage on core modules (Geo, Pathfinder, UAV)
- **SC-004**: Zero memory leaks detected by AddressSanitizer in a 1-hour simulation run
- **SC-005**: All raw `new`/`delete` usage is eliminated; code uses `std::unique_ptr` or containers exclusively
- **SC-006**: WebSocket command latency stays under 50ms p99 for command → UI update round trip
- **SC-007**: Build is reproducible across 3 consecutive `cmake --build` invocations

## Assumptions

- Users will run the C++ backend and web frontend on the same machine during development
- The simulation runs in flat geographic space (WGS84 ellipsoid accuracy not required)
- Node.js 18+ is available for the web frontend
- The project targets Linux development primarily (Windows support is secondary)
- External drone telemetry uses the same pub/sub topic format as internal messages
