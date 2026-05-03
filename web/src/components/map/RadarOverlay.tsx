"use client";

import React, { useRef } from 'react';
import { useFrame } from '@react-three/fiber';
import * as THREE from 'three';

interface RadarOverlayProps {
  radius?: number;
  enabled?: boolean;
}

export default function RadarOverlay({ radius = 6, enabled = true }: RadarOverlayProps) {
  const meshRef = useRef<THREE.Mesh>(null);

  useFrame((_, delta) => {
    if (meshRef.current && enabled) {
      meshRef.current.rotation.y += delta * 0.5;
    }
  });

  if (!enabled) return null;

  return (
    <mesh ref={meshRef} rotation={[0, 0, 0]}>
      <ringGeometry args={[radius - 0.01, radius, 128]} />
      <meshBasicMaterial
        color="#00d4ff"
        transparent
        opacity={0.08}
        side={THREE.DoubleSide}
        depthWrite={false}
      />
    </mesh>
  );
}
