"use client";

import React from 'react';
import Slider from '../ui/Slider';
import Button from '../ui/Button';
import { Crosshair, PlusCircle, Navigation } from 'lucide-react';
import { useSimulationStore } from '../../store/useSimulationStore';

type ClickMode = 'target' | 'obstacle' | 'spawn_drone';

interface ControlPanelProps {
  sendMessage: (msg: string) => void;
}

export default function ControlPanel({ sendMessage }: ControlPanelProps) {
  const params = useSimulationStore((s) => s.params);
  const setParams = useSimulationStore((s) => s.setParams);
  const clickMode = useSimulationStore((s) => s.clickMode);
  const setClickMode = useSimulationStore((s) => s.setClickMode);
  const activeGroup = useSimulationStore((s) => s.activeGroup);

  const handleParamChange = (key: keyof typeof params, val: number) => {
    setParams({ [key]: val });
    sendMessage(
      JSON.stringify({ topic: `cmd/fleet/${activeGroup}/control`, payload: { [key]: val } })
    );
  };

  return (
    <div className="space-y-6">
      <Slider
        label="Velocity"
        value={params.speed}
        min={0}
        max={1000}
        step={0.01}
        formatValue={(v) => `${v.toFixed(2)} km/s`}
        accentColor="accent-blue-500"
        onChange={(v) => handleParamChange('speed', v)}
      />
      <Slider
        label="Energy Drain"
        value={params.batteryDrain}
        min={0}
        max={0.5}
        step={0.001}
        formatValue={(v) => `${v.toFixed(3)}%/s`}
        accentColor="accent-red-500"
        onChange={(v) => handleParamChange('batteryDrain', v)}
      />

      <div className="pt-4 flex gap-2 border-t border-white/10">
        <Button
          variant="primary"
          active={clickMode === 'target'}
          onClick={() => setClickMode('target')}
        >
          <Crosshair className="inline mr-2" size={14} /> Target
        </Button>
        <Button
          variant="danger"
          active={clickMode === 'obstacle'}
          onClick={() => setClickMode('obstacle')}
        >
          <PlusCircle className="inline mr-2" size={14} /> No-Fly
        </Button>
        <Button
          variant="success"
          active={clickMode === 'spawn_drone'}
          onClick={() => setClickMode('spawn_drone')}
        >
          <Navigation className="inline mr-2" size={14} /> Spawn
        </Button>
      </div>
    </div>
  );
}
