# UAV Simulator & Fleet Management Blueprint

## Phase 1: Engine Physics & Kinematics Correctness (Current Focus)
**Objective:** Ensure the simulation behaves realistically in 3D space.

1. **Target Arrival Logic:** Skip kinematic updates when the distance to the target is negligible (fixes the drone flying to the North Pole when bearing calculations break down at 0 distance).
2. **Accurate Great Circle Rendering:** Update the React Three Fiber frontend to interpolate lines using Spherical Linear Interpolation (`slerp`) or Vector3 `#lerp` along the globe's radius rather than linear lat/lon space, preventing lines from clipping through the Earth.

## Phase 2: Beautiful UI & Immersive 3D Experience
**Objective:** Overhaul the visual interface to be a stunning, production-ready command center.

1. **3D HTML Overlays (`@react-three/drei`)**
   * **SVG Icons:** Replace primitive `Three.js` meshes (boxes/spheres) with `<Html>` tags wrapping `lucide-react` SVG icons (e.g., `<Navigation />` for drones, `<MapPin />` for targets, `<AlertTriangle />` for obstacles).
   * **Dynamic Styling:** Style the SVGs with TailwindCSS for glowing effects (`drop-shadow-[0_0_8px_rgba(50,205,50,0.8)]`).
2. **Telemetry HUD (Heads-Up Display)**
   * **Glassmorphism:** Build beautiful floating panels using `backdrop-blur-md bg-black/40 border border-white/10`.
   * **Animations:** Use `framer-motion` for smooth panel transitions when selecting entities on the globe.
   * **Live Charts:** Integrate `recharts` to plot real-time telemetry (battery drain curves, velocity history).
3. **Environmental Visualizations**
   * Render the UAV's 60-degree sensor cone as a faint, transparent 3D polygon.
   * Draw the A* pathfinding routes globally.
4. **Interactive Scenario Builder (UI Controls)**
   * **Spawn Drones:** Build a control panel to spawn new drones dynamically (either randomly or by clicking a coordinate on the globe).
   * **Set Destinations:** Allow users to set destinations for specific spawned drones via globe-click.
   * **No-Fly Zones:** Introduce tools to draw/place "No-Fly Zones" or obstructions (static/dynamic) directly from the UI onto the globe.

## Phase 3: Multi-Agent System & Advanced Fleet Logic
**Objective:** Scale the simulation to multiple drones with real-world complexities.

1. **Swarm Support:** Refactor C++ to track `std::map<string, UAV>` and send multi-drone updates over WebSockets.
2. **Target Assignment:** UI clicks can assign specific targets to specific drones (`{"type": "set_target", "id": "uav_1", "lat": ...}`).
3. **Advanced Physics:** Implement inertia, wind resistance, and acceleration/deceleration curves.

## Phase 4: External Hardware Connectivity (The "Real Drone" API)
**Objective:** Shift the simulation into a real fleet-management server that actual drones can query. *(Note: Currently deferred to focus strictly on UI & Simulator richness).*

1. **Message Broker Integration**
   * Replace direct WebSocket bindings with a Pub/Sub message broker standard for IoT (**MQTT** or NATS).
   * UI publishes to `cmd/fleet/uav_1/target`; C++ engine subscribes and updates state.
2. **MAVLink / Telemetry Ingestion**
   * Build a bridge for real drones (PX4/ArduPilot). The real drone connects via 4G/LTE and publishes its actual GPS and battery state to the MQTT broker.
   * **Hybrid Reality:** The UI dashboard will display both simulated C++ drones and real-world physical drones seamlessly on the same 3D globe.

## Next Action Items
1. Add `@react-three/drei` `<Html>` markers for the drone and obstacles.
2. Update line rendering in the canvas to use `Vector3.lerp` and spherical interpolation so lines curve around the Earth nicely.
3. Implement `lucide-react` for beautiful iconography in the HUD.
4. Build the UI control panel for spawning custom drones, targets, and no-fly zones/obstacles interactively.