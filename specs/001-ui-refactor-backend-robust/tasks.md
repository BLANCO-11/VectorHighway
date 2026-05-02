---

description: "Task list for UI Refactor & Backend Robustness feature"
---

# Tasks: UI Refactor & Backend Robustness

**Input**: Design documents from `/specs/001-ui-refactor-backend-robust/`
**Prerequisites**: plan.md (required), spec.md (required), research.md, data-model.md, contracts/

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (US1, US2, US3, US4)
- Include exact file paths in descriptions

## Path Conventions

- **C++ backend**: `include/`, `src/` at repository root
- **Web frontend**: `web/src/`

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Remove dead code and clean up project structure

- [X] T001 Remove dead code files (src/main.cpp, web/src/hooks/useWebSocket.ts, web/src/store/useSimulationStore.ts)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Build system hardening and shared type infrastructure that MUST be complete before user stories

**CRITICAL**: No user story work can begin until this phase is complete

- [X] T002 [P] Pin websocketpp to v0.8.2 tag in CMakeLists.txt
- [X] T003 [P] Add Catch2 v3.7.1 FetchContent declaration and test target in CMakeLists.txt
- [X] T004 Create Types.h with common type aliases and constants in include/Types.h

**Checkpoint**: Foundation ready — user story implementation can proceed

---

## Phase 3: User Story 1 — Professional Simulation Dashboard (Priority: P1) 🎯 MVP

**Goal**: Transform monolithic UI into a polished, dark-themed dashboard with reusable components, 3D globe, telemetry panels, and smooth animations

**Independent Test**: Load web UI and verify: dark-themed dashboard renders, 3D globe with Earth texture displays, UAV markers appear, control panel responds to slider changes, telemetry panel shows selected entity data, battery chart updates in real-time

### Implementation for User Story 1

**UI Base Components:**

- [X] T005 [P] [US1] Create glassmorphism Card component in web/src/components/ui/Card.tsx
- [X] T006 [P] [US1] Create styled Button component with variants in web/src/components/ui/Button.tsx
- [X] T007 [P] [US1] Create styled Slider component with labels in web/src/components/ui/Slider.tsx

**3D Map Components:**

- [X] T008 [P] [US1] Create Earth sphere with atmosphere glow component in web/src/components/map/Earth.tsx
- [X] T009 [P] [US1] Create UAVMarker with HTML heading indicator and lerp animation in web/src/components/map/UAVMarker.tsx
- [X] T010 [P] [US1] Create ObstacleMarker with dynamic/static visual distinction in web/src/components/map/ObstacleMarker.tsx
- [X] T011 [P] [US1] Create DestinationMarker with cyan pin icon in web/src/components/map/DestinationMarker.tsx
- [X] T012 [P] [US1] Create ConnectionLines for great-circle paths and position history in web/src/components/map/ConnectionLines.tsx

**Panel Components:**

- [X] T013 [P] [US1] Create ControlPanel with velocity, energy drain, turn rate, charging rate sliders in web/src/components/panels/ControlPanel.tsx
- [X] T014 [P] [US1] Create SwarmPanel with group tabs and add-group input in web/src/components/panels/SwarmPanel.tsx
- [X] T015 [P] [US1] Create TelemetryPanel showing selected entity lat/lon/heading/battery in web/src/components/panels/TelemetryPanel.tsx
- [X] T016 [P] [US1] Create BatteryChart with recharts LineChart in web/src/components/panels/BatteryChart.tsx

**Integration Tasks:**

- [X] T017 [US1] Implement dark theme CSS custom properties and glassmorphism utilities in web/src/app/globals.css
- [X] T018 [US1] Refactor page.tsx to orchestrate new component hierarchy (remove inline components) in web/src/app/page.tsx
- [X] T019 [US1] Update layout.tsx with proper metadata (title, description) and favicon in web/src/app/layout.tsx
- [X] T020 [US1] Connect useSimulationWebSocket hook to new component tree and wire control messages in web/src/app/page.tsx

**Checkpoint**: User Story 1 should be fully functional — professional dashboard with all panels, controls, and globe visualization

---

## Phase 4: User Story 2 — Robust Simulation Engine (Priority: P1)

**Goal**: Memory-safe, testable C++ simulation engine with no undefined behavior, zero leaks, and proper encapsulation replacing global mutable state

**Independent Test**: Run `ctest` — all tests pass. Compile with AddressSanitizer, run 1-hour sim — zero memory errors. Spawn 100 drones — no memory growth.

### Implementation for User Story 2

**New Headers:**

- [X] T021 [P] [US2] Create CommandEvent.h with CommandEvent struct and ExternalTelemetry struct in include/CommandEvent.h
- [X] T022 [P] [US2] Create SimulationContext.h with encapsulated IPC state class in include/SimulationContext.h

**Refactoring:**

- [X] T023 [US2] Refactor exec.cpp to use std::unique_ptr for obstacles, SimulationContext for IPC, remove raw new/delete
- [X] T024 [US2] Refactor websockets.cpp to use SimulationContext pointer instead of extern globals

**Tests:**

- [X] T025 [P] [US2] Write Geo utility tests (Coordinate validation, distance calculation) in src/tests.cpp
- [X] T026 [P] [US2] Write Pathfinder tests (A* pathfinding, local avoidance) in src/tests.cpp
- [X] T027 [P] [US2] Write UAV physics tests (movement, battery drain, heading interpolation) in src/tests.cpp

**Configuration:**

- [X] T028 [US2] Add --port CLI argument and UAV_SIM_PORT env var support in exec.cpp
- [X] T029 [US2] Fix WebSocket++ type errors and ensure C++17 STL compatibility in websockets.cpp

**Checkpoint**: Backend should be fully robust — test suite passes, no leaks, no globals, configurable port

---

## Phase 5: User Story 3 — Simplified Setup & Documentation (Priority: P2)

**Goal**: Clear build instructions and working test suite so new contributors can get started in under 5 minutes

**Independent Test**: Fresh clone → README instructions → `cmake --build` succeeds → `ctest` passes → `npm install && npm run build` succeeds

### Implementation for User Story 3

- [X] T030 [P] [US3] Fix README cd path (Locator → VectorHighway) and update build/run instructions in README.md
- [X] T031 [P] [US3] Add CMake build options, test suite commands, and WebSocket config docs in README.md

**Checkpoint**: README provides accurate setup guide; build and test commands work from scratch

---

## Phase 6: User Story 4 — External Telemetry Integration (Priority: P3)

**Goal**: Ingest real external drone telemetry via WebSocket and display alongside simulated drones

**Independent Test**: Send mock MAVLink-style telemetry via WebSocket → new drone appears on globe with correct position. Stop updates → drone marked stale after 30s.

### Implementation for User Story 4

- [X] T032 [P] [US4] Add external telemetry ingestion fields and stale-tracking timestamps to SimulationContext.h
- [X] T033 [US4] Add telemetry/external/{droneID} topic handler in websockets.cpp
- [X] T034 [US4] Add stale-drone detection (30s timeout) and removal in exec.cpp simulation tick
- [X] T035 [US4] Broadcast external drone states alongside internal UAV updates via WebSocket

**Checkpoint**: External drones appear on globe, shown as distinct from simulated drones, removed when stale

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Verification, cleanup, and hardening across all stories

- [X] T036 Run test suite and verify all tests pass with ctest
- [X] T037 Remove unused zustand and socket.io-client dependencies from web/package.json
- [X] T038 Run AddressSanitizer build and verify zero memory errors in 1-hour simulation

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion — BLOCKS all user stories
- **User Stories (Phase 3-6)**: All depend on Foundational phase completion
  - US1 (UI) and US2 (Backend) are P1 — can proceed in parallel (different tech stacks)
  - US3 (Docs) can start after Phase 2, runs parallel to US1/US2
  - US4 (Telemetry) depends on US2 backend foundation
- **Polish (Phase 7)**: Depends on all desired user stories

### User Story Dependencies

- **US1 (P1)**: Can start after Phase 2 — frontend-only, no backend code changes
- **US2 (P1)**: Can start after Phase 2 — backend-only, no frontend changes
- **US3 (P2)**: Can start after Phase 2 — documentation only, parallel with US1/US2
- **US4 (P3)**: Depends on US2 (uses SimulationContext and websockets.cpp refactor)

### Within Each User Story

- Models/headers before services/implementations
- Individual components (marked [P]) before integration
- Core implementation before configuration/enhancement
- Story complete before moving to next priority

### Parallel Opportunities

- Phase 2 tasks T002, T003 marked [P] — can run in parallel
- Phase 3 UI components T005-T016 marked [P] — all independent files, can run in parallel
- Phase 4 headers T021-T022 marked [P] — independent headers
- Phase 4 tests T025-T027 marked [P] — independent test sections in same file
- Phase 5 docs T030-T031 marked [P] — independent doc sections
- Phase 6 T032 can run before T033-T035 (SimulationContext changes needed first)
- US1 and US2 can be implemented by different developers in parallel

---

## Parallel Example: User Story 1

```bash
# Launch all UI base components together:
Task: "Create Card component in web/src/components/ui/Card.tsx"
Task: "Create Button component in web/src/components/ui/Button.tsx"
Task: "Create Slider component in web/src/components/ui/Slider.tsx"

# Launch all 3D map components together:
Task: "Create Earth component in web/src/components/map/Earth.tsx"
Task: "Create UAVMarker component in web/src/components/map/UAVMarker.tsx"
Task: "Create ObstacleMarker component in web/src/components/map/ObstacleMarker.tsx"
Task: "Create DestinationMarker component in web/src/components/map/DestinationMarker.tsx"
Task: "Create ConnectionLines component in web/src/components/map/ConnectionLines.tsx"

# Launch all panel components together:
Task: "Create ControlPanel in web/src/components/panels/ControlPanel.tsx"
Task: "Create SwarmPanel in web/src/components/panels/SwarmPanel.tsx"
Task: "Create TelemetryPanel in web/src/components/panels/TelemetryPanel.tsx"
Task: "Create BatteryChart in web/src/components/panels/BatteryChart.tsx"
```

## Parallel Example: User Story 2

```bash
# Launch new headers together:
Task: "Create CommandEvent.h in include/CommandEvent.h"
Task: "Create SimulationContext.h in include/SimulationContext.h"

# Launch tests together (after headers are done):
Task: "Write Geo tests in src/tests.cpp"
Task: "Write Pathfinder tests in src/tests.cpp"
Task: "Write UAV physics tests in src/tests.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup (cleanup)
2. Complete Phase 2: Foundational (build system + shared types)
3. Complete Phase 3: User Story 1 (UI Dashboard)
4. **STOP and VALIDATE**: Load web UI, verify all panels and globe render correctly
5. Deploy/demo if ready

### Incremental Delivery

1. Setup + Foundational → Foundation ready
2. Add US1 (UI Dashboard) → Test independently → Deploy/Demo (MVP!)
3. Add US2 (Backend Robustness) → Test independently → Deploy/Demo
4. Add US3 (Documentation) → Test independently → Deploy/Demo
5. Add US4 (External Telemetry) → Test independently → Deploy/Demo
6. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1 (UI — frontend components)
   - Developer B: User Story 2 (Backend — C++ refactoring)
   - Developer C: User Story 3 (Docs — can start immediately after Phase 2)
3. Developer A and B merge independently (different tech stacks, no file conflicts)
4. Developer C merges independently (documentation only)
5. Developer D (or A after US1) picks up US4 once US2 foundations are in place

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story is independently completable and testable
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence
- US1 and US2 are both P1 — can be implemented in parallel since they touch different files and technologies
