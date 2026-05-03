# Contracts: WebSocket Message Schemas

All messages use JSON format over WebSocket (text frames).

---

## 1. Inbound: Client → Server (Commands)

All commands follow pub/sub topic format:
```json
{"topic": "<topic_path>", "payload": {<fields>}}
```

### 1.1 Set Target
```
Topic: cmd/fleet/{drone_id_or_group_or_all}/target
Payload: {"lat": 48.8566, "lon": 2.3522}
```

### 1.2 Control Parameters
```
Topic: cmd/fleet/{drone_id_or_group}/control
Payload: {"speed": 0.25, "batteryDrain": 0.05}
```
Both fields are optional; only provided fields are updated.

### 1.3 Spawn Drone
```
Topic: cmd/fleet/spawn
Payload: {"lat": 48.8566, "lon": 2.3522, "groupId": "alpha"}
```

### 1.4 Create Obstacle
```
Topic: cmd/environment/obstacle
Payload: {"lat": 48.0, "lon": 2.0, "radius": 250.0, "groupId": "alpha"}
```
groupId is optional; empty = global.

### 1.5 External Telemetry (Real Drone Ingestion)
```
Topic: telemetry/external/{drone_id}
Payload: {
  "lat": 48.8566, "lon": 2.3522, "alt": 0.5,
  "heading": 90.0, "battery": 85.0,
  "groupId": "alpha", "speed": 0.25, "status": "FLYING"
}
```
All fields except lat/lon are optional and default to reasonable values.

### 1.6 Upload Mission (NEW)
```
Topic: cmd/mission/{drone_id}/upload
Payload: {
  "waypoints": [
    {"lat": 48.8566, "lon": 2.3522, "alt": 0.5, "action": "FLY_TO"},
    {"lat": 51.5074, "lon": -0.1278, "alt": 0.5, "action": "LOITER", "loiterDuration": 30}
  ],
  "homeLat": 48.8566,
  "homeLon": 2.3522
}
```

### 1.7 Start Mission (NEW)
```
Topic: cmd/mission/{drone_id}/start
Payload: {}
```

### 1.8 Abort Mission / Return to Home (NEW)
```
Topic: cmd/mission/{drone_id}/rth
Payload: {}
```

### 1.9 Toggle Flight Recording (NEW)
```
Topic: cmd/system/recording
Payload: {"enabled": true}
```

### 1.10 Replay Flight Log (NEW)
```
Topic: cmd/system/replay
Payload: {"file": "flightlog_20260502_120000.jsonl", "speed": 1.0}
```

---

## 2. Outbound: Server → Client (State Updates)

### 2.1 UAV State Update (20Hz per drone)
```json
{
  "type": "uav_update",
  "id": "alpha_1",
  "groupId": "alpha",
  "lat": 48.856600,
  "lon": 2.352200,
  "alt": 0.5,
  "heading": 90.0,
  "battery": 95.5,
  "speed": 0.25,
  "startLat": 26.870601,
  "startLon": 75.794110,
  "targetLat": 48.423366,
  "targetLon": 2.345332,
  "missionState": "IN_PROGRESS"
}
```

### 2.2 Obstacle State Update (on creation)
```json
{
  "type": "obstacle_update",
  "id": "obs_1",
  "groupId": "",
  "lat": 48.000000,
  "lon": 2.000000,
  "radius": 250.0,
  "dynamic": false
}
```

### 2.3 Health Status (NEW, 1Hz)
```json
{
  "type": "state/health",
  "engineTickHz": 20.0,
  "wsConnectionCount": 2,
  "alerts": [
    {"level": "WARNING", "source": "drone_alpha_1", "message": "Battery below 20%", "timestamp": 1234567890.5}
  ],
  "drones": {
    "alpha_1": {"battery": 18.5, "connectionStatus": "CONNECTED", "missionState": "RTH"},
    "ext_1": {"battery": 90.0, "connectionStatus": "STALE", "missionState": "IDLE"}
  }
}
```

### 2.4 Mission Status Update (NEW, on state change)
```json
{
  "type": "state/mission",
  "droneId": "alpha_1",
  "missionState": "IN_PROGRESS",
  "currentWaypoint": 2,
  "totalWaypoints": 5,
  "estimatedTimeRemaining": 180.0
}
```

### 2.5 Alert (NEW, on alert trigger)
```json
{
  "type": "state/alert",
  "level": "CRITICAL",
  "source": "drone_alpha_1",
  "message": "Battery critically low (5%), initiating RTH",
  "autoAction": "RTH"
}
```

### 2.6 Connection Status (on connect/disconnect)
```json
{
  "type": "connection_status",
  "connected": true,
  "message": "Connected to UAV Engine v2.0 on port 7200"
}
```

---

## 3. Message Flow Diagram

```
Client                          Server
  |                               |
  |--- cmd/fleet/{id}/target ---->|  Set destination
  |--- cmd/fleet/{id}/control --->|  Adjust speed/drain
  |--- cmd/fleet/spawn ----------->|  Spawn new drone
  |--- cmd/environment/obstacle -->|  Create obstacle
  |--- telemetry/external/{id} --->|  Real drone telemetry
  |--- cmd/mission/{id}/upload --->|  Upload mission plan
  |--- cmd/mission/{id}/start ---->|  Begin mission
  |--- cmd/mission/{id}/rth ------>|  Return to home
  |                               |
  |<-- uav_update (20Hz) ---------|  Drone state
  |<-- obstacle_update ------------|  Obstacle state
  |<-- state/health (1Hz) --------|  System health
  |<-- state/mission ---------------|  Mission progress
  |<-- state/alert -----------------|  Alerts
  |<-- connection_status -----------|  Connection info
```

## 4. Error Responses

Malformed JSON, unknown topics, or invalid payloads produce no response (silently ignored with console log). Future: error topic.

## 5. Serialization Format

- Numbers: JSON number (doubles with up to 6 decimal places for lat/lon)
- Strings: UTF-8
- Booleans: JSON true/false
- Nulls: Omitted (not sent) rather than JSON null
