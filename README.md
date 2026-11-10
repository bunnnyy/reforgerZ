# ReforgerZ

A zombie survival game mode for **Arma Reforger**, set in a sealed government
**Quarantine Zone**. Eleven days ago the region was closed off: drones over the line,
roads ending in concrete, snipers nobody sees. Nothing comes in and nobody gets out.
You're a civilian who was inside when the gates closed. Survive alone, team up with
strangers, or turn on them. The whole server shares one outbreak that slowly gets worse,
and one way to end it: **research the cure.**

## Features

- **Backgrounds** picked in the spawn menu (Nurse, Mechanic, Hunter, Police Officer, Ex-Con,
  Lab Technician, Radio Operator, Unemployed), each with a real perk and flaw. Five more are
  unlocked through Legacy.
- **The Director** moves the server through four outbreak stages, spreads infection from town
  to town and sends hordes toward noise. The infected adapt a little as the cure advances.
- **Four factions that remember you**: the Haven, Task Force Bastion, the Red Hand and Halcyon.
  Fame and Infamy unlock quartermaster shops and perks, or bring hunter squads, bounties and AI
  that shoots *you* and nobody else. A faction's uniform gets you past its people, until you're caught.
- **A living background**: strongholds that never fall, a few outposts, real supply trucks with
  AI crews, Haven safehouses that grow as you rescue people, named wanderers and leaders with
  loyalties of their own, story arcs and session headlines.
- **160 missions**: faction contracts, world events, puzzle missions, unarmed stealth jobs,
  missions at the quarantine line, morally grey Haven work and leaders' off-book jobs.
- **Search the world**: cupboards, car boots and bodies, locked doors that keep the infected
  out (they bang on them) and keys you can lose or trade, letters that point somewhere, rare fixable cars, crafting, traps, fire, water and
  secure radio channels.
- **Teaches itself**: no tutorial pop-ups, just a note where you wake, a wind-up radio and
  people who tell you things.
- **Any map**: towns, the quarantine line and its checkpoints are found automatically.

## Status

All of it is **written but has never been compiled**. It's about 57,000 lines of Enforce
Script under `Scripts/Game/ReforgerZ/`, written outside Workbench. There are **113 `VERIFY`
markers** on engine calls whose exact names need checking, no prefabs or configs yet, and
nothing has been play-tested. Start with [WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md).

## Docs

| Doc | What's in it |
|---|---|
| [LORE.md](docs/LORE.md) | The sealed Zone, the factions as people, tone, writing guide, endings |
| [DESIGN.md](docs/DESIGN.md) | How every system works, by system |
| [ROADMAP.md](docs/ROADMAP.md) | Built, next, play-test priorities, dropped ideas |
| [REPUTATION.md](docs/REPUTATION.md) | Factions, Fame/Infamy/Favor, shops, perks, leaders, AI hostility |
| [MISSIONS.md](docs/MISSIONS.md) | Every mission and event, with requirements and status |
| [MOD_STACK.md](docs/MOD_STACK.md) | Which Workshop mods the server runs and why |
| [WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md) | Turning the scripts into a playable mod |

## Code

```
Scripts/Game/ReforgerZ/
  Core/          game mode component (the hub), helpers, map markers
  World/         settlements, the Perimeter, safehouses, wanderers, town memory,
                 the power grid, the radio station
  Director/      outbreak stages, spread, heat, adaptation, time and weather,
                 the broadcast, story arcs, headlines
  Zombies/       spawner (any zombie mod's prefabs), bite detection
  Backgrounds/   backgrounds, personal goals, player profiles
  Factions/      reputation, quartermasters and shops, perks, hunter squads, bounties,
                 leaders, AI hostility, disguises and retaliation, territory, supply trucks, faction life
  Missions/      framework, requirements, bases, building blocks (Blocks/: escorts, live
                 capture, vehicle zones and convoys, health cost, step chains); Haven/ Bastion/ RedHand/ Halcyon/
                 Events/ Leaders/ Unarmed/ Perimeter/
  Puzzles/       clues, puzzle objects and actions, noisemakers, landmarks; Missions/
  Cure/          research, lab, samples, antidotes
  Guidance/      experience levels, tips, spawn scenarios, open radio channels
  Interactions/  searching, door locks, doorway blockers and keys, cars, crafting, water,
                 traps, generators, fire, secure radio, treating others
  Persistence/   save and load of mode state
```
