"use client";

import React, { useRef, useEffect } from 'react';

interface TimelineEvent {
  id: string;
  time: string;
  message: string;
  level?: string;
}

interface TimelineProps {
  events: TimelineEvent[];
  maxHeight?: number;
  filterLevel?: string;
}

const LEVEL_COLORS: Record<string, string> = {
  NOMINAL: '#00ff88',
  CAUTION: '#ffaa00',
  WARNING: '#ff8800',
  CRITICAL: '#ff3355',
};

export default function Timeline({ events, maxHeight = 200, filterLevel }: TimelineProps) {
  const listRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (listRef.current) {
      listRef.current.scrollTop = listRef.current.scrollHeight;
    }
  }, [events]);

  const filtered = filterLevel
    ? events.filter((e) => e.level === filterLevel)
    : events;

  return (
    <div
      ref={listRef}
      className="overflow-y-auto font-mono text-[11px] space-y-0.5 pr-1"
      style={{ maxHeight }}
    >
      {filtered.length === 0 && (
        <p className="text-[#555566] italic">No events</p>
      )}
      {filtered.map((event) => (
        <div key={event.id} className="flex gap-2 py-0.5 border-b border-white/5 last:border-0">
          <span className="text-[#555566] shrink-0 w-16">{event.time}</span>
          {event.level && (
            <span
              className="uppercase font-bold shrink-0 w-14 text-[9px]"
              style={{ color: LEVEL_COLORS[event.level] || '#888899' }}
            >
              {event.level}
            </span>
          )}
          <span className="text-[#e0e0e0] truncate">{event.message}</span>
        </div>
      ))}
    </div>
  );
}
