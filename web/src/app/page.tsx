"use client";

import React, { useState, useRef, useMemo, useEffect } from 'react';
import { Canvas, useFrame } from '@react-three/fiber';
import { OrbitControls, Stars, useTexture, Html } from '@react-three/drei';
import * as THREE from 'three';
import { useSimulationWebSocket, ObstacleState, UAVState } from '../useSimulationWebSocket';
import { Navigation, MapPin, AlertTriangle, ShieldAlert, Crosshair, PlusCircle } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';
import { LineChart, Line, YAxis, ResponsiveContainer } from 'recharts';

const EARTH_RADIUS = 5;

// --- Utilities ---

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

/**
 * Generates points along a Great Circle arc between two lat/lon points
 */
const getGreatCirclePoints = (lat1: number, lon1: number, lat2: number, lon2: number, radius: number, alt: number = 0, segments: number = 32) => {
  const points = [];
  const start = getCartesian(lat1, lon1, radius, alt);
  const end = getCartesian(lat2, lon2, radius, alt);
  for (let i = 0; i <= segments; i++) {
    const t = i / segments;
    // Spherical linear interpolation for accurate arcs
    const pt = start.clone().lerp(end, t).normalize().multiplyScalar(radius + alt);
    points.push(pt);
  }
  return points;
};

interface ControlParams {
  speed: number;
  batteryDrain: number;
  turnRate: number;
  chargingRate: number;
}

interface SelectedEntity {
  type: 'uav' | 'obstacle' | 'destination' | 'none' | 'spawn_drone';
  id?: string;
  lat?: number;
  lon?: number;
}

// --- 3D Components ---

function Earth({ onClick }: { onClick?: (lat: number, lon: number) => void }) {
  const texture = useTexture('https://raw.githubusercontent.com/mrdoob/three.js/master/examples/textures/planets/earth_atmos_2048.jpg');

  const handleClick = (e: any) => {
    e.stopPropagation();
    if (!onClick) return;
    const pt = e.point.clone().normalize();
    const phi = Math.acos(pt.y);
    const theta = Math.atan2(pt.z, -pt.x);
    
    const lat = 90 - (phi * 180 / Math.PI);
    let lon = (theta * 180 / Math.PI) - 180;
    while (lon < -180) lon += 360;
    while (lon > 180) lon -= 360;
    
    onClick(lat, lon);
  };
  
  return (
    <group>
      <mesh onClick={handleClick}>
        <sphereGeometry args={[EARTH_RADIUS, 64, 64]} />
        <meshStandardMaterial 
          map={texture} 
          roughness={0.4}
          metalness={0.1}
          emissive="#112244"
          emissiveIntensity={0.5}
        />
      </mesh>
      {/* Atmosphere Glow */}
      <mesh scale={[1.05, 1.05, 1.05]}>
        <sphereGeometry args={[EARTH_RADIUS, 64, 64]} />
        <meshPhongMaterial 
          color="#4488ff" 
          transparent 
          opacity={0.3} 
          side={THREE.BackSide} 
        />
      </mesh>
    </group>
  );
}

function UAVMarker({ state, onSelect }: { state: UAVState, onSelect: (e: SelectedEntity) => void }) {
  const groupRef = useRef<THREE.Group>(null);

  const targetPos = useMemo(() => {
    if (!state) return new THREE.Vector3();
    return getCartesian(state.lat, state.lon, EARTH_RADIUS, state.alt / 1000 + 0.1);
  }, [state?.lat, state?.lon, state?.alt]);

  useFrame((_, delta) => {
    if (groupRef.current && state) {
      groupRef.current.position.lerp(targetPos, delta * 15);
      
      const distance = 0.01;
      const headingRad = state.heading * Math.PI / 180;
      const latRad = state.lat * Math.PI / 180;
      const lonRad = state.lon * Math.PI / 180;
      
      const newLatRad = Math.asin(Math.sin(latRad) * Math.cos(distance) + Math.cos(latRad) * Math.sin(distance) * Math.cos(headingRad));
      const newLonRad = lonRad + Math.atan2(Math.sin(headingRad) * Math.sin(distance) * Math.cos(latRad), Math.cos(distance) - Math.sin(latRad) * Math.sin(newLatRad));
      
      const lookAtPos = getCartesian(newLatRad * 180 / Math.PI, newLonRad * 180 / Math.PI, EARTH_RADIUS, state.alt / 1000 + 0.1);
      
      const matrix = new THREE.Matrix4();
      matrix.lookAt(groupRef.current.position, lookAtPos, groupRef.current.position.clone().normalize());
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
      <mesh rotation={[-Math.PI / 2, 0, 0]}>
        <coneGeometry args={[0.12, 0.4, 8]} />
        <meshStandardMaterial color="#32CD32" emissive="#32CD32" emissiveIntensity={0.3} />
      </mesh>
      <mesh rotation={[-Math.PI / 2, 0, 0]}>
        <circleGeometry args={[1.5, 32]} />
        <meshBasicMaterial color="#32CD32" transparent opacity={0.05} side={THREE.DoubleSide} />
      </mesh>
    </group>
  );
}

function ObstacleMarker({ obs, onSelect }: { obs: ObstacleState, onSelect: (e: SelectedEntity) => void }) {
  const pos = useMemo(() => getCartesian(obs.lat, obs.lon, EARTH_RADIUS), [obs]);
  
  return (
    <group 
      position={pos} 
      onClick={(e) => {
        e.stopPropagation();
        onSelect({ type: 'obstacle', id: obs.id, lat: obs.lat, lon: obs.lon });
      }}
    >
      <mesh visible={false}>
        <sphereGeometry args={[0.3, 16, 16]} />
        <meshBasicMaterial />
      </mesh>
      <Html center>
        <div className={`pointer-events-none drop-shadow-[0_0_8px_rgba(255,136,0,0.8)] ${obs.dynamic ? 'text-orange-500' : 'text-red-500'}`}>
          {obs.dynamic ? <AlertTriangle size={20} className="fill-current" /> : <ShieldAlert size={20} className="fill-current" />}
        </div>
      </Html>
    </group>
  );
}

function DestinationMarker({ lat, lon, onSelect }: { lat: number, lon: number, onSelect: (e: SelectedEntity) => void }) {
  const pos = useMemo(() => getCartesian(lat, lon, EARTH_RADIUS), [lat, lon]);

  return (
    <group 
      position={pos}
      onClick={(e) => {
        e.stopPropagation();
        onSelect({ type: 'destination', lat, lon });
      }}
    >
      <mesh visible={false}>
        <sphereGeometry args={[0.3, 16, 16]} />
        <meshBasicMaterial />
      </mesh>
      <Html center>
        <div className="text-cyan-400 drop-shadow-[0_0_8px_rgba(0,255,255,0.8)] pointer-events-none">
          <MapPin size={24} className="fill-current" />
        </div>
      </Html>
    </group>
  );
}

function ConnectionLines({ uavState, history }: { uavState: UAVState, history: THREE.Vector3[] }) {
  const lines = useMemo(() => {
    
    const activePoints = getGreatCirclePoints(uavState.lat, uavState.lon, uavState.targetLat, uavState.targetLon, EARTH_RADIUS, uavState.alt / 1000);

    return (
      <group>
        {history && history.length > 1 && (
          <line>
            <bufferGeometry attach="geometry" setFromPoints={history} />
            <lineBasicMaterial attach="material" color="#4488ff" transparent opacity={0.4} linewidth={2} />
          </line>
        )}
        <line>
          <bufferGeometry attach="geometry" setFromPoints={activePoints} />
          <lineBasicMaterial attach="material" color="#00ff00" transparent opacity={0.8} linewidth={2} />
        </line>
      </group>
    );
  }, [uavState, history]);

  return lines;
}

// --- Main UI Component ---

export default function SimulatorPage() {
  const { uavs, obstacles, isConnected, sendMessage } = useSimulationWebSocket('ws://localhost:8080');
  const [selected, setSelected] = useState<SelectedEntity>({ type: 'none' });
  const [clickMode, setClickMode] = useState<'target' | 'obstacle' | 'spawn_drone'>('target');
  const [history, setHistory] = useState<{time: string, battery: number}[]>([]);
  const [droneHistory, setDroneHistory] = useState<Record<string, THREE.Vector3[]>>({});
  const status = isConnected ? 'Connected to Engine' : 'Awaiting signal...';
  const [params, setParams] = useState<ControlParams>({
    speed: 0.25,
    batteryDrain: 0.05,
    turnRate: 15.0,
    chargingRate: 2.0
  });

  useEffect(() => {
    let updatedHistory = false;
    let newDroneHistory = { ...droneHistory };
    
    uavs.forEach(uav => {
      const pos = getCartesian(uav.lat, uav.lon, EARTH_RADIUS, uav.alt / 1000);
      const hist = newDroneHistory[uav.id] || [pos];
      const lastPos = hist[hist.length - 1];
      if (lastPos.distanceTo(pos) > 0.05) {
        newDroneHistory[uav.id] = [...hist, pos];
        updatedHistory = true;
      }
    });
    
    if (updatedHistory) setDroneHistory(newDroneHistory);

    const alpha = uavs.find(u => u.id === 'alpha');
    if (alpha) {
      setHistory(prev => {
        if (prev.length === 0 || prev[prev.length - 1].battery !== alpha.battery) {
           return [...prev, { time: new Date().toLocaleTimeString(), battery: alpha.battery }].slice(-20);
        }
        return prev;
      });
    }
  }, [uavs]);

  const handleParamChange = (key: keyof ControlParams, val: number) => {
    setParams(p => ({ ...p, [key]: val }));
    sendMessage(JSON.stringify({ type: 'control_update', [key]: val }));
  };

  const handleGlobeClick = (lat: number, lon: number) => {
    if (clickMode === 'target') {
      sendMessage(JSON.stringify({ type: 'set_target', lat, lon }));
    } else if (clickMode === 'obstacle') {
      sendMessage(JSON.stringify({ type: 'add_obstacle', lat, lon, radius: 2.0 }));
    } else if (clickMode === 'spawn_drone') {
      sendMessage(JSON.stringify({ type: 'spawn_drone', lat, lon }));
    }
  };

  return (
    <main className="relative flex h-screen w-full bg-[#050505] overflow-hidden text-white">
      <div className="absolute left-6 top-6 bottom-6 z-20 w-80 flex flex-col gap-4">
        <div className="p-6 rounded-2xl bg-black/40 backdrop-blur-md border border-white/10 shadow-2xl">
          <h1 className="text-xl font-light tracking-widest text-white/90 mb-1">UAV COMMAND</h1>
          <p className="text-[10px] text-blue-400 uppercase tracking-tighter mb-6">{status}</p>

          <div className="space-y-6">
            <div className="space-y-2">
              <div className="flex justify-between text-[11px] text-white/60 uppercase">
                <span>Velocity</span>
                <span>{params.speed.toFixed(2)} km/s</span >
              </div >
              <input 
                type="range" min="0" max="1000" step="0.01" 
                value={params.speed}
                onChange={(e) => handleParamChange('speed', parseFloat(e.target.value))}
                className="w-full h-1 bg-white/10 rounded-lg appearance-none cursor-pointer accent-blue-500"
              />
            </div >

            <div className="space-y-2">
              <div className="flex justify-between text-[11px] text-white/60 uppercase">
                <span>Energy Drain</span>
                <span>{params.batteryDrain.toFixed(3)}%/s</span >
              </div >
              <input 
                type="range" min="0" max="0.5" step="0.001" 
                value={params.batteryDrain}
                onChange={(e) => handleParamChange('batteryDrain', parseFloat(e.target.value))}
                className="w-full h-1 bg-white/10 rounded-lg appearance-none cursor-pointer accent-red-500"
              />
            </div >
            
            <div className="pt-4 flex gap-2 border-t border-white/10">
              <button 
                onClick={() => setClickMode('target')}
                className={`flex-1 py-2 text-[10px] uppercase tracking-widest rounded transition-colors ${clickMode === 'target' ? 'bg-cyan-500/20 text-cyan-400 border border-cyan-500/50' : 'bg-white/5 text-white/40 border border-transparent hover:bg-white/10'}`}
              >
                <Crosshair className="inline mr-2" size={14}/> Target
              </button>
              <button 
                onClick={() => setClickMode('obstacle')}
                className={`flex-1 py-2 text-[10px] uppercase tracking-widest rounded transition-colors ${clickMode === 'obstacle' ? 'bg-red-500/20 text-red-400 border border-red-500/50' : 'bg-white/5 text-white/40 border border-transparent hover:bg-white/10'}`}
              >
                <PlusCircle className="inline mr-2" size={14}/> No-Fly
              </button>
              <button 
                onClick={() => setClickMode('spawn_drone')}
                className={`flex-1 py-2 text-[10px] uppercase tracking-widest rounded transition-colors ${clickMode === 'spawn_drone' ? 'bg-lime-500/20 text-lime-400 border border-lime-500/50' : 'bg-white/5 text-white/40 border border-transparent hover:bg-white/10'}`}
              >
                <Navigation className="inline mr-2" size={14}/> Spawn
              </button>
            </div>
          </div >
        </div >

        <AnimatePresence>
          {selected.type !== 'none' && (
            <motion.div 
              initial={{ opacity: 0, x: -20 }}
              animate={{ opacity: 1, x: 0 }}
              exit={{ opacity: 0, x: -20 }}
              className="p-6 rounded-2xl bg-black/40 backdrop-blur-md border border-white/10 shadow-2xl"
            >
              <h2 className="text-[10px] text-white/40 uppercase mb-4 tracking-widest">Telemetry</h2>
              <div className="space-y-3">
              <div className="flex justify-between items-center">
                <span className="text-[10px] text-white/40 uppercase">Entity</span>
                <span className="text-xs font-mono text-blue-400 uppercase">{selected.type}</span>
              </div >
              <div className="grid grid-cols-2 gap-2 text-sm">
                <div>
                  <p className="text-white/40 text-[9px] uppercase">Lat</p>
                  <p className="font-mono text-white">{selected.lat?.toFixed(4)}</p>
                </div >
                <div>
                  <p className="text-white/40 text-[9px] uppercase">Lon</p>
                  <p className="font-mono text-white">{selected.lon?.toFixed(4)}</p>
                </div >
              </div >
              {selected.id && (
                <div>
                  <p className="text-white/40 text-[9px] uppercase">ID</p>
                  <p className="font-mono text-xs text-white/80">{selected.id}</p>
                </div >
              )}
            </div >
            </motion.div>
          )}
        </AnimatePresence>
        
        {history.length > 0 && (
          <div className="p-4 rounded-2xl bg-black/40 backdrop-blur-md border border-white/10 shadow-2xl flex-1 flex flex-col min-h-0">
            <h2 className="text-[10px] text-white/40 uppercase mb-2 tracking-widest">Battery History</h2>
            <div className="flex-1 min-h-0">
              <ResponsiveContainer width="100%" height="100%">
                <LineChart data={history}>
                  <YAxis domain={[0, 100]} hide />
                  <Line type="monotone" dataKey="battery" stroke="#32CD32" strokeWidth={2} dot={false} isAnimationActive={false} />
                </LineChart>
              </ResponsiveContainer>
            </div>
          </div>
        )}
      </div >

      <div className="grow cursor-crosshair">
        <Canvas camera={{ position: [12, 12, 12], fov: 45 }}>
          <color attach="background" args={['#050505']} />
          <ambientLight intensity={0.6} />
          <pointLight position={[10, 10, 10]} intensity={2} />
          <spotLight position={[-10, 10, 10]} angle={0.15} penumbra={1} />
          <Stars radius={300} depth={60} count={20000} factor={7} saturation={0} fade speed={1} />
          
          <Earth onClick={handleGlobeClick} />
          
          {uavs.map(uav => (
            <React.Fragment key={uav.id}>
              <ConnectionLines uavState={uav} history={droneHistory[uav.id] || []} />
              <UAVMarker state={uav} onSelect={(e) => setSelected(e)} />
              <DestinationMarker lat={uav.targetLat} lon={uav.targetLon} onSelect={(e) => setSelected(e)} />
            </React.Fragment>
          ))}

          {obstacles.map(obs => (
            <ObstacleMarker 
              key={obs.id} 
              obs={obs} 
              onSelect={(e) => setSelected(e)} 
            />
          ))}


          <OrbitControls enablePan={false} minDistance={7} maxDistance={25} />
        </Canvas>
      </div >
    </main>
  );
}
