"use client";

import React from 'react';
import Card from '../ui/Card';
import Badge from '../ui/Badge';

export default function TenantPanel() {
  const [selectedTenant, setSelectedTenant] = React.useState('tenant_a');

  const tenants = ['tenant_a', 'tenant_b', 'admin'];
  const isAdmin = selectedTenant === 'admin';

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">
        {isAdmin ? 'Global Admin View' : 'Tenant View'}
      </h2>

      <div className="space-y-2 mb-3">
        <label className="text-[9px] text-[#888899] uppercase">Tenant</label>
        <div className="flex flex-wrap gap-1">
          {tenants.map((t) => (
            <button
              key={t}
              onClick={() => setSelectedTenant(t)}
              className={`text-[9px] px-2 py-1 rounded uppercase ${
                selectedTenant === t
                  ? 'bg-[#00d4ff]20 text-[#00d4ff] border border-[#00d4ff]/30'
                  : 'text-[#555566] border border-white/10'
              }`}
            >
              {t}
            </button>
          ))}
        </div>
      </div>

      <div className="space-y-2 pt-2 border-t border-white/10">
        <div className="flex justify-between text-[10px]">
          <span className="text-[#888899]">Drones</span>
          <span className="text-white/80">{isAdmin ? '12' : '4'}</span>
        </div>
        <div className="flex justify-between text-[10px]">
          <span className="text-[#888899]">Operators</span>
          <span className="text-white/80">{isAdmin ? '3' : '1'}</span>
        </div>
        <div className="flex justify-between text-[10px]">
          <span className="text-[#888899]">Active Missions</span>
          <span className="text-white/80">2</span>
        </div>
      </div>
    </Card>
  );
}
