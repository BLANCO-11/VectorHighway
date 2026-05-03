"use client";

import React from 'react';
import Toggle from '../ui/Toggle';
import Alert from '../ui/Alert';
import Card from '../ui/Card';
import { useSimulationStore } from '../../store/useSimulationStore';

export default function EmergencyPanel() {
  const alerts = useSimulationStore((s) => s.alerts);
  const clearAlerts = useSimulationStore((s) => s.clearAlerts);

  const [rules, setRules] = React.useState([
    { id: 'battery_low', name: 'Battery < 20%', condition: 'battery < 20 && alt > 400', action: 'RTH', priority: 1, enabled: true },
    { id: 'battery_critical', name: 'Battery < 10%', condition: 'battery < 10', action: 'LAND', priority: 0, enabled: true },
    { id: 'gps_loss', name: 'GPS Loss', condition: 'gps_loss && alt > 200', action: 'DESCEND', priority: 2, enabled: true },
  ]);

  const toggleRule = (ruleId: string) => {
    setRules((prev) =>
      prev.map((r) => (r.id === ruleId ? { ...r, enabled: !r.enabled } : r))
    );
  };

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">Emergency Engine</h2>

      <div className="space-y-2 mb-4">
        {rules.map((rule) => (
          <div key={rule.id} className="flex items-center justify-between py-1 border-b border-white/5 last:border-0">
            <div className="flex-1">
              <p className="text-xs text-white/80">{rule.name}</p>
              <p className="text-[9px] text-[#555566] font-mono">{rule.action}</p>
            </div>
            <Toggle
              enabled={rule.enabled}
              onChange={() => toggleRule(rule.id)}
              accentColor="#ff3355"
            />
          </div>
        ))}
      </div>

      <div className="border-t border-white/10 pt-3 mt-3">
        <div className="flex justify-between items-center mb-2">
          <h3 className="text-[10px] text-white/40 uppercase tracking-widest">Active Alerts</h3>
          {alerts.length > 0 && (
            <button onClick={clearAlerts} className="text-[9px] text-[#555566] hover:text-white transition-colors">
              Clear
            </button>
          )}
        </div>
        <div className="space-y-1 max-h-32 overflow-y-auto">
          {alerts.length === 0 && (
            <p className="text-[10px] text-[#555566] italic">No alerts</p>
          )}
          {alerts.slice(-5).reverse().map((alert, idx) => (
            <Alert
              key={idx}
              level={alert.level as any}
              message={alert.message}
              source={alert.source}
            />
          ))}
        </div>
      </div>
    </Card>
  );
}
