import { useState, useEffect, useRef } from 'react';

export interface UAVState {
  lat: number;
  lon: number;
  alt: number;
  heading: number;
  battery: number;
  startLat: number;
  startLon: number;
  targetLat: number;
  targetLon: number;
}

export interface ObstacleState {
  id: string;
  lat: number;
  lon: number;
  radius: number;
  dynamic: boolean;
}

export const useSimulationWebSocket = (url: string = 'ws://localhost:8080') => {
  const [uavState, setUavState] = useState<UAVState | null>(null);
  const [obstacles, setObstacles] = useState<Record<string, ObstacleState>>({});
  const [isConnected, setIsConnected] = useState<boolean>(false);
  const wsRef = useRef<WebSocket | null>(null);

  useEffect(() => {
    // Initialize WebSocket connection
    const ws = new WebSocket(url);
    wsRef.current = ws;

    ws.onopen = () => {
      console.log('Connected to C++ Simulation Engine');
      setIsConnected(true);
    };

    ws.onclose = () => {
      console.log('Disconnected from Simulation Engine');
      setIsConnected(false);
    };

    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);

        if (data.type === 'uav_update') {
          setUavState({
            lat: data.lat,
            lon: data.lon,
            alt: data.alt,
            heading: data.heading,
            battery: data.battery,
            startLat: data.startLat,
            startLon: data.startLon,
            targetLat: data.targetLat,
            targetLon: data.targetLon,
          });
        } else if (data.type === 'obstacle_update') {
          setObstacles((prev) => ({
            ...prev,
            [data.id]: data,
          }));
        }
      } catch (error) {
        console.error('Failed to parse WebSocket message:', error);
      }
    };

    return () => {
      ws.close();
    };
  }, [url]);

  // Return obstacles as an array for easier rendering in 3D/UI
  return { 
    uavState, 
    obstacles: Object.values(obstacles), 
    isConnected,
    sendMessage: (msg: string) => wsRef.current?.send(msg) 
  };
};
