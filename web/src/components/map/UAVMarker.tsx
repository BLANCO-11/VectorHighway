"use client";

import React, { useRef, useMemo } from 'react';
import { useFrame } from '@react-three/fiber';
import { Html } from '@react-three/drei';
import * as THREE from 'three';
import { Navigation } from 'lucide-react';
import { useSimulationStore } from '../../store/useSimulationStore';
import { getCartesian } from '../../lib/geo';
import { GROUP_COLORS, EARTH_RADIUS } from '../../lib/constants';
import ProgressBar from '../ui/ProgressBar';

const getColorForGroup = (group?: string): string => {
  if (!group) return GROUP_COLORS.default;
  if (GROUP_COLORS[group]) return GROUP_COLORS[group];
  let hash = 0;
  for (let i = 0; i < group.length; i++) hash = group.charCodeAt(i) + ((hash << 5) - hash);
  const c = (hash & 0x00ffffff).toString(16).toUpperCase();
  return '#' + '00000'.substring(0, 6 - c.length) + c;
};

interface UAVMarkerProps {
  id: string;
}

export default function UAVMarker({ id }: UAVMarkerProps) {
  const groupRef = useRef<THREE.Group>(null);
  const targetPosRef = useRef(new THREE.Vector3());

  const uav = useSimulationStore((s) => s.uavs[id]);
  const selectEntity = useSimulationStore((s) => s.setSelectedEntity);
  const selectedEntity = useSimulationStore((s) => s.selectedEntity);

  const isSelected = selectedEntity.type === 'uav' && selectedEntity.id === id;

  const color = getColorForGroup(uav?.groupId);

  targetPosRef.current = getCartesian(
    uav?.lat ?? 0,
    uav?.lon ?? 0,
    EARTH_RADIUS,
    ((uav?.alt ?? 0) / 6371) + 0.1
  );

  useFrame((_, delta) => {
    if (groupRef.current) {
      groupRef.current.position.lerp(targetPosRef.current, delta * 15);
    }
  });

  if (!uav) return null;

  const headingDeg = uav.heading ?? 0;

  return (
    <group
      ref={groupRef}
      onClick={(e) => {
        e.stopPropagation();
        selectEntity({ type: 'uav', id: uav.id, lat: uav.lat, lon: uav.lon });
      }}
    >
      {isSelected && (
        <mesh>
          <ringGeometry args={[0.4, 0.5, 32]} />
          <meshBasicMaterial color={color} transparent opacity={0.6} side={THREE.DoubleSide} />
        </mesh>
      )}
      <mesh visible={false}>
        <sphereGeometry args={[0.3, 16, 16]} />
        <meshBasicMaterial />
      </mesh>
      <Html center zIndexRange={[100, 0]}>
        <div
          className="pointer-events-none drop-shadow-md flex flex-col items-center gap-1"
          style={{ filter: `drop-shadow(0 0 8px ${color})` }}
        >
          <div style={{ transform: `rotate(${headingDeg}deg)` }}>
            <Navigation size={22} color={color} className="fill-current" />
          </div>
          <div className="w-12">
            <ProgressBar value={uav.battery} max={100} showPercentage={false} height={3} color={color} />
          </div>
          <span className="text-[8px] font-mono text-white/60">{uav.id}</span>
        </div>
      </Html>
    </group>
  );
}
