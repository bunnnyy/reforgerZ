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
- **The Director** moves the server through four outbreak stages over a season of about 45
  real days, spreads infection from town to town and sends hordes toward noise. The infected adapt
  a little as the cure advances. Research is scaled to how much the server really plays over the
  last week, so the season takes about as long with two regulars as with twenty, and it waits at
  the lab, which works through about 3% a day even with nobody online and refuses more when its
  cold store is full, so no server can rush the cure.
- **Stealth**: stance, speed, light, weather and noise make you more or less visible. The
  infected near you see the most visible of you, lose a player who lies low, search where they
  last had them and give up; the HUD shows how visible you are.
- **Bites you can survive**: most hits are scratches, a bite can be cleaned in its first two
  minutes, and the infection runs through fever, weakness and a final stage. Antidotes, Halcyon's
  trial dose, stabilisers and five low-grade meds with side effects exist from day one; research
  makes them more plentiful.
- **Four factions that remember you**: the Haven, Task Force Bastion, the Red Hand and Halcyon.
  Fame and Infamy unlock quartermaster shops and perks (artillery on a horde, a fence, a bounty
  board, an immunity booster), or bring hunter squads, bounties and AI that shoots *you* and
  nobody else. Bastion checkpoints and Red Hand roadblocks on the roads search you, take a toll
  or let you surrender. A faction's uniform gets you past its people, until you're caught.
- **A living background**: strongholds that never fall, a few outposts, real supply trucks with
  AI crews, Haven safehouses that grow as you rescue people, named wanderers and leaders with
  loyalties of their own, story arcs and session headlines.
- **Questlines** that run over many sessions: each stage is a contract or a wait, you can talk,
  persuade, sneak, fight, bribe, betray or side with another faction, and what you chose changes
  the ending, the rewards and the world. Fifteen: three per faction, two for the Long Table and
  the example Short Weight, crossing each other through shared flags ([QUESTLINES.md](docs/QUESTLINES.md)).
- **182 missions** (plus about 90 questline stages): faction contracts (a few about what a faction would rather not say), world
  events, puzzle missions, unarmed stealth jobs, missions at the quarantine line, morally grey
  Haven and Red Hand work and leaders' off-book jobs. Contracts
  name a rough area until you're close, unless you're a Radio Operator or trusted by the giver.
- **Seasons**: the faction that got the cure shapes the next season's start, admins can add rule
  sets (Winter, Fog, Military Collapse, No Vehicles, or their own), the server wipes itself only
  after the cure's epilogue (the line stands down, the gates open with the wipe; a timed wipe is
  there for admins who pick it, off by default), and a hall of fame remembers who carried samples, held the final stand and rescued the
  most, season after season.
- **One world, one story**: missions, arcs, questlines and voices read the same world facts
  (the cure, the day, who holds a town, fixed facts of the setting), so the line doesn't shoot
  after the cure and two stories don't use the same safehouse at once. Most mission content is
  still to be switched over to them.
- **The Long Table**: some seasons, a small family in a farmhouse past the emptiest village. They
  set a place for whoever comes, the host on the road is the friendliest man in the Zone, and
  people who take that road don't always arrive. Eat with them, trade, find out, tell someone or
  end it yourself ([LORE.md](docs/LORE.md#the-long-table)).
- **Notes between players**: a notice board at every Haven safehouse.
- **Search the world**: cupboards, car boots and bodies, locked doors that keep the infected
  out (they bang on them) and keys you can lose or trade, letters that point somewhere, rare fixable cars, crafting, traps, fire, water and
  secure radio channels.
- **Teaches itself**: no tutorial pop-ups, just a note where you wake, a wind-up radio and
  people who tell you things.
- **Any map**: towns, the quarantine line and its checkpoints are found automatically.
- **A UI that looks like the Zone**: quarantine signage and survivor marks, original faction
  emblems, an infection timer with the golden window, styled notice cards in each faction's
  colour, screens for standing, missions and the radio, and on the map only what you know
  (the HUD and notice cards show in Workbench play; the map icons, and opening the screens from
  the map, are still to do).

## Status

The game scripts **compile without errors or warnings** against Arma Reforger 1.9 (checked on
the Linux dedicated server, see [WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md#compile-check-without-workbench)).
That's about 113,000 lines of Enforce Script under `scripts/Game/ReforgerZ/`. Every engine call
was also checked against the base game's script source; the 41 `VERIFY` markers left in the game
scripts (none in the Workbench plugin) are behaviours only a play-test can show, such as
replication and what the AI does. The game mode also **runs on the server** in a test world with a
fake player (`tools/runtime_test/`): every mission type, the Perimeter, death and respawn, with no
script exceptions.

The **first Workbench build** is in the repo. The plugin **ReforgerZ: Create prefabs** made 156
prefabs from the **prefab manifest**, on vanilla, Bacon Zombies and Special Zombies bases; many are
stand-ins for now, and four more have no base yet, so they aren't made. With the player controller
and the faction and loadout managers, that's 159 prefabs. The world, a sub-scene of Arland, holds
the game mode, the four factions with their hidden variants and the 13 background loadouts, and a
scenario plays it. The UI layouts and the icon atlas are there too. In Workbench's World Editor
**Play**, the mode starts with no script exceptions: towns, the Perimeter, safehouses, wanderers,
leaders, territory, supply trucks, questlines and a first mission, with the HUD and notice cards on
screen. Those first plays had no AI world in the world, so no AI or infected spawned; the next ones
crashed when the first infected spawned, because the world had no perception manager. The world has
Arland's AI world and vanilla's world managers now; AI in play is still to test
([the roadmap](docs/ROADMAP.md#play-test-priorities)). Nobody has played it from the game client
yet. Start with [PC_SETUP.md](docs/PC_SETUP.md) (Claude Code on your PC can do it for you), then
[WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md).

**Not built yet:** the content pass onto the world facts (designed) and opening the screens from
the map instead of with keys (being designed). See the roadmap's *Next*.

## Docs

| Doc | What's in it |
|---|---|
| [LORE.md](docs/LORE.md) | The sealed Zone, the factions as people, tone, writing guide, endings |
| [DESIGN.md](docs/DESIGN.md) | How every system works, by system |
| [ROADMAP.md](docs/ROADMAP.md) | Built, next, play-test priorities, dropped ideas |
| [REPUTATION.md](docs/REPUTATION.md) | Factions, Fame/Infamy/Favor, shops, perks, leaders, AI hostility |
| [MISSIONS.md](docs/MISSIONS.md) | Every mission and event, with requirements and status |
| [QUESTLINES.md](docs/QUESTLINES.md) | Long questlines over many sessions, three per faction and two for the Long Table |
| [MOD_STACK.md](docs/MOD_STACK.md) | Which Workshop mods the server runs and why |
| [PC_SETUP.md](docs/PC_SETUP.md) | Setting up your Windows PC with Claude Code: Steam apps, Workbench, local tests, play-testing |
| [UI.md](docs/UI.md) | Art direction, the icon set, the screens, the UI scripts, built and planned |
| [WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md) | Turning the scripts into a playable mod |

## Code

```
scripts/Game/ReforgerZ/
  Core/          game mode component (the hub), helpers, map markers, loose-item cleanup,
                 seasons (the winner's rules, rule sets, the wipe cycle)
  World/         settlements, the Perimeter, safehouses and their notice boards, wanderers,
                 town memory, the power grid, the radio station, the world facts (what content
                 reads: season flags, canon, claims, text tokens), the Long Table
  Director/      outbreak stages, spread, heat, adaptation, time and weather,
                 the broadcast, story arcs, the questline engine, headlines
  Zombies/       spawner (any zombie mod's prefabs), bite detection, stealth (visibility, the
                 infected's senses, losing them)
  Backgrounds/   backgrounds, personal goals, player profiles
  Factions/      reputation, quartermasters and shops, perks, services, hunter squads, bounties,
                 road posts and camps, leaders, AI hostility, disguises and retaliation,
                 territory, supply trucks, faction life
  Missions/      framework, requirements, bases, building blocks (Blocks/: escorts, live
                 capture, vehicle zones and convoys, health cost, step chains); Haven/ Bastion/ RedHand/ Halcyon/
                 Events/ Leaders/ Unarmed/ Perimeter/; Questlines/ (the author's base class,
                 approach helpers, the registry, one folder per faction and one for the Long Table)
  Puzzles/       clues, puzzle objects and actions, noisemakers, landmarks; Missions/
  Cure/          research, lab, samples, antidotes, infection stages, treatments and meds
  Guidance/      experience levels, tips, spawn scenarios, open radio channels
  Interactions/  searching, door locks, doorway blockers and keys, building queries (mission
                 spots kept out of locked buildings), cars, crafting, water,
                 traps, generators, fire, secure radio, treating others
  Persistence/   save and load of mode state, the hall of fame
  UI/            the per-player data feed, HUD, menus, deploy-menu panel and loadout lock,
                 the map's icon layer, notice styles, chat commands
scripts/WorkbenchGame/ReforgerZ/
                 the Workbench plugin: ReforgerZ: Create prefabs, Write prefab checklist
Worlds/RZ_Arland/
                 the world: a sub-scene of Arland with the game mode, the faction and
                 loadout managers, Arland's AI world and vanilla's world managers
Missions/        the scenario that plays it (RZ_Arland.conf)
Prefabs/         ReforgerZ/: the prefabs made from the manifest; the player controller
                 (Characters/Core/) and the faction and loadout managers (MP/Managers/)
Configs/ReforgerZ/PrefabManifest.json
                 every prefab, slot, settings object, faction and loadout to set up
Configs/System/  the vanilla menu and input configs, with the ReforgerZ screens and keys added
tools/prefab_manifest.py
                 checks the manifest against the scripts and writes the setup doc's tables
UI/              icon sources and PNGs (Icons/), HTML mockups (Mockups/), previews (Previews/),
                 the layout guide (LAYOUTS.md), the layouts (layouts/ReforgerZ/), the icon
                 atlas (Textures/ReforgerZ/) and its imageset (Imagesets/ReforgerZ/)
tools/render_ui.js
                 renders the icon PNGs and the mockup previews (Node and Playwright)
tools/icon_atlas.py
                 packs the icons into the atlas and writes the imageset (Python and Pillow)
tools/pacing_sim.py
                 simulates a season's research, the lab and the stages for 2, 5 and 10 players
tools/compile_check.sh
                 compiles the scripts on the dedicated server, no Workbench needed (Linux,
                 or Windows in Git Bash)
tools/runtime_test/
                 runs the game mode on that server with a fake player (test only, not the mod)
.claude/skills/pc-setup/
                 /pc-setup for Claude Code on your PC: points it at docs/PC_SETUP.md
addon.gproj      the project file (mod ID, dependencies)
```
