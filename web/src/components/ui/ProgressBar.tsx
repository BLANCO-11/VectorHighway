"use client";

import React from 'react';

interface ProgressBarProps {
  value: number;
  max?: number;
  label?: string;
  showPercentage?: boolean;
  color?: string;
  height?: number;
}

export default function ProgressBar({
  value,
  max = 100,
  label,
  showPercentage = true,
  color = '#00d4ff',
  height = 6,
}: ProgressBarProps) {
  const pct = Math.min(Math.max((value / max) * 100, 0), 100);

  let barColor = color;
  if (pct < 20) barColor = '#ff3355';
  else if (pct < 40) barColor = '#ffaa00';

  return (
    <div className="w-full">
      {(label || showPercentage) && (
        <div className="flex justify-between items-center mb-1">
          {label && <span className="text-[10px] text-[#888899] uppercase tracking-wider">{label}</span>}
          {showPercentage && (
            <span className="text-[10px] font-mono" style={{ color: barColor }}>
              {pct.toFixed(1)}%
            </span>
          )}
        </div>
      )}
      <div
        className="w-full rounded-full overflow-hidden"
        style={{ height, backgroundColor: 'rgba(255,255,255,0.08)' }}
      >
        <div
          className="h-full rounded-full transition-all duration-300"
          style={{ width: `${pct}%`, backgroundColor: barColor }}
        />
      </div>
    </div>
  );
}
