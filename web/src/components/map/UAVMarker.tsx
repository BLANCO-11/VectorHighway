"use client";

import React, { useRef, useMemo } from 'react';
import { useFrame } from '@react-three/fiber';
import { Html } from '@react-three/drei';
import * as THREE from 'three';
import { Navigation } from 'lucide-react';

const EARTH_RADIUS = 5;

const getCartesian = (lat: number, lon: number, radius: number, alt: number = 0) => {
  const phi = (90 - lat) * (Math.PI / 180);
  const theta = (lon + 180) * (Math.PI / 180);
  const r = radius + alt;
  return new THREE.Vector3(
    -(r * Math.sin(phi) * Math.cos(theta)),
    r * Math.cos(phi),
    r * Math.sin(phi) * Math.sin(theta)
  );
};

const GROUP_COLORS: Record<string, string> = {
  alpha: '#32CD32',
  bravo: '#4488ff',
  charlie: '#ff00ff',
  default: '#ffffff',
};

const getColorForGroup = (group?: string) => {
  if (!group) return GROUP_COLORS.default;
  if (GROUP_COLORS[group]) return GROUP_COLORS[group];
  let hash = 0;
  for (let i = 0; i < group.length; i++) hash = group.charCodeAt(i) + ((hash << 5) - hash);
  const c = (hash & 0x00ffffff).toString(16).toUpperCase();
  const hex = '#' + '00000'.substring(0, 6 - c.length) + c;
  GROUP_COLORS[group] = hex;
  return hex;
};

interface UAVMarkerState {
  id: string;
  lat: number;
  lon: number;
  alt: number;
  heading: number;
  groupId?: string;
}

interface SelectedEntity {
  type: 'uav' | 'obstacle' | 'destination' | 'none' | 'spawn_drone';
  id?: string;
  lat?: number;
  lon?: number;
}

export default function UAVMarker({
  state,
  onSelect,
}: {
  state: UAVMarkerState;
  onSelect: (e: SelectedEntity) => void;
}) {
  const groupRef = useRef<THREE.Group>(null);
  const color = getColorForGroup(state.groupId);

  const targetPos = useMemo(
    () => getCartesian(state.lat, state.lon, EARTH_RADIUS, state.alt / 1000 + 0.1),
    [state.lat, state.lon, state.alt]
  );

  useFrame((_, delta) => {
    if (groupRef.current) {
      groupRef.current.position.lerp(targetPos, delta * 15);
      const distance = 0.01;
      const headingRad = state.heading * (Math.PI / 180);
      const latRad = state.lat * (Math.PI / 180);
      const lonRad = state.lon * (Math.PI / 180);
      const newLatRad = Math.asin(
        Math.sin(latRad) * Math.cos(distance) +
          Math.cos(latRad) * Math.sin(distance) * Math.cos(headingRad)
      );
      const newLonRad =
        lonRad +
        Math.atan2(
          Math.sin(headingRad) * Math.sin(distance) * Math.cos(latRad),
          Math.cos(distance) - Math.sin(latRad) * Math.sin(newLatRad)
        );
      const lookAtPos = getCartesian(
        (newLatRad * 180) / Math.PI,
        (newLonRad * 180) / Math.PI,
        EARTH_RADIUS,
        state.alt / 1000 + 0.1
      );
      const matrix = new THREE.Matrix4();
      matrix.lookAt(
        groupRef.current.position,
        lookAtPos,
        groupRef.current.position.clone().normalize()
      );
      const targetQuat = new THREE.Quaternion().setFromRotationMatrix(matrix);
      groupRef.current.quaternion.slerp(targetQuat, delta * 15);
    }
  });

  return (
    <group
      ref={groupRef}
      onClick={(e) => {
        e.stopPropagation();
        onSelect({ type: 'uav', id: state.id, lat: state.lat, lon: state.lon });
      }}
    >
      <mesh visible={false}>
        <sphereGeometry args={[0.3, 16, 16]} />
        <meshBasicMaterial />
      </mesh>
      <Html center zIndexRange={[100, 0]}>
        <div
          className="pointer-events-none drop-shadow-md"
          style={{
            color,
            filter: `drop-shadow(0 0 8px ${color})`,
            transform: `rotate(${state.heading}deg)`,
          }}
        >
          <Navigation size={24} className="fill-current" />
        </div>
      </Html>
    </group>
  );
}