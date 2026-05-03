# Quickstart: Navigation Overhaul & Globe/UI Redesign

## Prerequisites

- C++17 compiler (GCC/Clang)
- CMake 3.15+
- Node.js 18+
- Existing repo with build environment (see [Spec](/specs/002-military-grade-engine/spec.md))

## Setup

### Backend

1. Build the C++ engine (if not already built):
   ```bash
   cd /home/blanco/workspace/VectorHighway/build
   cmake ..
   make -j$(nproc)
   ```

2. Run the engine:
   ```bash
   ./build/uav_sim
   ```

### Frontend

1. Install web dependencies:
   ```bash
   cd /home/blanco/workspace/VectorHighway/web
   npm install
   ```

2. Download Natural Earth GeoJSON data for border overlays:
   ```bash
   mkdir -p public/data
   curl -o public/data/ne_110m_admin_0_countries.geojson \
     https://raw.githubusercontent.com/nvkelso/natural-earth-vector/master/geojson/ne_110m_admin_0_countries.geojson
   curl -o public/data/ne_50m_admin_1_states_provinces.geojson \
     https://raw.githubusercontent.com/nvkelso/natural-earth-vector/master/geojson/ne_50m_admin_1_states_provinces.geojson
   ```

3. Start dev server:
   ```bash
   npm run dev
   ```

## Verification

1. Open `http://localhost:3000` — globe should render with country borders
2. Place an obstacle — right-click to delete, use radius slider to set size
3. Set a target for a drone — observe smooth path computed around obstacles
4. Panels should animate in with spring physics, telemetry values should count-up smoothly
