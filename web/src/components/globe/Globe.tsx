'use client';

import React, { useMemo } from 'react';
import { Canvas } from '@react-three/fiber';
import { OrbitControls, Sphere, useTexture } from '@react-three/drei';
import * as THREE from 'three';
import { UAVState, ObstacleState } from '../../useSimulationWebSocket';

const EARTH_RADIUS = 5;

// Utility function to convert Lat/Lon to 3D Cartesian coordinates
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

const Earth = () => {
  const texture = useTexture('https://raw.githubusercontent.com/mrdoob/three.js/master/examples/textures/planets/earth_atmos_2048.jpg');
  
  return (
    <Sphere args={[EARTH_RADIUS, 64, 64]}>
      <meshStandardMaterial 
        map={texture} 
        roughness={0.7} 
        metalness={0.2} 
      />
    </Sphere>
  );
};

const UavMarker = ({ uav }: { uav: UAVState | null }) => {
  if (!uav) return null;
  
  // Place the UAV slightly above the Earth's surface
  const pos = getCartesian(uav.lat, uav.lon, EARTH_RADIUS, 0.2); 
  
  return (
    <mesh position={pos}>
      <boxGeometry args={[0.15, 0.15, 0.3]} />
      <meshStandardMaterial color="#ff0000" emissive="#ff0000" emissiveIntensity={0.5} />
    </mesh>
  );
};

const ObstacleMarkers = ({ obstacles }: { obstacles: ObstacleState[] }) => {
  return (
    <>
      {obstacles.map((obs) => {
        const pos = getCartesian(obs.lat, obs.lon, EARTH_RADIUS);
        return (
          <mesh key={obs.id} position={pos}>
            <sphereGeometry args={[0.08, 16, 16]} />
            <meshStandardMaterial color={obs.dynamic ? "#ff8800" : "#888888"} emissive={obs.dynamic ? "#ff8800" : "#000000"} />
          </mesh>
        );
      })}
    </>
  );
};

interface GlobeProps {
  uavState: UAVState | null;
  obstacles: ObstacleState[];
}

export default function Globe({ uavState, obstacles }: GlobeProps) {
  const lines = useMemo(() => {
    if (!uavState) return [];
    
    const startPos = getCartesian(uavState.startLat, uavState.startLon, EARTH_RADIUS);
    const currentPos = getCartesian(uavState.lat, uavState.lon, EARTH_RADIUS, 0.2);
    const targetPos = getCartesian(uavState.targetLat, uavState.targetLon, EARTH_RADIUS);

    return [
      {
        key: 'start-to-drone',
        geometry: new THREE.BufferGeometry().setFromPoints([startPos, currentPos]),
        material: new THREE.LineBasicMaterial({ color: 0xffffff, transparent: true, opacity: 0.3 })
      },
      {
        key: 'drone-to-target',
        geometry: new THREE.BufferGeometry().setFromPoints([currentPos, targetPos]),
        material: new THREE.LineBasicMaterial({ color: 0x00ff00 })
      }
    ];
  }, [uavState]);

  return (
    <Canvas camera={{ position: [0, 0, 12], fov: 50 }}>
      <ambientLight intensity={0.8} />
      <pointLight position={[10, 10, 10]} intensity={1.5} />
      <Earth />
      <UavMarker uav={uavState} />
      <ObstacleMarkers obstacles={obstacles} />
      {lines.map((line) => (
        <line key={line.key} geometry={line.geometry} material={line.material} />
      ))}
      <OrbitControls enablePan={false} minDistance={6} maxDistance={20} />
    </Canvas>
  );
}
