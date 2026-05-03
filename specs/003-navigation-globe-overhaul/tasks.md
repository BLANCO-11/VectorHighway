---

description: "Task list for Navigation Overhaul, Globe Redesign, and UI Polish feature"
---

# Tasks: Navigation Overhaul & Globe/UI Redesign

**Input**: Design documents from `/specs/003-navigation-globe-overhaul/`
**Prerequisites**: plan.md, spec.md (002-military-grade-engine), research.md, data-model.md, contracts/

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Data downloads and project structure preparation

- [X] T001 [P] Download Natural Earth GeoJSON data to web/public/data/ (ne_110m_admin_0_countries.geojson, ne_50m_admin_1_states_provinces.geojson)
- [X] T002 Record feature branch 003-navigation-globe-overhaul and spec paths in specs/003-navigation-globe-overhaul/

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core C++ and TypeScript infrastructure that MUST be complete before user stories

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T003 Add ENU coordinate utility functions to include/Geo.h: crossTrackDistance, fractionAlong, ecefToEnu, enuToCoordinate
- [X] T004 [P] Add TangentBypass struct and computeTangentBypass declaration to include/Geo.h
- [X] T005 Add ObstacleAvoidanceConfig struct to include/Pathfinder.h
- [X] T006 Create include/AnchorPointGraph.h with AnchorNode, Edge structs and graph API
- [X] T075 [P] Add WebSocket delete/clear message types to web/src/lib/types.ts (DeletionState, PlacementConfig, ContextMenu, path_update type)

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 2 - Obstacle-Aware Pathfinding (Priority: P1) 🎯 MVP

**Goal**: Replace broken LocalAvoidance (90° evasion) with anchor-point based predictive pathfinding that pre-computes smooth routes around no-fly zones before the drone enters them.

**Independent Test**: Place an obstacle in a drone's flight path; verify the drone smoothly reroutes around it and continues to the target without getting stuck.

### Implementation

- [X] T007 [P] [US2] Implement ENU utility functions in src/geo_utils.cpp: crossTrackDistance, fractionAlong, ecefToEnu, enuToCoordinate
- [X] T008 [P] [US2] Implement computeTangentBypass in src/geo_utils.cpp (compute approach/tangent/exit points around circular obstacles in local ENU frame)
- [X] T009 [P] [US2] Implement AnchorPointGraph in src/anchor_point_graph.cpp: addNode, addEdge, connectLayer (sparse inter-layer connectivity with obstacle-free edge validation), findPath (A*)
- [X] T010 [P] [US2] Implement ObstacleAvoidancePathfinder in src/path_smoother.cpp: findBlockingObstacles (cross-track scan), generateAnchorGraph, catmullRomSmooth, collisionFreeNudge
- [X] T011 [US2] Implement computePath entry point in ObstacleAvoidancePathfinder (orchestrate blocking detection → anchor graph → A* → smoothing → nudge → fallback to RRT)
- [X] T012 [P] [US2] Create include/PathSmoother.h with Catmull-Rom spline interpolation declarations
- [X] T013 [US2] Integrate ObstacleAvoidancePathfinder into exec.cpp simulation loop: replace LocalAvoidance::recalculateHeading calls with computePath on target change
- [X] T014 [US2] Add path_update broadcast to exec.cpp when new path is computed (broadcast path waypoints to frontend)
- [X] T015 [US2] Handle dynamic obstacles with periodic replanning (predict position, recompute path every 1-3 seconds)
- [X] T016 [US2] Handle UNREACHABLE_TARGET case — drone halts and broadcasts error instead of getting stuck

**Checkpoint**: At this point, drones should smoothly navigate around obstacles without getting stuck

---

## Phase 4: User Story 1 - Professional Dashboard & Item Management (Priority: P1)

**Goal**: Professional UI with polished animations, configurable obstacle radius, right-click deletion, group/all clear, and path visualization.

**Independent Test**: Load the web UI — verify all panels animate in smoothly, place an obstacle with custom radius, right-click to delete it, use group clear to remove all obstacles.

### Implementation

- [X] T017 [P] [US1] Add framer-motion spring animations to Card.tsx: layout prop, whileHover scale 1.01/border glow, initial opacity 0/y:12 spring entrance
- [X] T018 [P] [US1] Add AnimatePresence mode="popLayout" wrapper around sidebar panels in web/src/app/page.tsx
- [X] T019 [P] [US1] Add pulsing dot animation to Badge.tsx for CONNECTED status (scale/opacity spring loop)
- [X] T020 [P] [US1] Add whileHover/whileTap spring scale to Button.tsx
- [X] T021 [P] [US1] Add spring-animated fill width to ProgressBar.tsx
- [X] T022 [P] [US1] Add spring-animated knob to Toggle.tsx
- [X] T023 [P] [US1] Add useSpring number transitions to TelemetryPanel.tsx for lat/lon/heading/speed/battery values
- [X] T024 [P] [US1] Add spring enter/exit with AnimatePresence to Alert.tsx
- [X] T025 [P] [US1] Add animated value display on slider change to Slider.tsx
- [X] T026 [P] [US1] Add gradient text and glow border utility classes to web/src/app/globals.css (text-gradient-cyan, border-glow-cyan/green/red, glass-elevated)
- [X] T027 [P] [US1] Add obstacle radius slider to ControlPanel.tsx (range 10-1000m, default 250m)
- [X] T028 [P] [US1] Pass radius field in obstacle placement message in page.tsx handleGlobeClick
- [X] T029 [US1] Add deletion state machine to web/src/store/useSimulationStore.ts (selectedForDelete, deletionMode, contextMenu state)
- [X] T030 [US1] Add right-click context menu component at web/src/components/panels/ObstaclePanel.tsx (Delete Single / Clear Group / Clear All options)
- [X] T031 [US1] Implement cmd/environment/remove, cmd/environment/clear_group, cmd/environment/clear_all message sending in page.tsx (triggered by context menu actions)
- [X] T032 [US1] Handle obstacle_removed and environment_cleared incoming messages in web/src/useSimulationWebSocket.ts
- [X] T033 [US1] Backend: handle cmd/environment/remove topic in websockets.cpp set_message_handler (remove entity from obstacles/swarm by ID)
- [X] T034 [US1] Backend: handle cmd/environment/clear_group topic in websockets.cpp (remove all entities matching groupId)
- [X] T035 [US1] Backend: handle cmd/environment/clear_all topic in websockets.cpp (clear all user-placed obstacles)
- [X] T036 [US1] Backend: broadcast obstacle_removed and environment_cleared confirmations from exec.cpp command processing
- [X] T037 [US1] Render path_update waypoints as a dashed path line on the globe (add PathLine per computed path in page.tsx)

**Checkpoint**: At this point, UI is fluid, obstacles are configurable/deletable, and computed paths are visible

---

## Phase 5: User Story 7 - Geospatial Border Overlay (Priority: P2)

**Goal**: Render country and state borders on the 3D globe with level-of-detail that updates as the camera zooms in/out.

**Independent Test**: Load the web UI and verify country borders are visible on the globe. Zoom in and verify state borders appear at close range.

### Implementation

- [X] T038 [P] [US7] Create web/src/components/map/GlobeBorders.tsx with GeoJSON fetch, ring extraction, and THREE.LineSegments buffer geometry
- [X] T039 [US7] Implement LOD via useFrame camera distance — countries 110m always visible at varying opacity, states 50m visible only when distance < 12
- [X] T040 [US7] Render country borders as thin #88bbff lines, state borders as thinner #6699cc lines, both with depthWrite:false and transparent
- [X] T041 [US7] Integrate GlobeBorders into page.tsx Canvas as sibling of Earth component

**Checkpoint**: Globe now shows country and state borders with smooth LOD transitions

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [X] T042 [P] Add AnchorPointMarker.tsx at web/src/components/map/AnchorPointMarker.tsx (visualize computed anchor points on globe for debugging/tuning)
- [X] T043 [P] Animate the status bar entrance in page.tsx with spring slide-down on load
- [X] T044 [P] Add CollapsibleSection helper component for panel expand/collapse with AnimatePresence height animation
- [X] T045 Run quickstart.md validation — verify backend starts, frontend loads, pathfinding demo works end-to-end
- [ ] T046 Keep LocalAvoidance as fallback (still used in exec.cpp when no computed path)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup — BLOCKS all user stories
- **User Story 2 (Phase 3)**: P1 — new C++ pathfinding engine, core of feature
- **User Story 1 (Phase 4)**: P1 — UI overhaul, depends on T006 (types) from Foundational
- **User Story 7 (Phase 5)**: P2 — globe borders, independent of other stories
- **Polish (Phase 6)**: Depends on all user stories complete

### Parallel Opportunities

- All Phase 1 tasks are parallel ([P])
- All Phase 2 tasks are parallel ([P]) — Geo utils, TangentBypass, types
- Within Phase 3: T007-T010, T012 can run in parallel (geo_utils, graph, pathfinder, smoother are independent)
- Within Phase 4: T017-T027 are all parallel (independent UI component changes)
- Within Phase 5: Single file, sequential
- Polish: T042-T044 are parallel

### Parallel Example: Phase 4 (User Story 1)

```bash
Task: "Add spring animations to Card.tsx"
Task: "Add pulsing dot to Badge.tsx"
Task: "Add spring scale to Button.tsx"
Task: "Add number transitions to TelemetryPanel.tsx"
Task: "Add obstacle radius slider to ControlPanel.tsx"
Task: "Add glow utilities to globals.css"
```

---

## Implementation Strategy

### MVP First (Phase 3 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational
3. Complete Phase 3: Obstacle-Aware Pathfinding (US2)
4. **STOP and VALIDATE**: Place an obstacle in a drone's path — verify smooth rerouting
5. Deploy/demo if ready

### Incremental Delivery

1. Setup + Foundational → Foundation ready
2. Add User Story 2 (Pathfinding) → Test → Deploy (MVP!)
3. Add User Story 1 (UI + Deletion) → Test → Deploy
4. Add User Story 7 (Borders) → Test → Deploy
5. Each story adds value without breaking previous stories

### Validation

- Phase 3 checkpoint: Obstacle placed in path → drone routes around it smoothly
- Phase 4 checkpoint: UI animates, obstacles can be placed with custom radius, right-click deletes, group clear works
- Phase 5 checkpoint: Country/state borders visible at appropriate zoom levels
- Final checkpoint: Quickstart.md validation passes end-to-end

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Stop at any checkpoint to validate story independently
