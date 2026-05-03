"use client";

import React from 'react';
import { motion, useSpring, useTransform } from 'framer-motion';
import Card from '../ui/Card';
import Badge from '../ui/Badge';
import { useSimulationStore } from '../../store/useSimulationStore';

function AnimatedNumber({ value, decimals = 1 }: { value: number; decimals?: number }) {
  const spring = useSpring(value, { stiffness: 80, damping: 15 });
  const display = useTransform(spring, (v) => v.toFixed(decimals));
  React.useEffect(() => { spring.set(value); }, [value, spring]);
  return <motion.span className="font-mono text-white text-xs">{display}</motion.span>;
}

export default function TelemetryPanel() {
  const selected = useSimulationStore((s) => s.selectedEntity);
  const uavs = useSimulationStore((s) => s.uavs);
  const selectedUav = selected.type === 'uav' && selected.id ? uavs[selected.id] : null;

  if (!selectedUav) {
    return (
      <Card animate>
        <h2 className="text-[10px] text-white/40 uppercase mb-4 tracking-widest">Telemetry</h2>
        <p className="text-[#555566] text-xs">Select a UAV to view telemetry</p>
      </Card>
    );
  }

  const uav = selectedUav;
  const connStatus = (uav as any).isExternal ? 'STALE' : 'CONNECTED';

  return (
    <Card animate>
      <h2 className="text-[10px] text-white/40 uppercase mb-4 tracking-widest">Telemetry</h2>
      <div className="space-y-3">
        <div className="flex justify-between items-center">
          <span className="text-[10px] text-white/40 uppercase">Entity</span>
          <div className="flex items-center gap-2">
            <Badge status={connStatus as any} size="sm" />
            <span className="text-xs font-mono text-[#00d4ff] uppercase">{uav.id}</span>
          </div>
        </div>
        <div className="grid grid-cols-2 gap-2 text-sm">
          <div>
            <p className="text-white/40 text-[9px] uppercase">Lat</p>
            <AnimatedNumber value={uav.lat} decimals={4} />
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Lon</p>
            <AnimatedNumber value={uav.lon} decimals={4} />
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Heading</p>
            <AnimatedNumber value={uav.heading} decimals={1} />
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Battery</p>
            <AnimatedNumber value={uav.battery} decimals={1} />
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Speed</p>
            <motion.span className="font-mono text-white text-xs">{(uav as any).speed?.toFixed(3) ?? '-'}</motion.span>
          </div>
          <div>
            <p className="text-white/40 text-[9px] uppercase">Mission</p>
            <span className="font-mono text-white text-xs">{(uav as any).missionState ?? 'IDLE'}</span>
          </div>
        </div>
      </div>
    </Card>
  );
}
