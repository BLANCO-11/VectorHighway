"use client";

import React, { useRef, useMemo } from 'react';
import { useFrame } from '@react-three/fiber';
import { Stars } from '@react-three/drei';
import * as THREE from 'three';
import { useSimulationStore } from '../../store/useSimulationStore';
import Earth from './Earth';
import GlobeBorders from './GlobeBorders';
import RadarOverlay from './RadarOverlay';
import UAVMarker from './UAVMarker';
import ObstacleMarker from './ObstacleMarker';
import DestinationMarker from './DestinationMarker';
import ConnectionLines from './ConnectionLines';
import PathLine from './PathLine';
import { GROUP_COLORS } from '../../lib/constants';

const getColorForGroup = (group?: string): string => {
  if (!group) return GROUP_COLORS.default;
  if (GROUP_COLORS[group]) return GROUP_COLORS[group];
  let hash = 0;
  for (let i = 0; i < (group?.length ?? 0); i++) hash = group.charCodeAt(i) + ((hash << 5) - hash);
  const c = (hash & 0x00ffffff).toString(16).toUpperCase();
  return '#' + '00000'.substring(0, 6 - c.length) + c;
};

const UavItems = React.memo(function UavItems({ droneHistory }: { droneHistory: Record<string, THREE.Vector3[]> }) {
  const uavs = useSimulationStore((s) => s.uavs);
  const pathOverlays = useSimulationStore((s) => s.pathOverlays);
  const selectEntity = useSimulationStore((s) => s.setSelectedEntity);

  return (
    <>
      {Object.values(uavs).map((uav) => (
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
            onSelect={(e) => selectEntity(e)}
          />
        </React.Fragment>
      ))}
      {Object.entries(pathOverlays).map(([droneId, path]) =>
        path.length >= 2 ? (
          <PathLine key={`path-${droneId}`} droneId={droneId} color={getColorForGroup(droneId)} />
        ) : null
      )}
    </>
  );
});

const ObstacleItems = React.memo(function ObstacleItems() {
  const obstacles = useSimulationStore((s) => s.obstacles);
  const selectEntity = useSimulationStore((s) => s.setSelectedEntity);
  const setPendingContextDelete = useSimulationStore((s) => s.setPendingContextDelete);

  return (
    <>
      {Object.values(obstacles).map((obs) => (
        <ObstacleMarker key={obs.id} obs={obs} onSelect={(e) => selectEntity(e)} onContext={(e) => {
          e.stopPropagation();
          setPendingContextDelete({ id: obs.id, type: 'obstacle' });
        }} />
      ))}
    </>
  );
});

interface GlobeSceneProps {
  handleGlobeClick: (lat: number, lon: number) => void;
  droneHistory: Record<string, THREE.Vector3[]>;
}

export default React.memo(function GlobeScene({ handleGlobeClick, droneHistory }: GlobeSceneProps) {
  return (
    <>
      <color attach="background" args={['#0a0a0f']} />
      <ambientLight intensity={0.6} />
      <pointLight position={[10, 10, 10]} intensity={2} />
      <spotLight position={[-10, 10, 10]} angle={0.15} penumbra={1} />
      <Stars radius={300} depth={60} count={5000} factor={4} saturation={0} fade speed={0.5} />

      <Earth onClick={handleGlobeClick} />
      <GlobeBorders />
      <RadarOverlay enabled={true} />

      <UavItems droneHistory={droneHistory} />
      <ObstacleItems />
    </>
  );
});
