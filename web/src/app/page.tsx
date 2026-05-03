"use client";

import React, { useEffect, useRef } from 'react';
import { Canvas } from '@react-three/fiber';
import { OrbitControls, Stars } from '@react-three/drei';
import { useSimulationStore } from '../store/useSimulationStore';
import { useSimulationWebSocket } from '../useSimulationWebSocket';
import Earth from '../components/map/Earth';
import UAVMarker from '../components/map/UAVMarker';
import ObstacleMarker from '../components/map/ObstacleMarker';
import DestinationMarker from '../components/map/DestinationMarker';
import ConnectionLines from '../components/map/ConnectionLines';
import RadarOverlay from '../components/map/RadarOverlay';
import PathLine from '../components/map/PathLine';
import ControlPanel from '../components/panels/ControlPanel';
import SwarmPanel from '../components/panels/SwarmPanel';
import TelemetryPanel from '../components/panels/TelemetryPanel';
import BatteryChart from '../components/panels/BatteryChart';
import MissionPanel from '../components/panels/MissionPanel';
import HealthPanel from '../components/panels/HealthPanel';
import FlightLogPanel from '../components/panels/FlightLogPanel';
import EmergencyPanel from '../components/panels/EmergencyPanel';
import ChecklistPanel from '../components/panels/ChecklistPanel';
import Card from '../components/ui/Card';
import Badge from '../components/ui/Badge';
import { GROUP_COLORS, EARTH_RADIUS } from '../lib/constants';
import { getCartesian } from '../lib/geo';
import * as THREE from 'three';

const getColorForGroup = (group?: string): string => {
  if (!group) return GROUP_COLORS.default;
  if (GROUP_COLORS[group]) return GROUP_COLORS[group];
  let hash = 0;
  for (let i = 0; i < (group?.length ?? 0); i++) hash = group.charCodeAt(i) + ((hash << 5) - hash);
  const c = (hash & 0x00ffffff).toString(16).toUpperCase();
  return '#' + '00000'.substring(0, 6 - c.length) + c;
};

export default function SimulatorPage() {
  const { uavs, obstacles, isConnected, sendMessage } = useSimulationWebSocket('ws://localhost:7200');
  const prevUavsRef = useRef<string>('');

  const setUavs = useSimulationStore((s) => s.setUavs);
  const setObstacles = useSimulationStore((s) => s.setObstacles);
  const setConnectionStatus = useSimulationStore((s) => s.setConnectionStatus);
  const selectedEntity = useSimulationStore((s) => s.selectedEntity);
  const activeGroup = useSimulationStore((s) => s.activeGroup);
  const clickMode = useSimulationStore((s) => s.clickMode);

  const [droneHistory, setDroneHistory] = React.useState<Record<string, THREE.Vector3[]>>({});

  useEffect(() => {
    setConnectionStatus(isConnected);
  }, [isConnected, setConnectionStatus]);

  useEffect(() => {
    const uavRecord: Record<string, any> = {};
    uavs.forEach((uav) => {
      uavRecord[uav.id] = uav;
    });
    setUavs(uavRecord);
  }, [uavs, setUavs]);

  useEffect(() => {
    const obsRecord: Record<string, any> = {};
    obstacles.forEach((obs) => {
      obsRecord[obs.id] = obs;
    });
    setObstacles(obsRecord);
  }, [obstacles, setObstacles]);

  useEffect(() => {
    let updatedHistory = false;
    const newHistory: Record<string, THREE.Vector3[]> = { ...droneHistory };

    uavs.forEach((uav) => {
      const pos = getCartesian(uav.lat, uav.lon, EARTH_RADIUS, uav.alt / 6371);
      const hist = newHistory[uav.id] || [pos];
      const lastPos = hist[hist.length - 1];
      if (lastPos.distanceTo(pos) > 0.01) {
        newHistory[uav.id] = [...hist.slice(-100), pos];
        updatedHistory = true;
      }
    });

    if (updatedHistory) setDroneHistory(newHistory);
  }, [uavs]);

  const handleGlobeClick = (lat: number, lon: number) => {
    let msg = '';
    if (clickMode === 'target') {
      const topic =
        selectedEntity.type === 'uav' && selectedEntity.id
          ? `cmd/fleet/${selectedEntity.id}/target`
          : `cmd/fleet/${activeGroup}/target`;
      msg = JSON.stringify({ topic, payload: { lat, lon } });
    } else if (clickMode === 'obstacle') {
      msg = JSON.stringify({ topic: 'cmd/environment/obstacle', payload: { lat, lon, radius: 250.0, groupId: activeGroup } });
    } else if (clickMode === 'spawn_drone') {
      msg = JSON.stringify({ topic: 'cmd/fleet/spawn', payload: { lat, lon, groupId: activeGroup } });
    }
    if (msg) {
      console.log('[click] sending:', msg);
      sendMessage(msg);
    }
  };

  const status = isConnected ? 'Connected to Engine' : 'Awaiting signal...';

  return (
    <main className="relative flex h-screen w-full bg-[#0a0a0f] overflow-hidden text-[#e0e0e0]">
      {/* Top Status Bar */}
      <div className="absolute top-0 left-0 right-0 z-30 h-10 glass flex items-center justify-between px-4 rounded-none">
        <div className="flex items-center gap-3">
          <h1 className="text-sm font-light tracking-widest text-white/90">VECTORHIGHWAY</h1>
          <Badge status={isConnected ? 'CONNECTED' : 'DISCONNECTED'} size="sm" />
          <span className="text-[10px] text-[#888899]">{status}</span>
        </div>
        <div className="flex items-center gap-4 text-[10px] text-[#888899]">
          <span>UAVs: {uavs.length}</span>
          <span>Group: {activeGroup.toUpperCase()}</span>
        </div>
      </div>

      {/* Left Sidebar */}
      <div className="absolute left-3 top-14 bottom-3 z-20 w-80 flex flex-col gap-3 overflow-y-auto pr-1">
        <Card>
          <SwarmPanel />
          <ControlPanel sendMessage={sendMessage} />
        </Card>

        <MissionPanel sendMessage={sendMessage} />

        <TelemetryPanel />

        <HealthPanel />

        <EmergencyPanel />

        <ChecklistPanel />

        <BatteryChart />

        <FlightLogPanel sendMessage={sendMessage} />
      </div>

      {/* Globe Area */}
      <div className="grow cursor-crosshair">
        <Canvas camera={{ position: [12, 12, 12], fov: 45 }}>
          <color attach="background" args={['#0a0a0f']} />
          <ambientLight intensity={0.6} />
          <pointLight position={[10, 10, 10]} intensity={2} />
          <spotLight position={[-10, 10, 10]} angle={0.15} penumbra={1} />
          <Stars radius={300} depth={60} count={20000} factor={7} saturation={0} fade speed={1} />

          <Earth onClick={handleGlobeClick} />
          <RadarOverlay enabled={true} />

          {uavs.map((uav) => (
            <React.Fragment key={uav.id}>
              <ConnectionLines
                uavState={uav}
                history={droneHistory[uav.id] || []}
                color={getColorForGroup(uav.groupId)}
              />
              <UAVMarker id={uav.id} />
              <DestinationMarker
                lat={uav.targetLat}
                lon={uav.targetLon}
                onSelect={(e) => useSimulationStore.getState().setSelectedEntity(e)}
              />
            </React.Fragment>
          ))}

          {obstacles.map((obs) => (
            <ObstacleMarker key={obs.id} obs={obs} onSelect={(e) => useSimulationStore.getState().setSelectedEntity(e)} />
          ))}

          <OrbitControls enablePan={false} minDistance={7} maxDistance={25} />
        </Canvas>
      </div>
    </main>
  );
}
