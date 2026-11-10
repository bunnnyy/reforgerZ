# ReforgerZ: design and roadmap

You are a lone civilian in a zombie outbreak. You can survive alone, team up
with strangers, or prey on them. The server is one shared story that moves
from **Early Outbreak** to **Aftermath**. The only way to win is together:
research the cure.

## Pillars

1. **Every life is different.** You choose a background in the spawn menu. You
   start in a random town. You get random missions, and the world state changes between visits.
2. **The world moves without you.** The Director spreads infection, moves
   the server through stages and sends hordes toward noise.
3. **One shared goal.** Cure research is server-wide and needs many players.
   It's also something to fight over: samples, antidotes, the lab.
4. **Factions and reputation.** The Haven, Task Force Bastion, the Red Hand and Halcyon
   remember what *this character* did, as Fame and Infamy (see [REPUTATION.md](REPUTATION.md)).
5. **Works on any map.** Everything is placed relative to settlements that
   are detected automatically.
6. **Use the Workshop.** Zombies, hunger and thirst, traders and building come from existing
   mods (see [MOD_STACK.md](MOD_STACK.md)).

Out of scope: sleep and time skipping.

## Systems (what exists in code today)

| System | File | Status |
|---|---|---|
| Settlement detection (markers, map labels, building clusters) | `World/RZ_SettlementScanner.c` | Written |
| Director: four outbreak stages, infection spread between towns | `Director/RZ_Director.c`, `RZ_OutbreakStage.c` | Written |
| Heat: noise draws zombies, enough heat triggers a horde | `Director/RZ_HeatMap.c` | Written (presence, vehicles, gunfire, suppressors) |
| Zombie spawner: near players only, despawns far away, counts kills | `Zombies/RZ_ZombieSpawner.c` | Written |
| Infection adapts a little: specials after breakthroughs (+15% at most), one trait per stage (runners, helmets, screamers), favourite spots (see [WORLD.md](WORLD.md)) | `Director/RZ_Adaptation.c` | Written |
| Backgrounds (8, plus 5 unlockable through Legacy) | `Backgrounds/RZ_Background.c` | Written |
| Fame, Infamy and Favor per faction per life; standing tiers; rival Infamy | `Factions/RZ_Reputation.c`, `Backgrounds/RZ_PlayerProfile.c` | Written |
| Champion carry-over ("word gets around") and background unlocks | `Core/RZ_GameModeComponent.c` | Written |
| Missions: 42 faction contracts + 10 world events, requirements, amends, background bonuses | `Missions/` (see [MISSIONS.md](MISSIONS.md)) | Written |
| Haven safehouses: picked or placed, population tiers (shop stock, guards, medic, radio), spawn points, food upkeep, rescues adding people, bodies only near players | `World/RZ_Safehouses.c` (see [WORLD.md](WORLD.md)) | Written |
| Haven house rules: no firing near a safehouse, declaring or hiding a bite, exile, location leaks to the Red Hand | `World/RZ_Safehouses.c`, `Factions/RZ_FactionPerks.c` | Written |
| Wanderers: seeded roster of 6-10 with roles, data travel, reactions (hordes, events, bad destinations, bites), per-player memory, permanent deaths in the news, guidance hook | `World/RZ_Wanderers.c` | Written |
| Haven house contracts: Walk Him Out, The Ledger, A Quiet Arrangement; the Courier's letters | `Missions/Haven/RZ_HavenHouseMissions.c` | Written |
| Named faction leaders: one per stronghold, safehouse and outpost (interim leader posts with territory off); seeded names, traits with capped effects, loyalty (Loyal, Independent, Rogue) that drifts, per-player memory, voiced briefs, first-contact hook, deaths, replacements and headlines; outposts registered by territory | `Factions/RZ_Leaders.c`, `RZ_LeaderVoice.c` (see [REPUTATION.md](REPUTATION.md), [WORLD.md](WORLD.md)) | Written |
| Leader missions: 30 personal jobs from Independent and Rogue leaders, discovery, turning a leader in, the stronghold's Rein In | `Missions/Leaders/` (see [MISSIONS.md](MISSIONS.md)) | Written |
| Choice missions: hand an item to one of two factions | `Missions/RZ_MissionItems.c`, `RZ_MissionBases.c` | Written |
| Quartermasters, Favor shops, standing check, Requisition (from a Bastion outpost's truck-supplied stock) | `Factions/RZ_Quartermaster.c`, `RZ_FactionPerks.c` | Written |
| Territory: one Stronghold per faction (never falls; Under Pressure for a while), outposts in towns that suit each faction scaled to map size, a quiet tick every 20-30 min (found, take, abandon, swing back), faction strength, the floor and the cap, migration of the interim posts, guards, named people, routines and an outpost quartermaster near players (see [WORLD.md](WORLD.md)) | `Factions/RZ_Territory.c` | Written |
| Supply trucks: one real truck with an AI crew per faction on the road, departures every 40-60 min, unsticking out of sight, shortages (items out, +25-50% prices, top tier locked), escort, drive-in and theft rewards, Haven food | `Factions/RZ_SupplyLines.c` | Written |
| Faction life: needs, morale, radio chatter with real information, life events, truces and feuds, remembered moments, rumours (see [WORLD.md](WORLD.md), [REPUTATION.md](REPUTATION.md)) | `Factions/RZ_FactionLife.c`, `RZ_FactionVoice.c` | Written |
| Faction perks, hunter squads, Kill-on-Sight bounties | `Factions/RZ_FactionPerks.c` | Written |
| Per-player AI hostility: hidden player-faction variants (bitmask), stance per faction, armed exiles at safehouses, mission fights, wanderers fighting back, retaliation for shooting a faction's AI (setting), Outlaw fallback (see [REPUTATION.md](REPUTATION.md)) | `Factions/RZ_Hostility.c`, `RZ_Retaliation.c`, `RZ_FactionRelations.c`, `RZ_Factions.c` | Written |
| Season ending: the cure goes to the faction with the most Fame | `Core/RZ_GameModeComponent.c` | Written |
| Map markers for world events and the lab | `Core/RZ_MapMarkers.c` | Written (toggle) |
| Personal objective per life, Legacy points | `Backgrounds/RZ_Objectives.c` | Written |
| Bite detection (health drop + zombie within reach → chance to infect) | `Zombies/RZ_BiteDetector.c` | Written |
| Gunfire heat via `OnProjectileShot` | `Core/RZ_GameModeComponent.c` | Written |
| Antidote: world action and inventory consumable | `Cure/RZ_AntidoteAction.c`, `RZ_AntidoteConsumable.c` | Written |
| Queued hints, missions announced with distance and direction | `Core/RZ_GameModeComponent.c` | Written |
| Cure research: samples, 4 breakthroughs, antidotes, final stand | `Cure/` | Written |
| Infection timer: turns you when it runs out, cure slows it | `Core/RZ_GameModeComponent.c` | Written |
| Rotating spawn points in random towns | `Core/RZ_GameModeComponent.c` | Written |
| Save and load of mode state | `Persistence/RZ_SaveData.c` | Written |
| Puzzle missions: 17 contracts built on clues, answer actions and wrong-answer consequences | `Puzzles/`, `Puzzles/Missions/` | Written |
| Readable clues and puzzle objects (keypad, choice, use) with server-side routing | `Puzzles/RZ_PuzzleComponents.c`, `RZ_PuzzleActions.c`, `RZ_PuzzleMission.c` | Written |
| Noisemakers (thrown or timed heat) and "being noticed" (heat or a zombie too close for too long) | `Puzzles/RZ_Noisemaker.c`, `RZ_RadioStealth.c` | Written |
| Radio checks (carries a radio or Radio Operator), Morse light, game clock and night detection | `Puzzles/RZ_RadioStealth.c`, `RZ_BlinkingLight.c`, `RZ_GameTime.c` | Written |
| Landmark finder (church, water tower, well... by prefab name, fallback prop), compass bearings and paces | `Puzzles/RZ_Landmarks.c` | Written |
| Seeded note text: names, days, personal lines in the Zone's voice | `Puzzles/RZ_NoteText.c` | Written |
| Zombie-free settlements while a mission runs (The Silent Village) | `Zombies/RZ_ZombieSpawner.c` | Written |
| Government broadcast: a looping flavour message whose tone follows the Zone's state | `Director/RZ_Broadcast.c` | Written |
| The Perimeter: map edge from the world bounds fitted to land, warning zone (drone, loudspeaker), kill zone (warning shot, then fatal), blind spots | `World/RZ_Perimeter.c` | Written |
| Sealed road checkpoints from the road network, or hand-placed markers | `World/RZ_Perimeter.c`, `RZ_CheckpointMarkerEntity.c` | Written |
| Perimeter missions: The Blind Spot, The Crossing, Shot at the Line | `Missions/Perimeter/` | Written |
| New-player guidance: experience levels that fade (never fully) with a refresher, data-driven contextual tips from the nearest plausible speaker, gentle first 15 minutes, "Find the Haven" first-life goal, six spawn scenarios with clues, wind-up radio and open channels, Bastion loudspeakers and Red Hand taunts, Halcyon flyers in safehouses, wanderer gossip, a wanderer who finds new players (see [WORLD.md](WORLD.md)) | `Guidance/` | Written |

**None of it has been compiled yet.** It was written outside Workbench. Calls
marked `// VERIFY:` are engine APIs whose exact names can differ between
game versions. Fix those first. See [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md).

## Backgrounds

Starting standing below is Fame (+) or Infamy (−).

| Background | Perk | Flaw |
|---|---|---|
| Nurse | Medical supplies. Infection takes 50% longer to kill you | Unarmed |
| Mechanic | Toolbox and jerrycan | 25% louder |
| Hunter | Hunting rifle, 25% quieter | Little ammo, Bastion -10 |
| Police Officer | Sidearm, Bastion +20 (Known) | Red Hand -25 (Watched) |
| Ex-Con | Red Hand +25 (Known), knife | Bastion -25 (Watched), Haven -10, infection kills 20% faster |
| Lab Technician | Samples worth double research, Halcyon +20 (Known) | Unarmed, 15% louder |
| Radio Operator | Radio, horde warnings and intel map-wide | 20% louder |
| Unemployed | +50% mission Fame | Starts with nothing |
| Soldier (unlock: Champion with Bastion) | Service rifle and armour, Bastion +20 | Red Hand -15 (Watched) |
| Doctor (unlock: Champion with Halcyon) | Infection takes 2.5x as long, Halcyon +20 | Unarmed |
| Raider (unlock: Champion with the Red Hand) | Red Hand +40 (Trusted) | Haven and Bastion -15 (Watched) |
| Elder (unlock: Champion with the Haven) | Haven +40 (Trusted) | 30% louder |
| Drifter (unlock: 10 Legacy) | Known with every faction | No gear |

Gear comes from the loadout prefab. Perks and flaws come from `RZ_Background`
and change ReforgerZ systems, so they work with any other mods loaded.

## Missions

Two kinds, described in full in [MISSIONS.md](MISSIONS.md):

- **World events** (10 in code) are announced to everyone, and anyone who takes part shares the reward.
- **Faction contracts** (42 in code, plus 17 puzzle missions, 3 Perimeter missions, 3 Haven house contracts and Rein In) are offered only to players who meet their requirements:
  tier with the giver, standing with other factions, background, world state and player count.
  There's no "accept" button: eligible players who take part get paid, others get nothing.
  Each player gets at most one new offer a minute, by radio or at the faction's quartermaster.

- **Leader missions** (30 in code) are personal jobs that Independent and Rogue faction leaders give in person, off their stronghold's books.

Contracts pay Fame and Favor with the giver, and often Infamy with a rival. Radio contracts are
briefed in the voice of a named leader, whose traits nudge the pay. *Amends* contracts
work Infamy off instead while you're Watched. ★ backgrounds get something better (faster
holds, more Fame). Choice missions end with an item you hand to one of two factions.
Puzzle missions are solved with clues and answer actions instead of guns; most low-tier ones
can be done unarmed.

Missions are announced to each player with distance and direction ("1.2 km NE").
A freshly spawned player hears every world event and the contracts they qualify for.

## Personal objectives

Each life gets one goal, flavoured by background (the Hunter heads for their cabin,
the Ex-Con digs up a stash): reach a town, survive 45 min, deliver 2 samples,
be part of 2 missions, or earn a faction's trust. Completing one gives a
**Legacy** point, saved across lives. 10 Legacy unlocks the Drifter background.
A player's first life that lasts 10 minutes is always "Find the Haven" (reach a safehouse);
see [WORLD.md](WORLD.md), "New-player guidance".

## Cure research

```
special infected killed ─▶ sample item drops ─▶ carried to the lab ─▶ "Deliver samples"
                                                                │
 25% Field antiviral   spread -15%, bites take 25% longer      ◀┘
 50% Bite treatment    lab produces an antidote every 15 min
 75% Immunity serum    spread -45% total, bites take 50% longer
100% FINAL STAND       hold the lab 15 min against hordes every 2 min
       win  ─▶ infection everywhere -70%, Aftermath, season over,
               the cure goes to the faction with the most Fame this season
       lose ─▶ lab abandoned for 90 s, research drops to 90%
```

The lab is a placed prefab if the mission maker places one. Otherwise it spawns in a random town and stays there across restarts.

## Roadmap

### Phase 1: the core loop on Arland (now)
- [ ] Set up the Workbench project, fix every `VERIFY`, and get it compiling
- [ ] Faction setup (SURV, MIL, BAND, INST plus the Bacon zombie faction, and the 15 hidden SURV variants)
- [ ] Check in play that AI turns on a player whose character changes faction, and that the deploy menu, chat and groups stay on SURV; if AI ignores the variants, switch Hostility to Outlaw mode
- [ ] 8 loadouts for the backgrounds
- [ ] Lab prefab with the two actions, sample item prefab, reward crate prefab
- [ ] Play a 2-hour session on Arland, then tune density, spread rate and stage lengths

### Phase 2: make it feel alive
- [x] Bite detection: works with any zombie mod (`RZ_BiteDetector`)
- [x] Gunfire heat via `OnProjectileShot`
- [x] Antidote use action (world action for now)
- [x] Personal objective per life, Legacy points
- [x] Survivors trapped, Last Helicopter, Burn the nest, Bandit dead drop
- [x] Missions announced with distance and direction; queued hints
- [x] Map markers for world events and the lab (`RZ_MapMarkers`, toggle on the game mode)
- [ ] Custom deploy-menu panel that shows each background's perk and flaw before you pick it
- [x] Antidote as an inventory consumable (`RZ_ConsumableAntidote`, needs checking in Workbench)
- [x] Suppressors lower gunfire heat (detected from attachment prefab names)
- [ ] Supply convoy mission (moving target)
- [x] 17 puzzle missions: navigation, logic, timing and stealth, mechanisms, mysteries
- [x] Noisemakers and "being noticed" (WORLD.md, unarmed missions)
- [ ] Make the puzzle prefabs (note, generic puzzle object, keypad, scale crate, Morse lamp, noisemakers) and play-test each puzzle
- [ ] The other unarmed missions in WORLD.md (Pharmacy Lure, Silent Courier, The Safe Code...) on the puzzle framework
- [x] The Perimeter: warning and kill zones, one drone per player, sealed road checkpoints, government broadcast
- [x] 3 Perimeter missions: The Blind Spot, The Crossing, Shot at the Line
- [ ] Make the Perimeter prefabs (drone, warning shot, barricade, guard, smuggler, package, body) and test the edge on an island map and a land-locked one
- [x] Haven safehouses with population tiers, food upkeep, house rules and exile (WORLD.md, "People")
- [x] Wanderer roster: data travel, reactions, memory, deaths in the news, the bitten wanderer
- [x] 3 grey Haven house contracts (Walk Him Out, The Ledger, A Quiet Arrangement) and the Courier's job
- [ ] Make the safehouse and wanderer prefabs (occupants, guard, medic, one body per role) and add the `RZ_SafehouseAction`s to the Haven quartermaster; play-test upkeep timing and food keywords
- [x] New-player guidance: experience levels, contextual tips, gentle start, "Find the Haven" (WORLD.md, section 9)
- [x] New-player guidance in the world: spawn scenarios, wind-up radio, a wanderer who finds new players, faction voices
- [ ] Make the guidance prefabs (wind-up radio, flyer, scratched door, car wreck, table radio, dead raider and soldier, barricade)
- [ ] Play-test the tip table: is anything too chatty for a level-1 player? Do the speakers pick right?
- [ ] Barter chains across wanderers
- [x] Supply trucks that carry food to safehouses (WORLD.md, "Supply lines")
- [ ] Wanderers react when their faction loses an outpost
- [x] Infection adapts a little: breakthrough specials, one trait per stage, favourite spots (WORLD.md, section 4)
- [ ] Make the runner, helmet and screamer prefab lists from the zombie mod; play-test the favourite-spot rate

### Phase 3: factions with teeth
- [x] Fame / Infamy / Favor, standing tiers, rival Infamy, Champion carry-over
- [x] Five unlockable backgrounds
- [x] Contract requirements (tier, standing with others, background, world state), amends, ★ bonuses
- [x] Choice missions, delivery missions, drop points
- [x] 42 contracts and 10 world events from MISSIONS.md
- [x] Quartermasters with Favor shops gated by tier; Requisition (half price, hourly); standing check
- [x] Requisition from a Bastion outpost's truck-supplied stock (WORLD.md supply trucks)
- [x] Cheap perks: sample scanner, Halcyon research bonus, Neighbours, toll pass (message only)
- [x] Hunter squads for Hunted players; Kill-on-Sight bounties
- [x] Per-player AI hostility: AI of each faction shoots only the players it wants dead (hidden faction variants)
- [x] Haven safehouse spawn points (one per safehouse, for everyone)
- [x] Named faction leaders with traits, loyalty, memory, voiced briefs, first contact, deaths and headlines
- [x] 30 leader missions (Desertion Route and Private Collection not built), discovery, turning a leader in, Rein In
- [ ] Add `RZ_LeaderAction` to the quartermaster prefabs (and optional leader bodies); play-test loyalty drift and discovery rates
- [x] Territory outposts register their leaders (`RZ_Leaders.RegisterOutpost`) and the interim leader posts are turned off
- [x] Background territory: strongholds, outposts, the territory tick, faction strength (WORLD.md, section 3)
- [x] Real supply trucks with AI crews, shortages, escort, drive-in and theft
- [x] Faction life: needs, morale, radio chatter, routines, named people, life events, truces and feuds, remembered moments, rumours
- [ ] Make the territory and supply prefabs (guards, occupants, trucks, crews) and play-test a truck run from stronghold to outpost; check AI seating and driving first
- [ ] Crack secure radio channels (the `RZ_FactionLife.GetSecureTraffic` hook and the "channel" tip)
- [ ] Desertion Route (needs people who walk with you) and Private Collection (needs a trap for a live special)
- [ ] Remaining perks: Haven Community respawn with a supply bag, Fire Mission, Fence, Bounty board, immunity booster, Halcyon medic
- [ ] Convoy, escort and AI-heavy contracts (Convoy Escort, Refugee Column, Hijack the Convoy, Rescue the Pilot...)
- [ ] Disguises (faction uniforms) and surrendering at checkpoints
- [x] Faction outposts with guards, named people and a quartermaster, spawned only near players
- [ ] Bastion checkpoints and Red Hand roadblocks on the roads as AI
- [ ] Shop System prices tied to standing; safe-zone entry
- [ ] AI camp behaviour per player standing: warn and refuse (shooting by standing is in)
- [x] Shooting at a faction's AI makes its AI nearby fight back for a few minutes (retaliation, a setting)
- [ ] Reputation clues visible to other players (e.g. bandit armband)

### Phase 4: Project Zomboid-style character creation
- [ ] A trait-points system on top of backgrounds (positive and negative traits)
- [ ] Journal left on your body: a hint for whoever finds it

### Phase 5: seasons
- [x] The season's ending is decided by faction Fame; the winner is saved to `last_season.json`
- [ ] TODO: next-season starting rules from the last winner (more safe towns, strong checkpoints,
      lawless roads, or lower infection with more specials; see REPUTATION.md)
- [ ] Wipe cycle tied to cure completion, or a fixed number of weeks
- [ ] Season rule sets: Winter (ENDURE), Fog, Military Collapse, No Vehicles
- [ ] Hall of fame: who delivered the most samples, who held the final stand

### Dropped

- **Airdrop and Supply Plane Crash world events:** the Zone is sealed; nothing comes in from outside.
- **Crates from the evac helicopter:** free loot; the Last Helicopter pays Fame and Favor instead.
- **Free hourly Requisition crate:** free loot; Requisition is now a half-price purchase.

## Any-map checklist

Every system finds its locations from `RZ_Settlement`, so for a new map:

1. Make a sub-scene of the map with the ReforgerZ game mode prefab.
2. Start it. The scanner logs how many settlements it found.
3. If the map has no name labels and few buildings, place a handful of
   `RZ_SettlementMarkerEntity` entities. Placing any markers turns off auto-detection.
4. Optionally place a lab prefab. Otherwise one spawns in a random town.
5. Check the logged Perimeter rectangle and checkpoint count. If the edge is wrong, set the
   override rectangle; for checkpoints on roads the scan missed, place `RZ_CheckpointMarkerEntity` entities.
6. Check the logged safehouse count. To choose where the Haven holds out, place
   `RZ_SafehouseMarkerEntity` entities (placing any turns the automatic picking off).
7. Check the logged territory (holdings per faction). To fix an outpost in place, put an
   `RZ_OutpostMarkerEntity` there.
