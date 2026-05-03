"use client";

import React from 'react';
import Card from '../ui/Card';

interface MissionEditorProps {
  sendMessage: (msg: string) => void;
  droneId?: string;
}

export default function MissionEditor({ sendMessage, droneId }: MissionEditorProps) {
  const [editing, setEditing] = React.useState(false);

  const handleEditMission = () => {
    if (!droneId) return;
    setEditing(true);
    sendMessage(JSON.stringify({
      topic: `cmd/mission/${droneId}/edit`,
      payload: { action: 'begin_edit' },
    }));
  };

  const handleAddWaypoint = () => {
    if (!droneId) return;
    sendMessage(JSON.stringify({
      topic: `cmd/mission/${droneId}/edit`,
      payload: { action: 'add_waypoint', lat: 0, lon: 0, alt: 0.5 },
    }));
  };

  return (
    <Card>
      <h2 className="text-[10px] text-white/40 uppercase mb-3 tracking-widest">Mission Editor</h2>
      <p className="text-[10px] text-[#555566] mb-2">
        {droneId ? `Editing: ${droneId}` : 'Select a drone to edit mission'}
      </p>
      <div className="flex gap-2">
        <button
          onClick={handleEditMission}
          disabled={!droneId}
          className="text-[10px] px-3 py-1.5 rounded bg-[#00d4ff]20 text-[#00d4ff] border border-[#00d4ff]/30 disabled:opacity-30"
        >
          {editing ? 'Continue Edit' : 'Edit Mission'}
        </button>
        <button
          onClick={handleAddWaypoint}
          disabled={!editing}
          className="text-[10px] px-3 py-1.5 rounded bg-white/5 border border-white/10 disabled:opacity-30"
        >
          + WP
        </button>
      </div>
    </Card>
  );
}
