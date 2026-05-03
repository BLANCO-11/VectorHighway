# WebSocket Protocol Contract

## Version 2.1 — Deletion & Obstacle Sizing Extension

### Outgoing Messages (Frontend → Backend)

#### `cmd/environment/remove`
Remove a single entity from the simulation.

```json
{
  "topic": "cmd/environment/remove",
  "payload": {
    "id": "obs_3",
    "type": "obstacle"
  }
}
```

Payload fields:
- `id` (string, required): Entity identifier
- `type` (string, required): Entity type — `"obstacle"`, `"uav"`, `"charging_station"`, `"noflyzone"`

---

#### `cmd/environment/clear_group`
Remove all entities of a given type belonging to a group.

```json
{
  "topic": "cmd/environment/clear_group",
  "payload": {
    "groupId": "alpha",
    "type": "obstacle"
  }
}
```

Payload fields:
- `groupId` (string, required): Group identifier
- `type` (string, required): Entity type to clear

---

#### `cmd/environment/clear_all`
Remove all user-placed entities from the simulation.

```json
{
  "topic": "cmd/environment/clear_all",
  "payload": {}
}
```

---

#### `cmd/environment/obstacle` (Extended)
Place an obstacle with configurable radius.

```json
{
  "topic": "cmd/environment/obstacle",
  "payload": {
    "lat": 48.85,
    "lon": 2.35,
    "radius": 500.0,
    "groupId": "alpha",
    "dynamic": false
  }
}
```

Payload fields:
- `lat` (number, required): Latitude in degrees
- `lon` (number, required): Longitude in degrees
- `radius` (number, optional, default 250.0): Obstacle radius in meters
- `groupId` (string, optional, default ""): Group filter for avoidance
- `dynamic` (boolean, optional, default false): Whether obstacle moves

---

### Incoming Messages (Backend → Frontend)

#### `obstacle_removed`
Confirms an obstacle was removed.

```json
{
  "type": "obstacle_removed",
  "id": "obs_3"
}
```

#### `environment_cleared`
Confirms a group or all entities were removed.

```json
{
  "type": "environment_cleared",
  "groupId": "alpha"
}
```

Omitting `groupId` or setting it to `"*"` indicates a full clear.

#### `path_update`
Provides the drone's pre-computed path for visualization.

```json
{
  "type": "path_update",
  "droneId": "alpha_1",
  "path": [
    {"lat": 48.85, "lon": 2.35},
    {"lat": 48.86, "lon": 2.37},
    {"lat": 48.88, "lon": 2.38}
  ]
}
```

---

### Error Responses

```json
{
  "type": "error",
  "code": "ENTITY_NOT_FOUND",
  "message": "No obstacle found with id: obs_99"
}
```

| Code | Description |
|------|-------------|
| `ENTITY_NOT_FOUND` | Referenced entity does not exist |
| `INVALID_PAYLOAD` | Missing required fields |
| `GROUP_NOT_FOUND` | Referenced group does not exist |
| `UNREACHABLE_TARGET` | No path exists to target |
