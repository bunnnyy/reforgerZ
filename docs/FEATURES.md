# Features, stage by stage

**Draft, for the author to check; the switches aren't built yet.** This is the list for cutting the
mod back to its core and bringing each feature in again, one at a time, tested
before the next (the author's plan, 2026-11-15). Nothing here is switched off yet: today every
feature runs, and each row says what switch it has *now*.

Once the author has checked it, the plan is:
1. one **Features** panel on the game mode, a switch per row;
2. "off" means off: no tick, no spawn, no prompt, nothing in talk, the notebook or Tonight, and the
   features that use it carry on as the "Without it" column says;
3. only stage 0 on by default in `RZ_GameMode.et`;
4. a runtime check that boots with everything off and finds no script exception.

The other designs in [designs/](../designs/README.md) wait until every stage is back on.

**How to read a row.**

| Column | Says |
|---|---|
| Needs | the features it can't run without, so they come in first |
| Without it | what uses it, and how that carries on while it's off |
| Switch today | the setting on `RZ_GameMode.et` that turns it off now, "partly" when only parts of it have one, or none |
| Check | its runtime check (`tools/runtime_test/run.sh`), or none |
| Play-test | what to look at on Everon when it comes back on |

Script paths are under `scripts/Game/ReforgerZ/`.

## Stage 0: Core

Spawn, loot, fight, die, come round. These are on from the start.

| Feature | Scripts | Needs | Without it | Switch today | Check | Play-test |
|---|---|---|---|---|---|---|
| **Towns and the map scan**: finding the settlements, the mainland, the land | `World/RZ_SettlementScanner`, `RZ_Settlement`, `RZ_Mainland`, `RZ_Land` | — | everything places itself by towns; can't be off | none (always on) | none (the boot log) | the log lists Everon's towns; no town on an island or at sea |
| **The infected**: spawner, heat, bodies near players | `Zombies/RZ_ZombieSpawner`, `RZ_InfectedBodies`, `Director/RZ_HeatMap`, `RZ_Director` | Towns | hordes, tribes, crawlers, samples build on it | none (Zombies > density) | none | infected near you in town, few in the open; gunfire draws more; they despawn when you leave |
| **Bites and infection**: the bite, the stages, the meds | `Zombies/RZ_BiteDetector`, `Cure/RZ_Infection`, `RZ_Meds`, `Zombies/RZ_PlayerEffects` | The infected | the cure's doses, the trial dose, declaring a bite | none (bite chance 0 does it) | none | a scratch isn't a bite; a bite runs Fever, Weakness, Final; a med once since your last death |
| **Loot**: things in houses, wrecks' boots, bodies | `Interactions/RZ_Loot`, `RZ_Search`, `Core/RZ_LooseItems` | Towns | crafting, missions' finds | Interactions > Loot on, Search on | none | houses hold things on furniture; refill after a while; nothing in a wall |
| **Backgrounds and the spawn**: the 13 backgrounds, their kit, the spawn screen | `Backgrounds/RZ_Background`, `UI/RZ_SpawnMenu`, `UI/RZ_LoadoutLock` | — | the opening's favours, habits | UI > Lock loadouts (only the lock) | none | each background spawns with its kit; locked ones say why |
| **A character who lasts**: death keeps the character, coming round, reset | `Backgrounds/RZ_Character`, `RZ_PlayerProfile`, `Persistence/RZ_SaveData` | Backgrounds | coming round at a camp needs Territory (else a random start) | none | RZT_Persist, RZT_Fresh | die: same character, things gone, quest items back; reset on the Standing page |
| **Factions holding towns**: strongholds and outposts | `Factions/RZ_Territory`, `RZ_Factions`, `RZ_FactionRelations` | Towns | camps, shops, trucks, patrols, missions all need a holding | Territory > on | none | four factions, each in a few towns that make sense; nobody in most of the map |
| **Reputation**: Fame, Infamy, Favor, tiers, who shoots you | `Factions/RZ_Reputation`, `RZ_Hostility`, `RZ_FactionPerks` | Factions | nearly everything reads it: see Hard to switch off | none | none | shooting a faction drops you; Hunted means they fire; the Standing page shows it |
| **The notebook, notices and HUD** | `UI/RZ_Notebook`, `RZ_UIFeed`, `RZ_HudDisplay`, `RZ_NoticeLanes`, `RZ_DeathBrief`, `RZ_StatusStrip` | — | every feature shows itself here | UI > Styled notices, Death brief (parts) | none | the notebook opens and closes on a pad; one card at spawn; the death brief |
| **Saving**: the world and profiles across restarts | `Persistence/RZ_SaveData`, `RZ_HallOfFame` | — | everything saved | Ignore saved state (a wipe) | none | restart the server: standing, character and the world come back |

## Stage 1: People

The camps and the people in them, and talking to them.

| Feature | Scripts | Needs | Without it | Switch today | Check | Play-test |
|---|---|---|---|---|---|---|
| **Camp sites**: the layout of each holding, hand-built sites | `Factions/RZ_SiteGrid`, `RZ_SiteSolver`, `RZ_SiteRules`, `RZ_SiteAudit`, `World/HandBuilt/*` | Factions | camp life stands in it; with it off a holding is its people only | partly (Camp life > Buildings) | `run.sh sites` | fences closed all round, gates at openings, nothing in a road; a hand-built site is used |
| **Camp life**: people's routines, dressing, indoors, the scenery | `Factions/RZ_CampLife`, `RZ_SitePeople`, `RZ_SiteStory` | Camp sites | errands' givers stand in it | partly (Routines, Indoors, Extra, Dressing) | none | people sit, eat, stand guard by the hour; filling a place is fast as you come |
| **Haven safehouses** | `World/RZ_Safehouses` | Factions | coming round, errands, the Haven's work | Haven > on | none | safehouses with people and food; they grow or empty |
| **The talk screen and conversations** | `Dialogue/*`, `UI/RZ_TalkMenu` | Reputation | every hand-over by reply, the opening, errands: see Hard to switch off | Dialogue > Talk screen, Trees, Radio calls | the dialogue lint | talk to anyone named; replies on a pad; checks and tags show |
| **Named people and leaders** | `World/RZ_NamedPeople`, `Factions/RZ_Leaders`, `RZ_LeaderVoice` | Camp life, Talk | leaders' jobs, majors' acts | Leaders > on | RZT_PersonObj | each holding has named people; a leader has a personality and a job |
| **Shops and quartermasters** | `Factions/RZ_Quartermaster`, `RZ_ShopScreen`, `RZ_FactionServices` | Reputation, Talk | Favor has nothing to buy | none | none | buy and sell by tier; once-only items stay once per character |
| **Errands and duties**: named people's small jobs, the dealer, the duties played at their place | `Missions/Errands/*`, `Interactions/RZ_HandsOn`, `UI/RZ_HandsOnMenu` | Camp life, Talk | Tonight's work lines | Errands > on, Duties, Strangers | RZT_Kitchen, RZT_Patrol, RZT_Rules, RZT_Story | someone offers work in walking reach; a duty is played with your hands; pay arrives |
| **Wanderers**: named people on the roads | `World/RZ_Wanderers` | Talk | gossip, the Doctor, the Trader | Wanderers > on | none | a wanderer on a road talks and moves on |
| **Faction life**: needs, morale, chatter, truces | `Factions/RZ_FactionLife`, `RZ_FactionVoice` | Factions | world events about need | Faction life > on | none | a hungry camp says so; morale shows on the Standing page |
| **Checkpoints and road posts** | `Factions/RZ_Checkpoints` | Factions | the road posts' errands | Checkpoints > Road posts, Camps | none | a post stops you; surrender works; Hunted means they shoot |

## Stage 2: Start

The first hour: the opening and what guides you.

| Feature | Scripts | Needs | Without it | Switch today | Check | Play-test |
|---|---|---|---|---|---|---|
| **Guidance and tips**: levels, tips, the wind-up radio and open channels | `Guidance/RZ_Guidance`, `RZ_GuidanceTips`, `RZ_GuidanceWorld`, `Director/RZ_Broadcast` | Backgrounds | the opening hangs on it | Guidance > on (all of it) | none | a new player gets a radio and the open channels; tips once; `/level` changes the level |
| **The opening**: the door, the favour, the line, the case, Ferda, Proof | `Guidance/RZ_Openings`, `RZ_OpeningFavours`, `RZ_OpeningLine`, `RZ_OpeningCase`, `RZ_ColdCase`, `RZ_OpeningFixer` | Guidance, Talk, Errands, Factions | with it off a new character starts at a random place with "Find the Haven" | Guidance > Openings | RZT_Openings, RZT_Favour, RZT_Line, RZT_Case, RZT_Ferda, RZT_OpeningChain | each background's favour; the runner's case; Ferda comes 3 min after; a death mid-opening goes on |
| **Habits**: the door's question, three at a time | `Backgrounds/RZ_Habits` | The opening | — | none | (in RZT_Persist) | your answer at the door becomes a habit; it lasts across deaths |
| **The quest tracker and journal** | `UI/RZ_Quests`, `Backgrounds/RZ_Objectives`, `Core/RZ_MapMarkers`, `Missions/RZ_JobCards` | The notebook | every job shows here: see Hard to switch off | none | RZT_ObjPlace | a job taken by talk is tracked at once; the marker is on the live body |

**Moved:** the case's three papers start the majors, so they belong to stage 6. Until then the
case should come without its papers (a guard to add with the switches: today the papers don't ask
whether the majors are on).

## Stage 3: Map

What the map shows, and what moves on it.

| Feature | Scripts | Needs | Without it | Switch today | Check | Play-test |
|---|---|---|---|---|---|---|
| **The map's fog** | `World/RZ_MapFog`, `UI/RZ_MapFogDraw` | A character who lasts | Tonight's "never been" line | Map fog on | RZT_Fog | the map shows only where you've walked; it lasts across deaths |
| **Road patrols** | `Factions/RZ_RoadPatrols` | Camp sites | Tonight's happening lines | Road patrols on, Vehicles | RZT_RoadPatrols | a few walk the roads from a holding; a vehicle drives now and then |
| **Supply trucks** | `Factions/RZ_SupplyLines` | Factions | Tonight, the truck missions | Supply trucks on | none | a truck drives between holdings on the roads |
| **Tonight**: three true things near you | `UI/RZ_Tonight` | Errands (and reads trucks, patrols, events) | — | UI > Tonight | RZT_Tonight | three lines on the spawn card; each is true; A tracks one |

## Stage 4: Work

Jobs, events, puzzles, and the things you handle.

| Feature | Scripts | Needs | Without it | Switch today | Check | Play-test |
|---|---|---|---|---|---|---|
| **Contracts and the mission engine**: the director's offers by faction | `Missions/RZ_MissionManager`, `RZ_MissionRegistry`, `Missions/Haven`, `Bastion`, `RedHand`, `Halcyon`, `Unarmed`, `Leaders`, `Missions/Story/*` | Factions, Talk, the tracker | questlines and majors run their stages as missions | none (missions only turn off by an empty prefab) | RZT_ObjPlace, RZT_PersonObj, `run.sh tasks` | each faction offers work; every task follows [MISSION_RULES.md](MISSION_RULES.md) |
| **World events** | `Missions/Events/*` | The mission engine | Tonight's event lines | none | none | an event near players, joined by walking up |
| **Puzzles** | `Puzzles/*` | The mission engine | — | none (prefab-gated) | none | a puzzle's notes read true; its objects use one menu |
| **Work sites and carrying** | `Missions/Blocks/RZ_WorkSites`, `Interactions/RZ_WorkHold`, `RZ_WorkSiteAction` | — | missions' "do this here" steps | Work sites on, Carry | RZT_Carry | you carry a crate for real; a work site stands at its place |
| **Doors and locks**: real locks and keys, AI kept off them | `Interactions/RZ_DoorLocks`, `RZ_DoorHooks`, `RZ_AIDoorHook`, `RZ_KeyItem` | Towns | missions behind a lock, lockpicking | partly (Search > AI door hook) | (in RZT_Craft) | a locked door needs its key or a pick; AI doesn't walk through it |
| **Cars**: fixable cars, wrecks, hotwiring | `Interactions/RZ_Cars`, `RZ_CarRoom`, `UI/RZ_CarMenu` | Loot | vehicle missions | Cars > on, Wire cutting | none | a rare car can be fixed; the hotwire game; never parked in a wall |
| **Crafting, fires and traps** | `Interactions/RZ_Crafting`, `RZ_Fires`, `RZ_Traps`, `UI/RZ_CraftMenu` | Loot | — | Crafting on, Traps on | RZT_Craft | a recipe at a workbench; a fire warms and cooks; a trap trips |
| **Radios**: secure channels, house radios | `Interactions/RZ_SecureRadio`, `RZ_RadioScreen`, `UI/RZ_RadioMenu` | Guidance's radio | the Radio Operator's work | Secure radio on | none | a Radio Operator cracks a channel; the radio screen on a pad |

## Stage 5: Threat

What hunts you, besides the town's infected.

| Feature | Scripts | Needs | Without it | Switch today | Check | Play-test |
|---|---|---|---|---|---|---|
| **Heat hordes and adaptation** | `Zombies/RZ_PackHordes`, `Director/RZ_Adaptation`, `RZ_OutbreakStage` | The infected | — | Adaptation on; hordes none | none | loud play brings a horde; specials after a breakthrough; nothing too strong |
| **Crawlers**: the towns' slow infected | `Zombies/RZ_Crawlers` | The infected | — | Crawlers on | none | a crawler lies up by day and moves at night |
| **Tribes**: the land grid, packs, the Great One | `Zombies/RZ_Tribe*` | The infected | tribe events and quests | Tribes on (and its parts) | the zombie lab | a pack holds wild land; its border moves |
| **The behaviour layer**: how they move and hunt | `Zombies/Behaviour/*` | Tribes | tribes fall back to Bacon's own AI | Behaviour on (and its parts) | the zombie lab | they flank, sprint, lie still; nothing stuck in walls |
| **Stealth** | `Zombies/RZ_Stealth`, `UI` noise meter | The infected | — | Stealth on; UI > Noise meter | none | crouched in the dark you're harder to see; one bar shows it |
| **Being hunted**: hunter squads, bounties, retaliation, disguise | `Factions/RZ_Retaliation`, `RZ_Disguise*`, `RZ_FactionServices` | Reputation | — | partly (hunter minutes 0, Bounty board, Hostility > Disguises) | none | Hunted brings a squad; a disguise fools a post until it doesn't |
| **The Long Table** and its reach | `World/RZ_LongTable`, `RZ_LongTableReach` | Towns, Talk | — | Long Table on, Reach on | RZT_LongTable | the farm, its signs, the host on the road; their absence when they're gone |
| **Time and weather** | `Director/RZ_Environment` | The infected | — | Environment on | none | night: they hear further; fog changes sight |
| **The Perimeter**: the sealed edge | `World/RZ_Perimeter` | Towns | perimeter missions | Perimeter on | none | warning zone, kill zone, drones and loudspeakers; no visible wall |

## Stage 6: Story

The long stories.

| Feature | Scripts | Needs | Without it | Switch today | Check | Play-test |
|---|---|---|---|---|---|---|
| **Story arcs**: short storylines | `Director/RZ_StoryArcs` | The mission engine | — | Story arcs on | RZT_Story | an arc's rumour, its missions, its end |
| **Questlines**: three per faction, two for the Long Table | `Director/RZ_Questlines`, `Missions/Questlines/*` | The mission engine, Leaders | majors use the same engine | Questlines on | RZT_Prep (forces each) | a stage starts, you pick an approach, it ends |
| **The majors and the case's papers** | `Director/RZ_Majors`, `RZ_MajorBudget`, `Guidance/RZ_CasePapers` | Questlines, The opening | the finale reads the majors | Majors on | RZT_Majors, RZT_Papers | a paper from the lid starts its major; its first act |
| **Memory, names and deeds**: callbacks, lies, rep names, world facts | `Core/RZ_Callbacks`, `RZ_Deeds`, `World/RZ_Lies`, `RZ_WorldFacts`, `RZ_WorldMemory`, `Factions/RZ_RepNames*` | Reputation, Talk | conversations lose their remembered lines | partly (Names, Crew names, Shared > Callbacks) | none | they remember what you did; a name follows you |

## Stage 7: World

The world changing around you.

| Feature | Scripts | Needs | Without it | Switch today | Check | Play-test |
|---|---|---|---|---|---|---|
| **Power and radio**: the grid, substations, masts, the loop relay, the power jobs | `World/RZ_PowerGrid`, `RZ_Substations`, `RZ_Masts`, `RZ_LoopRelay`, `Missions/RZ_PowerJobs` | Towns, the mission engine | — | Power on, Substations on, Masts on | RZT_Grid, RZT_Masts, RZT_Relay, RZT_Power, RZT_PowerRest | a blackout; a mast repaired; lights back |
| **The radio station** | `World/RZ_RadioStation` | Factions | — | Radio station on | none | take it and broadcast |
| **Traces**: door marks, scenes, sounds and light | `World/Traces/*`, `World/RZ_Marks` | Towns | — | Traces on (and parts) | none | a mark on a door reads by your level; scenes of the first days |
| **Town memory**: looted, barricaded, powered, burned | `World/RZ_TownMemory` | Towns | traces and the season marks | Town memory on | none | a looted town stays looted |
| **Wildlife and placed things** | `World/RZ_Wildlife`, `RZ_PlacedThings` | — | — | Wildlife on, Placed things on | none | deer in the country; a fire you build stays, then clears |
| **Seasons**: rules, the record, the winner | `Core/RZ_Seasons`, `Director/RZ_SeasonMood` | Reputation | the epilogue's record | partly (Curfew, Black market, Winner rules) | none | a season's rules show; the winner's start next season |

## Stage 8: Endgame

| Feature | Scripts | Needs | Without it | Switch today | Check | Play-test |
|---|---|---|---|---|---|---|
| **The cure's research and the lab** | `Cure/RZ_CureResearch`, `RZ_LabComponents`, `RZ_Antidote*` | Factions, samples from the infected | the finale | none (Cure pace) | none | samples at the lab move research; the backlog |
| **The cure finale** | `Cure/RZ_CureFinale`, `RZ_FinaleRest`, `World/RZ_FinalePeople` | Research, Majors | research runs to 100 with no finale (the fallback) | Finale on | RZT_Finale | at 90 the routes open; the stand; the batch and the seal |
| **The epilogue week** | `Director/Epilogue/*` | The finale | the season ends at the cure | Epilogue on | RZT_Epilogue | the week's phases, its carriers, the closing work |

## Counts

| Stage | Features | Switch today: none or partly |
|---|---|---|
| 0 Core | 10 | 7 (core stays on anyway) |
| 1 People | 10 | 3 |
| 2 Start | 4 | 2 |
| 3 Map | 4 | 0 |
| 4 Work | 8 | 4 |
| 5 Threat | 9 | 2 |
| 6 Story | 4 | 1 |
| 7 World | 6 | 1 |
| 8 Endgame | 3 | 1 |
| **All** | **58** | **21** |

Stages 1 to 8 have 48 features; 14 of them have no switch or only a partial one. Those 14 need one
built before they can be cut (stage 0's are on anyway).

## Hard to switch off

Woven through many others, so cutting them breaks more than it saves. Proposal for each:

| Feature | Why it's woven | Proposal |
|---|---|---|
| **Reputation** | shops, hostility, talk checks, coming round, missions' pay, Tonight, the finale all read tiers | core, always on |
| **The talk screen** | every hand-over is a reply (MISSION_RULES rule 6), the opening, errands, leaders | on from stage 1; with it off there's nothing to hand anything to, so stages 2 to 8 need it |
| **Places** (`World/RZ_PlaceNeeds`, `RZ_PlaceFind`, `RZ_PlaceAudit`) | every task's spot, the opening's places, events | core, always on (it's a helper, no world of its own) |
| **The quest tracker** | every job and the opening show and point through it | core with the notebook (move it to stage 0) |
| **The mission engine** | questlines, majors, arcs, power jobs and the finale run their stages as missions | its switch turns off only the director's own offers; the engine stays for the stories that use it |
| **Memory and world facts** | conversations, conditions, the epilogue read them | the store stays always on; only what writes the remembered lines (callbacks, rep names) switches |
| **Camp sites** | everything at a holding stands in its layout | on with stage 1; off leaves a holding as its people at the town's centre |
| **The director and heat** | spawning, hordes and offers all run on it | core, always on |
