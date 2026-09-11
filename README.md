Pretty simple project focusing on character movement for immersive world gameplay. Inspiration from GTA and Far Cry while being original. 


### Implemented

- Camera-relative third-person movement
- Walking, jogging and sprinting
- Physical acceleration and braking
- Sprint-stop momentum
- Direction changes and pivot animations
- Turn-in-place animations
- Eight-direction crouch movement
- Falling and severity-based landing animations
- Slope-aware movement
- Adaptive stair traversal
- Alternating stair-foot tracking
- Mesh and camera stair smoothing
- Basic foot IK
- Speed-responsive camera FOV and distance
- Camera collision recovery
- Indoor movement detection
- Initial aiming camera and movement controls
- Runtime movement diagnostics

## Controls

| Input | Action |
| WASD | Move |
| Mouse | Look |
| Left Shift | Sprint |
| C / Left Ctrl | Toggle crouch |
| Right Mouse Button | Aim |
| Q | Swap aiming shoulder |
| F6 | Toggle movement diagnostics |

## Needs fixing

- Sprint-pivot transitions still require additional polishing.
- The aiming camera and movement foundation exists, but weapon and upper-body aiming animations are not yet connected.
- Dedicated Enhanced Input assets for aiming are not yet configured; native fallback bindings are currently used.
- Foot IK is disabled during stair traversal and on continuous ramps.
- Combat, interaction, inventory, AI, vehicles and survival systems have not been implemented yet.
