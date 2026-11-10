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
| Heat: noise draws zombies, enough heat triggers a horde | `Director/RZ_HeatMap.c` | Written (presence and vehicles; gunfire is phase 2) |
| Zombie spawner: near players only, despawns far away, counts kills | `Zombies/RZ_ZombieSpawner.c` | Written |
| Backgrounds (8, chosen as loadouts in the deploy menu) | `Backgrounds/RZ_Background.c` | Written |
| Per-life reputation with 4 factions | `Factions/`, `Backgrounds/RZ_PlayerProfile.c` | Written |
| Random missions: Supply cache, Clear town, Alpha hunt, Radio relay | `Missions/` | Written |
| Cure research: samples, 4 breakthroughs, antidotes, final stand | `Cure/` | Written |
| Infection timer: turns you when it runs out, cure slows it | `Core/RZ_GameModeComponent.c` | Written (the bite trigger is phase 2) |
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
- [ ] **Bite trigger**: hook Bacon Zombies' melee damage → `InfectPlayer()`
- [ ] **Gunfire heat**: weapon-fired hook → `RZ_GameModeComponent.ReportNoise()`
- [ ] **Antidote use action**: calls `CurePlayer()`
- [ ] Map markers for missions and the lab
- [ ] Custom deploy-menu panel that shows each background's perk and flaw before you pick it
- [ ] **Personal objective per life** ("find insulin", "reach the coast") with a small reward
- [ ] More missions: Rescue survivor (escort AI), Burn the nest (Zombie Hive), Military evac window, Supply convoy
- [ ] Bandit missions: steal samples, raid a supply cache. This is the betrayal path.

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
