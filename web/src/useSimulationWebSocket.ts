import { useState, useEffect, useRef, useCallback } from 'react';
import { useSimulationStore } from './store/useSimulationStore';

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
  missionState?: string;
  isExternal?: boolean;
}

export interface ChargingStationState {
  id: string;
  lat: number;
  lon: number;
  name: string;
  chargingRate: number;
}

export interface ObstacleState {
  id: string;
  lat: number;
  lon: number;
  radius: number;
  dynamic: boolean;
  groupId?: string;
}

export const useSimulationWebSocket = (url: string = 'ws://localhost:7200') => {
  const [uavs, setUavs] = useState<Record<string, UAVState>>({});
  const [obstacles, setObstacles] = useState<Record<string, ObstacleState>>({});
  const [chargingStations, setChargingStations] = useState<Record<string, ChargingStationState>>({});
  const [isConnected, setIsConnected] = useState<boolean>(false);

  const wsRef = useRef<WebSocket | null>(null);
  const reconnectTimer = useRef<ReturnType<typeof setTimeout> | null>(null);
  const delayRef = useRef(1000);
  const uavsRef = useRef<Record<string, UAVState>>({});
  const obstaclesRef = useRef<Record<string, ObstacleState>>({});

  const sendMessage = useCallback((msg: string) => {
    const ws = wsRef.current;
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.send(msg);
      console.log('[ws] sent:', msg.substring(0, 100));
      return true;
    }
    console.warn('[ws] cannot send — state:', ws ? ['CONNECTING', 'OPEN', 'CLOSING', 'CLOSED'][ws.readyState] : 'null');
    return false;
  }, []);

  useEffect(() => {
    let cancelled = false;

    const connect = () => {
      if (wsRef.current) {
        wsRef.current.onopen = null;
        wsRef.current.onclose = null;
        wsRef.current.onmessage = null;
        wsRef.current.onerror = null;
        wsRef.current.close();
      }

      const ws = new WebSocket(url);
      wsRef.current = ws;

      ws.onopen = () => {
        if (cancelled) return;
        console.log('[ws] connected');
        setIsConnected(true);
        delayRef.current = 1000;
      };

      ws.onclose = () => {
        if (cancelled) return;
        console.log('[ws] disconnected');
        setIsConnected(false);
        wsRef.current = null;
        reconnectTimer.current = setTimeout(connect, delayRef.current);
        delayRef.current = Math.min(delayRef.current * 2, 30000);
      };

      ws.onmessage = (event) => {
        if (cancelled) return;
        try {
          const data = JSON.parse(event.data);
          if (data.type === 'uav_update') {
            if (uavsRef.current) uavsRef.current = { ...uavsRef.current, [data.id]: data };
            setUavs((prev) => ({ ...prev, [data.id]: data }));
          } else if (data.type === 'obstacle_update') {
            if (obstaclesRef.current) obstaclesRef.current = { ...obstaclesRef.current, [data.id]: data };
            setObstacles((prev) => ({ ...prev, [data.id]: data }));
          } else if (data.type === 'charging_station_update') {
            setChargingStations((prev) => ({ ...prev, [data.id]: data }));
          } else if (data.type === 'connection_status') {
            console.log('[ws] connection_status:', data.connected);
            setIsConnected(data.connected);
          } else if (data.type === 'obstacle_removed') {
            useSimulationStore.getState().removeObstacle(data.id);
            if (obstaclesRef.current) {
              const newObs = { ...obstaclesRef.current };
              delete newObs[data.id];
              obstaclesRef.current = newObs;
            }
            setObstacles((prev) => {
              const newObs = { ...prev };
              delete newObs[data.id];
              return newObs;
            });
          } else if (data.type === 'environment_cleared') {
            if (data.groupId) {
              useSimulationStore.getState().clearGroupObstacles(data.groupId);
              if (obstaclesRef.current) {
                const newObs: Record<string, ObstacleState> = {};
                for (const [k, v] of Object.entries(obstaclesRef.current)) {
                  if (v.groupId !== data.groupId) newObs[k] = v;
                }
                obstaclesRef.current = newObs;
              }
              setObstacles((prev) => {
                const newObs: Record<string, ObstacleState> = {};
                for (const [k, v] of Object.entries(prev)) {
                  if (v.groupId !== data.groupId) newObs[k] = v;
                }
                return newObs;
              });
            } else {
              useSimulationStore.getState().clearAllObstacles();
              obstaclesRef.current = {};
              setObstacles({});
            }
          } else if (data.type === 'path_update') {
            useSimulationStore.getState().setPathOverlay(data.droneId, data.path);
          }
        } catch (error) {
          console.error('Failed to parse message:', error);
        }
      };

      ws.onerror = () => {
        if (!cancelled) console.error('[ws] error');
      };
    };

    connect();

    return () => {
      cancelled = true;
      if (reconnectTimer.current) clearTimeout(reconnectTimer.current);
      if (wsRef.current) {
        wsRef.current.onopen = null;
        wsRef.current.onclose = null;
        wsRef.current.onmessage = null;
        wsRef.current.onerror = null;
        wsRef.current.close();
        wsRef.current = null;
      }
    };
  }, [url]);

  return {
    uavs: Object.values(uavs),
    obstacles: Object.values(obstacles),
    chargingStations: Object.values(chargingStations),
    isConnected,
    sendMessage,
  };
};
