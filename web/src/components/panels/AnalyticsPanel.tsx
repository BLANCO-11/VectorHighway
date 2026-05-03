"use client";

import React from 'react';
import Card from '../ui/Card';

export default function AnalyticsPanel() {
  const stats = [
    { label: 'Total Flight Hours', value: '1,247' },
    { label: 'Total Battery Cycles', value: '892' },
    { label: 'Incidents Reported', value: '3' },
    { label: 'Active Drones', value: '12' },
    { label: 'Avg Mission Duration', value: '24m' },
  ];

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">Fleet Analytics</h2>
      <div className="grid grid-cols-2 gap-3">
        {stats.map((stat) => (
          <div key={stat.label} className="p-2 bg-white/5 rounded-lg">
            <p className="text-[18px] font-mono text-[#00d4ff]">{stat.value}</p>
            <p className="text-[9px] text-[#888899] uppercase mt-1">{stat.label}</p>
          </div>
        ))}
      </div>
    </Card>
  );
}
