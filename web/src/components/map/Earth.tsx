"use client";

import React, { useRef } from 'react';
import { useTexture } from '@react-three/drei';
import * as THREE from 'three';
import { useSimulationStore } from '../../store/useSimulationStore';

const EARTH_RADIUS = 5;
const ATMOSPHERE_COLOR = '#4488ff';

export default function Earth({ onClick }: { onClick?: (lat: number, lon: number) => void }) {
  const setSelectedEntity = useSimulationStore((s) => s.setSelectedEntity);
  const texture = useTexture(
    'https://raw.githubusercontent.com/mrdoob/three.js/master/examples/textures/planets/earth_atmos_2048.jpg'
  );
  const atmosphereRef = useRef<THREE.Mesh>(null);

  const handleClick = (e: any) => {
    e.stopPropagation();
    const pt = e.point.clone().normalize();
    const phi = Math.acos(pt.y);
    const theta = Math.atan2(pt.z, -pt.x);
    const lat = 90 - (phi * 180) / Math.PI;
    let lon = (theta * 180) / Math.PI - 180;
    while (lon < -180) lon += 360;
    while (lon > 180) lon -= 360;
    setSelectedEntity({ type: 'none' });
    if (onClick) onClick(lat, lon);
  };

  return (
    <group>
      <mesh onClick={handleClick}>
        <sphereGeometry args={[EARTH_RADIUS, 64, 64]} />
        <meshStandardMaterial map={texture} roughness={0.4} metalness={0.1} emissive="#112244" emissiveIntensity={0.5} />
      </mesh>
      <mesh ref={atmosphereRef} scale={[1.08, 1.08, 1.08]}>
        <sphereGeometry args={[EARTH_RADIUS, 64, 64]} />
        <meshPhongMaterial
          color={ATMOSPHERE_COLOR}
          transparent
          opacity={0.15}
          side={THREE.BackSide}
          blending={THREE.AdditiveBlending}
        />
      </mesh>
    </group>
  );
}
