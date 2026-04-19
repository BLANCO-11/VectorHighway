import { create } from 'zustand';

interface UAVState {
  latitude: number;
  longitude: number;
  altitude: number;
  heading: number;
  battery: number;
}

interface ObstacleState {
  type: string;
  lat: number;
  lon: number;
  radius: number;
  dynamic: boolean;
}

interface SimulationStore {
  uav: UAVState | null;
  obstacles: ObstacleState[];
  setUAV: (uav: UAVState) => void;
  setObstacles: (obstacles: ObstacleState[]) => void;
  updateUAV: (update: Partial<UAVState>) => void;
}

export const useSimulationStore = create<SimulationStore>((set) => ({
  uav: null,
  obstacles: [],
  setUAV: (uav) => set({ uav }),
  setObstacles: (obstacles) => set({ obstacles }),
  updateUAV: (update) =>
    set((state) => ({
      uav: state.uav ? { ...state.uav, ...update } : null,
    })),
}));
