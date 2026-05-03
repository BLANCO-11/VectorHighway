"use client";

import React from 'react';
import Card from '../ui/Card';
import ProgressBar from '../ui/ProgressBar';

export default function PayloadPanel({ sendMessage }: { sendMessage: (msg: string) => void }) {
  const [pan, setPan] = React.useState(0);
  const [tilt, setTilt] = React.useState(0);
  const [zoom, setZoom] = React.useState(1);
  const [payloadType, setPayloadType] = React.useState('EO_IR');

  const handleGimbal = (p: number, t: number) => {
    setPan(p);
    setTilt(t);
    sendMessage(JSON.stringify({
      topic: 'cmd/payload/gimbal',
      payload: { pan: p, tilt: t },
    }));
  };

  const handleZoom = (z: number) => {
    setZoom(z);
    sendMessage(JSON.stringify({
      topic: 'cmd/payload/zoom',
      payload: { zoom: z },
    }));
  };

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">Payload & Sensor</h2>

      <div className="space-y-3">
        <div>
          <label className="text-[9px] text-[#888899] uppercase">Type</label>
          <select
            value={payloadType}
            onChange={(e) => setPayloadType(e.target.value)}
            className="w-full bg-white/5 border border-white/10 rounded px-2 py-1 text-[10px] text-white"
          >
            <option value="EO_IR">EO/IR Camera</option>
            <option value="SAR">SAR Radar</option>
            <option value="LIDAR">LIDAR</option>
            <option value="CARGO">Cargo</option>
            <option value="RELAY">Comms Relay</option>
          </select>
        </div>

        <div>
          <p className="text-[9px] text-[#888899] uppercase mb-1">Gimbal Pan: {pan.toFixed(0)}°</p>
          <input
            type="range"
            min="-180"
            max="180"
            value={pan}
            onChange={(e) => handleGimbal(parseInt(e.target.value), tilt)}
            className="w-full"
          />
        </div>

        <div>
          <p className="text-[9px] text-[#888899] uppercase mb-1">Gimbal Tilt: {tilt.toFixed(0)}°</p>
          <input
            type="range"
            min="-90"
            max="90"
            value={tilt}
            onChange={(e) => handleGimbal(pan, parseInt(e.target.value))}
            className="w-full"
          />
        </div>

        <div>
          <p className="text-[9px] text-[#888899] uppercase mb-1">Zoom: {zoom.toFixed(1)}x</p>
          <input
            type="range"
            min="0.1"
            max="100"
            step="0.1"
            value={zoom}
            onChange={(e) => handleZoom(parseFloat(e.target.value))}
            className="w-full"
          />
        </div>
      </div>
    </Card>
  );
}
