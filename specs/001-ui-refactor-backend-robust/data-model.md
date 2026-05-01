# Data Model: VectorHighway

## Entities

### Coordinate
| Field | Type | Description |
|-------|------|-------------|
| latitude | double | WGS84 latitude in degrees [-90, 90] |
| longitude | double | WGS84 longitude in degrees [-180, 180] |
| altitude | double | Altitude above WGS84 ellipsoid in km |

**Validation**: latitude in [-90, 90], longitude in [-180, 180]

---

### Vector3
| Field | Type | Description |
|-------|------|-------------|
| x | double | X component |
| y | double | Y component |
| z | double | Z component |

---

### Waypoint
| Field | Type | Description |
|-------|------|-------------|
| coordinate | Coordinate | Geographic position |
| name | string | Human-readable label |

---

### UAV
| Field | Type | Description |
|-------|------|-------------|
| groupId | string | Swarm group identifier (e.g., "alpha") |
| position | Coordinate | Current geographic position |
| heading | double | Current heading in degrees [0, 360) |
| targetSpeed | double | Desired speed in km/s |
| currentSpeed | double | Actual speed with inertia applied |
| acceleration | double | Acceleration factor in km/s² |
| turnRate | double | Maximum turn rate in degrees/s |
| batteryLevel | double | Battery percentage [0.0, 100.0] |
| batteryDrainRate | double | Base drain rate in %/s |

**Relationships**: Belongs to a SwarmGroup (via groupId). Can have a target Coordinate.

---

### Obstacle (abstract base)
| Field | Type | Description |
|-------|------|-------------|
| position | Coordinate | Center position |
| radius | double | Collision radius in km |
| isDynamic | bool | Whether the obstacle moves |
| groupId | string | Empty = global, otherwise group-scoped |

**Subtypes**:
- **StaticObstacle**: No movement (updatePosition is no-op)
- **DynamicObstacle**: Has `speed` (km/s) and `heading` (degrees), moves linearly each tick

---

### ChargingStation
| Field | Type | Description |
|-------|------|-------------|
| position | Coordinate | Geographic position |
| name | string | Display name |
| chargingRate | double | Battery charge rate in %/s |

---

### SwarmGroup
| Field | Type | Description |
|-------|------|-------------|
| id | string | Unique group identifier |
| color | string | Hex color for UI rendering |
| activeTarget | Coordinate | Group-level target (optional, overrides per-UAV) |
| defaultSpeed | double | Default speed for newly spawned drones |

---

### CommandEvent (shared struct)
| Field | Type | Description |
|-------|------|-------------|
| type | string | "target", "control", "spawn", "obstacle" |
| targetId | string | UAV/group identifier or "all" |
| lat | double | Latitude parameter |
| lon | double | Longitude parameter |
| radius | double | Radius (for obstacle commands) |
| speed | double | Speed setting |
| drain | double | Battery drain rate setting |
| groupId | string | Group scope |

---

### ExternalTelemetry (shared struct)
| Field | Type | Description |
|-------|------|-------------|
| id | string | External drone identifier |
| groupId | string | Group assignment |
| lat | double | Current latitude |
| lon | double | Current longitude |
| alt | double | Current altitude (km) |
| heading | double | Current heading (degrees) |
| battery | double | Battery level (percent) |

---

### SimulationContext (encapsulated state)
| Field | Type | Description |
|-------|------|-------------|
| commandQueue | vector<CommandEvent> | Incoming commands from WebSocket thread |
| externalTelemetry | vector<ExternalTelemetry> | External drone telemetry packets |
| mtx | mutex | Protects commandQueue and externalTelemetry |

**Ownership**: Single instance owned by main()/exec.cpp. Passed by pointer to WebSocket server.

---

## State Transitions

### UAV Lifecycle
```
SPAWNED → FLYING → ARRIVED → (RETARGET) → FLYING
                    → BATTERY_DEAD → IDLE
                    
FLYING: adjusting heading, consuming battery, broadcasting state
ARRIVED: at target, speed=0, hovering until retargeted
BATTERY_DEAD: battery=0%, no movement
```

### Simulation Tick (20Hz)
```
1. Read CommandQueue → update swarm state
2. For each UAV: compute heading, check obstacles, update position, drain battery
3. For each dynamic obstacle: update position
4. Broadcast all UAV/obstacle states via WebSocket
5. Sleep until next tick
```
