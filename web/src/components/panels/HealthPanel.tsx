"use client";

import React from 'react';
import Card from '../ui/Card';
import Badge from '../ui/Badge';
import ProgressBar from '../ui/ProgressBar';
import Alert from '../ui/Alert';
import { useSimulationStore } from '../../store/useSimulationStore';

export default function HealthPanel() {
  const uavs = useSimulationStore((s) => s.uavs);
  const connectionStatus = useSimulationStore((s) => s.connectionStatus);
  const alerts = useSimulationStore((s) => s.alerts);
  const clearAlerts = useSimulationStore((s) => s.clearAlerts);

  const engineTickHz = useSimulationStore((s) => s.health.engineTickHz);
  const wsConnectionCount = useSimulationStore((s) => s.health.wsConnectionCount);

  const droneCount = Object.keys(uavs).length;
  const avgBattery =
    droneCount > 0
      ? Object.values(uavs).reduce((sum, u) => sum + u.battery, 0) / droneCount
      : 0;

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">System Health</h2>

      {/* Engine status */}
      <div className="grid grid-cols-2 gap-3 mb-4">
        <div>
          <p className="text-[9px] text-[#888899] uppercase">Engine Tick</p>
          <p className="font-mono text-xs" style={{ color: engineTickHz < 15 ? '#ffaa00' : '#00ff88' }}>
            {engineTickHz > 0 ? `${engineTickHz.toFixed(1)} Hz` : '--'}
          </p>
        </div>
        <div>
          <p className="text-[9px] text-[#888899] uppercase">WS Clients</p>
          <p className="font-mono text-xs text-[#00d4ff]">{wsConnectionCount}</p>
        </div>
        <div>
          <p className="text-[9px] text-[#888899] uppercase">Drones</p>
          <p className="font-mono text-xs text-white/80">{droneCount}</p>
        </div>
        <div>
          <p className="text-[9px] text-[#888899] uppercase">Connection</p>
          <Badge status={connectionStatus ? 'CONNECTED' : 'DISCONNECTED'} size="sm" />
        </div>
      </div>

      {/* Per-drone health */}
      <div className="space-y-2 mb-4 max-h-40 overflow-y-auto">
        {Object.values(uavs).map((uav) => {
          const connStatus = uav.isExternal ? 'STALE' : 'CONNECTED';
          return (
            <div key={uav.id} className="flex items-center gap-2 py-1 border-b border-white/5">
              <Badge status={connStatus as any} size="sm" label={false} />
              <span className="text-[10px] font-mono text-white/60 w-16 truncate">{uav.id}</span>
              <div className="flex-1">
                <ProgressBar value={uav.battery} max={100} showPercentage={false} height={3} />
              </div>
              <span className="text-[9px] font-mono" style={{ color: uav.battery < 20 ? '#ff3355' : '#00ff88' }}>
                {uav.battery.toFixed(0)}%
              </span>
            </div>
          );
        })}
      </div>

      {/* Alerts */}
      <div className="border-t border-white/10 pt-3">
        <div className="flex justify-between items-center mb-2">
          <h3 className="text-[9px] text-[#888899] uppercase">Alerts</h3>
          {alerts.length > 0 && (
            <button onClick={clearAlerts} className="text-[9px] text-[#555566] hover:text-white">
              Clear
            </button>
          )}
        </div>
        <div className="space-y-1 max-h-28 overflow-y-auto">
          {alerts.length === 0 && (
            <p className="text-[10px] text-[#555566] italic">No active alerts</p>
          )}
          {alerts.slice(-5).reverse().map((alert, idx) => (
            <Alert
              key={idx}
              level={alert.level as any}
              message={alert.message}
              source={alert.source}
            />
          ))}
        </div>
      </div>
    </Card>
  );
}
