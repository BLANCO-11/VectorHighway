"use client";

import React, { useEffect, useMemo, useRef } from 'react';
import { useFrame } from '@react-three/fiber';
import * as THREE from 'three';
import { EARTH_RADIUS } from '../../lib/constants';
import { getCartesian } from '../../lib/geo';

interface Feature {
  type: string;
  geometry: {
    type: string;
    coordinates: number[][][][] | number[][][] | number[][];
  };
}

export default function GlobeBorders() {
  const countryLinesRef = useRef<THREE.LineSegments>(null);
  const stateLinesRef = useRef<THREE.LineSegments>(null);
  const cameraRef = useRef<THREE.Camera | null>(null);

  useEffect(() => {
    const loadBorders = async () => {
      try {
        const [countriesResp, statesResp] = await Promise.all([
          fetch('/data/ne_110m_admin_0_countries.geojson'),
          fetch('/data/ne_50m_admin_1_states_provinces.geojson'),
        ]);
        const countriesData = await countriesResp.json();
        const statesData = await statesResp.json();

        const extractRings = (features: Feature[]): THREE.Vector3[][] => {
          const rings: THREE.Vector3[][] = [];
          for (const feature of features) {
            if (!feature.geometry) continue;
            const { type, coordinates } = feature.geometry;
            if (type === 'Polygon') {
              for (const ring of coordinates as number[][][]) {
                rings.push(ring.map(([lon, lat]) => getCartesian(lat, lon, EARTH_RADIUS, 0.002)));
              }
            } else if (type === 'MultiPolygon') {
              for (const polygon of coordinates as number[][][][]) {
                for (const ring of polygon) {
                  rings.push(ring.map(([lon, lat]) => getCartesian(lat, lon, EARTH_RADIUS, 0.002)));
                }
              }
            }
          }
          return rings;
        };

        const countryRings = extractRings(countriesData.features);
        const stateRings = extractRings(statesData.features);

        const buildLineSegments = (rings: THREE.Vector3[][]): { positions: Float32Array; count: number } => {
          const positions: number[] = [];
          let count = 0;
          for (const ring of rings) {
            for (let i = 1; i < ring.length; i++) {
              const a = ring[i - 1];
              const b = ring[i];
              positions.push(a.x, a.y, a.z, b.x, b.y, b.z);
              count += 2;
            }
          }
          return { positions: new Float32Array(positions), count };
        };

        const countryGeo = buildLineSegments(countryRings);
        if (countryLinesRef.current) {
          const geom = new THREE.BufferGeometry();
          geom.setAttribute('position', new THREE.BufferAttribute(countryGeo.positions, 3));
          countryLinesRef.current.geometry.dispose();
          countryLinesRef.current.geometry = geom;
        }

        const stateGeo = buildLineSegments(stateRings);
        if (stateLinesRef.current) {
          const geom = new THREE.BufferGeometry();
          geom.setAttribute('position', new THREE.BufferAttribute(stateGeo.positions, 3));
          stateLinesRef.current.geometry.dispose();
          stateLinesRef.current.geometry = geom;
        }
      } catch (e) {
        console.warn('[GlobeBorders] Failed to load border data:', e);
      }
    };

    loadBorders();
  }, []);

  useFrame(({ camera }) => {
    cameraRef.current = camera;
    if (countryLinesRef.current) {
      const dist = camera.position.length();
      let countryOpacity = 0.4;
      let stateOpacity = 0.0;
      if (dist >= 18) {
        countryOpacity = 0.15;
        stateOpacity = 0.0;
      } else if (dist >= 10) {
        countryOpacity = 0.4;
        stateOpacity = 0.0;
      } else {
        countryOpacity = 0.6;
        stateOpacity = 0.3;
      }
      if (countryLinesRef.current.material) {
        (countryLinesRef.current.material as THREE.LineBasicMaterial).opacity = countryOpacity;
      }
      if (stateLinesRef.current && stateLinesRef.current.material) {
        (stateLinesRef.current.material as THREE.LineBasicMaterial).opacity = stateOpacity;
      }
    }
  });

  const countryMat = useMemo(() => new THREE.LineBasicMaterial({
    color: '#88bbff',
    transparent: true,
    opacity: 0.4,
    depthWrite: false,
  }), []);

  const stateMat = useMemo(() => new THREE.LineBasicMaterial({
    color: '#6699cc',
    transparent: true,
    opacity: 0.0,
    depthWrite: false,
  }), []);

  return (
    <>
      <lineSegments ref={countryLinesRef} material={countryMat}>
        <bufferGeometry />
      </lineSegments>
      <lineSegments ref={stateLinesRef} material={stateMat}>
        <bufferGeometry />
      </lineSegments>
    </>
  );
}