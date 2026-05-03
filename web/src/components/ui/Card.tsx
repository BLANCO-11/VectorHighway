"use client";

import React from 'react';
import { motion } from 'framer-motion';

interface CardProps {
  children: React.ReactNode;
  className?: string;
  animate?: boolean;
}

export default function Card({ children, className = '', animate = true }: CardProps) {
  const baseClasses = 'p-6 rounded-2xl bg-black/40 backdrop-blur-md border border-white/10 shadow-2xl';

  if (animate) {
    return (
      <motion.div
        layout
        initial={{ opacity: 0, y: 12 }}
        animate={{ opacity: 1, y: 0 }}
        whileHover={{ scale: 1.01, boxShadow: '0 0 12px rgba(0,212,255,0.15)' }}
        transition={{ type: 'spring', stiffness: 200, damping: 20 }}
        className={`${baseClasses} ${className}`}
      >
        {children}
      </motion.div>
    );
  }

  return <div className={`${baseClasses} ${className}`}>{children}</div>;
}