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
- **Two kinds of infected** ([INFECTED.md](docs/INFECTED.md)): the towns hold many slow ones
  that lie up by day, come out at night, drift to noise, remember what you killed and give up on a
  locked door; the countryside's few walk somewhere, and a horde comes from a town's side. Bodies
  only near players, counted in a fixed budget, never spawned in your sight. The wild land belongs to
  **tribes**: packs with a lair, scouts on the ridges, two or three out by day, a call at dusk that
  tells you how they are, and a hunt at night that flanks, stalks, rings, flushes, baits or ambushes
  you, breaks off when it loses too many, and walks home at dawn. They live: leaders age, are
  challenged and die, guards fight for the call, packs split, quarrel over bridges, move when
  disturbed, starve, form from towns gone dark, and remember where they died, where they fed and who
  hurt them. Our code moves them; the zombie mod fights. Every faction has its own name for each
  pack, always with the place (the Haven's "Quarry lot" is Bastion's "Contact Papa-two" and the Red
  Hand's "Postmen"); you learn them, from a rumour to a mapped lair, and your own dead character may
  run with one until somebody brings them home.
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
  nobody else. Each of the 16 Fame x Infamy cells has a name ("Merciful Thug", "Useful Trouble"),
  shown with what it counts as; each faction forgives at most 30 Infamy a life, and at 75 Fame
  every new wrong costs Fame too. Bastion checkpoints and Red Hand roadblocks on the roads search you, take a toll
  or let you surrender. A faction's uniform gets you past its people, until you're caught.
- **A living background**: strongholds that never fall, a few outposts, real supply trucks with
  AI crews, Haven safehouses that grow as you rescue people, named wanderers and leaders with
  loyalties of their own, story arcs and session headlines.
- **Camps that look lived in**: tents, fires, tables and each faction's own things round its posts,
  camps, safehouses and field stations (scenery only: nothing to loot or use), and people who keep
  their faction's routine: the sentry, the patrol, Bastion's shift change and the roll at 20:30, the
  Red Hand's toll, the Haven's supper, Halcyon's courier, and the off shift asleep.
- **Questlines** that run over many sessions: each stage is a contract or a wait, you can talk,
  persuade, sneak, fight, bribe, betray or side with another faction, and what you chose changes
  the ending, the rewards and the world. Fifteen: three per faction, two for the Long Table and
  the example Short Weight, crossing each other through shared flags ([QUESTLINES.md](docs/QUESTLINES.md)).
- **161 missions** (plus about 90 questline stages): faction contracts (a few about what a faction would rather not say), world
  events, puzzle missions, unarmed stealth jobs, missions at the quarantine line, morally grey
  Haven and Red Hand work and leaders' off-book jobs. Contracts
  name a rough area until you're close, unless you're a Radio Operator or trusted by the giver.
  Some start because the world asked for them (a hungry house, a lost truck, a town gone bad), and
  the choice missions pay fairly whichever side the result goes to.
- **Errands**: small, true jobs named people ask in person: the cook whose pot is thin, the
  sergeant whose relief is asleep in a car, the technician whose freezers are short of fuel, the
  Doctor on the road with a bitten stranger before dark. 24 routines at safehouses, posts, camps and
  stations, the wanderers' asks, strangers met only where one makes sense, and 18 hand-written
  cards; they pay little, and remember you ([ERRANDS.md](docs/ERRANDS.md)).
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
- **People who talk back** ([DIALOGUE.md](docs/DIALOGUE.md)): with **Dialogue > Trees** on (off
  by default until it's play-tested), the people you talk to have topics in their own voices, with
  what each answer needs shown before you choose (persuade, a threat that always costs, a price, your
  background) and no dice; they remember how you spoke to them this life. Each faction has its own
  word for you, the Red Hand's crews give you a name you'd rather they hadn't, and leaders tell you
  what they make of you. Talk never pays Fame.
- **Notes between players**: a notice board at every Haven safehouse.
- **Loot the world**: things lie loose in houses, on worktops and floors, and you pick them up;
  bodies looted like any body, car boots to search; locked doors that keep the infected out (they bang on them)
  and keys you can lose or trade, letters that point somewhere, rare fixable cars, crafting,
  traps, fire (a full fuel can in your hands at the door), water and secure radio channels.
- **Teaches itself**: no tutorial pop-ups, just a note where you wake, a wind-up radio and
  people who tell you things.
- **Any map**: towns, the quarantine line and its checkpoints are found automatically.
- **A UI that looks like the Zone**: quarantine signage and survivor marks, original faction
  emblems, an infection timer with the golden window, styled notice cards in each faction's
  colour that come a few at a time by importance, a notebook (standing, journal, radio, map key,
  hall of fame) opened with N, the pause menu or the notebook every life carries, a talk screen
  for everyone who talks, a repair screen for the cars and a shop screen at the quartermasters,
  spoken lines as subtitles, notes on a paper reader, and on the map only what you know. Gamepad
  and console as well as keyboard and mouse (the HUD and notice cards show in Workbench play; the
  notebook, talk, repair and shop screens, reader and subtitles are written and not yet played; the
  map icons are still to do).

## Status

The game scripts **compile without errors or warnings** against Arma Reforger 1.9 (checked on
the Linux dedicated server, see [WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md#compile-check-without-workbench)).
On a Linux server with Special Zombies loaded, though, none of them load: host on Windows or drop
Special Zombies until that's fixed ([MOD_STACK.md](docs/MOD_STACK.md#special-zombies-stops-reforgerz-loading-on-a-linux-server)).
That's about 167,000 lines of Enforce Script under `scripts/Game/ReforgerZ/`. Every engine call
was also checked against the base game's script source; the 70 `VERIFY` markers left in the game
scripts (none in the Workbench plugin) are behaviours only a play-test can show, such as
replication and what the AI does. The game mode also **runs on the server** in a test world with a
fake player (`tools/runtime_test/`): every mission type, the Perimeter, death and respawn, with no
script exceptions. A **zombie lab** (`tools/zombie_lab/`) measures Bacon Zombies' own AI on the same
server, on Everon: what it sees and hears, speeds, groups and waypoints, and server frame time with
up to 140 agents.

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

**Not built yet:** the content pass onto the world facts (see the roadmap's *Next*), and the
approved designs for the rest of the season: the infected tribes and crawlers, the four majors, the
cure's finale, the epilogue week, locals and twists, errands' other cards and more, built in the
order of [designs/PLAN.md](designs/PLAN.md). Built so far: the shared engine (phase 1); small
things everywhere (phase 3: errands' core, the mission style's framework and the world memory's
writers, [ERRANDS.md](docs/ERRANDS.md)); most of the mission style in full (phase 4: the six story
jobs with truths, lies and fallout, reputation's parley and settling up, lies and witnesses,
dialogue's scenes for several players, borrowed kit, strangers at their spots,
[MISSIONS.md](docs/MISSIONS.md)); talk
(phase 2: conversation trees, reputation's voice, the season mood and the voice bank,
[DIALOGUE.md](docs/DIALOGUE.md)); the towns' crawlers, the loose ones, heat hordes from towns, the
land grid, the tribes' daily life and hunts, their life cycle (succession, borders, splits, lair
moves, new tribes, memory), the behaviour layer and the crawlers' daily life, and the people's side
(each faction's words and marks, knowing, skills, your dead run, the moat, 12 contracts and the
reworked events, the cure's hooks and the last week; tribes phases 1 to 4,
[INFECTED.md](docs/INFECTED.md)). The great alpha and the packs' signs (tribes phase 5) are next.

## Docs

| Doc | What's in it |
|---|---|
| [LORE.md](docs/LORE.md) | The sealed Zone, the factions as people, endings |
| [WRITING.md](docs/WRITING.md) | How player text is written: tone, the voices, the season mood and the voice bank, conversations, grey choices, the humour line, the text lint's rules |
| [DESIGN.md](docs/DESIGN.md) | How every system works, by system |
| [DIALOGUE.md](docs/DIALOGUE.md) | Talking to people: the talk screen, conversation trees, checks, memory, the `.rzd` format and its tools |
| [ROADMAP.md](docs/ROADMAP.md) | Built, next, play-test priorities, dropped ideas |
| [REPUTATION.md](docs/REPUTATION.md) | Factions, Fame/Infamy/Favor, shops, perks, leaders, AI hostility |
| [INFECTED.md](docs/INFECTED.md) | The infected: towns' crawlers, loose ones, hordes, the land grid, bodies and the agent budget, the tribes and their life cycle, borders, memory, who they are, how they move and hunt, the crawlers' life, the people's side (words, knowing, skills, your dead run, the moat, the cure, the last week), the simulator's numbers |
| [MISSIONS.md](docs/MISSIONS.md) | Every mission and event, with requirements and status |
| [ERRANDS.md](docs/ERRANDS.md) | The small jobs named people ask in person: givers, the dealer, pay, the faults table, the routines and cards |
| [QUESTLINES.md](docs/QUESTLINES.md) | Long questlines over many sessions, three per faction and two for the Long Table |
| [MOD_STACK.md](docs/MOD_STACK.md) | Which Workshop mods the server runs and why |
| [PC_SETUP.md](docs/PC_SETUP.md) | Setting up your Windows PC with Claude Code: Steam apps, Workbench, local tests, play-testing |
| [UI.md](docs/UI.md) | Art direction, the icon set, the screens, the UI scripts, built and planned |
| [WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md) | Turning the scripts into a playable mod |
| [designs/](designs/README.md) | Approved designs not built yet (the four majors, the cure's finale, the epilogue week, infected tribes, dialogue, errands, the mission style, reputation names, disguise, tone) and the build plan. Each moves into `docs/` as it's built |

## Code

```
scripts/Game/ReforgerZ/
  Core/          game mode component (the hub), helpers, map markers, loose-item cleanup,
                 seasons (the winner's rules, rule sets, the wipe cycle), AI groups' move
                 orders (one held waypoint each); the shared engine: the clocks, deeds, the
                 callback queue
  World/         settlements, the Perimeter, safehouses and their notice boards, wanderers,
                 town memory, the power grid, the radio station, the world facts (what content
                 reads: season flags, canon, claims and soft holds, text tokens) and its world
                 memory (reasons, signals, the town log, world records, the truth deck) with its
                 writers, the marks spawner, the Long Table, the land grid (water, height, trees,
                 roads, bridges and wild land), named people and the faults table
  Director/      outbreak stages, spread, heat, adaptation, time and weather,
                 the broadcast, the season mood and grief, story arcs, the questline engine,
                 headlines, the season budget and the set-piece calendar
  Zombies/       spawner (any zombie mod's prefabs), the towns' crawlers, bodies and the agent
                 budget (roles, data parts, the sight rule, runtime groups), the tribes (seeds,
                 the daily rhythm, hunts, scouts, their bodies), their life cycle, borders and
                 memory, the people's side (words, knowing, your dead run, the moat, the last
                 week; the tag and the call box), bite detection, stealth
                 (visibility, the infected's senses, losing them)
    Behaviour/   the behaviour layer: Bacon's hooks, steering modes, senses, formations,
                 hunt strategies, navigation, the kinds' attacks, the crawlers' daily life
  Backgrounds/   backgrounds, personal goals, player profiles
  Factions/      reputation and its 16 names, quartermasters, shops and the shop screen's server
                 side, perks, services, hunter squads, bounties,
                 road posts and camps, leaders, AI hostility, disguises and retaliation,
                 territory, supply trucks, faction life, camp life (scenery and routines),
                 reputation's voice (the word for you, crew names, the picture), the voice bank
  Missions/      framework, requirements, bases, building blocks (Blocks/: escorts, live
                 capture, vehicle zones and convoys, health cost, step chains); Haven/ Bastion/ RedHand/ Halcyon/
                 Events/ Leaders/ Unarmed/ Perimeter/ Tribes/ (the packs' contracts); the shared checks (Trust, Fear, prices) and
                 ways through a job; Story/ (the story kit, who decides, who benefits, reasons);
                 Errands/ (the dealer, the routines, Cards/); Questlines/ (the author's base class,
                 approach helpers, the registry, one folder per faction and one for the Long Table)
  Puzzles/       clues, puzzle objects and actions, noisemakers, landmarks; Missions/
  Cure/          research, lab, samples, antidotes, infection stages, treatments and meds
  Guidance/      experience levels, tips, spawn scenarios, open radio channels
  Interactions/  loose loot, searching, door locks, doorway blockers and keys, building queries (mission
                 spots kept out of locked buildings), cars, crafting, water,
                 traps, generators, fire, secure radio, treating others
  Persistence/   save and load of mode state, the hall of fame
  Dialogue/      the talk screen's engine: conversations, adapters for each kind of person,
                 what a reply needs; the trees (hubs, nodes, checks, effects), conditions,
                 memory (dialogue.json); Pack/: the conversations, generated from dialogue/
  UI/            the per-player data feed, HUD (notice cards, subtitles), the notebook and its
                 pages, the pause menu button and notebook item, the talk screen, the reader,
                 the car repair and shop screens, the screen guard, the notice board, deploy-menu panel and loadout lock, the
                 map's icon layer, notice styles, notice lanes (the interruption budget), chat commands
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
                 atlas and the car picture (Textures/ReforgerZ/) and its imageset (Imagesets/ReforgerZ/)
tools/render_ui.js
                 renders the icon PNGs, the mockup previews and the textures (Node and Playwright)
tools/icon_atlas.py
                 packs the icons into the atlas and writes the imageset (Python and Pillow)
tools/pacing_sim.py
                 simulates a season's research, the lab and the stages for 2, 5 and 10 players
tools/tribe_sim.py
                 simulates the infected tribes as data over a season on that pacing, on the
                 code's rules and the measured wild land of Arland, Everon and Cain and a
                 Chernarus-size map, 2, 5 and 10 players
dialogue/        the conversations as .rzd text (leaders, wanderers, residents, posts, table)
                 and shipped.lock, the ids that have shipped
tools/dialogue_pack.py, tools/dialogue_sim.py
                 the dialogue lint and packer; walks a conversation as one character sees it
tools/voice_sim.py
                 replays the voice bank's picker: pool sizes and joke shares by mood
tools/strings_inventory.py
                 lists every line of player text in the scripts and the conversations
tools/voice_lint.py, tools/voice_lint_baseline.txt
                 the text lint: checks player text against the writing rules; the baseline
                 holds today's findings, so only new or changed text fails
tools/compile_check.sh
                 compiles the scripts on the dedicated server, no Workbench needed (Linux,
                 or Windows in Git Bash)
tools/runtime_test/
                 runs the game mode on that server with a fake player (test only, not the mod)
tools/zombie_lab/
                 measures Bacon Zombies' own AI on that server, on Everon: groups, waypoints,
                 sight, sharing, speeds, health, the clock, frame time (test only, not the mod)
tools/land_survey/
                 reads the land grid on Arland, Everon and Cain on that server and reports what
                 it found (test only, not the mod)
.claude/skills/pc-setup/
                 /pc-setup for Claude Code on your PC: points it at docs/PC_SETUP.md
addon.gproj      the project file (mod ID, dependencies)
```
