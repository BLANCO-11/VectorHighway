"use client";

import React, { useMemo } from 'react';
import { Line } from '@react-three/drei';
import * as THREE from 'three';
import { useSimulationStore } from '../../store/useSimulationStore';
import { getCartesian } from '../../lib/geo';
import { EARTH_RADIUS } from '../../lib/constants';

interface PathLineProps {
  droneId: string;
  color?: string;
}

export default function PathLine({ droneId, color = '#00d4ff' }: PathLineProps) {
  const uav = useSimulationStore((s) => s.uavs[droneId]);

  const points = useMemo(() => {
    if (!uav) return [];
    const current = getCartesian(uav.lat, uav.lon, EARTH_RADIUS, 0.15);
    const target = getCartesian(uav.targetLat, uav.targetLon, EARTH_RADIUS, 0.15);

    const pts: THREE.Vector3[] = [];
    const steps = 20;
    for (let i = 0; i <= steps; i++) {
      const t = i / steps;
      const lat = uav.lat + (uav.targetLat - uav.lat) * t;
      const lon = uav.lon + (uav.targetLon - uav.lon) * t;
      pts.push(getCartesian(lat, lon, EARTH_RADIUS, 0.15));
    }
    return pts;
  }, [uav?.lat, uav?.lon, uav?.targetLat, uav?.targetLon]);

  if (!uav || points.length < 2) return null;

  return (
    <Line
      points={points}
      color={color}
      lineWidth={1}
      dashed
      dashSize={0.05}
      gapSize={0.05}
      transparent
      opacity={0.4}
    />
  );
}
