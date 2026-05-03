"use client";

import React from 'react';
import Card from '../ui/Card';
import Badge from '../ui/Badge';

interface ChecklistItem {
  label: string;
  required: boolean;
  autoCheck: boolean;
  status: 'PENDING' | 'PASS' | 'FAIL' | 'SKIP';
}

const DEFAULT_ITEMS: ChecklistItem[] = [
  { label: 'Battery Level > 20%', required: true, autoCheck: true, status: 'PENDING' },
  { label: 'GPS Signal Lock', required: true, autoCheck: true, status: 'PENDING' },
  { label: 'Control Link Active', required: true, autoCheck: true, status: 'PENDING' },
  { label: 'No Active Alerts', required: false, autoCheck: true, status: 'PENDING' },
  { label: 'Propulsion System', required: true, autoCheck: false, status: 'PENDING' },
];

export default function ChecklistPanel() {
  const [items, setItems] = React.useState(DEFAULT_ITEMS);

  const overrideItem = (label: string, status: 'PASS' | 'FAIL' | 'SKIP') => {
    setItems((prev) =>
      prev.map((item) => (item.label === label ? { ...item, status } : item))
    );
  };

  const allRequiredPassed = items.filter((i) => i.required).every((i) => i.status === 'PASS');
  const canLaunch = allRequiredPassed;

  return (
    <Card>
      <div className="flex justify-between items-center mb-3">
        <h2 className="text-[10px] text-white/40 uppercase tracking-widest">Pre-Flight Checklist</h2>
        <Badge status={canLaunch ? 'CONNECTED' : 'DISCONNECTED'} label={canLaunch ? 'Ready' : 'Blocked'} />
      </div>

      <div className="space-y-1">
        {items.map((item) => {
          const statusColor =
            item.status === 'PASS' ? '#00ff88' :
            item.status === 'FAIL' ? '#ff3355' :
            item.status === 'SKIP' ? '#888899' : '#555566';

          return (
            <div key={item.label} className="flex items-center justify-between py-1 border-b border-white/5 last:border-0">
              <div className="flex-1">
                <span className="text-xs" style={{ color: statusColor }}>{item.label}</span>
                {item.required && <span className="text-[9px] text-[#ff3355] ml-1">*</span>}
                {item.autoCheck && <span className="text-[9px] text-[#555566] ml-1">(auto)</span>}
              </div>
              <div className="flex gap-1">
                <button
                  onClick={() => overrideItem(item.label, 'PASS')}
                  className={`text-[9px] px-1.5 py-0.5 rounded ${
                    item.status === 'PASS' ? 'bg-[#00ff88]20 text-[#00ff88]' : 'text-[#555566] hover:text-[#00ff88]'
                  }`}
                >
                  PASS
                </button>
                <button
                  onClick={() => overrideItem(item.label, 'FAIL')}
                  className={`text-[9px] px-1.5 py-0.5 rounded ${
                    item.status === 'FAIL' ? 'bg-[#ff3355]20 text-[#ff3355]' : 'text-[#555566] hover:text-[#ff3355]'
                  }`}
                >
                  FAIL
                </button>
                <button
                  onClick={() => overrideItem(item.label, 'SKIP')}
                  className={`text-[9px] px-1.5 py-0.5 rounded ${
                    item.status === 'SKIP' ? 'bg-[#888899]20 text-[#888899]' : 'text-[#555566] hover:text-[#888899]'
                  }`}
                >
                  SKIP
                </button>
              </div>
            </div>
          );
        })}
      </div>

      <div className="mt-3 pt-3 border-t border-white/10 text-center">
        <span className={`text-[10px] uppercase tracking-wider ${canLaunch ? 'text-[#00ff88]' : 'text-[#ff3355]'}`}>
          {canLaunch ? '✓ Ready for Launch' : '✗ Checklist Incomplete'}
        </span>
      </div>
    </Card>
  );
}
