"use client";

import React, { useMemo } from 'react';
import { Html } from '@react-three/drei';
import * as THREE from 'three';
import { MapPin } from 'lucide-react';

const EARTH_RADIUS = 5;

const getCartesian = (lat: number, lon: number, radius: number) => {
  const phi = (90 - lat) * (Math.PI / 180);
  const theta = (lon + 180) * (Math.PI / 180);
  return new THREE.Vector3(
    -(radius * Math.sin(phi) * Math.cos(theta)),
    radius * Math.cos(phi),
    radius * Math.sin(phi) * Math.sin(theta)
  );
};

interface SelectedEntity {
  type: 'uav' | 'obstacle' | 'destination' | 'none' | 'spawn_drone';
  id?: string;
  lat?: number;
  lon?: number;
}

export default function DestinationMarker({
  lat,
  lon,
  onSelect,
}: {
  lat: number;
  lon: number;
  onSelect: (e: SelectedEntity) => void;
}) {
  const pos = useMemo(() => getCartesian(lat, lon, EARTH_RADIUS), [lat, lon]);

  return (
    <group
      position={pos}
      onClick={(e) => {
        e.stopPropagation();
        onSelect({ type: 'destination', lat, lon });
      }}
    >
      <mesh visible={false}>
        <sphereGeometry args={[0.3, 16, 16]} />
        <meshBasicMaterial />
      </mesh>
      <Html center>
        <div className="text-cyan-400 drop-shadow-[0_0_8px_rgba(0,255,255,0.8)] pointer-events-none">
          <MapPin size={24} className="fill-current" />
        </div>
      </Html>
    </group>
  );
}