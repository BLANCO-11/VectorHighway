import { useEffect } from 'react';
import { useSimulationStore } from '../store/useSimulationStore';

export function useHealthMonitor() {
  const addAlert = useSimulationStore((s) => s.addAlert);
  const setHealth = useSimulationStore((s) => s.setHealth);

  const handleHealthMessage = (data: any) => {
    if (data.type === 'state/health') {
      setHealth({
        engineTickHz: data.engineTickHz || 0,
        wsConnectionCount: data.wsConnectionCount || 0,
      });

      if (data.alerts && Array.isArray(data.alerts)) {
        data.alerts.forEach((alert: any) => {
          addAlert({
            level: alert.level || 'NOMINAL',
            source: alert.source || 'system',
            message: alert.message || '',
            timestamp: alert.timestamp || Date.now(),
          });
        });
      }
    }

    if (data.type === 'state/alert') {
      addAlert({
        level: data.level || 'WARNING',
        source: data.source || 'system',
        message: data.message || '',
        timestamp: data.timestamp || Date.now(),
      });
    }
  };

  return { handleHealthMessage };
}
