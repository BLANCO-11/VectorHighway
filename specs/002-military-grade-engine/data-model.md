# Data Model: Military-Grade UAV Engine

## Entity: UAV (Drone Agent)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | `DroneID` (string) | — | Unique identifier (e.g. "alpha_1") |
| groupId | `GroupID` (string) | "alpha" | Swarm group membership |
| position | `Coordinate` | — | Current geographic position (lat, lon, alt) |
| heading | double | 0.0 | Current heading in degrees (0-360) |
| targetSpeed | double | 0.25 | Desired speed in km/s |
| currentSpeed | double | 0.0 | Actual speed with inertia |
| acceleration | double | 0.1 | Acceleration factor km/s² |
| turnRate | double | 90.0 | Max turn rate in degrees/s |
| batteryLevel | double | 100.0 | Battery percentage (0-100) |
| batteryDrainRate | double | 0.01 | Base drain % per second |
| missionState | `MissionState` | IDLE | Current mission execution state (NEW) |
| currentWaypointIndex | int | 0 | Active waypoint index in mission (NEW) |
| isExternal | bool | false | True if this is a real-drone ingestion (NEW) |

### Validation
- Latitude: -90 to 90
- Longitude: -180 to 180
- Heading: 0-360 (wrapped)
- Battery: 0.0-100.0
- Speed: >= 0

### State Transitions (Mission)
```
IDLE → UPLOADED → IN_PROGRESS → COMPLETED
                 → IN_PROGRESS → FAILED
                 → IN_PROGRESS → RTH (low battery / connection loss)
RTH → COMPLETED
Any State → EMERGENCY (manual abort)
```

---

## Entity: Obstacle

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| position | `Coordinate` | — | Center position |
| radius | double | — | Obstacle radius in km |
| isDynamic | bool | false | True = moving obstacle |
| groupId | string | "" | Group scoping (empty = global) |
| heading | double | 0.0 | Movement direction (if dynamic) |
| speed | double | 0.0 | Movement speed km/s (if dynamic) |
| isNoFlyZone | bool | false | True = restricted area, not physical (NEW) |
| polygon | `vector<Coordinate>` | empty | Polygonal shape for no-fly zones (NEW) |

### Relationships
- Can be scoped to a `SwarmGroup` via `groupId`
- Polygons used for complex no-fly zone shapes

---

## Entity: NoFlyZone (NEW)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | string | — | Unique identifier |
| name | string | — | Human-readable name |
| shape | enum | circle | CIRCLE, POLYGON |
| center | `Coordinate` | — | Center point (if circular) |
| radius | double | — | Radius in km (if circular) |
| vertices | `vector<Coordinate>` | empty | Polygon vertices (if polygonal) |
| active | bool | true | Whether zone is enforced |

### Validation
- Circular: radius > 0
- Polygonal: at least 3 vertices, non-self-intersecting

---

## Entity: Waypoint (NEW)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| coordinate | `Coordinate` | — | Position |
| name | string | "" | Label |
| altitude | double | 0.0 | Target altitude at waypoint |
| action | enum | FLY_OVER | FLY_OVER, FLY_TO, LOITER, RTH |
| loiterDuration | double | 0.0 | Seconds to loiter (if LOITER action) |

---

## Entity: Mission (NEW)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | string | — | Unique identifier |
| droneId | `DroneID` | — | Assigned drone |
| waypoints | `vector<Waypoint>` | — | Ordered waypoint sequence |
| homeBase | `Coordinate` | — | Return-to-home location |
| totalDistance | double | — | Calculated total route distance (km) |
| estimatedDuration | double | — | Estimated flight time (s) |
| estimatedBatteryUse | double | — | Estimated battery consumption (%) |
| state | `MissionState` | IDLE | Current execution state |

### State Transitions
```
IDLE → UPLOADED (mission received)
UPLOADED → IN_PROGRESS (start command)
IN_PROGRESS → COMPLETED (all waypoints visited)
IN_PROGRESS → FAILED (unreachable waypoint / error)
IN_PROGRESS → RTH (low battery / connection loss)
RTH → COMPLETED (reached home base)
Any → ABORTED (manual abort)
```

---

## Entity: SwarmGroup

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | `GroupID` (string) | — | Group name (e.g. "alpha") |
| drones | `vector<DroneID>` | — | Member drone IDs |
| color | string | — | Hex color for UI rendering |
| target | `Coordinate` | optional | Shared group target |

---

## Entity: ChargingStation

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| position | `Coordinate` | — | Station location |
| name | string | — | Station name |
| chargingRate | double | — | Battery % per second |

---

## Entity: ExternalTelemetry

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | string | — | Drone identifier |
| groupId | string | "alpha" | Group membership |
| lat | double | — | Latitude |
| lon | double | — | Longitude |
| alt | double | 0.0 | Altitude (km) |
| heading | double | 0.0 | Heading (degrees) |
| battery | double | 100.0 | Battery percentage |
| speed | double | 0.0 | Current speed km/s (NEW) |
| status | string | "FLYING" | ARMED, FLYING, LANDED, EMERGENCY (NEW) |

---

## Entity: CommandEvent

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| type | string | — | target, control, spawn, obstacle, mission_upload, mission_start, mission_abort, rth |
| targetId | string | — | Drone, group, or "all" |
| lat | double | 0.0 | Latitude for target/spawn/obstacle |
| lon | double | 0.0 | Longitude for target/spawn/obstacle |
| radius | double | 0.0 | Radius for obstacle creation |
| speed | double | -1.0 | Speed for control command |
| drain | double | -1.0 | Battery drain rate for control |
| groupId | string | "" | Group scoping |
| missionJson | string | "" | Serialized mission plan (NEW) |

---

## Entity: HealthStatus (NEW)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| engineTickHz | double | 20.0 | Actual tick rate |
| wsConnectionCount | int | 0 | Connected WebSocket clients |
| wsLastMessageTime | double | — | Seconds since last message |
| drones | `map<DroneID, DroneHealth>` | — | Per-drone health |
| alerts | `vector<Alert>` | empty | Active alerts |

### DroneHealth (NEW)
| Field | Type | Description |
|-------|------|-------------|
| batteryLevel | double | Current battery % |
| lastTelemetryTime | double | Seconds since last update |
| missionState | string | Current mission state |
| connectionStatus | enum | CONNECTED, STALE, DISCONNECTED |

### Alert (NEW)
| Field | Type | Description |
|-------|------|-------------|
| level | enum | NOMINAL, CAUTION, WARNING, CRITICAL |
| source | string | Component name |
| message | string | Human-readable alert |
| timestamp | double | Unix timestamp |

---

## Entity: FlightLog (NEW)

| Field | Type | Description |
|-------|------|-------------|
| entries | `vector<LogEntry>` | Time-ordered telemetry snapshots |

### LogEntry (NEW)
| Field | Type | Description |
|-------|------|-------------|
| timestamp | double | Unix timestamp |
| droneId | string | Drone identifier |
| lat | double | Latitude |
| lon | double | Longitude |
| alt | double | Altitude |
| heading | double | Heading degrees |
| speed | double | Speed km/s |
| battery | double | Battery % |
| missionState | string | Current mission state |

---

## Relationships Diagram

```
SwarmGroup (1) ──── (*) UAV
UAV (1) ──── (0..1) Mission
Mission (*) ──── Waypoint
Obstacle (*) ──── (0..1) SwarmGroup (scoping)
NoFlyZone (*) ──── (0..1) SwarmGroup (scoping)
UAV (*) ──── (0..1) ChargingStation (assigned charger)
MirrorContext (1) ──── (*) UAV (cloned)
Operator (*) ──── (*) DroneID (permissions)
Tenant (1) ──── (*) Operator
Tenant (1) ──── (*) DroneID
DroneID (1) ──── (0..1) LinkState
DroneID (1) ──── (*) Payload
Tenant (1) ──── (*) Mission (if multi-tenant)
```

## Phase E Entities (Future)

### LinkState (Phase C — NEW)
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| droneId | DroneID | — | Associated drone |
| linkType | enum | LOS | LOS, SATCOM, CELLULAR, NONE |
| signalStrength | double | 1.0 | 0.0–1.0 normalized |
| latencyMs | double | 0.0 | Round-trip ms |
| packetLoss | double | 0.0 | 0.0–1.0 ratio |
| lastHeartbeat | double | — | Seconds since last |
| backupLinkType | enum | NONE | Failover link type |
| lolProcedure | enum | LOITER_AND_RTH | Loss-of-link procedure |

### Payload (Phase E — NEW)
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | string | — | Unique ID |
| droneId | DroneID | — | Mounted on |
| type | enum | EO_IR | EO_IR, SAR, LIDAR, CARGO, RELAY |
| weight | double | 0.0 | kg |
| powerDraw | double | 0.0 | Extra battery drain %/s |
| gimbalPan | double | 0.0 | Pan angle (deg) |
| gimbalTilt | double | 0.0 | Tilt angle (deg) |
| zoom | double | 1.0 | Camera zoom |

### EmergencyRule (Phase C — NEW)
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | string | — | Rule ID |
| name | string | — | Display name |
| condition | string | — | e.g. "battery < 20 && alt > 400" |
| action | enum | — | RTH, DESCEND, LAND, NOTIFY, ABORT |
| priority | int | 0 | Lower = higher priority |
| enabled | bool | true | Active or not |

### Operator (Phase D — NEW)
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | string | — | Operator ID |
| name | string | — | Display name |
| role | enum | VIEWER | COMMANDER, SENSOR_OP, SUPERVISOR, VIEWER |
| permissions | vector\<string\> | — | Granular permissions |
| connectedAt | double | — | Connection timestamp |

### Tenant (Phase E — NEW)
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | string | — | Tenant ID |
| name | string | — | Org name |
| drones | vector\<DroneID\> | — | Owned drones |
| operators | vector\<string\> | — | Associated operators |

### Checklist (Phase C — NEW)
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| name | string | — | e.g. "Pre-Flight Alpha" |
| items | vector\<ChecklistItem\> | — | Ordered items |

ChecklistItem: label, required(bool), autoCheck(bool), status(PENDING/PASS/FAIL/SKIP)

### MirrorContext (Phase C — NEW)
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| stateId | string | — | Snapshot ID |
| parentStateId | string | — | Origin live state |
| drones | map\<DroneID, UAV\> | — | Cloned drone states |
| mission | Mission | optional | Sandbox mission |
| conflicts | vector\<string\> | empty | Blocking issues |
| createdAt | double | — | Timestamp |
