"use client";

import React from 'react';

interface BadgeProps {
  status: 'CONNECTED' | 'STALE' | 'DISCONNECTED' | 'IDLE';
  label?: string | false;
  size?: 'sm' | 'md';
}

const STATUS_COLORS: Record<string, string> = {
  CONNECTED: '#00ff88',
  STALE: '#ffaa00',
  DISCONNECTED: '#ff3355',
  IDLE: '#888899',
};

export default function Badge({ status, label, size = 'sm' }: BadgeProps) {
  const color = STATUS_COLORS[status] || '#888899';
  const dotSize = size === 'sm' ? 6 : 8;

  return (
    <span className="inline-flex items-center gap-1.5">
      <span
        className="rounded-full"
        style={{
          width: dotSize,
          height: dotSize,
          backgroundColor: color,
          boxShadow: `0 0 6px ${color}40`,
        }}
      />
      {label !== false && (
        <span className="text-[10px] font-medium uppercase tracking-wider" style={{ color }}>
          {label || status}
        </span>
      )}
    </span>
  );
}
