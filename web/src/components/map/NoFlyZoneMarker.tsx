"use client";

import React from 'react';
import * as THREE from 'three';

interface NoFlyZoneMarkerProps {
  center?: { lat: number; lon: number };
  radius?: number;
  active?: boolean;
  vertices?: { lat: number; lon: number }[];
  shape?: 'CIRCLE' | 'POLYGON';
}

const getCartesian = (lat: number, lon: number, radius: number) => {
  const phi = (90 - lat) * (Math.PI / 180);
  const theta = (lon + 180) * (Math.PI / 180);
  return new THREE.Vector3(
    -(radius * Math.sin(phi) * Math.cos(theta)),
    radius * Math.cos(phi),
    radius * Math.sin(phi) * Math.sin(theta)
  );
};

export default function NoFlyZoneMarker({
  center,
  radius = 5,
  active = true,
  shape = 'CIRCLE',
}: NoFlyZoneMarkerProps) {
  if (!active) return null;

  if (shape === 'CIRCLE' && center) {
    const pos = getCartesian(center.lat, center.lon, 5);

    return (
      <mesh position={pos}>
        <sphereGeometry args={[radius * 0.02, 32, 32]} />
        <meshBasicMaterial
          color="#ff3355"
          transparent
          opacity={0.15}
          side={THREE.DoubleSide}
          depthWrite={false}
        />
      </mesh>
    );
  }

  return null;
}
