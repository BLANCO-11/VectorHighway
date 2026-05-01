# Research: UI Refactor & Backend Robustness

## Unknowns Resolved

### 1. React Three.js component architecture at scale

**Decision**: Use a single-page Canvas with instanced rendering via `@react-three/drei` Instances for UAV markers when count exceeds 50. For the current scale (<50), individual `group` + `useFrame` animation is sufficient.

**Rationale**: Each UAV marker currently uses a WebGL `group` with `useFrame` lerp -- this is acceptable for <100 markers. The `ConnectionLines` component uses Three.js `line` primitives which batch well. No instancing needed at current scale.

**Alternatives considered**: 
- Three.js `InstancedMesh` — adds complexity, only needed at 100+ objects
- WebGL2 point sprites — less visual fidelity, harder to show heading direction
- HTML overlays for all markers — breaks 3D depth cues

### 2. Catch2 v3 with FetchContent

**Decision**: Use Catch2 v3.7.x via `FetchContent` with `CATCH_CONFIG_MAIN` in a single test translation unit.

**CMake snippet**:
```cmake
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(uav_tests src/tests.cpp)
target_link_libraries(uav_tests PRIVATE Catch2::Catch2WithMain)
include(CTest)
include(Catch)
catch_discover_tests(uav_tests)
```

**Rationale**: Catch2 v3 is the modern standard for C++17 unit testing. FetchContent avoids system-install dependency. `Catch2WithMain` provides a main() so `tests.cpp` doesn't need one.

**Alternatives considered**:
- GoogleTest — heavier, more boilerplate
- doctest — lighter but fewer ecosystem tools
- Boost.Test — unnecessary Boost dependency

### 3. WebSocket++ version pinning

**Decision**: Pin to `websocketpp-0.8.2` tag.

**Rationale**: 0.8.2 is the latest stable release tag. The current `master` branch introduces breaking changes periodically and is non-reproducible. 0.8.2 is compatible with asio-standalone 1.28.0.

**Alternatives considered**:
- `master` — current (non-reproducible, risky)
- No tag — same as master

### 4. Asio standalone version

**Decision**: Keep `asio-1-28-0` — this is the latest stable standalone tag as of early 2025/2026.

**Rationale**: Asio's standalone mode (no Boost) is well-tested with websocketpp 0.8.2. The 1.28.x series is stable.

### 5. Zustand vs. local state for simulation data

**Decision**: Keep using local React state via `useSimulationWebSocket` hook. Do NOT reintroduce zustand.

**Rationale**: The zustand store (`useSimulationStore.ts`) is currently unused and adds an unnecessary abstraction layer. The simulation data flows uni-directionally (WebSocket → component tree) with no cross-component mutation. React local state + `useState` is simpler and sufficient.

**If cross-component state sharing becomes necessary** (e.g., a settings modal needs to read drone state), add a React Context provider instead of zustand — it's built-in and equivalent for this use case.

**Alternatives considered**:
- Zustand — added complexity for no benefit (unused currently)
- Redux — overkill for ~5 state fields
- Jotai — same reasoning as zustand

### 6. Dark theme design system

**Decision**: Use a CSS custom properties approach with a defined color scale, glassmorphism panels, and framer-motion for transitions.

**Color palette**:
- Background: `#050505` → `#0a0a0a` (near-black)
- Surface: `rgba(255, 255, 255, 0.03-0.08)` (glass)
- Border: `rgba(255, 255, 255, 0.06-0.12)` (subtle)
- Primary: `#3b82f6` (blue-500)
- Accent: `#22d3ee` (cyan-400)
- Danger: `#ef4444` (red-500)
- Success: `#22c55e` (green-500)
- Text primary: `rgba(255, 255, 255, 0.9)`
- Text secondary: `rgba(255, 255, 255, 0.5)`

**Patterns**:
- Panels: `bg-black/40 backdrop-blur-md border border-white/10 rounded-2xl shadow-2xl`
- Typography: Geist font family (already configured), `text-[10px] uppercase tracking-widest` for labels
- Animations: framer-motion `AnimatePresence` for panel transitions, `useFrame` lerp for 3D objects
- Icons: `lucide-react` (already used)
- Charts: `recharts` with transparent backgrounds

**Rationale**: The existing UI already has the right aesthetic direction (dark, glass, glow effects). The refactor focuses on extraction into reusable components rather than redesigning the visual language. CSS custom properties make theme customization straightforward.

## Technology Best Practices

### C++ Memory Safety in Simulation Loops
- Prefer `std::vector<Obstacle>` over `std::vector<Obstacle*>` when polymorphism isn't needed
- When polymorphism is needed (StaticObstacle vs DynamicObstacle), use `std::vector<std::unique_ptr<Obstacle>>`
- Avoid raw `new`/`delete` entirely — use `std::make_unique` and `std::make_shared`
- Use RAII wrappers for all resources (mutex locks via `std::lock_guard`, sockets via scoped lifecycle)

### WebSocket Server Patterns
- Encapsulate server state (connection set, command queue) in a class instance
- Pass a reference/pointer to the simulation loop rather than using globals
- Use a command queue pattern (lock-free ring buffer or `std::queue` + mutex) between WS thread and sim thread
- The current pattern (mutex-protected vector + `extern` globals) works but is fragile — extract to `SimulationContext`

### React Component Decomposition
- One component file per visual entity (UAVMarker, ObstacleMarker, Earth, etc.)
- Extract business logic into hooks, keep components as presentation
- Use `useMemo` and `React.memo` judiciously — only when profiling shows a bottleneck
- Keep the Canvas and 3D scene management in one parent component (the page or a `Scene3D` wrapper)

### Tailwind v4 + CSS Variables
- Tailwind v4 uses `@theme inline` for CSS custom property integration (current `globals.css` is already set up)
- Define semantic color tokens in `:root` as CSS variables
- Use `@layer base` for global resets

## Decisions Log

| # | Decision | Rationale | Alternatives |
|---|----------|-----------|-------------|
| 1 | Keep local React state, drop zustand | Simpler, sufficient for current scale | Zustand, Jotai, Context |
| 2 | Catch2 v3 for C++ tests | Modern standard, FetchContent-friendly | GoogleTest, doctest |
| 3 | websocketpp 0.8.2 | Stable release, reproducible | master (current, non-reproducible) |
| 4 | asio-1-28-0 | Latest stable standalone | asio-1-30+ (may break WS++) |
| 5 | Unique_ptr for obstacles | RAII, no leaks | raw new/delete (current), shared_ptr |
| 6 | CSS custom properties for theme | Zero-runtime, easy to customize | CSS-in-JS, Tailwind-only |
| 7 | Keep inline Canvas approach | Simple, adequate for <100 markers | InstancedMesh, separate scene manager |
