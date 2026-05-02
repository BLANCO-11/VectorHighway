"use client";

import React from 'react';
import Button from '../ui/Button';

const GROUP_COLORS: Record<string, string> = {
  alpha: '#32CD32',
  bravo: '#4488ff',
  charlie: '#ff00ff',
  default: '#ffffff',
};

const getColorForGroup = (group: string) => {
  if (GROUP_COLORS[group]) return GROUP_COLORS[group];
  let hash = 0;
  for (let i = 0; i < group.length; i++) hash = group.charCodeAt(i) + ((hash << 5) - hash);
  const c = (hash & 0x00ffffff).toString(16).toUpperCase();
  return '#' + '00000'.substring(0, 6 - c.length) + c;
};

interface SwarmPanelProps {
  groups: string[];
  activeGroup: string;
  newGroup: string;
  onNewGroupChange: (val: string) => void;
  onAddGroup: () => void;
  onSetActiveGroup: (group: string) => void;
}

export default function SwarmPanel({
  groups,
  activeGroup,
  newGroup,
  onNewGroupChange,
  onAddGroup,
  onSetActiveGroup,
}: SwarmPanelProps) {
  return (
    <div className="flex flex-col gap-2 mb-6">
      <div className="flex flex-wrap gap-2">
        {groups.map((g) => (
          <button
            key={g}
            onClick={() => onSetActiveGroup(g)}
            className={`flex-1 text-[10px] uppercase font-bold tracking-wider py-1.5 px-2 rounded transition-all border ${
              activeGroup === g
                ? 'border-white text-white'
                : 'border-white/10 text-white/50 hover:bg-white/5'
            }`}
            style={{
              backgroundColor:
                activeGroup === g ? getColorForGroup(g) + '40' : 'transparent',
              borderColor: activeGroup === g ? getColorForGroup(g) : '',
            }}
          >
            {g}
          </button>
        ))}
      </div>
      <div className="flex gap-2 mt-1">
        <input
          type="text"
          value={newGroup}
          onChange={(e) => onNewGroupChange(e.target.value)}
          placeholder="NEW SWARM GROUP"
          className="flex-1 bg-white/5 border border-white/10 rounded px-2 py-1 text-[10px] text-white focus:outline-none focus:border-white/30 uppercase tracking-widest"
          onKeyDown={(e) => {
            if (e.key === 'Enter') onAddGroup();
          }}
        />
        <Button size="sm" onClick={onAddGroup}>
          Add
        </Button>
      </div>
    </div>
  );
}