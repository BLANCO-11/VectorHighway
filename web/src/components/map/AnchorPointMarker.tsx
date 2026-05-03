"use client";

import React, { useMemo, useRef } from 'react';
import * as THREE from 'three';
import { EARTH_RADIUS } from '../../lib/constants';
import { getCartesian } from '../../lib/geo';

interface AnchorPointMarkerProps {
  points: { lat: number; lon: number }[];
  color?: string;
}

export default function AnchorPointMarker({ points, color = '#ff8800' }: AnchorPointMarkerProps) {
  if (!points || points.length === 0) return null;

  const geometry = useMemo(() => {
    const positions = new Float32Array(points.length * 3);
    points.forEach((p, i) => {
      const v = getCartesian(p.lat, p.lon, EARTH_RADIUS, 0.01);
      positions[i * 3] = v.x;
      positions[i * 3 + 1] = v.y;
      positions[i * 3 + 2] = v.z;
    });
    const geom = new THREE.BufferGeometry();
    geom.setAttribute('position', new THREE.BufferAttribute(positions, 3));
    return geom;
  }, [points]);

  return (
    <points geometry={geometry}>
      <pointsMaterial
        size={0.04}
        color={color}
        transparent
        opacity={0.8}
        sizeAttenuation
      />
    </points>
  );
}
