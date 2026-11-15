# Features, stage by stage

The mod is cut back to its core and brought in again one stage at a time, each tested before the
next (the author's plan, 2026-11-15). **`RZ_GameMode.et` ships with only the core on**: every switch
below is off. The other designs in [designs/](../designs/README.md) wait until every stage is back on.

## How the switches work

- **One panel.** `RZ_GameModeComponent` → **ReforgerZ > Features** (`RZ_FeatureSettings`), one
  switch per row below, named by stage ("Stage 3. Tonight: ..."). The core (stage 0) has none: it's
  always on. Code: `scripts/Game/ReforgerZ/Core/RZ_Features.c`.
- **Both must be on.** A feature runs only if its Features switch and its own setting (the "Own
  setting" column) are both on. At start, a switch that's off turns its feature's own setting off,
  so the feature behaves exactly as it always did with that setting off. A feature with no setting
  of its own asks the switch where it starts, ticks, offers or shows a prompt (marked "the switch
  only").
- **Off means off.** No tick, no spawn, no prompt, nothing in talk, the notebook or Tonight. What
  uses it carries on as the "Without it" column says.
- **Needs.** A switch whose needs are off is off too, whatever it says: Camp life needs Camp sites;
  the Opening needs Guidance, Errands and Work sites; Puzzles need Contracts; Behaviour needs
  Tribes; Majors need Questlines; the Finale needs Research and Majors; the Epilogue needs the Finale.
- **The log** says what's on at start: `Features: <n> of 46 on: <names>. Off: <names>`.
- **The clients** get the result from the server (`RZ_GameModeComponent.GetFeaturesOff`), so a
  prompt or a reply hidden on the server is hidden on their screen too.

### The profile file (server only)

`$profile:RZ_Features.txt` in the server's profile overrides the prefab, one word a line, in order
(`#` starts a comment):

| Line | Does |
|---|---|
| `all` | every switch on |
| `core` | every switch off |
| `stage 3` | stages 1 to 3 on, the rest off |
| `Tonight` | that switch on (the names are in the table's Switch column) |
| `-Tonight` | that switch off |

The runtime test writes `all` for every mode but `core`. A Workbench play reads it too, from the
profile folder Workbench was started with.

## How to bring in a stage

1. In Workbench, open `Prefabs/ReforgerZ/Core/RZ_GameMode.et`, **RZ_GameModeComponent → Features**,
   and tick the stage's switches. Or, for a quick try on the server, put `stage N` in
   `RZ_Features.txt` (no prefab change).
2. Run the stage's checks here (the Check column; `tools/runtime_test/run.sh`).
3. Play it on Everon with the Play-test column. Post what's wrong on the board.
4. Fixed: commit the prefab with the stage on, and go on to the next stage.

`run.sh core` boots with every switch off and fails if a switched-off feature did anything
(`RZT_CoreOnly.c`, [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#runtime-test-without-workbench)).

**How to read a row.**

| Column | Says |
|---|---|
| Switch | its name on the Features panel and in the profile file |
| Own setting | the feature's own on/off setting the switch turns off, or "the switch only" |
| Needs | the features it can't run without, so they come in first |
| Without it | what uses it, and how that carries on while it's off |
| Check | its runtime check (`tools/runtime_test/run.sh`), or none |
| Play-test | what to look at on Everon when it comes back on |

Script paths are under `scripts/Game/ReforgerZ/`.

## Stage 0: Core

Spawn, loot, fight, die, come round. Always on: these have no switch.

| Feature | Scripts | Without it | Check | Play-test |
|---|---|---|---|---|
| **Towns and the map scan**: finding the settlements, the mainland, the land | `World/RZ_SettlementScanner`, `RZ_Settlement`, `RZ_Mainland`, `RZ_Land` | everything places itself by towns | the boot log | the log lists Everon's towns; no town on an island or at sea |
| **The infected**: spawner, heat, the director, bodies near players | `Zombies/RZ_ZombieSpawner`, `RZ_InfectedBodies`, `Director/RZ_HeatMap`, `RZ_Director` | hordes, tribes, crawlers, samples build on it | `run.sh core` | infected near you in town, few in the open; gunfire draws more; they go when you leave |
| **Bites and infection** | `Zombies/RZ_BiteDetector`, `Cure/RZ_Infection`, `RZ_Meds`, `Zombies/RZ_PlayerEffects` | the cure's doses, declaring a bite | none | a scratch isn't a bite; a bite runs Fever, Weakness, Final |
| **Loot** | `Interactions/RZ_Loot`, `RZ_Search`, `Core/RZ_LooseItems` | crafting, missions' finds | none | houses hold things on furniture; refill after a while |
| **Backgrounds and the spawn** | `Backgrounds/RZ_Background`, `UI/RZ_SpawnMenu`, `UI/RZ_LoadoutLock` | the opening's favours | none | each background spawns with its kit |
| **A character who lasts** | `Backgrounds/RZ_Character`, `RZ_PlayerProfile`, `Persistence/RZ_SaveData` | coming round at a camp needs a holding that takes you in (else a random start) | RZT_Persist, RZT_Fresh | die: same character, things gone, quest items back |
| **Factions holding towns** | `Factions/RZ_Territory`, `RZ_Factions`, `RZ_FactionRelations` | camps, shops, trucks, patrols, missions need a holding | none | four factions, each in a few towns; nobody in most of the map |
| **Reputation** | `Factions/RZ_Reputation`, `RZ_Hostility`, `RZ_FactionPerks` | nearly everything reads it | none | shooting a faction drops you; Hunted means they fire |
| **The talk screen** | `Dialogue/*`, `UI/RZ_TalkMenu` | every hand-over is a reply (MISSION_RULES rule 6) | the dialogue lint | talk to a guard; replies on a pad |
| **The notebook, notices, HUD and the quest tracker** | `UI/RZ_Notebook`, `RZ_UIFeed`, `RZ_Quests`, `RZ_HudDisplay`, `RZ_DeathBrief`, `Core/RZ_MapMarkers` | every feature shows itself here | RZT_ObjPlace | the notebook opens on a pad; one card at spawn; the death brief |
| **Places** | `World/RZ_PlaceNeeds`, `RZ_PlaceFind`, `RZ_PlaceAudit` | every task's spot | `run.sh tasks` | (a helper: no world of its own) |
| **The mission engine** | `Missions/RZ_MissionManager`, `RZ_MissionRegistry` | questlines, majors, leaders and the finale run their steps through it | none | (its offers are the Contracts switch) |
| **Memory's store and world facts** | `RZ_WorldFacts`, `RZ_WorldMemory`, `World/RZ_Lies`, `Core/RZ_Deeds` | conversations, conditions, the epilogue read them | none | (the spoken parts are the Memory switch) |
| **Things players build** cleared in time | `World/RZ_PlacedThings` | — | none | a fire you build clears after a few days unused |
| **Saving** | `Persistence/RZ_SaveData`, `RZ_HallOfFame` | everything saved | none | restart the server: standing, character and the world come back |

## Stage 1: People

| Feature | Switch | Own setting | Needs | Without it | Check | Play-test |
|---|---|---|---|---|---|---|
| **Camp sites**: each holding's layout, hand-built sites | CampSites | Camp life > Dressing, Buildings, Layers | — | a holding is its people at the town's centre | `run.sh sites` | fences closed all round, gates at openings; a hand-built site is used |
| **Camp life**: routines and extra people | CampLife | Camp life > Routines, Extras | Camp sites | people stand | none | people sit, eat, stand guard by the hour |
| **Haven safehouses** | Safehouses | Haven > Enabled | — | coming round only at holdings | none | safehouses with people and food |
| **Named leaders** | Leaders | Leaders > Enabled | — | no leader jobs; a holding's named people stay | RZT_PersonObj | a leader has a personality and a job |
| **Shops and quartermasters** | Shops | the switch only | — | a quartermaster has no Trade reply and no shop actions | none | buy and sell by tier; once-only items stay once |
| **Errands and duties** | Errands | Errands > Enabled | — | nobody offers small work; Tonight has no work line | RZT_Kitchen, RZT_Patrol, RZT_Rules, RZT_Story | someone offers work in walking reach; a duty with your hands |
| **Work sites and carrying** | WorkSites | Shared > Work sites > Enabled | — | errands' and the opening's "do this here" steps can't run | RZT_Carry | you carry a crate for real; a work site at its place |
| **Wanderers** | Wanderers | Wanderers > Enabled | — | no gossip, Doctor or Trader on the roads | none | a wanderer on a road talks and moves on |
| **Faction life** | FactionLife | Faction life > Enabled | — | no need-driven events | none | a hungry camp says so |
| **Checkpoints and road posts** | Checkpoints | Checkpoints > Road posts, Camps | — | no road posts; camps don't warn and refuse | none | a post stops you; surrender works |

Work sites moved here from stage 4: errands and the opening are made of their steps.

## Stage 2: Start

| Feature | Switch | Own setting | Needs | Without it | Check | Play-test |
|---|---|---|---|---|---|---|
| **Guidance and tips**: levels, tips, the wind-up radio, the open channels | Guidance | Guidance > Enabled | — | no radio given, no tips, no first goal | none | a new player gets a radio; tips once; `/level` |
| **The opening** | Opening | Guidance > Openings | Guidance, Errands, Work sites | a new character starts at a random place | RZT_Openings, RZT_Favour, RZT_Line, RZT_Case, RZT_Ferda, RZT_OpeningChain | each background's favour; the runner's case; Ferda comes 3 min after |
| **Habits** | Habits | the switch only | — | the door asks nothing; nobody has a habit (one settled earlier waits) | (in RZT_Favour, RZT_Persist) | your answer at the door becomes a habit |

## Stage 3: Map

| Feature | Switch | Own setting | Needs | Without it | Check | Play-test |
|---|---|---|---|---|---|---|
| **The map's fog** | MapFog | Map fog > On | — | the whole map shows | RZT_Fog | the map shows only where you've walked |
| **Road patrols** | RoadPatrols | Road patrols > Enabled | — | — | RZT_RoadPatrols, `run.sh patrols` | a few walk the roads; a vehicle now and then |
| **Supply trucks** | SupplyTrucks | Supply trucks > Enabled | — | no truck missions | none | a truck drives between holdings |
| **Tonight** | Tonight | UI > Tonight | — | no lines on the card, no Tonight rows | RZT_Tonight | three true lines; A tracks one |

## Stage 4: Work

| Feature | Switch | Own setting | Needs | Without it | Check | Play-test |
|---|---|---|---|---|---|---|
| **Contracts**: the director's own offers by faction and its reason lane | Contracts | the switch only | — | no contract offered by the director; stories still start theirs | RZT_ObjPlace, `run.sh tasks` | each faction offers work; every task follows [MISSION_RULES.md](MISSION_RULES.md) |
| **World events** | WorldEvents | the switch only | — | no event near players | none | an event near players, joined by walking up |
| **Puzzles** | Puzzles | the switch only | Contracts | no puzzle contract | none | a puzzle's notes read true; its objects use one menu |
| **Doors and locks** | Locks | Search > Locks | — | no door is locked; jobs behind a lock find another place | (in RZT_Craft) | a locked door needs its key or a pick |
| **Cars** | Cars | Cars > Enabled | — | — | none | a rare car can be fixed; the hotwire game |
| **Crafting, fires and traps** | Crafting | Crafting > Enabled, Traps > Enabled | — | — | RZT_Craft | a recipe at a workbench; a trap trips |
| **Radios**: secure channels, house radios | Radios | Secure radio > Enabled | — | — | none | a Radio Operator cracks a channel |

## Stage 5: Threat

| Feature | Switch | Own setting | Needs | Without it | Check | Play-test |
|---|---|---|---|---|---|---|
| **Heat hordes and adaptation** | Hordes | Adaptation > Enabled; Zombies > horde heat out of reach | — | heat draws the town's infected, never a horde | none | loud play brings a horde; specials after a breakthrough |
| **Crawlers** | Crawlers | Crawlers > Enabled | — | towns as before | none | a crawler lies up by day and moves at night |
| **Tribes** | Tribes | Tribes > Enabled | — | no packs in the wild | the zombie lab | a pack holds wild land |
| **The behaviour layer** | Behaviour | Behaviour > Enabled | Tribes | Bacon's own AI | `run.sh layer` | they flank, sprint, lie still |
| **Stealth** | Stealth | Stealth > Enabled | — | no visibility rules, no noise meter | none | crouched in the dark you're harder to see |
| **Being hunted**: hunter squads, the bounty board, disguises | Hunted | Hunter squad minutes 0, Services > Bounty board, Hostility > Disguises | — | Hunted still means they fire; nobody comes looking | none | Hunted brings a squad; a disguise fools a post |
| **The Long Table** | LongTable | Long Table > Enabled | — | — | RZT_LongTable | the farm, its signs, the host on the road |
| **Time and weather's effects** | Weather | Environment > Enabled | — | night and fog change nothing | none | at night they hear further |
| **The Perimeter** | Perimeter | Perimeter > Enabled | — | no sealed edge, no perimeter missions | none | warning zone, kill zone, drones; no visible wall |

## Stage 6: Story

| Feature | Switch | Own setting | Needs | Without it | Check | Play-test |
|---|---|---|---|---|---|---|
| **Story arcs** | StoryArcs | Story arcs > Enabled | — | — | RZT_Story | an arc's rumour, its missions, its end |
| **Questlines** | Questlines | Questlines > Enabled | — | — | (the full run's questline pass) | a stage starts, you pick an approach, it ends |
| **The majors and the case's papers** | Majors | Majors > Enabled | Questlines | the case comes without its papers | RZT_Majors, RZT_Papers | a paper from the lid starts its major |
| **Memory's spoken parts**: callbacks, reputation names | Memory | Shared > Callbacks, Reputation > Names, Crew names | — | nobody brings up what you did; no name follows you | none | they remember what you did; a name follows you |

## Stage 7: World

| Feature | Switch | Own setting | Needs | Without it | Check | Play-test |
|---|---|---|---|---|---|---|
| **Power and radio**: the station, the grid, substations, masts, the loop relay | Power | Power > Enabled, its Substations > Enabled, Masts > Enabled | — | no blackout, no power jobs | RZT_Grid, RZT_Masts, RZT_Relay, RZT_PowerRest | a blackout; a mast repaired; lights back |
| **The radio station** | RadioStation | Radio station > Enabled | — | — | none | take it and broadcast |
| **Traces** | Traces | Traces > Enabled | — | no door marks or scenes | none | a mark on a door reads by your level |
| **Town memory** | TownMemory | Town memory > Enabled | — | towns forget being looted | none | a looted town stays looted |
| **Wildlife** | Wildlife | Wildlife > Enabled | — | — | none | deer in the country |
| **Season rules** | SeasonRules | Seasons > Winner rules, Story rules, Rule sets | — | every season starts plain | none | a season's rules show; the winner's start next season |

## Stage 8: Endgame

| Feature | Switch | Own setting | Needs | Without it | Check | Play-test |
|---|---|---|---|---|---|---|
| **The cure's research** | Research | the switch only | — | the lab takes no samples (as when full) and research stays where it is | none | samples at the lab move research |
| **The cure finale** | Finale | Finale > Enabled | Research, Majors | — | RZT_Finale | at 90 the routes open; the stand; the batch |
| **The epilogue week** | Epilogue | Epilogue > Enabled | Finale | the season ends at the cure | RZT_Epilogue | the week's phases and the closing work |

## What stays on, and why

| Feature | Why it's core |
|---|---|
| Reputation | shops, hostility, talk checks, coming round, pay, Tonight and the finale read tiers |
| The talk screen | every hand-over is a reply; guards and quartermasters are people from the core |
| Places | a helper with no world of its own |
| The quest tracker | every job and the opening show and point through it |
| The mission engine | questlines, majors, arcs, power jobs and the finale run their steps as missions; its switch (Contracts) stops only the director's own offers |
| Memory's store and world facts | conversations, conditions and the epilogue read them; only the spoken parts switch (Memory) |
| The director and heat | spawning runs on it; only hordes switch (Hordes) |
| Things players build cleared | performance: without it a built fire stays for ever |
