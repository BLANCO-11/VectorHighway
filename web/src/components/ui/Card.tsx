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
        initial={{ opacity: 0, x: -20 }}
        animate={{ opacity: 1, x: 0 }}
        className={`${baseClasses} ${className}`}
      >
        {children}
      </motion.div>
    );
  }

  return <div className={`${baseClasses} ${className}`}>{children}</div>;
}