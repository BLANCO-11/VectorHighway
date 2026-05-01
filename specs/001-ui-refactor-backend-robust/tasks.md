---

description: "Task list for UI Refactor & Backend Robustness"
---

# Tasks: UI Refactor & Backend Robustness

**Input**: Design documents from `specs/001-ui-refactor-backend-robust/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Organization**: Tasks grouped by user story for independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (US1-US4)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Cleanup dead code, remove unused dependencies, prepare for refactoring

- [ ] T001 Remove empty dead code: delete `src/main.cpp`
- [ ] T002 [P] Remove unused frontend code: delete `web/src/hooks/useWebSocket.ts` and `web/src/store/useSimulationStore.ts`
- [ ] T003 [P] Remove unused npm dependency `socket.io-client` from `web/package.json`
- [ ] T004 [P] Remove unused orphan component `web/src/components/globe/Globe.tsx` (page.tsx uses inline rendering)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before user story work begins

- [ ] T005 Extract shared inter-thread structs to `include/CommandEvent.h` (CommandEvent, ExternalTelemetry) eliminating duplicate definitions in `src/websockets.cpp` and `src/exec.cpp`
- [ ] T006 [P] Create `include/SimulationContext.h` — encapsulate command queue + mutex + external telemetry vector into a class
- [ ] T007 [P] Pin websocketpp to tag `0.8.2` (replace `GIT_TAG master`) in `CMakeLists.txt`
- [ ] T008 [P] Remove `CMAKE_POLICY_VERSION_MINIMUM 3.5` hack and fix CMake compatibility properly in `CMakeLists.txt`
- [ ] T009 [P] Add Catch2 v3.7.1 via FetchContent and enable test target in `CMakeLists.txt` (uncomment `add_executable(uav_tests)`)
- [ ] T010 [P] Create component directory structure: `web/src/components/panels/`, `web/src/components/map/`, `web/src/components/ui/`

**Checkpoint**: Foundation ready — shared headers created, deps pinned, test framework wired, component directories exist

---

## Phase 3: User Story 2 - Robust Simulation Engine (Priority: P1)

**Goal**: Eliminate memory leaks, global mutable state, and undefined behavior; make simulation memory-safe and testable

**Independent Test**: Simulation runs 1 hour with 50 drones with zero memory growth (monitor RSS); AddressSanitizer reports zero errors

### Implementation for User Story 2

- [ ] T011 [P] [US2] Refactor `SimulationContext` into `exec.cpp` — replace globals `g_ui_simMtx`, `g_ui_commands`, `g_ui_externalTelemetry` with a `SimulationContext` instance; pass pointer to WebSocket server
- [ ] T012 [US2] Refactor `src/websockets.cpp` — accept `SimulationContext*` instead of writing to globals; update `MockSimulationServer` to hold context reference
- [ ] T013 [P] [US2] Replace raw `new`/`delete` for obstacles in `src/exec.cpp` with `std::vector<std::unique_ptr<Obstacle>>`
- [ ] T014 [US2] Add `--port` CLI argument parsing in `src/exec.cpp` and `UAV_SIM_PORT` env var support; pass port to `server.start()`
- [ ] T015 [US2] Write Catch2 unit tests for `Coordinate::distanceTo` and `bearingTo` in `src/tests.cpp`
- [ ] T016 [P] [US2] Write Catch2 unit tests for `GlobalPathfinder::findPathAStar` in `src/tests.cpp`
- [ ] T017 [P] [US2] Write Catch2 unit tests for `UAV::update` kinematics and `UAV::adjustHeading` in `src/tests.cpp`

**Checkpoint**: Backend is memory-safe (unique_ptr everywhere), no globals, configurable port, test suite covers Geo/Pathfinder/UAV

---

## Phase 4: User Story 1 - Professional Simulation Dashboard (Priority: P1)

**Goal**: Elegant, professional dark-theme dashboard with decomposed components and smooth animations

**Independent Test**: Load web UI connected to running simulation — all panels render, globe shows drones, controls respond in real-time

### Implementation for User Story 1

- [ ] T018 [P] [US1] Create `Card` UI component in `web/src/components/ui/Card.tsx` with glassmorphism styling
- [ ] T019 [P] [US1] Create `Slider` UI component in `web/src/components/ui/Slider.tsx` with styled range input
- [ ] T020 [P] [US1] Create `Button` UI component in `web/src/components/ui/Button.tsx` with variant styles
- [ ] T021 [US1] Create `ControlPanel` component in `web/src/components/panels/ControlPanel.tsx` — extract velocity, energy drain controls from `page.tsx`
- [ ] T022 [US1] Create `SwarmPanel` component in `web/src/components/panels/SwarmPanel.tsx` — extract group tabs + add group from `page.tsx`
- [ ] T023 [US1] Create `TelemetryPanel` component in `web/src/components/panels/TelemetryPanel.tsx` — extract selected entity telemetry display from `page.tsx`
- [ ] T024 [US1] Create `BatteryChart` component in `web/src/components/panels/BatteryChart.tsx` — extract recharts battery history chart from `page.tsx`
- [ ] T025 [P] [US1] Create `Earth` 3D component in `web/src/components/map/Earth.tsx` — extract earth sphere + atmosphere from `page.tsx`
- [ ] T026 [P] [US1] Create `UAVMarker` 3D component in `web/src/components/map/UAVMarker.tsx` — extract UAV marker with heading animation from `page.tsx`
- [ ] T027 [P] [US1] Create `ObstacleMarker` 3D component in `web/src/components/map/ObstacleMarker.tsx` — extract obstacle visualization from `page.tsx`
- [ ] T028 [P] [US1] Create `DestinationMarker` 3D component in `web/src/components/map/DestinationMarker.tsx` — extract target pin marker from `page.tsx`
- [ ] T029 [P] [US1] Create `ConnectionLines` component in `web/src/components/map/ConnectionLines.tsx` — extract great-circle path + history lines from `page.tsx`
- [ ] T030 [US1] Refactor `web/src/app/page.tsx` — thin orchestrator importing all extracted components; maintain Canvas + scene setup in page
- [ ] T031 [US1] Implement dark theme CSS design system in `web/src/app/globals.css` — add CSS custom properties for color palette, glassmorphism tokens, typography scale
- [ ] T032 [US1] Update Next.js metadata: fix title to "VectorHighway" and description in `web/src/app/layout.tsx`; replace favicon
- [ ] T033 [US1] Fix type safety: replace `any` with proper Three.js event type in `page.tsx` (Earth `handleClick` `e: any` → `ThreeEvent<MouseEvent>`)

**Checkpoint**: UI is fully componentized with professional dark theme; all panels and 3D markers render correctly

---

## Phase 5: User Story 3 - Simplified Setup & Documentation (Priority: P2)

**Goal**: Clear build instructions and quick onboarding

**Independent Test**: Fresh clone → build instructions produce working `uav_sim` binary; `ctest` passes

### Implementation for User Story 3

- [ ] T034 [US3] Fix README `cd Locator` → `cd VectorHighway` in `README.md`
- [ ] T035 [US3] Update README with proper build, test, and web UI instructions from `specs/001-ui-refactor-backend-robust/quickstart.md`
- [ ] T036 [US3] Validate full build chain: `cmake -B build -DBUILD_TESTING=ON && cmake --build build && ctest --test-dir build`

**Checkpoint**: README is accurate, build + test pipeline works from scratch

---

## Phase 6: User Story 4 - External Telemetry Integration (Priority: P3)

**Goal**: Real external drone telemetry alongside simulated drones with stale detection

**Independent Test**: Send mock MAVLink telemetry via WebSocket → new drone appears on globe; stop sending → drone marked stale

### Implementation for User Story 4

- [ ] T037 [US4] Add stale drone tracking in `src/exec.cpp` — track last-update timestamp per external drone; remove from broadcast after 30s inactivity
- [ ] T038 [US4] Broadcast stale removal event via WebSocket (`"type": "uav_remove"`) from `src/exec.cpp`
- [ ] T039 [US4] Handle `uav_remove` events in `web/src/useSimulationWebSocket.ts` — remove stale drone from state on receipt
- [ ] T040 [US4] Render stale/offline visual state in `UAVMarker` component (dimmed opacity, "STALE" label)

**Checkpoint**: External drones show on globe with live telemetry; stale drones are removed after 30s

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Final verification and build hardening

- [ ] T041 [P] Run AddressSanitizer build: `cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON && cmake --build build && ctest --test-dir build` — verify zero memory errors
- [ ] T042 [P] Verify `npm run build` succeeds in `web/` directory with no TypeScript errors
- [ ] T043 Run final end-to-end check: start `build/uav_sim`, start `npm run dev`, confirm WebSocket connection and real-time updates in browser

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: Depends on Setup — BLOCKS all user stories
- **US2 Backend Robustness (Phase 3)**: Depends on T005, T006 (SimulationContext + CommandEvent headers)
- **US1 UI Refactor (Phase 4)**: Depends on T010 (component directory structure)
- **US3 Documentation (Phase 5)**: Depends on nothing but README — can be parallelized
- **US4 External Telemetry (Phase 6)**: Depends on US2 completion (backend encapsulation first)
- **Polish (Phase 7)**: Depends on US1 + US2 completion

### User Story Dependencies

- **US1 (P1) UI**: Can start after Foundational — independent of US2
- **US2 (P1) Backend**: Can start after Foundational — independent of US1
- **US3 (P2) Docs**: No dependencies — can run any time
- **US4 (P3) External Telemetry**: Depends on US2 (backend changes required)

### Parallel Opportunities

- All Phase 1 tasks (T001-T004) can run in parallel
- All Phase 2 Foundational tasks (T005-T010) can run in parallel
- **US1 and US2 can be implemented in parallel** (different codebases: web/ vs src/)
- Within US2: T011, T013, T015-T017 can run in parallel
- Within US1: All component creation tasks (T018-T029) can run in parallel
- T042 and T041 can run in parallel

---

## Parallel Example: Phase 3 (US2) + Phase 4 (US1) Simultaneously

```bash
# US2: Backend robustness
Task: "Refactor SimulationContext into exec.cpp"
Task: "Replace raw new/delete with unique_ptr"
Task: "Write Catch2 tests for Geo, Pathfinder, UAV"

# US1: UI Refactor (parallel with US2 — different codebase)
Task: "Create ControlPanel component"
Task: "Create SwarmPanel component"
Task: "Create TelemetryPanel component"
Task: "Create UAVMarker 3D component"
Task: "Create ObstacleMarker 3D component"
```

---

## Implementation Strategy

### MVP First (US1 + US2 — both P1)

1. Complete Phase 1: Setup (dead code removal)
2. Complete Phase 2: Foundational (shared headers, CMake, dirs)
3. **Parallel Sprint A**: Phase 3 (US2 Backend) + Phase 4 (US1 UI)
4. **STOP and VALIDATE**: Both US1 and US2 independently testable
5. Deliver: Backend is memory-safe + UI is professional

### Incremental Delivery

1. Setup + Foundational → Foundation ready
2. US2 (Backend) done → memory-safe, tested, configurable
3. US1 (UI) done → professional dashboard
4. US3 (Docs) done → onboarding ready
5. US4 (External Telemetry) done → full feature set

### Parallel Team Strategy

With 2 developers:
- Developer A: US2 Backend (Phase 3) — memory safety, encapsulation, tests
- Developer B: US1 UI Refactor (Phase 4) — component extraction, theming
- Together: Phase 1 + Phase 2 + Phase 7 final verification

---

## Notes

- [P] tasks = different files, no dependencies
- [US1-US4] label maps task to specific user story
- Each user story is independently completable and testable
- Commit after each logical group of tasks
- Stop at any checkpoint to validate independently
- US1 and US2 have **no cross-dependencies** — fully parallelizable
