"use client";

import React from 'react';
import Card from '../ui/Card';
import Badge from '../ui/Badge';
import { useSimulationStore } from '../../store/useSimulationStore';

export default function TelemetryPanel() {
  const selected = useSimulationStore((s) => s.selectedEntity);
  const uavs = useSimulationStore((s) => s.uavs);
  const selectedUav = selected.type === 'uav' && selected.id ? uavs[selected.id] : null;

  if (!selectedUav) {
    return (
      <Card animate>
        <h2 className="text-[10px] text-white/40 uppercase mb-4 tracking-widest">Telemetry</h2>
        <p className="text-[#555566] text-xs">Select a UAV to view telemetry</p>
      </Card>
    );
  }

  const uav = selectedUav;
  const connStatus = (uav as any).isExternal ? 'STALE' : 'CONNECTED';

  return (
    <Card animate>
      <h2 className="text-[10px] text-white/40 uppercase mb-4 tracking-widest">Telemetry</h2>
      <div className="space-y-3">
        <div className="flex justify-between items-center">
          <span className="text-[10px] text-white/40 uppercase">Entity</span>
          <div className="flex items-center gap-2">
            <Badge status={connStatus as any} size="sm" />
            <span className="text-xs font-mono text-[#00d4ff] uppercase">{uav.id}</span>
          </div>
        </div>
        <div className="grid grid-cols-2 gap-2 text-sm">
          <div>
            <p className="text-white/40 text-[9px] uppercase">Lat</p>
            <p className="font-mono text-white text-xs">{uav.lat.toFixed(4)}</p>
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Lon</p>
            <p className="font-mono text-white text-xs">{uav.lon.toFixed(4)}</p>
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Heading</p>
            <p className="font-mono text-white text-xs">{uav.heading.toFixed(1)}°</p>
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Battery</p>
            <p className="font-mono text-white text-xs">{uav.battery.toFixed(1)}%</p>
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Speed</p>
            <p className="font-mono text-white text-xs">{(uav as any).speed?.toFixed(3) ?? '-'}</p>
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Mission</p>
            <p className="font-mono text-white text-xs">{(uav as any).missionState ?? 'IDLE'}</p>
          </div>
        </div>
      </div>
    </Card>
  );
}
