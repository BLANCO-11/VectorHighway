"use client";

import React from 'react';
import Card from '../ui/Card';

interface SelectedEntity {
  type: 'uav' | 'obstacle' | 'destination' | 'none' | 'spawn_drone';
  id?: string;
  lat?: number;
  lon?: number;
}

interface TelemetryPanelProps {
  selected: SelectedEntity;
}

export default function TelemetryPanel({ selected }: TelemetryPanelProps) {
  return (
    <Card animate>
      <h2 className="text-[10px] text-white/40 uppercase mb-4 tracking-widest">
        Telemetry
      </h2>
      <div className="space-y-3">
        <div className="flex justify-between items-center">
          <span className="text-[10px] text-white/40 uppercase">Entity</span>
          <span className="text-xs font-mono text-blue-400 uppercase">
            {selected.type === 'none' ? '-' : selected.type}
          </span>
        </div>
        <div className="grid grid-cols-2 gap-2 text-sm">
          <div>
            <p className="text-white/40 text-[9px] uppercase">Lat</p>
            <p className="font-mono text-white">
              {selected.lat?.toFixed(4) ?? '-'}
            </p>
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Lon</p>
            <p className="font-mono text-white">
              {selected.lon?.toFixed(4) ?? '-'}
            </p>
          </div>
        </div>
        {selected.id && (
          <div>
            <p className="text-white/40 text-[9px] uppercase">ID</p>
            <p className="font-mono text-xs text-white/80">{selected.id}</p>
          </div>
        )}
      </div>
    </Card>
  );
}