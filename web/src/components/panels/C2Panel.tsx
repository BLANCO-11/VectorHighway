"use client";

import React from 'react';
import Card from '../ui/Card';
import Badge from '../ui/Badge';

interface C2PanelProps {
  sendMessage: (msg: string) => void;
}

const ROLES = ['COMMANDER', 'SENSOR_OP', 'SUPERVISOR', 'VIEWER'];

export default function C2Panel({ sendMessage }: C2PanelProps) {
  const [currentRole, setCurrentRole] = React.useState('VIEWER');
  const [operatorId, setOperatorId] = React.useState('');
  const [operators, setOperators] = React.useState<string[]>([]);

  const handleConnect = () => {
    if (!operatorId) return;
    sendMessage(JSON.stringify({
      topic: 'cmd/auth/connect',
      payload: { operatorId, role: currentRole },
    }));
    setOperators((prev) => [...prev, `${operatorId} (${currentRole})`]);
  };

  const permissionList =
    currentRole === 'COMMANDER' || currentRole === 'SUPERVISOR'
      ? ['All commands', 'Mission control', 'Telemetry', 'Admin']
      : currentRole === 'SENSOR_OP'
        ? ['Telemetry', 'Gimbal control']
        : ['View telemetry only'];

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">C2 Console</h2>

      <div className="space-y-2 mb-3">
        <label className="text-[9px] text-[#888899] uppercase">Operator ID</label>
        <input
          type="text"
          value={operatorId}
          onChange={(e) => setOperatorId(e.target.value)}
          placeholder="Enter operator ID..."
          className="w-full bg-white/5 border border-white/10 rounded px-2 py-1 text-xs text-white"
        />

        <label className="text-[9px] text-[#888899] uppercase">Role</label>
        <div className="flex flex-wrap gap-1">
          {ROLES.map((role) => (
            <button
              key={role}
              onClick={() => setCurrentRole(role)}
              className={`text-[9px] px-2 py-1 rounded uppercase tracking-wider ${
                currentRole === role
                  ? 'bg-[#00d4ff]20 text-[#00d4ff] border border-[#00d4ff]/30'
                  : 'text-[#555566] border border-white/10 hover:text-white'
              }`}
            >
              {role}
            </button>
          ))}
        </div>

        <button
          onClick={handleConnect}
          className="w-full text-[10px] py-1.5 rounded bg-[#00d4ff]20 text-[#00d4ff] border border-[#00d4ff]/30 uppercase tracking-wider"
        >
          Connect
        </button>
      </div>

      <div className="border-t border-white/10 pt-3">
        <p className="text-[9px] text-[#888899] uppercase mb-2">Permissions</p>
        <div className="space-y-1">
          {permissionList.map((perm) => (
            <div key={perm} className="flex items-center gap-2">
              <span className="text-[#00ff88] text-[9px]">✓</span>
              <span className="text-[10px] text-white/60">{perm}</span>
            </div>
          ))}
        </div>
      </div>

      {operators.length > 0 && (
        <div className="border-t border-white/10 pt-3 mt-3">
          <p className="text-[9px] text-[#888899] uppercase mb-2">Connected Operators</p>
          {operators.map((op, i) => (
            <div key={i} className="flex items-center gap-2 py-0.5">
              <Badge status="CONNECTED" size="sm" />
              <span className="text-[10px] text-white/60">{op}</span>
            </div>
          ))}
        </div>
      )}
    </Card>
  );
}
