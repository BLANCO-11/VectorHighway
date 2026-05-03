"use client";

import React from 'react';

interface AlertProps {
  level: 'CAUTION' | 'WARNING' | 'CRITICAL';
  message: string;
  source?: string;
  onDismiss?: () => void;
}

const LEVEL_COLORS: Record<string, string> = {
  CAUTION: '#ffaa00',
  WARNING: '#ff8800',
  CRITICAL: '#ff3355',
};

const LEVEL_BG: Record<string, string> = {
  CAUTION: 'rgba(255,170,0,0.1)',
  WARNING: 'rgba(255,136,0,0.1)',
  CRITICAL: 'rgba(255,51,85,0.1)',
};

export default function Alert({ level, message, source, onDismiss }: AlertProps) {
  const color = LEVEL_COLORS[level] || '#ffaa00';
  const bg = LEVEL_BG[level] || 'rgba(255,170,0,0.1)';

  return (
    <div
      className="flex items-start gap-2 px-3 py-2 rounded-lg text-xs animate-slide-in"
      style={{ backgroundColor: bg, borderLeft: `3px solid ${color}` }}
    >
      <div className="flex-1 min-w-0">
        {source && (
          <span className="font-medium uppercase tracking-wider text-[10px]" style={{ color }}>
            {source}
          </span>
        )}
        <p className="text-[#e0e0e0] truncate">{message}</p>
      </div>
      {onDismiss && (
        <button
          onClick={onDismiss}
          className="text-[#555566] hover:text-[#e0e0e0] transition-colors shrink-0"
        >
          ✕
        </button>
      )}
    </div>
  );
}
