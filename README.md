# VectorHighway

Script to find distance bw two points on a map/globe and also find the skewed angle b/w those points.
Could be used for automated transportation or drones.

## Overview

VectorHighway is a UAV simulation framework that calculates distances, bearings, and routes between geographic points. It supports obstacle generation, local avoidance, and swarm coordination. The simulation exposes a WebSocket API for real‑time UI integration.

## Prerequisites

- C++17 compiler (MSVC, GCC, or Clang)
- CMake >= 3.10
- Git (for fetching dependencies)
- Node.js (for the web UI)

## Build (C++ core)

```sh
git clone <repo-url>
cd Locator
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

The resulting executable `uav_sim` (or `uav_sim.exe` on Windows) will be placed in the `build` directory.

## Run Simulation

```sh
./uav_sim   # on Unix-like systems
uav_sim.exe # on Windows
```

## Web UI

The web UI lives in the `web` directory. To start it:

```sh
cd web
npm install
npm run dev
```

Open `http://localhost:3000` in a browser to view the live simulation.

