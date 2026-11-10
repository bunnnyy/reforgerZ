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
- **The Director** moves the server through four outbreak stages over a season of about ten
  days (a season speed setting fits it to the server's size), spreads infection from town
  to town and sends hordes toward noise. The infected adapt a little as the cure advances.
- **Bites you can survive**: most hits are scratches, a bite can be cleaned in its first two
  minutes, and the infection runs through fever, weakness and a final stage. Antidotes, Halcyon's
  trial dose, stabilisers and five low-grade meds with side effects exist from day one; research
  makes them more plentiful.
- **Four factions that remember you**: the Haven, Task Force Bastion, the Red Hand and Halcyon.
  Fame and Infamy unlock quartermaster shops and perks, or bring hunter squads, bounties and AI
  that shoots *you* and nobody else. A faction's uniform gets you past its people, until you're caught.
- **A living background**: strongholds that never fall, a few outposts, real supply trucks with
  AI crews, Haven safehouses that grow as you rescue people, named wanderers and leaders with
  loyalties of their own, story arcs and session headlines.
- **176 missions**: faction contracts, world events, puzzle missions, unarmed stealth jobs,
  missions at the quarantine line, morally grey Haven work and leaders' off-book jobs.
- **Search the world**: cupboards, car boots and bodies, locked doors that keep the infected
  out (they bang on them) and keys you can lose or trade, letters that point somewhere, rare fixable cars, crafting, traps, fire, water and
  secure radio channels.
- **Teaches itself**: no tutorial pop-ups, just a note where you wake, a wind-up radio and
  people who tell you things.
- **Any map**: towns, the quarantine line and its checkpoints are found automatically.
- **A UI that looks like the Zone**: quarantine signage and survivor marks, original faction
  emblems, an infection timer with the golden window, styled notice cards in each faction's
  colour, and screens for standing, missions and the radio (layouts still to be built).

## Status

All of it is **written but has never been compiled**. It's about 65,700 lines of Enforce
Script under `Scripts/Game/ReforgerZ/`, written outside Workbench. There are **186 `VERIFY`
markers** on engine and editor calls whose exact names need checking (150 in the game, 36 in the
Workbench plugin), no prefabs, layouts or configs yet, and nothing has been play-tested. The icon
set and mockups of every screen are done (`UI/`). Every prefab,
setting, faction and loadout to make is listed in one **prefab manifest**, and a Workbench
plugin, **ReforgerZ: Create prefabs**, makes what it can from it (also never run). Start with
[WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md).

## Docs

| Doc | What's in it |
|---|---|
| [LORE.md](docs/LORE.md) | The sealed Zone, the factions as people, tone, writing guide, endings |
| [DESIGN.md](docs/DESIGN.md) | How every system works, by system |
| [ROADMAP.md](docs/ROADMAP.md) | Built, next, play-test priorities, dropped ideas |
| [REPUTATION.md](docs/REPUTATION.md) | Factions, Fame/Infamy/Favor, shops, perks, leaders, AI hostility |
| [MISSIONS.md](docs/MISSIONS.md) | Every mission and event, with requirements and status |
| [MOD_STACK.md](docs/MOD_STACK.md) | Which Workshop mods the server runs and why |
| [UI.md](docs/UI.md) | Art direction, the icon set, the screens, the UI scripts, built and planned |
| [WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md) | Turning the scripts into a playable mod |

## Code

```
Scripts/Game/ReforgerZ/
  Core/          game mode component (the hub), helpers, map markers, loose-item cleanup
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
  Cure/          research, lab, samples, antidotes, infection stages, treatments and meds
  Guidance/      experience levels, tips, spawn scenarios, open radio channels
  Interactions/  searching, door locks, doorway blockers and keys, building queries (mission
                 spots kept out of locked buildings), cars, crafting, water,
                 traps, generators, fire, secure radio, treating others
  Persistence/   save and load of mode state
  UI/            the per-player data feed, HUD, menus, deploy-menu panel, notice styles
Scripts/WorkbenchGame/ReforgerZ/
                 the Workbench plugin: ReforgerZ: Create prefabs, Write prefab checklist
Configs/ReforgerZ/PrefabManifest.json
                 every prefab, slot, settings object, faction and loadout to set up
tools/prefab_manifest.py
                 checks the manifest against the scripts and writes the setup doc's tables
UI/              icon sources and PNGs (Icons/), HTML mockups (Mockups/), previews (Previews/),
                 the layout guide (LAYOUTS.md)
tools/render_ui.js
                 renders the icon PNGs and the mockup previews (Node and Playwright)
```
