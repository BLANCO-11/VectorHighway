"use client";

import React from 'react';
import { LineChart, Line, YAxis, ResponsiveContainer } from 'recharts';
import { useSimulationStore } from '../../store/useSimulationStore';
import { GROUP_COLORS } from '../../lib/constants';

export default function BatteryChart() {
  const uavs = useSimulationStore((s) => s.uavs);
  const activeGroup = useSimulationStore((s) => s.activeGroup);

  const activeDrones = Object.values(uavs).filter((u) => u.groupId === activeGroup);
  const avgBattery =
    activeDrones.length > 0
      ? activeDrones.reduce((sum, u) => sum + u.battery, 0) / activeDrones.length
      : 0;

  const color = GROUP_COLORS[activeGroup] || GROUP_COLORS.default;

  const data = [{ time: new Date().toLocaleTimeString(), battery: avgBattery }];

  return (
    <div className="glass-panel p-4 flex flex-col min-h-[120px]">
      <h2 className="text-[10px] text-white/40 uppercase mb-2 tracking-widest">
        Avg Battery ({activeGroup})
      </h2>
      <div className="flex-1" style={{ minHeight: 60 }}>
        <ResponsiveContainer width="100%" height="100%" minHeight={60}>
          <LineChart data={data}>
            <YAxis domain={[0, 100]} hide />
            <Line
              type="monotone"
              dataKey="battery"
              stroke={color}
              strokeWidth={2}
              dot={false}
              isAnimationActive={false}
            />
          </LineChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
}
