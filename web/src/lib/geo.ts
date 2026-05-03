import * as THREE from 'three';

const EARTH_RADIUS_KM = 6371;

export function toRadians(degrees: number): number {
  return degrees * (Math.PI / 180);
}

export function toDegrees(radians: number): number {
  return radians * (180 / Math.PI);
}

export function haversineDistance(
  lat1: number, lon1: number,
  lat2: number, lon2: number
): number {
  const dLat = toRadians(lat2 - lat1);
  const dLon = toRadians(lon2 - lon1);
  const a =
    Math.sin(dLat / 2) * Math.sin(dLat / 2) +
    Math.cos(toRadians(lat1)) * Math.cos(toRadians(lat2)) *
    Math.sin(dLon / 2) * Math.sin(dLon / 2);
  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  return EARTH_RADIUS_KM * c;
}

export function bearing(
  lat1: number, lon1: number,
  lat2: number, lon2: number
): number {
  const dLon = toRadians(lon2 - lon1);
  const y = Math.sin(dLon) * Math.cos(toRadians(lat2));
  const x =
    Math.cos(toRadians(lat1)) * Math.sin(toRadians(lat2)) -
    Math.sin(toRadians(lat1)) * Math.cos(toRadians(lat2)) * Math.cos(dLon);
  return (toDegrees(Math.atan2(y, x)) + 360) % 360;
}

export function getCartesian(
  lat: number, lon: number, radius: number, alt: number = 0
): THREE.Vector3 {
  const phi = (90 - lat) * (Math.PI / 180);
  const theta = (lon + 180) * (Math.PI / 180);
  const r = radius + alt / EARTH_RADIUS_KM;
  return new THREE.Vector3(
    -(r * Math.sin(phi) * Math.cos(theta)),
    r * Math.cos(phi),
    r * Math.sin(phi) * Math.sin(theta)
  );
}

export function interpolatePosition(
  current: THREE.Vector3,
  target: THREE.Vector3,
  fraction: number
): THREE.Vector3 {
  return new THREE.Vector3().lerpVectors(current, target, Math.min(fraction, 1));
}

export function midpoint(
  lat1: number, lon1: number,
  lat2: number, lon2: number
): { lat: number; lon: number } {
  return {
    lat: (lat1 + lat2) / 2,
    lon: (lon1 + lon2) / 2,
  };
}

export function pointInPolygon(
  lat: number, lon: number,
  vertices: { lat: number; lon: number }[]
): boolean {
  let inside = false;
  let j = vertices.length - 1;
  for (let i = 0; i < vertices.length; i++) {
    if (
      vertices[i].lon > lon !== vertices[j].lon > lon &&
      lat < ((vertices[j].lat - vertices[i].lat) * (lon - vertices[i].lon)) /
        (vertices[j].lon - vertices[i].lon) + vertices[i].lat
    ) {
      inside = !inside;
    }
    j = i;
  }
  return inside;
}

export function clamp(value: number, min: number, max: number): number {
  return Math.max(min, Math.min(max, value));
}
