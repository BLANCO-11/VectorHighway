# Quickstart: Military-Grade UAV Engine

## Prerequisites

- **C++17 compiler** (GCC 9+ or Clang 10+)
- **CMake** 3.15+
- **Node.js** 18+ and **npm**
- **Git** (for dependency fetching)

## Build & Run

### C++ Simulation Engine

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run the simulation (port 7200 default)
./build/uav_sim

# With custom port
./build/uav_sim --port 8080

# Or via environment variable
UAV_SIM_PORT=8080 ./build/uav_sim
```

### Web Dashboard

```bash
cd web
npm install
npm run dev
```

Open `http://localhost:3000` in a browser.

### Run Tests

```bash
# C++ tests
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure

# Web tests
cd web && npm test
```

## Quick Usage

1. Start the engine: `./build/uav_sim`
2. Open the dashboard: `http://localhost:3000`
3. You will see 2 drones (alpha_1, bravo_1) en route from India to Paris
4. Click on the globe to set new targets
5. Use the Control Panel to adjust speed and battery drain
6. Use the Swarm Panel to switch between groups

## New Features

- **Smooth obstacle avoidance**: Drones smoothly arc around obstacles (static + dynamic)
- **Mission planning**: Click Mission Panel to design multi-waypoint routes
- **Health monitoring**: Health Panel shows system status and drone health
- **Flight recorder**: Telemetry is logged to `flightlog_*.jsonl` files
- **External drone ingestion**: Send telemetry to `telemetry/external/{id}` WebSocket topic
