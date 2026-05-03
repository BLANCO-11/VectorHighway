"use client";

import React from 'react';
import Card from '../ui/Card';

export default function ConditionEditor() {
  const [conditions, setConditions] = React.useState([
    { id: '1', expr: 'battery < 20', action: 'RTH', enabled: true },
  ]);

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">Condition Branching</h2>
      <div className="space-y-2">
        {conditions.map((cond) => (
          <div key={cond.id} className="flex items-center gap-2 py-1 border-b border-white/5">
            <span className="text-[9px] text-[#00d4ff]">IF</span>
            <span className="text-[10px] font-mono text-white/80">{cond.expr}</span>
            <span className="text-[9px] text-[#ffaa00]">→ {cond.action}</span>
          </div>
        ))}
      </div>
      <button className="mt-2 text-[9px] text-[#555566] hover:text-white">
        + Add Condition
      </button>
    </Card>
  );
}
