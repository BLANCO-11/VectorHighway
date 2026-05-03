"use client";

import React, { useState } from 'react';
import Card from '../ui/Card';
import Badge from '../ui/Badge';
import Button from '../ui/Button';

interface MirrorState {
  isActive: boolean;
  conflicts: string[];
  isSimulating: boolean;
}

export default function MirrorPanel({ sendMessage }: { sendMessage: (msg: string) => void }) {
  const [mirror, setMirror] = useState<MirrorState>({
    isActive: false,
    conflicts: [],
    isSimulating: false,
  });

  const handleClone = () => {
    sendMessage(JSON.stringify({ topic: 'cmd/mirror/clone', payload: {} }));
    setMirror((prev) => ({ ...prev, isActive: true }));
  };

  const handleSimulate = () => {
    sendMessage(JSON.stringify({ topic: 'cmd/mirror/simulate', payload: { ticks: 60 } }));
    setMirror((prev) => ({ ...prev, isSimulating: true }));
  };

  const handlePromote = () => {
    sendMessage(JSON.stringify({ topic: 'cmd/mirror/promote', payload: {} }));
    setMirror({ isActive: false, conflicts: [], isSimulating: false });
  };

  const handleDiff = () => {
    sendMessage(JSON.stringify({ topic: 'cmd/mirror/diff', payload: {} }));
  };

  const handleDiscard = () => {
    setMirror({ isActive: false, conflicts: [], isSimulating: false });
  };

  const hasConflicts = mirror.conflicts.length > 0;

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">Digital Twin</h2>

      <div className="space-y-2 mb-3">
        <div className="flex justify-between items-center">
          <span className="text-[9px] text-[#888899] uppercase">Sandbox</span>
          <Badge status={mirror.isActive ? 'CONNECTED' : 'IDLE'} size="sm" />
        </div>
        {mirror.isSimulating && (
          <div className="flex justify-between items-center">
            <span className="text-[9px] text-[#888899] uppercase">Simulating</span>
            <span className="text-[9px] text-[#00ff88] animate-pulse">● Active</span>
          </div>
        )}
      </div>

      <div className="flex flex-wrap gap-2 mb-3">
        <Button size="sm" variant="primary" onClick={handleClone} disabled={mirror.isActive}>
          Clone
        </Button>
        <Button size="sm" variant="success" onClick={handleSimulate} disabled={!mirror.isActive}>
          Simulate
        </Button>
        <Button size="sm" variant="primary" onClick={handleDiff} disabled={!mirror.isActive}>
          Diff
        </Button>
      </div>

      <div className="flex gap-2">
        <Button
          size="sm"
          variant="success"
          onClick={handlePromote}
          disabled={!mirror.isActive || hasConflicts}
        >
          Promote
        </Button>
        <Button size="sm" variant="danger" onClick={handleDiscard} disabled={!mirror.isActive}>
          Discard
        </Button>
      </div>

      {hasConflicts && (
        <div className="mt-3 p-2 bg-[#ff3355]10 border border-[#ff3355]/20 rounded-lg">
          <p className="text-[9px] text-[#ff3355] uppercase mb-1">Conflicts</p>
          {mirror.conflicts.map((c, i) => (
            <p key={i} className="text-[10px] text-white/60">{c}</p>
          ))}
        </div>
      )}
    </Card>
  );
}
