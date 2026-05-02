"use client";

import React, { useMemo } from 'react';
import * as THREE from 'three';

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

const getGreatCirclePoints = (
  lat1: number,
  lon1: number,
  lat2: number,
  lon2: number,
  radius: number,
  alt: number = 0,
  segments: number = 32
) => {
  const points: THREE.Vector3[] = [];
  const start = getCartesian(lat1, lon1, radius, alt);
  const end = getCartesian(lat2, lon2, radius, alt);
  for (let i = 0; i <= segments; i++) {
    const t = i / segments;
    const pt = start.clone().lerp(end, t).normalize().multiplyScalar(radius + alt);
    points.push(pt);
  }
  return points;
};

interface UAVState {
  id: string;
  lat: number;
  lon: number;
  alt: number;
  heading: number;
  targetLat: number;
  targetLon: number;
  groupId?: string;
}

export default function ConnectionLines({
  uavState,
  history,
  color,
}: {
  uavState: UAVState;
  history: THREE.Vector3[];
  color: string;
}) {
  const lines = useMemo(() => {
    const activePoints = getGreatCirclePoints(
      uavState.lat,
      uavState.lon,
      uavState.targetLat,
      uavState.targetLon,
      EARTH_RADIUS,
      uavState.alt / 1000
    );

    return (
      <group>
        {history.map((pt, i, arr) => {
          if (i === 0) return null;
          const opacity = Math.max(0, i / arr.length);
          if (opacity <= 0) return null;
          return (
            <line key={`hist-${i}`}>
              <bufferGeometry
                attach="geometry"
                setFromPoints={[arr[i - 1], pt]}
              />
              <lineBasicMaterial
                attach="material"
                color={color}
                transparent
                opacity={opacity * 0.8}
                linewidth={2}
              />
            </line>
          );
        })}
        <line>
          <bufferGeometry attach="geometry" setFromPoints={activePoints} />
          <lineBasicMaterial
            attach="material"
            color={color}
            transparent
            opacity={0.4}
            linewidth={1}
          />
        </line>
      </group>
    );
  }, [uavState, history, color]);

  return lines;
}