"use client";

import React, { useMemo } from 'react';
import { Html } from '@react-three/drei';
import * as THREE from 'three';
import { AlertTriangle, ShieldAlert } from 'lucide-react';

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

const GROUP_COLORS: Record<string, string> = {
  alpha: '#32CD32',
  bravo: '#4488ff',
  charlie: '#ff00ff',
  default: '#ffffff',
};

const getColorForGroup = (group?: string) => {
  if (!group) return '#ff0000';
  if (GROUP_COLORS[group]) return GROUP_COLORS[group];
  let hash = 0;
  for (let i = 0; i < group.length; i++) hash = group.charCodeAt(i) + ((hash << 5) - hash);
  const c = (hash & 0x00ffffff).toString(16).toUpperCase();
  return '#' + '00000'.substring(0, 6 - c.length) + c;
};

interface ObstacleMarkerState {
  id: string;
  lat: number;
  lon: number;
  radius: number;
  dynamic?: boolean;
  groupId?: string;
}

interface SelectedEntity {
  type: 'uav' | 'obstacle' | 'destination' | 'none' | 'spawn_drone';
  id?: string;
  lat?: number;
  lon?: number;
}

export default function ObstacleMarker({
  obs,
  onSelect,
  onContext,
}: {
  obs: ObstacleMarkerState;
  onSelect: (e: SelectedEntity) => void;
  onContext?: (e: any) => void;
}) {
  const pos = useMemo(() => getCartesian(obs.lat, obs.lon, EARTH_RADIUS), [obs.lat, obs.lon]);
  const color = obs.groupId ? getColorForGroup(obs.groupId) : '#ff0000';
  const visualRadius = (obs.radius / 6371) * EARTH_RADIUS;

  const quaternion = useMemo(() => {
    const up = new THREE.Vector3(0, 1, 0);
    const normal = pos.clone().normalize();
    return new THREE.Quaternion().setFromUnitVectors(up, normal);
  }, [pos]);

  return (
    <group
      position={pos}
      onClick={(e) => {
        e.stopPropagation();
        onSelect({ type: 'obstacle', id: obs.id, lat: obs.lat, lon: obs.lon });
      }}
      onContextMenu={(e) => {
        e.stopPropagation();
        onContext?.(e);
      }}
    >
      <mesh visible={false}>
        <sphereGeometry args={[0.3, 16, 16]} />
        <meshBasicMaterial />
      </mesh>
      <mesh quaternion={quaternion}>
        <cylinderGeometry args={[visualRadius, visualRadius, 0.02, 64]} />
        <meshStandardMaterial
          color={color}
          transparent
          opacity={0.2}
          emissive={color}
          emissiveIntensity={0.3}
        />
      </mesh>
      <Html center>
        <div
          className="pointer-events-none text-white opacity-80"
          style={{ filter: `drop-shadow(0 0 8px ${color})` }}
        >
          {obs.dynamic ? (
            <AlertTriangle size={20} className="fill-current" />
          ) : (
            <ShieldAlert size={20} className="fill-current" />
          )}
        </div>
      </Html>
    </group>
  );
}