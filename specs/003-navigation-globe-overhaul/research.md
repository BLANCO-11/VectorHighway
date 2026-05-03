# Research: Navigation Overhaul & Globe/UI Redesign

## 1. Obstacle Avoidance Pathfinding

### Decision
Implement a predictive **anchor-point pathfinding** system that dynamically generates tangent bypass routes around obstacles using A* on a sparsely-connected anchor graph, followed by Catmull-Rom smoothing.

### Rationale
The existing `LocalAvoidance::recalculateHeading` produces a hard 90-degree evasion only when already close to an obstacle — it's reactive, janky, and has no destination awareness. The anchor-point approach:
- Pre-computes the path before the drone enters the no-fly zone
- Dynamically creates tangent points around each circular obstacle (2 bypass gates per obstacle)
- Runs A* on a sparse graph where edges are guaranteed obstacle-free
- Smooths via Catmull-Rom spline (C1 continuous, passes through all waypoints)
- Falls back to RRT if no tangent path exists

### Alternatives Considered
- **Pure RRT**: Too stochastic, paths not repeatable
- **Potential fields**: Local minima problem with concave obstacles
- **Dubins curves**: Adds turn-radius constraint but complex on sphere surface

### Algorithm Summary
1. `findBlockingObstacles()` — cross-track distance check for each obstacle against start→goal line
2. `computeTangentBypass()` — for each blocker, compute 2 approach/tangent/exit points in ENU local frame
3. `buildSparseGraph()` — connect adjacent obstacle layers only; validate edges with existing `segmentIntersectsObstacle`
4. `runAStar()` — reuse `GlobalPathfinder::findPathAStar`
5. `catmullRomSmooth()` — subdivide polyline 5x with Catmull-Rom interpolation in lat/lon
6. Collision-free nudge pass — 5-10 iterations of repulsion-field gradient descent

### Key Techniques
- ENU (East-North-Up) local tangent plane for 2D tangent point math
- Great-circle segment intersection test for graph edge validity
- Predicted position for dynamic obstacles (speed * deltaTime in lat/lon)
- Periodic replanning every 1-3 seconds for dynamic obstacles

---

## 2. Globe Vector Border Rendering

### Decision
Use **GeoJSON → THREE.LineSegments** on the sphere surface with distance-based LOD opacity. Zero additional NPM dependencies beyond Three.js/R3F already in the project.

### Rationale
- Pure vector quality at any zoom level
- Reuses existing `getCartesian()` coordinate conversion
- Single draw call per LOD level (2 total: countries + states)
- ~2-8 MB static GeoJSON data in `/public/data/`
- No API keys, no CDN dependency at runtime

### Alternatives Considered
- **Baked border texture**: Blurry when zoomed, fixed resolution
- **Mapbox/Maplibre GL**: Heavy library, different rendering paradigm
- **three-geo**: Depends on Mapbox tiles + API key

### Data Sources
- `ne_110m_admin_0_countries.geojson` — countries at 110m resolution (~150 KB)
- `ne_50m_admin_1_states_provinces.geojson` — states at 50m resolution (~600 KB)

### LOD Strategy (via `useFrame` camera distance)
| Distance | Countries | States |
|----------|-----------|--------|
| >= 18 | 110m, opacity 0.15 | hidden |
| 10–18 | 110m, opacity 0.4 | hidden |
| < 10 | 110m, opacity 0.6 | 50m, opacity 0.3 |

---

## 3. UI Fluidity Overhaul

### Decision
Leverage existing **framer-motion 12.38.0** throughout the component tree — currently used in only 1 file. Add spring physics, AnimatePresence, layout animations, and count-up telemetry.

### Rationale
- Zero new dependencies — framer-motion is already in `package.json`
- Spring physics feels natural/professional (not CSS transitions)
- `AnimatePresence` enables smooth panel enter/exit
- `useSpring`/`useMotionValue` enables animated number transitions
- TailwindCSS v4 glass utilities already exist (`glass`, `glass-panel`, `text-glow`)

### Key Patterns
| Component | Animation Change |
|---|---|
| `Card.tsx` | `layout` prop + `whileHover` spring (scale 1.01, border glow) |
| `page.tsx` sidebar | `AnimatePresence mode="popLayout"` |
| `Badge.tsx` | Pulsing dot for CONNECTED status |
| `Button.tsx` | `whileHover` scale 1.02, `whileTap` scale 0.97 |
| `ProgressBar.tsx` | Spring-animated fill width |
| `Toggle.tsx` | Spring-animated knob position |
| `TelemetryPanel.tsx` | `useSpring` number transitions for all values |
| `Alert.tsx` | Spring enter/exit with AnimatePresence |
| `Slider.tsx` | Animated value display on change |

### CSS Additions
- Gradient text utility (`text-gradient-cyan`)
- Accent-tinted glass backgrounds
- Noise texture overlay for depth
- Glow border utilities (cyan/green/red)

---

## 4. Deletion & Obstacle Sizing Protocol

### Decision
Extend WebSocket message protocol to support `cmd/environment/remove` and `cmd/environment/clear_group` topics. Add `radius` field to obstacle click payload. Frontend implements right-click context menu.

### Protocol Extensions

```
// Remove single item
{ "topic": "cmd/environment/remove", "payload": { "id": "obs_3", "type": "obstacle" } }
// Clear group
{ "topic": "cmd/environment/clear_group", "payload": { "groupId": "alpha", "type": "obstacle" } }
// Place obstacle with radius
{ "topic": "cmd/environment/obstacle", "payload": { "lat": 48.8, "lon": 2.3, "radius": 500.0, "groupId": "alpha" } }
// Clear all
{ "topic": "cmd/environment/clear_all", "payload": {} }
```
