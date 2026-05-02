"use client";

import React from 'react';
import Slider from '../ui/Slider';
import Button from '../ui/Button';
import { Crosshair, PlusCircle, Navigation } from 'lucide-react';

interface ControlParams {
  speed: number;
  batteryDrain: number;
  turnRate: number;
  chargingRate: number;
}

type ClickMode = 'target' | 'obstacle' | 'spawn_drone';

interface ControlPanelProps {
  params: ControlParams;
  clickMode: ClickMode;
  onParamChange: (key: keyof ControlParams, val: number) => void;
  onClickModeChange: (mode: ClickMode) => void;
}

export default function ControlPanel({
  params,
  clickMode,
  onParamChange,
  onClickModeChange,
}: ControlPanelProps) {
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
        onChange={(v) => onParamChange('speed', v)}
      />
      <Slider
        label="Energy Drain"
        value={params.batteryDrain}
        min={0}
        max={0.5}
        step={0.001}
        formatValue={(v) => `${v.toFixed(3)}%/s`}
        accentColor="accent-red-500"
        onChange={(v) => onParamChange('batteryDrain', v)}
      />

      <div className="pt-4 flex gap-2 border-t border-white/10">
        <Button
          variant="primary"
          active={clickMode === 'target'}
          onClick={() => onClickModeChange('target')}
        >
          <Crosshair className="inline mr-2" size={14} /> Target
        </Button>
        <Button
          variant="danger"
          active={clickMode === 'obstacle'}
          onClick={() => onClickModeChange('obstacle')}
        >
          <PlusCircle className="inline mr-2" size={14} /> No-Fly
        </Button>
        <Button
          variant="success"
          active={clickMode === 'spawn_drone'}
          onClick={() => onClickModeChange('spawn_drone')}
        >
          <Navigation className="inline mr-2" size={14} /> Spawn
        </Button>
      </div>
    </div>
  );
}