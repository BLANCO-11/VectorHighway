"use client";

import React, { useState, useEffect } from 'react';
import { Canvas } from '@react-three/fiber';
import { OrbitControls, Stars } from '@react-three/drei';
import * as THREE from 'three';
import { useSimulationWebSocket } from '../useSimulationWebSocket';
import Earth from '../components/map/Earth';
import UAVMarker from '../components/map/UAVMarker';
import ObstacleMarker from '../components/map/ObstacleMarker';
import DestinationMarker from '../components/map/DestinationMarker';
import ConnectionLines from '../components/map/ConnectionLines';
import ControlPanel from '../components/panels/ControlPanel';
import SwarmPanel from '../components/panels/SwarmPanel';
import TelemetryPanel from '../components/panels/TelemetryPanel';
import BatteryChart from '../components/panels/BatteryChart';
import Card from '../components/ui/Card';

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

type ClickMode = 'target' | 'obstacle' | 'spawn_drone';

export default function SimulatorPage() {
  const { uavs, obstacles, isConnected, sendMessage } = useSimulationWebSocket('ws://localhost:7200');
  const [selected, setSelected] = useState<SelectedEntity>({ type: 'none' });
  const [clickMode, setClickMode] = useState<ClickMode>('target');
  const [groups, setGroups] = useState<string[]>(['alpha', 'bravo', 'charlie']);
  const [newGroup, setNewGroup] = useState('');
  const [activeGroup, setActiveGroup] = useState<string>('alpha');
  const [history, setHistory] = useState<{ time: string; battery: number }[]>([]);
  const [droneHistory, setDroneHistory] = useState<Record<string, THREE.Vector3[]>>({});
  const status = isConnected ? 'Connected to Engine' : 'Awaiting signal...';
  const [params, setParams] = useState<ControlParams>({
    speed: 0.25,
    batteryDrain: 0.05,
    turnRate: 15.0,
    chargingRate: 2.0,
  });

  useEffect(() => {
    let updatedHistory = false;
    const newDroneHistory: Record<string, THREE.Vector3[]> = { ...droneHistory };

    uavs.forEach((uav) => {
      const pos = getCartesian(uav.lat, uav.lon, EARTH_RADIUS, uav.alt / 1000);
      const hist = newDroneHistory[uav.id] || [pos];
      const lastPos = hist[hist.length - 1];
      if (lastPos.distanceTo(pos) > 0.01) {
        newDroneHistory[uav.id] = [...hist.slice(-100), pos];
        updatedHistory = true;
      }
    });

    if (updatedHistory) setDroneHistory(newDroneHistory);

    const activeDrones = uavs.filter((u) => u.groupId === activeGroup);
    if (activeDrones.length > 0) {
      const avgBattery =
        activeDrones.reduce((sum, u) => sum + u.battery, 0) / activeDrones.length;
      setHistory((prev) => {
        if (
          prev.length === 0 ||
          Math.abs(prev[prev.length - 1].battery - avgBattery) > 0.01
        ) {
          return [...prev, { time: new Date().toLocaleTimeString(), battery: avgBattery }].slice(-20);
        }
        return prev;
      });
    }
  }, [uavs, activeGroup]);

  const handleParamChange = (key: keyof ControlParams, val: number) => {
    setParams((p) => ({ ...p, [key]: val }));
    sendMessage(
      JSON.stringify({ topic: `cmd/fleet/${activeGroup}/control`, payload: { [key]: val } })
    );
  };

  const handleGlobeClick = (lat: number, lon: number) => {
    if (clickMode === 'target') {
      const topic =
        selected.type === 'uav' && selected.id
          ? `cmd/fleet/${selected.id}/target`
          : `cmd/fleet/${activeGroup}/target`;
      sendMessage(JSON.stringify({ topic, payload: { lat, lon } }));
    } else if (clickMode === 'obstacle') {
      sendMessage(
        JSON.stringify({
          topic: 'cmd/environment/obstacle',
          payload: { lat, lon, radius: 250.0, groupId: activeGroup },
        })
      );
    } else if (clickMode === 'spawn_drone') {
      sendMessage(
        JSON.stringify({ topic: 'cmd/fleet/spawn', payload: { lat, lon, groupId: activeGroup } })
      );
    }
  };

  const handleAddGroup = () => {
    const g = newGroup.trim().toLowerCase();
    if (g && !groups.includes(g)) {
      setGroups([...groups, g]);
      setActiveGroup(g);
      setNewGroup('');
    }
  };

  return (
    <main className="relative flex h-screen w-full bg-[#050505] overflow-hidden text-white">
      <div className="absolute left-6 top-6 bottom-6 z-20 w-80 flex flex-col gap-4">
        <Card>
          <h1 className="text-xl font-light tracking-widest text-white/90 mb-1">UAV COMMAND</h1>
          <p className="text-[10px] text-blue-400 uppercase tracking-tighter mb-6">{status}</p>

          <SwarmPanel
            groups={groups}
            activeGroup={activeGroup}
            newGroup={newGroup}
            onNewGroupChange={setNewGroup}
            onAddGroup={handleAddGroup}
            onSetActiveGroup={setActiveGroup}
          />

          <ControlPanel
            params={params}
            clickMode={clickMode}
            onParamChange={handleParamChange}
            onClickModeChange={setClickMode}
          />
        </Card>

        {selected.type !== 'none' && (
          <TelemetryPanel selected={selected} />
        )}

        {history.length > 0 && (
          <BatteryChart
            history={history}
            color={getColorForGroup(activeGroup)}
            groupName={activeGroup}
          />
        )}
      </div>

      <div className="grow cursor-crosshair">
        <Canvas camera={{ position: [12, 12, 12], fov: 45 }}>
          <color attach="background" args={['#050505']} />
          <ambientLight intensity={0.6} />
          <pointLight position={[10, 10, 10]} intensity={2} />
          <spotLight position={[-10, 10, 10]} angle={0.15} penumbra={1} />
          <Stars radius={300} depth={60} count={20000} factor={7} saturation={0} fade speed={1} />

          <Earth onClick={handleGlobeClick} />

          {uavs.map((uav) => (
            <React.Fragment key={uav.id}>
              <ConnectionLines
                uavState={uav}
                history={droneHistory[uav.id] || []}
                color={getColorForGroup(uav.groupId)}
              />
              <UAVMarker state={uav} onSelect={(e) => setSelected(e)} />
              <DestinationMarker
                lat={uav.targetLat}
                lon={uav.targetLon}
                onSelect={(e) => setSelected(e)}
              />
            </React.Fragment>
          ))}

          {obstacles.map((obs) => (
            <ObstacleMarker key={obs.id} obs={obs} onSelect={(e) => setSelected(e)} />
          ))}

          <OrbitControls enablePan={false} minDistance={7} maxDistance={25} />
        </Canvas>
      </div>
    </main>
  );
}