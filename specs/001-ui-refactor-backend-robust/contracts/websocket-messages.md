# WebSocket Message Contracts

## Connection
- **URL**: `ws://{host}:{port}` (default: `ws://localhost:8080`)
- **Protocol**: Plain WebSocket (no Socket.IO)

---

## Server → Client Messages

### UAV State Update
```json
{
  "type": "uav_update",
  "id": "alpha_1",
  "groupId": "alpha",
  "lat": 48.423366,
  "lon": 2.345332,
  "alt": 0.0,
  "heading": 90.0,
  "battery": 85.3,
  "startLat": 26.870601,
  "startLon": 75.794110,
  "targetLat": 48.423366,
  "targetLon": 2.345332
}
```
**Frequency**: Every simulation tick (~20Hz per UAV)

### Obstacle State Update
```json
{
  "type": "obstacle_update",
  "id": "obs_1",
  "groupId": "",
  "lat": 48.0,
  "lon": 2.0,
  "radius": 5.0,
  "dynamic": false
}
```
**Frequency**: On creation + movement updates for dynamic obstacles

### Charging Station Update
```json
{
  "type": "charging_station_update",
  "id": "station_1",
  "lat": 25.2048,
  "lon": 55.2708,
  "name": "Dubai Charging Node",
  "chargingRate": 20.0
}
```

---

## Client → Server Messages

All client messages use a pub/sub topic format:

### Set Target
```
Topic: cmd/fleet/{targetID}/target
Payload: { "lat": 48.8566, "lon": 2.3522 }
```
- `targetID` can be a specific UAV ID, group ID, or `"all"`

### Set Control Parameters
```
Topic: cmd/fleet/{targetID}/control
Payload: { "speed": 0.5, "batteryDrain": 0.02 }
```
- Both fields optional; omitted fields remain unchanged

### Spawn Drone
```
Topic: cmd/fleet/spawn
Payload: { "lat": 48.8566, "lon": 2.3522, "groupId": "alpha" }
```

### Create Obstacle
```
Topic: cmd/environment/obstacle
Payload: { "lat": 48.0, "lon": 2.0, "radius": 250.0, "groupId": "" }
```

### External Telemetry (MAVLink bridge)
```
Topic: telemetry/external/{droneID}
Payload: { "lat": 51.5074, "lon": -0.1278, "alt": 150, "heading": 90, "battery": 88.5, "groupId": "bravo" }
```

## Error Handling
- Malformed JSON: Server logs error and ignores the message
- Unknown topic: Server logs warning and ignores
- Missing required payload fields: Server logs error and ignores
