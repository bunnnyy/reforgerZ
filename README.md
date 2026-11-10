# ReforgerZ

A zombie survival game mode for **Arma Reforger**. You're a lone civilian:
survive alone, team up with strangers, or turn on them. The whole server
shares one outbreak that gets worse over time, and one way out: **research
the cure.**

- **Pick a background in the spawn menu**, like an occupation in Project Zomboid:
  Nurse, Mechanic, Hunter, Police Officer, Ex-Con, Lab Technician, Radio Operator, Unemployed.
  Each has a real perk and a real flaw.
- **The Director** moves the server from Early Outbreak → Collapse → Overrun → Aftermath,
  spreads infection from town to town, and sends hordes toward noise.
- **Random missions** in real towns on whatever map you load: supply caches, retaking towns,
  hunting Alpha infected, holding radio relays.
- **Cure research**: kill special infected, take their samples to the lab, unlock
  server-wide breakthroughs, then hold the lab through the final stand.
- **Factions and reputation**: Survivors, Military, Bandits and the Institute remember each life.
- **Any map**: towns are detected from map labels and building clusters.
- **Built on the Workshop**: zombies, hunger and thirst, traders and base building come from
  existing mods. ReforgerZ ties them together.

## Docs

- [docs/PLAN.md](docs/PLAN.md): design, systems and roadmap
- [docs/MOD_STACK.md](docs/MOD_STACK.md): which Workshop mods the server runs and why
- [docs/WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md): turning these scripts into a playable mod

## Status

Phase 1. The core systems are written in Enforce Script under `Scripts/Game/ReforgerZ/`.
They **haven't been compiled in Workbench yet**. Start with the `VERIFY` list in
the setup guide.

```
Scripts/Game/ReforgerZ/
  Core/         game mode component (hub), helpers
  World/        settlement detection, manual markers
  Director/     outbreak stages, infection spread, heat
  Zombies/      spawner (works with any zombie mod's prefabs)
  Backgrounds/  civilian backgrounds, player profiles
  Factions/     reputation
  Missions/     mission framework and 4 mission types
  Cure/         research, lab, samples, actions
  Persistence/  save/load of mode state
```
