"use client";

import React from 'react';
import { motion } from 'framer-motion';

interface ToggleProps {
  enabled: boolean;
  onChange: (enabled: boolean) => void;
  label?: string;
  accentColor?: string;
}

export default function Toggle({ enabled, onChange, label, accentColor = '#00d4ff' }: ToggleProps) {
  return (
    <label className="flex items-center gap-2 cursor-pointer select-none">
      <button
        type="button"
        role="switch"
        aria-checked={enabled}
        onClick={() => onChange(!enabled)}
        className="relative w-10 h-5 rounded-full transition-colors duration-200"
        style={{
          backgroundColor: enabled ? accentColor : 'rgba(255,255,255,0.15)',
        }}
      >
        <motion.span
          className="absolute top-0.5 left-0.5 w-4 h-4 rounded-full bg-white shadow-sm"
          animate={{ x: enabled ? 20 : 0 }}
          transition={{ type: 'spring', stiffness: 300, damping: 25 }}
        />
      </button>
      {label && <span className="text-xs text-[#888899]">{label}</span>}
    </label>
  );
}
