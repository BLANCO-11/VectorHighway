# VectorHighway

UAV simulation framework with real-time 3D Web dashboard, swarm coordination, obstacle avoidance, and WebSocket-based telemetry.

## Overview

VectorHighway is a hybrid C++/TypeScript system:
- **C++ simulation engine**: Physics-based UAV movement, A* pathfinding, local collision avoidance, battery modeling
- **Next.js dashboard**: Dark-themed 3D globe (Three.js), real-time telemetry panels, swarm group management, battery charts

## Prerequisites

- C++17 compiler (GCC >= 9, Clang >= 10, or MSVC 2022)
- CMake >= 3.10
- Git (for fetching dependencies)
- Node.js >= 18

## Build & Run (C++ Simulation Engine)

```sh
git clone <repo-url>
cd VectorHighway
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./uav_sim
```

The simulation starts on port **8080** by default. To use a different port:
```sh
./uav_sim --port 7200
# or
UAV_SIM_PORT=7200 ./uav_sim
```

## Run Tests

```sh
cd build
cmake .. -DBUILD_TESTING=ON
cmake --build .
ctest --output-on-failure
```

## Run Web UI

```sh
cd web
npm install
npm run dev
```

Open `http://localhost:3000` in a browser.

## Verify It Works

1. Start the simulation engine (`./uav_sim` from `build/`)
2. Start the web UI (`npm run dev` from `web/`)
3. Open `http://localhost:3000`
4. You should see:
   - A 3D globe with Earth texture
   - Two UAV markers (alpha_1 from India heading to Paris)
   - Static and dynamic obstacle markers
   - A control panel on the left side
   - Real-time telemetry updates

## WebSocket Configuration

The WebSocket server listens by default on port **8080**. The UI connects to `ws://localhost:8080`.

Messages use a pub/sub topic format:
- `cmd/fleet/{id}/target` — Set target coordinates
- `cmd/fleet/{id}/control` — Adjust speed/battery drain
- `cmd/fleet/spawn` — Spawn a new drone
- `cmd/environment/obstacle` — Create an obstacle
- `telemetry/external/{id}` — Inject external drone telemetry

## Development

- **C++ code**: Edit files in `include/` and `src/`, rebuild with `cmake --build .`
- **TypeScript code**: Edit files in `web/src/`, Next.js hot-reloads automatically
- **Adding tests**: Add test cases to `src/tests.cpp` using the Catch2 framework
- **Debug mode**: Use `-DCMAKE_BUILD_TYPE=Debug` for ASan and debug symbols

## CMake Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTING` | OFF | Build the Catch2 test suite |
| `CMAKE_BUILD_TYPE` | Release | Debug/Release build |

## Project Layout

```
VectorHighway/
├── include/          # C++ headers
├── src/              # C++ source files
├── web/              # Next.js web frontend
├── specs/            # Feature specifications and plans
├── CMakeLists.txt    # C++ build configuration
└── README.md         # This file
```
