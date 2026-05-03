"use client";

import React from 'react';
import Card from '../ui/Card';
import ProgressBar from '../ui/ProgressBar';

export default function LinkPanel() {
  const links = [
    { droneId: 'alpha_1', type: 'LOS', strength: 0.95, latency: 12, packetLoss: 0.01 },
    { droneId: 'bravo_1', type: 'SATCOM', strength: 0.72, latency: 45, packetLoss: 0.05 },
  ];

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">Comms Links</h2>
      <div className="space-y-3">
        {links.map((link) => (
          <div key={link.droneId} className="border-b border-white/5 pb-2">
            <div className="flex justify-between items-center mb-1">
              <span className="text-[10px] font-mono text-white/60">{link.droneId}</span>
              <span className="text-[9px] text-[#00d4ff]">{link.type}</span>
            </div>
            <ProgressBar value={link.strength * 100} max={100} label="Signal" height={3} />
            <div className="flex gap-3 mt-1 text-[9px] text-[#555566]">
              <span>Latency: {link.latency}ms</span>
              <span>Loss: {(link.packetLoss * 100).toFixed(0)}%</span>
            </div>
          </div>
        ))}
      </div>
    </Card>
  );
}
