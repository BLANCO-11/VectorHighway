"use client";

import React, { useState } from 'react';
import Card from '../ui/Card';
import Button from '../ui/Button';
import Toggle from '../ui/Toggle';

export default function FlightLogPanel({ sendMessage }: { sendMessage: (msg: string) => void }) {
  const [isRecording, setIsRecording] = useState(false);
  const [replaySpeed, setReplaySpeed] = useState(1);
  const [selectedLog, setSelectedLog] = useState('');
  const [isReplaying, setIsReplaying] = useState(false);

  const availableLogs = ['flightlog_20260502_120000.jsonl'];

  const toggleRecording = () => {
    const newState = !isRecording;
    setIsRecording(newState);
    sendMessage(JSON.stringify({ topic: 'cmd/system/recording', payload: { enabled: newState } }));
  };

  const startReplay = () => {
    if (!selectedLog) return;
    setIsReplaying(true);
    sendMessage(JSON.stringify({ topic: 'cmd/system/replay', payload: { file: selectedLog, speed: replaySpeed } }));
  };

  const stopReplay = () => {
    setIsReplaying(false);
    sendMessage(JSON.stringify({ topic: 'cmd/system/replay', payload: { file: '', speed: 1 } }));
  };

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">Flight Recorder</h2>

      {/* Recording */}
      <div className="flex items-center justify-between mb-4 pb-3 border-b border-white/10">
        <div>
          <p className="text-xs text-white/80">Recording</p>
          <p className="text-[9px] text-[#555566]">
            {isRecording ? 'Recording to file...' : 'Idle'}
          </p>
        </div>
        <Toggle enabled={isRecording} onChange={toggleRecording} accentColor="#ff3355" />
      </div>

      {/* Replay */}
      <div className="space-y-2">
        <p className="text-[9px] text-[#888899] uppercase">Replay</p>
        <select
          value={selectedLog}
          onChange={(e) => setSelectedLog(e.target.value)}
          className="w-full bg-white/5 border border-white/10 rounded px-2 py-1 text-[10px] text-white"
        >
          <option value="">Select log file...</option>
          {availableLogs.map((log) => (
            <option key={log} value={log}>{log}</option>
          ))}
        </select>

        <div className="flex items-center gap-2">
          <span className="text-[9px] text-[#888899]">Speed:</span>
          <select
            value={replaySpeed}
            onChange={(e) => setReplaySpeed(parseFloat(e.target.value))}
            className="bg-white/5 border border-white/10 rounded px-1 py-0.5 text-[10px] text-white"
          >
            <option value={0.5}>0.5x</option>
            <option value={1}>1x</option>
            <option value={2}>2x</option>
            <option value={4}>4x</option>
          </select>
        </div>

        <div className="flex gap-2">
          {!isReplaying ? (
            <Button size="sm" variant="primary" onClick={startReplay} disabled={!selectedLog}>
              Play
            </Button>
          ) : (
            <Button size="sm" variant="danger" onClick={stopReplay}>
              Stop
            </Button>
          )}
        </div>

        {isReplaying && (
          <p className="text-[9px] text-[#00ff88] animate-pulse">● Replaying...</p>
        )}
      </div>
    </Card>
  );
}
