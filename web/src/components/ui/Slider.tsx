"use client";

import React from 'react';

interface SliderProps {
  label: string;
  value: number;
  min: number;
  max: number;
  step: number;
  formatValue?: (v: number) => string;
  accentColor?: string;
  onChange: (val: number) => void;
}

export default function Slider({
  label,
  value,
  min,
  max,
  step,
  formatValue,
  accentColor = 'accent-blue-500',
  onChange,
}: SliderProps) {
  return (
    <div className="space-y-2">
      <div className="flex justify-between text-[11px] text-white/60 uppercase">
        <span>{label}</span>
        <span>{formatValue ? formatValue(value) : value.toFixed(2)}</span>
      </div>
      <input
        type="range"
        min={min}
        max={max}
        step={step}
        value={value}
        onChange={(e) => onChange(parseFloat(e.target.value))}
        className={`w-full h-1 bg-white/10 rounded-lg appearance-none cursor-pointer ${accentColor}`}
      />
    </div>
  );
}