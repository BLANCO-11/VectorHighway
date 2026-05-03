# Data Model: Navigation Overhaul & Globe/UI Redesign

## New Entities

### AnchorPoint
| Field | Type | Description |
|-------|------|-------------|
| id | int | Unique node ID in graph |
| coord | Coordinate | Geographic position |
| tag | string | "start", "goal", or obstacleId |
| side | string | "left" / "right" / "" |
| obstacleId | string | Reference to parent obstacle (if any) |

### TangentGate
| Field | Type | Description |
|-------|------|-------------|
| obstacleId | string | Reference to parent obstacle |
| side | string | "left" / "right" |
| approach | Coordinate | Entry point before tangent |
| tangent | Coordinate | Point of tangency on circle |
| exit | Coordinate | Exit point after clearing obstacle |

### SmoothedPath
| Field | Type | Description |
|-------|------|-------------|
| waypoints | vector\<Coordinate\> | Pre-computed smooth path waypoints |
| rawNodes | vector\<AnchorPoint\> | Raw A* output graph nodes |
| totalDistance | double | Path length in km |
| isValid | bool | Whether path is collision-free |

### ObstacleAvoidanceConfig
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| safetyMarginKm | double | 0.5 | Clearance from obstacle edge |
| replanInterval | double | 2.0 | Seconds between dynamic replans |
| smoothSubdivisions | int | 5 | Catmull-Rom subdivision factor |
| maxAnchorPathSearch | int | 5000 | Max A* iterations |
| turnRateDegPerSec | double | 90.0 | Max turn rate for path feasibility |

---

## Extended Existing Entities

### Obstacle (C++)
| Field | Change | Description |
|-------|--------|-------------|
| groupId | now used for filtering/clear-group | Which group this obstacle blocks |

### ObstacleState (TypeScript)
| Field | Type | Change |
|-------|------|--------|
| radius | number | Now user-configurable via slider |

---

## New Frontend State (useSimulationStore.ts additions)

### Deletion State
```typescript
interface DeletionState {
  selectedForDelete: string | null;       // entity ID
  deletionMode: 'none' | 'single' | 'group' | 'all';
}
```

### Obstacle Placement State (extended)
```typescript
interface PlacementConfig {
  obstacleRadius: number;  // 0-1000m, slider-controlled
  dynamicEnabled: boolean;
}
```

### Right-Click Context Menu
```typescript
interface ContextMenu {
  visible: boolean;
  x: number;
  y: number;
  entityType: 'obstacle' | 'uav' | 'destination' | 'none';
  entityId?: string;
}
```

---

## WebSocket Message Types (Protocol Contract)

### Outgoing (Frontend → Backend)
| Topic | Payload | Description |
|-------|---------|-------------|
| `cmd/environment/remove` | `{ id: string, type: string }` | Remove single entity |
| `cmd/environment/clear_group` | `{ groupId: string, type: string }` | Clear group entities |
| `cmd/environment/clear_all` | `{}` | Clear all placed items |

### Incoming (Backend → Frontend)
| Type | Payload | Description |
|------|---------|-------------|
| `obstacle_removed` | `{ id: string }` | Confirmation of removal |
| `environment_cleared` | `{ groupId?: string }` | Confirmation of clear |
| `path_update` | `{ droneId: string, path: [{lat, lon}] }` | New computed path for rendering |
