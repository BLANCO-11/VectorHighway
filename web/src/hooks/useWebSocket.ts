import { useEffect, useCallback } from 'react';
import { useSimulationStore } from '../store/useSimulationStore';

export const useWebSocket = (url: string) => {
  const setUAV = useSimulationStore((state) => state.setUAV);
  const updateUAV = useSimulationStore((state) => state.updateUAV);
  const setObstacles = useSimulationStore((state) => state.setObstacles);

  const connect = useCallback(() => {
    const socket = new WebSocket(url);

    socket.onopen = () => {
      console.log('[WebSocket] Connected to', url);
    };

    socket.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        
        if (data.type === 'uav_update') {
          // If we don't have a UAV in store, set it. Otherwise update it.
          const currentUAV = useSimulationStore.getState().uav;
          if (!currentUAV) {
            setUAV({
              latitude: data.lat,
              longitude: data.lon,
              altitude: data.alt,
              heading: data.heading,
              battery: data.battery,
            });
          } else {
            updateUAV({
              latitude: data.lat,
              longitude: data.lon,
              altitude: data.alt,
              heading: data.heading,
              battery: data.battery,
            });
          }
        } else if (data.type === 'obstacle_update') {
          // For simplicity in this phase, we append obstacles. 
          // A production version would manage unique obstacle IDs.
          const currentObstacles = useSimulationStore.getState().obstacles;
          const newObstacle = {
            type: data.kind,
            lat: data.lat,
            lon: data.lon,
            radius: data.radius,
            dynamic: data.dynamic,
          };
          
          // Avoid duplicate obstacles based on position (very basic check)
          const isDuplicate = currentObstacles.some(
            (obs) => Math.abs(obs.lat - newObstacle.lat) < 0.0001 && Math.abs(obs.lon - newObstacle.lon) < 0.0001
          );

          if (!isDuplicate) {
            setObstacles([...currentObstacles, newObstacle]);
          }
        }
      } catch (error) {
        console.error('[WebSocket] Error parsing message:', error);
      }
    };

    socket.onerror = (error) => {
      console.error('[WebSocket] Error:', error);
    };

    socket.onclose = () => {
      console.log('[WebSocket] Disconnected');
    };

    return socket;
  }, [url, setUAV, updateUAV, setObstacles]);

  useEffect(() => {
    const socket = connect();
    return () => {
      socket.close();
    };
  }, [connect]);

  return { connect };
};
