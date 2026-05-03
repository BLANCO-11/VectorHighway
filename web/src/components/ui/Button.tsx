"use client";

import React from 'react';
import { motion } from 'framer-motion';

type ButtonVariant = 'primary' | 'secondary' | 'danger' | 'ghost' | 'success';

interface ButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: ButtonVariant;
  active?: boolean;
  size?: 'sm' | 'md';
}

const variantStyles: Record<ButtonVariant, string> = {
  primary: 'bg-cyan-500/20 text-cyan-400 border border-cyan-500/50 hover:bg-cyan-500/30',
  secondary: 'bg-white/5 text-white/40 border border-transparent hover:bg-white/10',
  danger: 'bg-red-500/20 text-red-400 border border-red-500/50 hover:bg-red-500/30',
  ghost: 'bg-white/10 hover:bg-white/20 border border-white/10',
  success: 'bg-lime-500/20 text-lime-400 border border-lime-500/50 hover:bg-lime-500/30',
};

const sizeStyles = {
  sm: 'py-1.5 px-2 text-[10px]',
  md: 'py-2 text-[10px]',
};

export default function Button({
  children,
  variant = 'secondary',
  active = false,
  size = 'md',
  className = '',
  ...props
}: ButtonProps) {
  const activeStyle = active ? variantStyles[variant] : '';
  const baseStyle = active ? activeStyle : variantStyles.secondary;

  return (
    <motion.button
      whileHover={{ scale: 1.02 }}
      whileTap={{ scale: 0.97 }}
      className={`${sizeStyles[size]} uppercase tracking-widest rounded transition-colors ${baseStyle} ${className}`}
      {...(props as any)}
    >
      {children}
    </motion.button>
  );
}