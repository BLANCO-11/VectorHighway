export const GROUP_COLORS: Record<string, string> = {
  alpha: '#32CD32',
  bravo: '#4488ff',
  charlie: '#ff00ff',
  delta: '#ffaa00',
  echo: '#00ff88',
  default: '#ffffff',
};

export const THEME = {
  background: '#0a0a0f',
  surface: '#1a1a2e',
  accent: '#00d4ff',
  danger: '#ff3355',
  success: '#00ff88',
  warning: '#ffaa00',
  textPrimary: '#e0e0e0',
  textSecondary: '#888899',
  borderColor: 'rgba(255,255,255,0.05)',
  glassBackground: 'rgba(0,0,0,0.4)',
  glassBlur: 'blur(12px)',
} as const;

export const DEFAULT_CONFIG = {
  speed: 0.25,
  batteryDrain: 0.01,
  turnRate: 90.0,
  chargingRate: 2.0,
  arrivalRadius: 0.1,
  maxMessagesPerSecond: 100,
} as const;

export const EARTH_RADIUS = 5;

export const RECONNECT_BASE_DELAY = 1000;
export const RECONNECT_MAX_DELAY = 30000;

export const SIDEBAR_WIDTH = 320;
export const PANEL_GAP = 16;
