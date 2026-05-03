"use client";

import React, { useEffect, useRef, useCallback, useState } from 'react';
import { Canvas } from '@react-three/fiber';
import { OrbitControls } from '@react-three/drei';
import { AnimatePresence, motion } from 'framer-motion';
import { useSimulationStore } from '../store/useSimulationStore';
import { useSimulationWebSocket } from '../useSimulationWebSocket';
import GlobeScene from '../components/map/GlobeScene';
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
import Button from '../components/ui/Button';
import Slider from '../components/ui/Slider';
import { GROUP_COLORS, EARTH_RADIUS } from '../lib/constants';
import { getCartesian } from '../lib/geo';
import { FastForward, Trash2, AlertTriangle, X } from 'lucide-react';
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
  const store = useSimulationStore;

  const [droneHistory, setDroneHistory] = useState<Record<string, THREE.Vector3[]>>({});
  const [contextMenu, setContextMenu] = useState<{ id: string; type: string } | null>(null);
  const [contextPosition, setContextPosition] = useState<{ x: number; y: number }>({ x: 0, y: 0 });
  const historyRef = useRef<Record<string, THREE.Vector3[]>>({});

  const setUavs = store((s) => s.setUavs);
  const setObstacles = store((s) => s.setObstacles);
  const selectedEntity = store((s) => s.selectedEntity);
  const activeGroup = store((s) => s.activeGroup);
  const clickMode = store((s) => s.clickMode);
  const obstacleRadius = store((s) => s.obstacleRadius);
  const simSpeed = store((s) => s.simSpeed);
  const setSimSpeed = store((s) => s.setSimSpeed);
  const setConnectionStatus = store((s) => s.setConnectionStatus);
  const pendingContextDelete = store((s) => s.pendingContextDelete);
  const setPendingContextDelete = store((s) => s.setPendingContextDelete);
  const clickModeRef = useRef(clickMode);
  const selectedRef = useRef(selectedEntity);
  const groupRef = useRef(activeGroup);
  const radiusRef = useRef(obstacleRadius);
  clickModeRef.current = clickMode;
  selectedRef.current = selectedEntity;
  groupRef.current = activeGroup;
  radiusRef.current = obstacleRadius;

  useEffect(() => {
    setConnectionStatus(isConnected);
  }, [isConnected, setConnectionStatus]);

  useEffect(() => {
    setUavs(Object.fromEntries(uavs.map((u) => [u.id, u])));
  }, [uavs, setUavs]);

  useEffect(() => {
    setObstacles(Object.fromEntries(obstacles.map((o) => [o.id, o])));
  }, [obstacles, setObstacles]);

  useEffect(() => {
    const newHistory: Record<string, THREE.Vector3[]> = {};
    let updated = false;
    for (const uav of uavs) {
      const pos = getCartesian(uav.lat, uav.lon, EARTH_RADIUS, (uav.alt ?? 0) / 6371);
      const hist = historyRef.current[uav.id] || [pos];
      const lastPos = hist[hist.length - 1];
      if (!lastPos || lastPos.distanceTo(pos) > 0.01) {
        newHistory[uav.id] = [...hist.slice(-100), pos];
        updated = true;
      } else {
        newHistory[uav.id] = hist;
      }
    }
    historyRef.current = newHistory;
    if (updated) setDroneHistory(newHistory);
  }, [uavs]);

  useEffect(() => {
    if (pendingContextDelete) {
      setContextMenu({ id: pendingContextDelete.id, type: pendingContextDelete.type });
      if (pendingContextDelete.x != null) {
        setContextPosition({ x: pendingContextDelete.x, y: pendingContextDelete.y ?? 200 });
      } else {
        setContextPosition({ x: window.innerWidth / 2, y: 200 });
      }
      setPendingContextDelete(null);
    }
  }, [pendingContextDelete, setPendingContextDelete]);

  const handleGlobeClick = useCallback((lat: number, lon: number) => {
    const mode = clickModeRef.current;
    const selected = selectedRef.current;
    const group = groupRef.current;
    const radius = radiusRef.current;
    let msg = '';
    if (mode === 'target') {
      const topic =
        selected.type === 'uav' && selected.id
          ? `cmd/fleet/${selected.id}/target`
          : `cmd/fleet/${group}/target`;
      msg = JSON.stringify({ topic, payload: { lat, lon } });
    } else if (mode === 'obstacle') {
      msg = JSON.stringify({ topic: 'cmd/environment/obstacle', payload: { lat, lon, radius, groupId: group } });
    } else if (mode === 'spawn_drone') {
      msg = JSON.stringify({ topic: 'cmd/fleet/spawn', payload: { lat, lon, groupId: group } });
    }
    if (msg) sendMessage(msg);
  }, [sendMessage]);

  const handleDeleteItem = () => {
    if (!contextMenu) return;
    sendMessage(JSON.stringify({
      topic: 'cmd/environment/remove',
      payload: { id: contextMenu.id, type: contextMenu.type }
    }));
    setContextMenu(null);
  };

  const handleSpeedChange = (val: number) => {
    setSimSpeed(val);
    sendMessage(JSON.stringify({ topic: 'cmd/system/speed', payload: { speed: val } }));
  };

  const status = isConnected ? 'Connected to Engine' : 'Awaiting signal...';

  return (
    <main className="relative flex h-screen w-full bg-[#0a0a0f] overflow-hidden text-[#e0e0e0]">
      {/* Top Status Bar */}
      <motion.div
        initial={{ opacity: 0, y: -20 }}
        animate={{ opacity: 1, y: 0 }}
        transition={{ type: 'spring', stiffness: 150, damping: 15 }}
        className="absolute top-0 left-0 right-0 z-30 h-10 glass flex items-center justify-between px-4 rounded-none"
      >
        <div className="flex items-center gap-3">
          <h1 className="text-sm font-light tracking-widest text-white/90">VECTORHIGHWAY</h1>
          <Badge status={isConnected ? 'CONNECTED' : 'DISCONNECTED'} size="sm" />
          <span className="text-[10px] text-[#888899] min-w-[140px]">{status}</span>
        </div>
        <div className="flex items-center gap-4 text-[10px] text-[#888899]">
          <div className="flex items-center gap-2">
            <FastForward size={12} />
            <span className="text-[10px]">{simSpeed.toFixed(1)}x</span>
            <input
              type="range"
              min={0.1}
              max={10}
              step={0.1}
              value={simSpeed}
              onChange={(e) => handleSpeedChange(parseFloat(e.target.value))}
              className="w-20 h-1 bg-white/10 rounded-lg appearance-none cursor-pointer accent-cyan-500"
            />
          </div>
          <span>UAVs: {uavs.length}</span>
          <span>Group: {activeGroup.toUpperCase()}</span>
        </div>
      </motion.div>

      {/* Context Menu */}
      {contextMenu && (
        <>
          <div className="fixed inset-0 z-40" onClick={() => setContextMenu(null)} />
          <div
            className="fixed z-50 bg-[#1a1a2e] border border-white/10 rounded-lg shadow-2xl py-1 min-w-[140px]"
            style={{ left: contextPosition.x, top: contextPosition.y }}
          >
            <div className="px-3 py-1.5 text-[10px] text-white/40 uppercase tracking-wider border-b border-white/10">
              {contextMenu.type}: {contextMenu.id}
            </div>
            <button
              onClick={handleDeleteItem}
              className="w-full text-left px-3 py-1.5 text-xs text-red-400 hover:bg-red-500/10 flex items-center gap-2"
            >
              <Trash2 size={12} /> Delete
            </button>
            <button
              onClick={() => {
                sendMessage(JSON.stringify({ topic: 'cmd/environment/clear_group', payload: { groupId: activeGroup, type: 'obstacle' } }));
                setContextMenu(null);
              }}
              className="w-full text-left px-3 py-1.5 text-xs text-orange-400 hover:bg-orange-500/10 flex items-center gap-2"
            >
              <AlertTriangle size={12} /> Clear Group
            </button>
            <button
              onClick={() => {
                sendMessage(JSON.stringify({ topic: 'cmd/environment/clear_all', payload: {} }));
                setContextMenu(null);
              }}
              className="w-full text-left px-3 py-1.5 text-xs text-red-400 hover:bg-red-500/10 flex items-center gap-2"
            >
              <Trash2 size={12} /> Clear All
            </button>
          </div>
        </>
      )}

      {/* Left Sidebar */}
      <AnimatePresence mode="popLayout">
        <motion.div
          key="sidebar"
          initial={{ opacity: 0, x: -20 }}
          animate={{ opacity: 1, x: 0 }}
          exit={{ opacity: 0, x: -20 }}
          transition={{ type: 'spring', stiffness: 200, damping: 20 }}
          className="absolute left-3 top-14 bottom-3 z-20 w-80 flex flex-col gap-3 overflow-y-auto pr-1"
        >
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
        </motion.div>
      </AnimatePresence>

      {/* Globe Area */}
      <div className="grow cursor-crosshair">
        <Canvas camera={{ position: [12, 12, 12], fov: 45 }}>
          <GlobeScene handleGlobeClick={handleGlobeClick} droneHistory={droneHistory} />
          <OrbitControls enablePan={false} minDistance={7} maxDistance={25} />
        </Canvas>
      </div>
    </main>
  );
}
