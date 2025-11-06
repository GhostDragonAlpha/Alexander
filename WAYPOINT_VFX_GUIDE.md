# Waypoint Particle Effects Implementation

## Quick Start
Add visual effects to make waypoints visually appealing and easier to see.

## Required Assets
1. **M_Waypoint_Glow** (Material)
   - Translucent, Unlit
   - Cyan emissive color
   - 0.7 opacity

2. **PS_Waypoint_Shimmer** (Particle System)
   - 20 particles/second
   - Cyan color, additive blend
   - 2-3 second lifetime
   - Slight upward velocity

3. **Point Light** (Component per waypoint)
   - Cyan color (0, 255, 255)
   - 5000 intensity
   - 1000 unit radius

## Implementation Checklist
- [ ] Create M_Waypoint_Glow material
- [ ] Create PS_Waypoint_Shimmer particle system
- [ ] For each waypoint (5 total):
  - [ ] Add Particle System component
  - [ ] Add Point Light component
  - [ ] Apply M_Waypoint_Glow material to mesh
  - [ ] Test visibility in PIE

## Visual Reference
Waypoints should look like glowing sci-fi objectives:
- Bright cyan core
- Shimmering particles
- Soft ambient light
- Visible from 2000+ units away

## Performance
- Total particle count: ~100 simultaneous
- Performance impact: LOW
- No shadows to improve framerate

## Next Step
After waypoints complete, add energy field effects to rings (VP02).
