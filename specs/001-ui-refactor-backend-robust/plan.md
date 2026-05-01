# Implementation Plan: UI Refactor & Backend Robustness

**Branch**: `001-ui-refactor-backend-robust` | **Date**: 2026-05-01 | **Spec**: `specs/001-ui-refactor-backend-robust/spec.md`
**Input**: Feature specification from `/specs/001-ui-refactor-backend-robust/spec.md`

## Summary

Refactor the VectorHighway project to achieve two goals: (1) transform the web UI from a monolithic prototype into a professional, elegant, componentized dashboard with proper theming and telemetry visualization, and (2) harden the C++ simulation engine to be memory-safe, testable, and architecturally clean — eliminating raw pointers, global mutable state, and dead code.

## Technical Context

**Language/Version**: C++17 (GCC/Clang), TypeScript 5.x (Next.js 16 / React 19)  
**Primary Dependencies**: websocketpp, asio (standalone), nlohmann/json, Three.js / @react-three/fiber, zustand, framer-motion, recharts, lucide-react  
**Storage**: N/A (in-memory simulation only — no persistence layer)  
**Testing**: Catch2 v3 (C++), Vitest / React Testing Library (TypeScript)  
**Target Platform**: Linux (primary), Windows via MinGW (secondary), modern browsers  
**Project Type**: Hybrid — C++ simulation engine + Next.js web dashboard  
**Performance Goals**: Simulation tick at 20Hz with 50+ concurrent drones; WebSocket command → UI update < 50ms p99  
**Constraints**: <100MB RSS memory during normal operation; zero memory leaks; header-only third-party deps preferred  
**Scale/Scope**: 2-person team; ~2,500 LOC C++, ~1,500 LOC TypeScript; 5 source files C++, ~10 TypeScript component files

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

The constitution file (`.specify/memory/constitution.md`) contains only template placeholders (`[PROJECT_NAME]`, `[PRINCIPLE_1_NAME]`, etc.) with no enforceable rules. No normative MUST/SHOULD statements exist. **Gate passes with no violations.**

*Post-Design Re-Check*: All decisions documented in research.md and contracts/ are consistent with the existing architecture. No design choices conflict with constitutional principles (none defined). **Gate still passes.**

## Project Structure

### Documentation (this feature)

```text
specs/001-ui-refactor-backend-robust/
├── plan.md              # This file
├── spec.md              # Feature specification
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output - WebSocket message schemas
└── tasks.md             # Phase 2 output (/speckit.tasks)
```

### Source Code (repository root)

```text
include/
├── Geo.h                    # Coordinate, Vector3, Waypoint (unchanged)
├── Environment.h            # Obstacle, ChargingStation, CoordinateConversion (unchanged)
├── Pathfinder.h             # GlobalPathfinder, LocalAvoidance (unchanged)
├── UAV.h                    # UAV agent (unchanged)
├── websockets.h             # SimulationServer interface (keep)
├── SimulationContext.h      # NEW: encapsulated IPC state (replaces globals)
├── CommandEvent.h           # NEW: shared command/telemetry structs
└── Types.h                  # NEW: common type aliases and constants

src/
├── exec.cpp                 # MAIN: simulation loop (refactored: unique_ptr, context)
├── websockets.cpp           # WebSocket server (refactored: context-based)
├── tests.cpp                # ENHANCED: Catch2 test suite
└── main.cpp                 # REMOVE (empty file)

web/src/
├── app/
│   ├── layout.tsx            # UPDATE: proper metadata
│   ├── page.tsx              # REFACTOR: thin orchestrator, delegate to components
│   └── globals.css           # ENHANCE: custom theme tokens
├── components/
│   ├── globe/
│   │   └── Globe.tsx         # MERGE: functionality into page or keep as standalone
│   ├── panels/
│   │   ├── ControlPanel.tsx  # NEW: speed, drain, turn controls
│   │   ├── SwarmPanel.tsx    # NEW: group tabs + add group
│   │   ├── TelemetryPanel.tsx # NEW: selected entity telemetry display
│   │   └── BatteryChart.tsx  # NEW: battery history chart extracted
│   ├── map/
│   │   ├── Earth.tsx         # NEW: earth sphere + atmosphere
│   │   ├── UAVMarker.tsx     # NEW: individual UAV 3D marker
│   │   ├── ObstacleMarker.tsx # NEW: obstacle visual
│   │   ├── DestinationMarker.tsx # NEW: target marker
│   │   └── ConnectionLines.tsx # NEW: great-circle path + history
│   └── ui/
│       ├── Slider.tsx        # NEW: styled range slider
│       ├── Button.tsx        # NEW: styled button variants
│       └── Card.tsx          # NEW: glassmorphism card container
├── hooks/
│   ├── useSimulationWebSocket.ts # KEEP (used by page.tsx)
│   └── useWebSocket.ts           # REMOVE (unused, replaced by above)
├── store/
│   └── useSimulationStore.ts     # REMOVE (unused, replaced by hook state)
└── useSimulationWebSocket.ts     # KEEP (already in use)

CMakeLists.txt                # UPDATE: Catch2 fetch, test target, pin websocketpp
```

**Structure Decision**: Single C++ project (`src/` + `include/`) with a separate Next.js frontend (`web/`). This matches the existing layout; no restructuring needed, only cleanup.

## Complexity Tracking

No constitution violations to justify. N/A.

## Phases

### Phase 0: Research

**Unknowns to resolve:**

1. **Best practices for React Three.js component architecture at scale** — How to structure 50+ 3D markers without FPS degradation
2. **Catch2 v3 integration with FetchContent** — Minimal CMake setup for unit tests in an existing project
3. **WebSocket++ version pinning** — What stable tag to use instead of `master`
4. **Asio standalone version pinning** — Confirm asio-1-28-0 is latest stable standalone
5. **Zustand vs. local state for simulation data** — Whether to reintroduce zustand or keep hook-local state
6. **Dark theme design system for dashboards** — Color palette, glassmorphism patterns, typography scale

**Output**: `research.md` in feature directory

### Phase 1: Design & Contracts

1. **Data model** (`data-model.md`): Document all entities (UAV, Obstacle, SwarmGroup, CommandEvent, etc.) with fields, types, and relationships
2. **Contracts** (`contracts/`): WebSocket message schemas — define all pub/sub topic formats, command payloads, state update JSON structures
3. **Quickstart** (`quickstart.md`): Updated setup instructions for the refactored project
4. **Agent context update**: Run the update-agent-context script

**Output**: `data-model.md`, `contracts/*`, `quickstart.md`, updated agent context

### Phase 2: Task Generation (handoff)

- Break the plan into concrete, ordered implementation tasks
- Each task references specific files and has acceptance criteria

## Implementation Order (draft)

1. **Cleanup**: Remove dead code (`main.cpp`, `useWebSocket.ts`, `useSimulationStore.ts`, unused `socket.io-client` dep)
2. **Backend**: Extract shared structs to `CommandEvent.h` / `SimulationContext.h`
3. **Backend**: Replace raw `new`/`delete` with `std::unique_ptr` in `exec.cpp`
4. **Backend**: Encapsulate global IPC state into `SimulationContext` class
5. **Backend**: Add Catch2 test framework, write tests for Geo, Pathfinder, UAV
6. **Backend**: Pin websocketpp to specific tag, fix CMake policy
7. **Backend**: Add `--port` CLI arg, read env for configurable WebSocket port
8. **UI**: Refactor monolithic `page.tsx` into component hierarchy
9. **UI**: Implement dark theme design system (CSS variables, glassmorphism, animations)
10. **UI**: Add proper Next.js metadata, favicon, title
11. **UI**: Fix type safety (remove `any`, proper Three.js event types)
12. **Docs**: Fix README `cd Locator` → `cd VectorHighway`, update build instructions
13. **Test**: Verify all builds, run test suite, run AddressSanitizer
