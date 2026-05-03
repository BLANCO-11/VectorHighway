"use client";

import React, { useState } from 'react';
import Card from '../ui/Card';
import Button from '../ui/Button';
import { useSimulationStore } from '../../store/useSimulationStore';

interface MissionWaypoint {
  lat: number;
  lon: number;
  alt: number;
  action: string;
}

export default function MissionPanel({ sendMessage }: { sendMessage: (msg: string) => void }) {
  const uavs = useSimulationStore((s) => s.uavs);
  const selectedEntity = useSimulationStore((s) => s.selectedEntity);

  const [waypoints, setWaypoints] = useState<MissionWaypoint[]>([]);
  const [selectedDroneId, setSelectedDroneId] = useState('');
  const [missionState, setMissionState] = useState('IDLE');

  const droneIds = Object.keys(uavs);

  const addWaypoint = () => {
    setWaypoints((prev) => [...prev, { lat: 0, lon: 0, alt: 0.5, action: 'FLY_TO' }]);
  };

  const updateWaypoint = (idx: number, field: keyof MissionWaypoint, value: number | string) => {
    setWaypoints((prev) =>
      prev.map((wp, i) => (i === idx ? { ...wp, [field]: value } : wp))
    );
  };

  const removeWaypoint = (idx: number) => {
    setWaypoints((prev) => prev.filter((_, i) => i !== idx));
  };

  const uploadMission = () => {
    if (!selectedDroneId || waypoints.length === 0) return;
    const payload = {
      waypoints: waypoints.map((wp) => ({
        lat: wp.lat,
        lon: wp.lon,
        alt: wp.alt,
        action: wp.action,
      })),
      homeLat: uavs[selectedDroneId]?.lat ?? 0,
      homeLon: uavs[selectedDroneId]?.lon ?? 0,
    };
    sendMessage(JSON.stringify({ topic: `cmd/mission/${selectedDroneId}/upload`, payload }));
    setMissionState('UPLOADED');
  };

  const startMission = () => {
    if (!selectedDroneId) return;
    sendMessage(JSON.stringify({ topic: `cmd/mission/${selectedDroneId}/start`, payload: {} }));
    setMissionState('IN_PROGRESS');
  };

  const rth = () => {
    if (!selectedDroneId) return;
    sendMessage(JSON.stringify({ topic: `cmd/mission/${selectedDroneId}/rth`, payload: {} }));
    setMissionState('RTH');
  };

  const moveUp = (idx: number) => {
    if (idx <= 0) return;
    const updated = [...waypoints];
    [updated[idx - 1], updated[idx]] = [updated[idx], updated[idx - 1]];
    setWaypoints(updated);
  };

  const moveDown = (idx: number) => {
    if (idx >= waypoints.length - 1) return;
    const updated = [...waypoints];
    [updated[idx], updated[idx + 1]] = [updated[idx + 1], updated[idx]];
    setWaypoints(updated);
  };

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">Mission Planner</h2>

      <div className="space-y-2 mb-3">
        <label className="text-[9px] text-[#888899] uppercase">Target Drone</label>
        <select
          value={selectedDroneId}
          onChange={(e) => setSelectedDroneId(e.target.value)}
          className="w-full bg-white/5 border border-white/10 rounded px-2 py-1 text-xs text-white"
        >
          <option value="">Select drone...</option>
          {droneIds.map((id) => (
            <option key={id} value={id}>{id}</option>
          ))}
        </select>
      </div>

      <div className="space-y-1 max-h-40 overflow-y-auto mb-3">
        {waypoints.map((wp, idx) => (
          <div key={idx} className="flex items-center gap-1 py-1 border-b border-white/5">
            <span className="text-[9px] text-[#555566] w-4">{idx + 1}</span>
            <input
              type="number"
              step="any"
              placeholder="lat"
              value={wp.lat || ''}
              onChange={(e) => updateWaypoint(idx, 'lat', parseFloat(e.target.value) || 0)}
              className="w-14 bg-white/5 rounded px-1 py-0.5 text-[9px] text-white"
            />
            <input
              type="number"
              step="any"
              placeholder="lon"
              value={wp.lon || ''}
              onChange={(e) => updateWaypoint(idx, 'lon', parseFloat(e.target.value) || 0)}
              className="w-14 bg-white/5 rounded px-1 py-0.5 text-[9px] text-white"
            />
            <select
              value={wp.action}
              onChange={(e) => updateWaypoint(idx, 'action', e.target.value)}
              className="bg-white/5 rounded px-1 py-0.5 text-[9px] text-white w-14"
            >
              <option value="FLY_TO">TO</option>
              <option value="FLY_OVER">OVER</option>
              <option value="LOITER">LOITER</option>
            </select>
            <div className="flex gap-0.5 ml-auto">
              <button onClick={() => moveUp(idx)} className="text-[9px] text-[#555566] hover:text-white px-0.5">↑</button>
              <button onClick={() => moveDown(idx)} className="text-[9px] text-[#555566] hover:text-white px-0.5">↓</button>
              <button onClick={() => removeWaypoint(idx)} className="text-[9px] text-[#ff3355] px-0.5">✕</button>
            </div>
          </div>
        ))}
      </div>

      <div className="flex gap-2 mb-3">
        <Button size="sm" variant="primary" onClick={addWaypoint}>
          + Add WP
        </Button>
        <Button size="sm" variant="success" onClick={uploadMission} disabled={!selectedDroneId || waypoints.length === 0}>
          Upload
        </Button>
      </div>

      <div className="flex gap-2">
        <Button size="sm" variant="success" onClick={startMission} disabled={missionState !== 'UPLOADED'}>
          Start
        </Button>
        <Button size="sm" variant="danger" onClick={rth}>
          RTH
        </Button>
      </div>

      <div className="mt-2 text-[9px] text-[#555566]">
        Status: <span className="text-white/60">{missionState}</span>
      </div>
    </Card>
  );
}
