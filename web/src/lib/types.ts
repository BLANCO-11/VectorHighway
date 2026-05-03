export type MissionState = 'IDLE' | 'UPLOADED' | 'IN_PROGRESS' | 'PAUSED' | 'COMPLETED' | 'FAILED' | 'RTH' | 'ABORTED' | 'EMERGENCY';
export type AlertLevel = 'NOMINAL' | 'CAUTION' | 'WARNING' | 'CRITICAL';
export type ConnectionStatus = 'CONNECTED' | 'STALE' | 'DISCONNECTED' | 'IDLE';
export type ClickMode = 'target' | 'obstacle' | 'spawn_drone';
export type WaypointAction = 'FLY_OVER' | 'FLY_TO' | 'LOITER' | 'RTH';

export interface UAVState {
  id: string;
  groupId: string;
  lat: number;
  lon: number;
  alt: number;
  heading: number;
  battery: number;
  speed?: number;
  startLat: number;
  startLon: number;
  targetLat: number;
  targetLon: number;
  missionState?: MissionState;
  isExternal?: boolean;
  connectionStatus?: ConnectionStatus;
}

export interface ObstacleState {
  id: string;
  lat: number;
  lon: number;
  radius: number;
  dynamic: boolean;
  groupId?: string;
}

export interface NoFlyZoneState {
  id: string;
  name: string;
  shape: 'CIRCLE' | 'POLYGON';
  center?: { lat: number; lon: number };
  radius?: number;
  vertices?: { lat: number; lon: number }[];
  active: boolean;
}

export interface ChargingStationState {
  id: string;
  lat: number;
  lon: number;
  name: string;
  chargingRate: number;
}

export interface MissionWaypoint {
  coordinate: { lat: number; lon: number; alt?: number };
  name: string;
  altitude: number;
  action: WaypointAction;
  loiterDuration: number;
}

export interface MissionPlan {
  id: string;
  droneId: string;
  waypoints: MissionWaypoint[];
  homeLat: number;
  homeLon: number;
  state: MissionState;
  currentWaypointIndex: number;
  totalDistance: number;
  estimatedDuration: number;
  estimatedBatteryUse: number;
}

export interface HealthStatus {
  engineTickHz: number;
  wsConnectionCount: number;
  alerts: Alert[];
  drones: Record<string, DroneHealth>;
}

export interface DroneHealth {
  batteryLevel: number;
  lastTelemetryTime: number;
  missionState: string;
  connectionStatus: ConnectionStatus;
}

export interface Alert {
  level: AlertLevel;
  source: string;
  message: string;
  timestamp: number;
}

export interface ExternalTelemetry {
  id: string;
  groupId: string;
  lat: number;
  lon: number;
  alt: number;
  heading: number;
  battery: number;
  speed: number;
  status: string;
}

export interface ControlParams {
  speed: number;
  batteryDrain: number;
  turnRate: number;
  chargingRate: number;
}

export interface SelectedEntity {
  type: 'uav' | 'obstacle' | 'destination' | 'none' | 'spawn_drone';
  id?: string;
  lat?: number;
  lon?: number;
}

export interface SwarmGroup {
  id: string;
  drones: string[];
  color: string;
  target?: { lat: number; lon: number };
}

export interface LogEntry {
  timestamp: number;
  droneId: string;
  lat: number;
  lon: number;
  alt: number;
  heading: number;
  speed: number;
  battery: number;
  missionState: string;
}

export interface EmergencyRule {
  id: string;
  name: string;
  condition: string;
  action: string;
  priority: number;
  enabled: boolean;
}

export interface ChecklistItem {
  label: string;
  required: boolean;
  autoCheck: boolean;
  status: 'PENDING' | 'PASS' | 'FAIL' | 'SKIP';
}

export interface Checklist {
  name: string;
  items: ChecklistItem[];
}
