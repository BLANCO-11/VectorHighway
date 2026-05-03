"use client";

import React from 'react';
import { motion } from 'framer-motion';

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
        <motion.span
          key={Math.round(value * 1000)}
          initial={{ opacity: 0, y: -4 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.15 }}
        >
          {formatValue ? formatValue(value) : value.toFixed(2)}
        </motion.span>
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