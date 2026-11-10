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
4. **Factions and reputation.** Military, Bandits, the Institute and Survivors remember
   what *this character* did.
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
| Heat: noise draws zombies, enough heat triggers a horde | `Director/RZ_HeatMap.c` | Written (presence, vehicles, gunfire) |
| Zombie spawner: near players only, despawns far away, counts kills | `Zombies/RZ_ZombieSpawner.c` | Written |
| Backgrounds (8, chosen as loadouts in the deploy menu) | `Backgrounds/RZ_Background.c` | Written |
| Per-life reputation with 4 factions | `Factions/`, `Backgrounds/RZ_PlayerProfile.c` | Written |
| Random missions (8 types, see below) | `Missions/` | Written |
| Personal objective per life, Legacy points | `Backgrounds/RZ_Objectives.c` | Written |
| Bite detection (health drop + zombie within reach → chance to infect) | `Zombies/RZ_BiteDetector.c` | Written |
| Gunfire heat via `OnProjectileShot` | `Core/RZ_GameModeComponent.c` | Written |
| Antidote use action | `Cure/RZ_AntidoteAction.c` | Written |
| Queued hints, missions announced with distance and direction | `Core/RZ_GameModeComponent.c` | Written |
| Cure research: samples, 4 breakthroughs, antidotes, final stand | `Cure/` | Written |
| Infection timer: turns you when it runs out, cure slows it | `Core/RZ_GameModeComponent.c` | Written |
| Rotating spawn points in random towns | `Core/RZ_GameModeComponent.c` | Written |
| Save and load of mode state | `Persistence/RZ_SaveData.c` | Written |

**None of it has been compiled yet.** It was written outside Workbench. Calls
marked `// VERIFY:` are engine APIs whose exact names can differ between
game versions. Fix those first. See [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md).

## Backgrounds

| Background | Perk | Flaw |
|---|---|---|
| Nurse | Medical supplies. Infection takes 50% longer to kill you | Unarmed |
| Mechanic | Toolbox and jerrycan | 25% louder |
| Hunter | Hunting rifle, 25% quieter | Little ammo, Military -10 |
| Police Officer | Sidearm, Military +20 | Bandits -25 |
| Ex-Con | Bandits +25, knife | Military -25, Survivors -10, infection kills 20% faster |
| Lab Technician | Samples worth double research, Institute +20 | Unarmed, 15% louder |
| Radio Operator | Radio, horde warnings and intel map-wide | 20% louder |
| Unemployed | +50% mission reputation | Starts with nothing |

Gear comes from the loadout prefab. Perks and flaws come from `RZ_Background`
and change ReforgerZ systems, so they work with any other mods loaded.

## Missions

| Mission | Giver | When | What |
|---|---|---|---|
| Supply cache | Survivors | always | Hold a spot in town for 60 s; zombie guards; loot crate |
| Retake the town | Military, then Survivors | infested towns | Kill 8 × size zombies in town; infection -35 |
| Alpha sighting | Institute | until cured | Kill a special; extra samples |
| Radio relay | Survivors | always | Hold high ground 3 min; noise brings a horde |
| Survivors trapped | Survivors | more in later stages | Reach them, survive 3 waves |
| Evac flight | Military | Collapse/Overrun only | Be at the LZ when it leaves; only those present get paid |
| Burn the nest | Survivors | nest prefab set | Destroy a hive; infection -30; fail = +15 |
| Bandit dead drop | Bandits | more in later stages | Sell samples for loot; Bandit rep up, Institute rep down, cure slows for everyone |

Missions are announced to each player with distance and direction ("1.2 km NE").
A freshly spawned player hears everything active on the radio.

## Personal objectives

Each life gets one goal, flavoured by background (the Hunter heads for their cabin,
the Ex-Con digs up a stash): reach a town, survive 45 min, deliver 2 samples,
be part of 2 missions, or earn a faction's trust. Completing one gives a
**Legacy** point, saved across lives and set aside for unlocks in phase 4.

## Cure research

```
special infected killed ─▶ sample item drops ─▶ carried to the lab ─▶ "Deliver samples"
                                                                │
 25% Field antiviral   spread -15%, bites take 25% longer      ◀┘
 50% Bite treatment    lab produces an antidote every 15 min
 75% Immunity serum    spread -45% total, bites take 50% longer
100% FINAL STAND       hold the lab 15 min against hordes every 2 min
       win  ─▶ infection everywhere -70%, Aftermath, season over
       lose ─▶ lab abandoned for 90 s, research drops to 90%
```

The lab is a placed prefab if the mission maker places one. Otherwise it spawns in a random town and stays there across restarts.

## Roadmap

### Phase 1: the core loop on Arland (now)
- [ ] Set up the Workbench project, fix every `VERIFY`, and get it compiling
- [ ] Faction setup (SURV, MIL, BAND, INST plus the Bacon zombie faction)
- [ ] 8 loadouts for the backgrounds
- [ ] Lab prefab with the two actions, sample item prefab, reward crate prefab
- [ ] Play a 2-hour session on Arland, then tune density, spread rate and stage lengths

### Phase 2: make it feel alive
- [x] Bite detection: works with any zombie mod (`RZ_BiteDetector`)
- [x] Gunfire heat via `OnProjectileShot`
- [x] Antidote use action (world action for now)
- [x] Personal objective per life, Legacy points
- [x] Survivors trapped, Evac flight, Burn the nest, Bandit dead drop
- [x] Missions announced with distance and direction; queued hints
- [ ] Map markers for missions and the lab
- [ ] Custom deploy-menu panel that shows each background's perk and flaw before you pick it
- [ ] Antidote as an inventory consumable (not just a world action)
- [ ] Suppressors lower gunfire heat
- [ ] Supply convoy mission (moving target)

### Phase 3: factions with teeth
- [ ] AI faction camps (Military checkpoint, Bandit camp, Institute outpost) spawned at settlements
- [ ] Reputation gates trader access and prices (Shop System), safe-zone entry and mission offers
- [ ] AI camp behaviour per player standing (warn, refuse, shoot)
- [ ] Reputation clues visible to other players (e.g. bandit armband)

### Phase 4: Project Zomboid-style character creation
- [ ] A trait-points system on top of backgrounds (positive and negative traits)
- [ ] Journal left on your body: a hint for whoever finds it

### Phase 5: seasons
- [ ] Wipe cycle tied to cure completion, or a fixed number of weeks
- [ ] Season rule sets: Winter (ENDURE), Fog, Military Collapse, No Vehicles
- [ ] Hall of fame: who delivered the most samples, who held the final stand

## Any-map checklist

Every system finds its locations from `RZ_Settlement`, so for a new map:

1. Make a sub-scene of the map with the ReforgerZ game mode prefab.
2. Start it. The scanner logs how many settlements it found.
3. If the map has no name labels and few buildings, place a handful of
   `RZ_SettlementMarkerEntity` entities. Placing any markers turns off auto-detection.
4. Optionally place a lab prefab. Otherwise one spawns in a random town.
