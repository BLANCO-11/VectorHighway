"use client";

import React from 'react';
import { useTexture } from '@react-three/drei';
import * as THREE from 'three';

const EARTH_RADIUS = 5;

export default function Earth({ onClick }: { onClick?: (lat: number, lon: number) => void }) {
  const texture = useTexture(
    'https://raw.githubusercontent.com/mrdoob/three.js/master/examples/textures/planets/earth_atmos_2048.jpg'
  );

  const handleClick = (e: any) => {
    e.stopPropagation();
    if (!onClick) return;
    const pt = e.point.clone().normalize();
    const phi = Math.acos(pt.y);
    const theta = Math.atan2(pt.z, -pt.x);
    const lat = 90 - (phi * 180) / Math.PI;
    let lon = (theta * 180) / Math.PI - 180;
    while (lon < -180) lon += 360;
    while (lon > 180) lon -= 360;
    onClick(lat, lon);
  };

  return (
    <group>
      <mesh onClick={handleClick}>
        <sphereGeometry args={[EARTH_RADIUS, 64, 64]} />
        <meshStandardMaterial map={texture} roughness={0.4} metalness={0.1} emissive="#112244" emissiveIntensity={0.5} />
      </mesh>
      <mesh scale={[1.05, 1.05, 1.05]}>
        <sphereGeometry args={[EARTH_RADIUS, 64, 64]} />
        <meshPhongMaterial color="#4488ff" transparent opacity={0.3} side={THREE.BackSide} />
      </mesh>
    </group>
  );
}