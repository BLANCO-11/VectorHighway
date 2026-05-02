"use client";

import React from 'react';
import { LineChart, Line, YAxis, ResponsiveContainer } from 'recharts';

interface BatteryChartProps {
  history: { time: string; battery: number }[];
  color: string;
  groupName: string;
}

export default function BatteryChart({ history, color, groupName }: BatteryChartProps) {
  return (
    <div className="p-4 rounded-2xl bg-black/40 backdrop-blur-md border border-white/10 shadow-2xl flex-1 flex flex-col min-h-0">
      <h2 className="text-[10px] text-white/40 uppercase mb-2 tracking-widest">
        Avg Battery ({groupName})
      </h2>
      <div className="flex-1 min-h-0">
        <ResponsiveContainer width="100%" height="100%">
          <LineChart data={history}>
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