"use client";

import React, { useEffect, useState, useRef } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls, Stars } from '@react-three/drei';
import * as THREE from 'three';
import { useSimulationWebSocket, ObstacleState, UAVState } from '../useSimulationWebSocket';
import { useTexture } from '@react-three/drei';

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

interface ControlParams {
  speed: number;
  batteryDrain: number;
  turnRate: number;
  chargingRate: number;
}

// --- 3D Components ---

function Earth() {
  const texture = useTexture('https://raw.githubusercontent.com/mrdoob/three.js/master/examples/textures/planets/earth_atmos_2048.jpg');
  
  return (
    <mesh>
      <sphereGeometry args={[EARTH_RADIUS, 64, 64]} />
      <meshStandardMaterial 
        map={texture} 
        roughness={0.7}
        metalness={0.2}
      />
    </mesh>
  );
}

function UAVModel({ heading }: { heading: number }) {
  const group = useRef<THREE.Group>(null);
  
  useFrame(() => {
    if (group.current) {
      group.current.rotation.y = heading * (Math.PI / 180);
    }
  });

  return (
    <group ref={group}>
      <mesh>
        <cylinderGeometry args={[0.05, 0.07, 0.2, 8]} />
        <meshStandardMaterial color="#ffffff" metalness={0.8} roughness={0.2} />
      </mesh>
      <mesh rotation={[Math.PI / 2, 0, 0]}>
        <boxGeometry args={[0.4, 0.02, 0.02]} />
        <meshStandardMaterial color="#333" />
      </mesh>
      <mesh rotation={[Math.PI / 2, 0, Math.PI / 2]}>
        <boxGeometry args={[0.4, 0.02, 0.02]} />
        <meshStandardMaterial color="#333" />
      </mesh>
      <mesh position={[0, 0.12, 0]}>
        <sphereGeometry args={[0.03, 16, 16]} />
        <meshBasicMaterial color="#00ff00" />
      </mesh>
    </group>
  );
}

function UAVMarker({ state }: { state: UAVState | null }) {
  const meshRef = useRef<THREE.Group>(null);

  useFrame(() => {
    if (meshRef.current && state) {
      const pos = getCartesian(state.lat, state.lon, EARTH_RADIUS, state.alt / 1000);
      meshRef.current.position.copy(pos);
    }
  });

  if (!state) return null;

  return (
    <group ref={meshRef}>
      <UAVModel heading={state.heading || 0} />
    </group>
  );
}

function ConnectionLines({ uavState }: { uavState: UAVState | null }) {
  if (!uavState) return null;

  const currentPos = getCartesian(uavState.lat, uavState.lon, EARTH_RADIUS, uavState.alt / 1000);
  const targetPos = getCartesian(uavState.targetLat, uavState.targetLon, EARTH_RADIUS);
  const startPos = getCartesian(uavState.startLat, uavState.startLon, EARTH_RADIUS);

  return (
    <group>
      {/* Passive line: Start to current position */}
      <line>
        <bufferGeometry attach="geometry" setFromPoints={[startPos, currentPos]} />
        <lineBasicMaterial attach="material" color="#4488ff" transparent opacity={0.3} />
      </line>

      {/* Active line: Current to target */}
      <line>
        <bufferGeometry attach="geometry" setFromPoints={[currentPos, targetPos]} />
        <lineBasicMaterial attach="material" color="#00ff00" />
      </line>
    </group>
  );
}

function ObstacleMarkers({ obstacles }: { obstacles: ObstacleState[] }) {
  return (
    <group>
      {obstacles.map(obs => {
        const pos = getCartesian(obs.lat, obs.lon, EARTH_RADIUS);
        return (
          <mesh key={obs.id} position={pos}>
            <sphereGeometry args={[0.08, 16, 16]} />
            <meshStandardMaterial color={obs.dynamic ? "#ff8800" : "#888888"} />
          </mesh>
        );
      })}
    </group>
  );
}

// --- Main UI Component ---

export default function SimulatorPage() {
  const { uavState, obstacles, isConnected, sendMessage } = useSimulationWebSocket('ws://localhost:8080');
  const status = isConnected ? 'Connected to Engine' : 'Awaiting signal...';
  const [params, setParams] = useState<ControlParams>({
    speed: 0.25,
    batteryDrain: 0.05,
    turnRate: 15.0,
    chargingRate: 2.0
  });

  const handleParamChange = (key: keyof ControlParams, val: number) => {
    setParams(p => ({ ...p, [key]: val }));
    sendMessage(JSON.stringify({ type: 'control_update', [key]: val }));
  };

  const handleGlobeClick = (lat: number, lon: number) => {
    sendMessage(JSON.stringify({ type: 'set_target', lat, lon }));
  };

  return (
    <main className="relative flex h-screen w-full bg-[#050505] overflow-hidden">
      <div className="absolute left-6 top-6 z-20 w-72 flex flex-col gap-4">
        <div className="p-6 rounded-2xl bg-white/5 backdrop-blur-md border border-white/10 shadow-2xl">
          <h1 className="text-xl font-light tracking-widest text-white/90 mb-1">UAV COMMAND</h1>
          <p className="text-[10px] text-blue-400 uppercase tracking-tighter mb-6">{status}</p>

          <div className="space-y-6">
            <div className="space-y-2">
              <div className="flex justify-between text-[11px] text-white/60 uppercase">
                <span>Velocity</span>
                <span>{params.speed.toFixed(2)} km/s</span>
              </div>
              <input 
                type="range" min="0" max="1000" step="0.01" 
                value={params.speed}
                onChange={(e) => handleParamChange('speed', parseFloat(e.target.value))}
                className="w-full h-1 bg-white/10 rounded-lg appearance-none cursor-pointer accent-blue-500"
              />
            </div>

            <div className="space-y-2">
              <div className="flex justify-between text-[11px] text-white/60 uppercase">
                <span>Energy Drain</span>
                <span>{params.batteryDrain.toFixed(3)}%/s</span>
              </div>
              <input 
                type="range" min="0" max="0.5" step="0.001" 
                value={params.batteryDrain}
                onChange={(e) => handleParamChange('batteryDrain', parseFloat(e.target.value))}
                className="w-full h-1 bg-white/10 rounded-lg appearance-none cursor-pointer accent-red-500"
              />
            </div>

            <div className="space-y-2">
              <div className="flex justify-between text-[11px] text-white/60 uppercase">
                <span>Agility</span>
                <span>{params.turnRate.toFixed(0)}&deg;/s</span>
              </div>
              <input 
                type="range" min="1" max="50" step="1" 
                value={params.turnRate}
                onChange={(e) => handleParamChange('turnRate', parseFloat(e.target.value))}
                className="w-full h-1 bg-white/10 rounded-lg appearance-none cursor-pointer accent-green-500"
              />
            </div>
          </div>
        </div>

        <div className="p-6 rounded-2xl bg-black/40 backdrop-blur-sm border border-white/5">
          <h2 className="text-[10px] text-white/40 uppercase mb-4 tracking-widest">Telemetry</h2>
          {uavState ? (
            <div className="grid grid-cols-2 gap-4 text-sm">
              <div>
                <p className="text-white/40 text-[9px] uppercase">Latitude</p>
                <p className="font-mono text-blue-300">{uavState.lat.toFixed(4)}</p>
              </div>
              <div>
                <p className="text-white/40 text-[9px] uppercase">Longitude</p>
                <p className="font-mono text-blue-300">{uavState.lon.toFixed(4)}</p>
              </div>
              <div className="col-span-2">
                <p className="text-white/40 text-[9px] uppercase">Battery Capacity</p>
                <div className="w-full h-1 bg-white/10 mt-1 rounded-full overflow-hidden">
                  <div 
                    className={`h-full transition-all duration-500 ${uavState.battery < 20 ? 'bg-red-500' : 'bg-green-500'}`} 
                    style={{ width: `${uavState.battery}%` }}
                  />
                </div>
              </div>
            </div>
          ) : (
            <p className="text-xs text-white/20 italic">Awaiting signal...</p>
          )}
        </div>
      </div>

      <div className="grow">
        <Canvas camera={{ position: [10, 10, 10], fov: 45 }}>
          <color attach="background" args={['#050505']} />
          <ambientLight intensity={0.4} />
          <pointLight position={[10, 10, 10]} intensity={1.5} />
          <spotLight position={[-10, 10, 10]} angle={0.15} penumbra={1} />
          <Stars radius={300} depth={60} count={20000} factor={7} saturation={0} fade speed={1} />
          <Earth />
          <ConnectionLines uavState={uavState} />
          <UAVMarker state={uavState} />
          <ObstacleMarkers obstacles={obstacles} />
          <OrbitControls enablePan={false} minDistance={6} maxDistance={20} />
        </Canvas>
      </div>
    </main>
  );
}
