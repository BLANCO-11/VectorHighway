# Implementation Plan: Navigation Overhaul & Globe/UI Redesign

**Branch**: `003-navigation-globe-overhaul` | **Date**: 2026-05-03 | **Spec**: specs/002-military-grade-engine/spec.md
**Input**: User request for anchor-point pathfinding, obstacle sizing/deletion, globe visual overhaul, UI polish

## Summary

Overhaul the obstacle avoidance system from a basic heading-steer evasion to a predictive anchor-point based pathfinding engine; add obstacle radius UI, item deletion, globe rendering with vector tile borders/LOD, and a professional UI fluidity pass with framer-motion animations and polished components.

## Technical Context

**Language/Version**: C++17 (GCC/Clang), TypeScript 5.x (Next.js 16 / React 19)
**Primary Dependencies**: websocketpp 0.8.2, asio 1-28-0 (standalone), nlohmann/json v3.11.2, Three.js 0.184.0, @react-three/fiber 9.6.0, @react-three/drei 10.7.7, zustand, framer-motion 12.38.0, lucide-react 1.8.0, TailwindCSS v4
**New Dependencies**: protobuf/nanosvg (for vector tile parsing), or a lightweight GeoJSON tile renderer; turf.js for client-side geofencing geometry ops
**Storage**: In-memory simulation state; flight recorder writes JSON-lines to disk
**Testing**: Catch2 v3.7.1 (C++), Jest/Playwright (TS, if added)
**Target Platform**: Linux server (backend), modern browsers Chrome/Firefox (frontend)
**Project Type**: Web application (C++ backend + Next.js frontend)
**Performance Goals**: Pathfinding recalculation <50ms for 10+ obstacles; globe render <30fps on mid-range GPU; UI transitions <16ms (60fps)
**Constraints**: Drone must pre-compute path before flying toward no-fly zones; obstacle radius in degrees/km not arbitrary units; cleanup operations must not cause state desync
**Scale/Scope**: Pathfinding for up to 100 drones with 50 obstacles; globe with vector tiles down to zoom level 10

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

- The constitution is a template with no enforceable rules (all placeholders). **GATE PASSED** — no conflicts to evaluate.

## Project Structure

### Documentation (this feature)

```text
specs/003-navigation-globe-overhaul/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
└── contracts/           # Phase 1 output
```

### Source Code Changes

```text
# Backend changes
include/
├── Pathfinder.h            # NEW: AnchorPoint, PathSmoother, NFZAvoidanceStrategy
├── AnchorPointGraph.h      # NEW: Dynamic graph construction around obstacles
src/
├── anchor_point_graph.cpp  # NEW: Graph building, A* on anchor nodes
├── path_smoother.cpp       # NEW: Path smoothing (Bezier/catmull-rom curves)

# Frontend changes
web/src/
├── components/map/
│   ├── Earth.tsx            # MODIFY: Vector tile rendering, LOD
│   ├── GlobeBorders.tsx     # NEW: Country/state border overlay
│   ├── AnchorPointMarker.tsx # NEW: Visual anchor points
├── store/
│   └── useSimulationStore.ts # MODIFY: Add delete action, obstacle radius
├── components/panels/
│   ├── ControlPanel.tsx     # MODIFY: Obstacle radius slider, delete buttons
│   ├── ObstaclePanel.tsx    # NEW: Dedicated obstacle management panel
├── lib/
│   ├── types.ts             # MODIFY: Extended types for anchor points, delete cmds
│   └── geo.ts              # MODIFY: Vector tile utility functions
├── hooks/
│   └── useSimulationWebSocket.ts # MODIFY: Handle delete/clear command responses
```

**Structure Decision**: The existing project structure (include/ + src/ for C++ backend, web/src/ for Next.js frontend) is preserved. New files are added alongside existing ones to minimize structural disruption.

## Complexity Tracking

No constitution violations detected — section unused.
