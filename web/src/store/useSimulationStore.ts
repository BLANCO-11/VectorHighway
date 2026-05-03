import { create } from 'zustand';

interface UAV {
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
  missionState?: string;
  isExternal?: boolean;
}

interface Obstacle {
  id: string;
  lat: number;
  lon: number;
  radius: number;
  dynamic: boolean;
  groupId?: string;
}

interface ChargingStation {
  id: string;
  lat: number;
  lon: number;
  name: string;
  chargingRate: number;
}

interface SelectedEntity {
  type: 'uav' | 'obstacle' | 'destination' | 'none' | 'spawn_drone';
  id?: string;
  lat?: number;
  lon?: number;
}

interface ControlParams {
  speed: number;
  batteryDrain: number;
  turnRate: number;
  chargingRate: number;
}

interface Alert {
  level: string;
  source: string;
  message: string;
  timestamp: number;
}

interface MissionState {
  currentMission: string | null;
  missionStatus: string;
  missions: Record<string, any>;
}

interface ReplayState {
  isReplaying: boolean;
  replaySpeed: number;
  replayTime: number;
  availableLogs: string[];
}

interface SimulationStore {
  uavs: Record<string, UAV>;
  obstacles: Record<string, Obstacle>;
  chargingStations: Record<string, ChargingStation>;
  selectedEntity: SelectedEntity;
  connectionStatus: boolean;
  clickMode: 'target' | 'obstacle' | 'spawn_drone';
  activeGroup: string;
  groups: string[];
  params: ControlParams;
  alerts: Alert[];
  health: {
    engineTickHz: number;
    wsConnectionCount: number;
  };
  mission: MissionState;
  replay: ReplayState;
  obstacleRadius: number;
  deletionMode: 'none' | 'single' | 'group' | 'all';
  pathOverlays: Record<string, { lat: number; lon: number }[]>;
  simSpeed: number;

  setUavs: (uavs: Record<string, UAV>) => void;
  updateUav: (id: string, data: Partial<UAV>) => void;
  setObstacles: (obstacles: Record<string, Obstacle>) => void;
  setChargingStations: (stations: Record<string, ChargingStation>) => void;
  setSelectedEntity: (entity: SelectedEntity) => void;
  setConnectionStatus: (status: boolean) => void;
  setClickMode: (mode: 'target' | 'obstacle' | 'spawn_drone') => void;
  setActiveGroup: (group: string) => void;
  setGroups: (groups: string[]) => void;
  addGroup: (group: string) => void;
  setParams: (params: Partial<ControlParams>) => void;
  addAlert: (alert: Alert) => void;
  clearAlerts: () => void;
  setHealth: (health: { engineTickHz: number; wsConnectionCount: number }) => void;
  setMissionState: (state: Partial<MissionState>) => void;
  setReplay: (state: Partial<ReplayState>) => void;
  setObstacleRadius: (radius: number) => void;
  setDeletionMode: (mode: 'none' | 'single' | 'group' | 'all') => void;
  removeObstacle: (id: string) => void;
  clearGroupObstacles: (groupId: string) => void;
  clearAllObstacles: () => void;
  setPathOverlay: (droneId: string, path: { lat: number; lon: number }[]) => void;
  clearPathOverlay: (droneId: string) => void;
  setSimSpeed: (speed: number) => void;
  pendingContextDelete: { id: string; type: string; x?: number; y?: number } | null;
  setPendingContextDelete: (target: { id: string; type: string; x?: number; y?: number } | null) => void;
}

export const useSimulationStore = create<SimulationStore>((set) => ({
  uavs: {},
  obstacles: {},
  chargingStations: {},
  selectedEntity: { type: 'none' },
  connectionStatus: false,
  clickMode: 'target',
  activeGroup: 'alpha',
  groups: ['alpha', 'bravo', 'charlie'],
  params: {
    speed: 0.25,
    batteryDrain: 0.01,
    turnRate: 90.0,
    chargingRate: 2.0,
  },
  alerts: [],
  health: {
    engineTickHz: 0,
    wsConnectionCount: 0,
  },
  mission: {
    currentMission: null,
    missionStatus: 'IDLE',
    missions: {},
  },
  replay: {
    isReplaying: false,
    replaySpeed: 1,
    replayTime: 0,
    availableLogs: [],
  },
  obstacleRadius: 250,
  deletionMode: 'none',
  pathOverlays: {},
  simSpeed: 1,
  pendingContextDelete: null,

  setUavs: (uavs) => set({ uavs }),
  updateUav: (id, data) =>
    set((state) => ({
      uavs: {
        ...state.uavs,
        [id]: { ...state.uavs[id], ...data },
      },
    })),
  setObstacles: (obstacles) => set({ obstacles }),
  setChargingStations: (stations) => set({ chargingStations: stations }),
  setSelectedEntity: (entity) => set({ selectedEntity: entity }),
  setConnectionStatus: (status) => set({ connectionStatus: status }),
  setClickMode: (mode) => set({ clickMode: mode }),
  setActiveGroup: (group) => set({ activeGroup: group }),
  setGroups: (groups) => set({ groups }),
  addGroup: (group) =>
    set((state) => ({
      groups: state.groups.includes(group) ? state.groups : [...state.groups, group],
    })),
  setParams: (params) =>
    set((state) => ({
      params: { ...state.params, ...params },
    })),
  addAlert: (alert) =>
    set((state) => ({
      alerts: [...state.alerts.slice(-99), alert],
    })),
  clearAlerts: () => set({ alerts: [] }),
  setHealth: (health) => set({ health }),
  setMissionState: (state) =>
    set((prev) => ({ mission: { ...prev.mission, ...state } })),
  setReplay: (state) =>
    set((prev) => ({ replay: { ...prev.replay, ...state } })),
  setObstacleRadius: (radius) => set({ obstacleRadius: radius }),
  setDeletionMode: (mode) => set({ deletionMode: mode }),
  removeObstacle: (id) =>
    set((state) => {
      const newObs = { ...state.obstacles };
      delete newObs[id];
      return { obstacles: newObs };
    }),
  clearGroupObstacles: (groupId) =>
    set((state) => {
      const newObs: Record<string, Obstacle> = {};
      for (const [k, v] of Object.entries(state.obstacles)) {
        if (v.groupId !== groupId) newObs[k] = v;
      }
      return { obstacles: newObs };
    }),
  clearAllObstacles: () => set({ obstacles: {} }),
  setPathOverlay: (droneId, path) =>
    set((state) => ({ pathOverlays: { ...state.pathOverlays, [droneId]: path } })),
  clearPathOverlay: (droneId) =>
    set((state) => {
      const newPaths = { ...state.pathOverlays };
      delete newPaths[droneId];
      return { pathOverlays: newPaths };
    }),
  setSimSpeed: (speed) => set({ simSpeed: speed }),
  setPendingContextDelete: (target) => set({ pendingContextDelete: target }),
}));
