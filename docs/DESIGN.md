# Systems design

How every ReforgerZ system works, organised by system. Everything here is written in code
unless it says **planned**; *Simplified* notes say where the code does less than the idea.
What's next and what was dropped is in [ROADMAP.md](ROADMAP.md). Factions, standing and
AI hostility are in [REPUTATION.md](REPUTATION.md); every mission is in [MISSIONS.md](MISSIONS.md).
Code paths are under `scripts/Game/ReforgerZ/`.

## Contents

1. [Principles](#principles)
2. [Settlements, spawn points and the map](#settlements-spawn-points-and-the-map)
3. [The Director: outbreak stages and spread](#the-director-outbreak-stages-and-spread)
4. [Heat and noise](#heat-and-noise)
5. [Stealth](#stealth)
6. [Zombies, bites and infection](#zombies-bites-and-infection)
7. [Infection adapts](#infection-adapts)
8. [The Perimeter](#the-perimeter)
9. [Time and environment](#time-and-environment)
10. [Backgrounds and personal goals](#backgrounds-and-personal-goals)
11. [Factions in the background](#factions-in-the-background)
12. [Haven safehouses](#haven-safehouses)
13. [Wanderers](#wanderers)
14. [Leaders](#leaders)
15. [Talking to people](#talking-to-people)
16. [Missions](#missions)
17. [Story arcs and headlines](#story-arcs-and-headlines)
18. [Questlines](#questlines)
19. [World facts](#world-facts)
20. [Town memory](#town-memory)
21. [World interactions](#world-interactions)
22. [New-player guidance](#new-player-guidance)
23. [Cure research and the season's end](#cure-research-and-the-seasons-end)
24. [Season pacing](#season-pacing)
25. [Seasons](#seasons)
26. [Loose items](#loose-items)
27. [Persistence](#persistence)

## Principles

You're a lone civilian in a sealed Quarantine Zone ([LORE.md](LORE.md)). You can survive
alone, team up with strangers, or prey on them. The server is one shared story that moves
from Early Outbreak to Aftermath, and the only way to win is together: research the cure.

**Pillars**

1. **Every life is different.** A background from the spawn menu, a random town, random
   missions, and a world that changed since your last visit.
2. **The world moves without you.** The Director spreads infection, moves the server
   through stages and sends hordes toward noise.
3. **One shared goal.** Cure research is server-wide, and also something to fight over.
4. **Factions remember.** The Haven, Task Force Bastion, the Red Hand and Halcyon remember
   what *this character* did ([REPUTATION.md](REPUTATION.md)).
5. **Any map.** Everything is placed relative to settlements found at start.
6. **Use the Workshop** for zombies and hunger ([MOD_STACK.md](MOD_STACK.md)).

**Rules every system follows**

- **Simulate as data, spawn bodies only near players.** Population, wanderers, leaders and
  territory are numbers on the server. AI and props exist only within a few hundred metres
  of a player. The one exception is supply trucks: at most one real truck per faction.
- **Effects are capped and recover.** Nothing a player or the Director does wipes out a
  faction, a town or a safehouse for good. Strongholds never fall.
- **Background, not foreground.** Territory, adaptation and faction life are slow systems
  you notice over a session, not something you manage.
- **No physical traces.** Nothing is left lying in the world: journals, burned towns and
  looted shelves are data.
- **One move waypoint per AI group.** Every order the mod gives a group (a horde toward noise, a
  wanderer's walk, a truck's route, a mission's crew) moves the same waypoint and puts it back on
  the group's list, after any waypoints of the group's own. It's deleted with the group, or on the
  next tick when a group dies out by itself. Code: `Core/RZ_GroupOrders.c`. Measured with Bacon
  Zombies (`tools/zombie_lab/`): a finished waypoint drops off the group's list but stays in the
  world, and moving one that's still on the list does nothing until it's added again. So before
  this, every order left a waypoint behind. The status line counts the waypoints in the world.
- **Nothing for free.** No free loot, no free Fame. No sleep and no time skip.

## Settlements, spawn points and the map

Code: `World/RZ_SettlementScanner.c`, `World/RZ_Settlement.c`, `Core/RZ_GameModeComponent.c`, `Core/RZ_MapMarkers.c`, `UI/RZ_MapIcons.c`.

- **Settlements** come from the map's city, town and village labels plus clusters of buildings
  (a 300 m grid cell with 8 or more buildings). A cluster takes the name of the nearest local
  place label within 400 m (a hamlet or a point), or "<name> outskirts" if that's taken, or a
  grid reference ("Outskirts 13-29"). Label names are string-table keys, translated on the
  server. Each has a centre, a size class and an infection level (0–100). Placing any `RZ_SettlementMarkerEntity` turns detection off and uses the markers.
- **Spawn points:** 6 at a time in random towns, and one is moved to a new town every
  20 minutes. Every Haven safehouse is also a spawn point for everyone.
- **Missions are announced** with distance and direction ("1.2 km NE"). Messages arrive a few at
  a time, the most important first, as styled notice cards or vanilla hints without the UI; the
  notebook's Radio tab keeps the radio, mission and world ones ([UI.md](UI.md#notices)).
- **The map** shows each player what they're allowed to know, with the ReforgerZ icons: the lab,
  the safehouses (not to exiles), every stronghold, the outposts of factions that know them or that
  they've been near, the power and radio stations, the sealed checkpoints and world events.
  **Contracts are never on the map**: they give a rough area in the journal. This needs the icon
  layer ([UI.md](UI.md#the-map)); without it, world events and the lab are vanilla text markers
  for everyone (a toggle).

### The land grid

Code: `World/RZ_Land.c`. **Off by default** (**Zombies > Tribes > Land survey**), and nothing uses it
yet: it's the ground the infected tribes will live on. The tribes are planned; their design and the
land grid's full description come with their own doc, `docs/INFECTED.md` (planned).

- **What it holds.** The heat map's 250 m squares. Each has its height (five samples), how much of
  it is sea, river or lake (the engine's water query at those samples), whether a river runs
  through it (the map's river entities, with their widths), how many trees (tree prefabs only, not
  bushes) and buildings it has, any barn, mill, silo, quarry, culvert, shed or ruin, its widest
  road, and any bridge or ford.
- **Bridges are inferred.** Road points sit on the ground even at a bridge, so a bridge shows only
  as the straight line between two road points running 3 m or more above the ground. Over a river
  or pond it's a bridge over water; a road through river water with no deck is a ford.
- **Worked out from it:** ridges (25 m above the land around, or the highest within 1 km), valley
  floors (15 m below both squares across them, 250 or 500 m away; three or more touching make a
  valley line), forests (600 or more trees in a square, in groups of three), fields (open, flat
  and near a town), lair candidates (forest, or one of the landmarks above, 600 m from every town
  centre and 800 m from where people live; 400 and 600 m on maps under 25 km² of land), and **wild
  land**: dry squares outside every settlement's radius + 100 m, 300 m round every lived-in place
  (a lived-in town's edge, or 100 m round a holding, safehouse or the lab), 700 m round the Long
  Table's farm, and the Perimeter's warning zone + 200 m.
- **Reading it.** At the first start on a map, from the game mode's one-second tick: up to 20 land
  squares a second, stopping after 8 ms. It's cached in `$profile:ReforgerZ/land_<map>_<number>.json`
  (the number comes from 64 terrain heights) and loads in milliseconds after that. Wild land, fields
  and lair candidates are worked out again on every load, from where people live then. Roads come
  from the world's `SCR_AIWorld`; without one there are no roads and no bridges.
- **Tribe slots** (for the planned tribes): 1 per 6 km² of wild land, rounded, 1 to 15; none under
  1.5 km² of wild land or with no lair candidate.

**Measured** (`tools/land_survey/`, four to six starts on each map; the wild land changes a little
with the holdings, safehouses and Long Table of the season):

| | Arland | Everon | Cain |
|---|---|---|---|
| Squares | 289 (17 × 17) | 2,652 (51 × 52) | 2,756 (52 × 53) |
| Land / wild | 6.3 / 0.4–1.1 km² | 51.0 / 23.8–25.8 km² | 39.0 / 19.7–21.3 km² |
| Tribe slots at 1 per 4 / 6 / 9 km² | 0 / 0 / 0 | 6 / 4 / 3 | 5 / 3–4 / 2 |
| First reading | 39 s (0.3 s of work) | 4.5 min (2.0 s of work) | 4.8 min (2.4 s of work) |
| Longest second | 12 ms | 14 ms | 23 ms (one dense square) |
| Cache | 6 KB, loads in 1 ms | 51 KB, 3 ms | 53 KB, 3 ms |
| Rivers, lakes | 3 (0.9 km), none | 28 (14.9 km), 2 | 7 (4.3 km), none |
| Bridges inferred (real), fords | 4 (4), 0 | 22 (19), 2 | 15 (14), 3 |
| Ridge posts, valley lines | 3, 1 | 71, 10 | 51, 6 |
| Lair candidates | 2–8 | 34–35 | 77–81 |

- **Arland has almost no wild land.** The margins of its 14 settlements leave 2.6 of its 5.9 km² of
  dry land, lived-in places 1.0–1.3, and the Long Table's farm and the line take most of the rest.
  It gets no tribe without the planned **Slots** override.
- **Everon gets 4 tribes and Cain 3 or 4** at 1 per 6 km², each with one slot kept for a split.
- **What eats the wild land** is the settlements, not the line: on Everon their margins take 19 of
  its 49 km² of dry land, lived-in places 5 more, the Long Table and the line under 1 each.
- **The server's memory** doesn't move measurably (Everon +4 MB, Cain +15 MB over the reading, in
  the noise): the grid is about 75 KB.

**What it can't see:** ponds and streams smaller than about 100 m between the samples; rivers made
any other way than as river entities; bridges whose deck is under 3 m above the ground, or that no
road crosses (castle and pier bridges). Of the bridge prefabs a road crosses it finds 4 of 5 on
Arland, 20 of 33 on Everon and 14 of 22 on Cain (some misses are other parts of a bridge it found).
Three 4 m tracks over gullies on Everon read as bridges, and one low castle bridge as a ford. Nor
does it see cliffs, banks and forest edges inside a square; trees whose prefab path doesn't say
"tree" (a Workshop map's own trees read as open land); or a map update that adds objects but leaves
the terrain alone (delete the cache).

## The Director: outbreak stages and spread

Code: `Director/RZ_Director.c`, `Director/RZ_OutbreakStage.c`.

The server moves through four stages over a season of about 45 real days ([Season pacing](#season-pacing)).
A stage ends when its time is up or when average infection passes its threshold. **The
outbreak's clock only runs while someone is playing**: stage time and spread stand still on an
empty server. Stage lengths are divided by the **season speed** and spread is multiplied by it.
Each stage's numbers can be overridden on the game mode. The days below are for 5 regulars
(about 8 hours a day with someone online).

| Stage | Lasts (hours played) | Or when average infection reaches | Density | Special chance | Spread / min | Missions | Bastion presence |
|---|---|---|---|---|---|---|---|
| Early Outbreak | 40 h (about days 1–5) | 45 | ×0.6 | 1% | 0.01 | 3 | ×1.0 |
| Collapse | 80 h (days 6–15) | 65 | ×1.0 | 1.5% | 0.016 | 4 | ×0.6 |
| Overrun | 120 h (days 16–30) | 85 | ×1.5 | 2% | 0.02 | 4 | ×0.25 |
| Aftermath | until the cure (from about day 31) | — | ×0.8 | 2.5% | 0.006 | 5 | ×0.1 |

- **Spread.** Every town creeps up slowly. A town at 60+ infection also spreads to every
  town within 2.5 km, in proportion to its own infection. Cure breakthroughs slow spread.
- **Pushing back.** Each zombie kill takes 1.5 infection off its town. Missions clear,
  burn, seal and shell towns.
- **Missions at once:** the stage's count, plus one per 4 players. "Bastion presence"
  (`RZ_WorldFacts.BastionPresence`) is how much of Bastion is still out on the roads: some
  Bastion contracts need a minimum of it and three of them scale with it. *Planned:* every
  Bastion contract and the road posts scaled by it.
- Each stage change is announced to everyone in a line of the Zone's voice, except the move to
  Aftermath that comes with the cure (the cure's own news is the line everyone hears).

## Heat and noise

Code: `Director/RZ_HeatMap.c`.

Heat is how much attention an area has drawn, on a 250 m grid. It decays 8 a minute and
caps at 200 per square.

| Source | Heat |
|---|---|
| A player being somewhere | 0.5 a tick |
| In a vehicle | +4 a tick |
| A shot | 3 × the background's noise multiplier; ×0.3 with a suppressor (detected by attachment name) |
| Noisemakers, bells, sirens, generators, car boot and body searches, forcing locks, infected at barricades and locked doors, fire | see [World interactions](#world-interactions) |

- Heat draws extra zombies. **80** heat in one area within 600 m of a living player sends a
  horde toward it (at most one every 10 minutes). A hot square nobody is near (a powered
  station, the Preacher's bells, a running generator) draws nothing.
- **Noises that lead.** A burst of 8+ heat at once is remembered as a noise (the last 16).
  Infected that a mission controls walk to a nearby noise and stay about 90 s. That's what
  the unarmed missions are built on ([MISSIONS.md](MISSIONS.md), "Unarmed missions").
- **Being noticed** (puzzles and unarmed missions): heat where you stand above a threshold,
  or an infected within 10–12 m of you for 6–8 s (slower while you're hard to see:
  [Stealth](#stealth)).
- Other mods can add heat with `RZ_GameModeComponent.ReportNoise(pos, heat)`.

## Stealth

Code: `Zombies/RZ_Stealth.c`. Settings: **Zombies > Stealth**.

How visible each living player is, worked out on the server once a second (not per frame), and
what the infected near them make of it. It works with any zombie mod. If the mod's infected use
the engine's perception, stealth changes how fast they recognise a player and reads who they're
after. If they use a plain aggro radius, their senses don't change and losing them goes by
distance. **Stealth off:** infected perception is never touched, nobody counts as hidden, and
missions work exactly as before.

### Visibility

Every living player has a visibility from 0 (hidden) to 1 (in plain sight). It's the stance and
speed times the surroundings (light and weather), plus recent noise, capped at 1.

| Input | Default |
|---|---|
| Stance | standing ×1, crouched ×0.6, prone ×0.5 (stealth can't see cover, so lying in the open in daylight isn't hidden on its own) |
| Speed (how far you moved since the last update) | still (under 0.4 m/s) ×0.7, walking (under 2.6) ×0.85, running (under 5) ×1, sprinting ×1.25 |
| Light | 60% the engine's light on you (sun, moon, lamps, your own torch) and 40% the clock (day 1, dusk 0.6, night 0.25). Turned into a multiplier from ×0.4 in the dark to ×1 in daylight. If the engine's light can't be read, the clock alone |
| Weather | fog or a fog bank ×0.75; rain ×0.85, or a storm ×0.7 instead; the Fog rule set's shorter sight (×0.85) |
| Surroundings floor | light × weather never counts for less than ×0.55, so darkness and bad weather alone never hide someone moving upright: only getting low and keeping still do |
| Noise | each shot adds to your noise and shots add up: one shot fills it, a suppressed one adds 0.35 (so about three in a row fill it too), times your background's noise (Hunters quieter). Full noise is +0.5 visibility, fading over 10 s. A loud noise within 15 m (a bell, a forced lock, a noisemaker at your feet) sets it to at least 0.7. While any noise is left, the infected can't lose you |
| In a vehicle | always 1 |

Some results: walking in daylight 0.85; crouched and still in daylight 0.42; prone and still in
daylight 0.35 (in rain 0.3); walking at night about 0.47 (never less, whatever the weather);
crouched and walking at night 0.28; prone and still at night 0.19. Under **0.35** counts as
hidden: that's the HUD's HIDDEN, and the only state in which the infected can lose you. Other
systems ask `RZ_Stealth.GetVisibility(playerId)` (1 with Stealth off) and
`RZ_Stealth.IsHidden(playerId, threshold)` (always false with Stealth off). A new life starts at 1.

### The infected near you

Every update, for the infected the spawner tracks (groups and their members) with a player within
**60 m** of a member, each member's perception factor is set to:

> its own factor × the base for its kind × a factor from the **most visible** player within 150 m of any member of its group, never more than ×1.2 on top of its own

- **Base by kind:** common ×1, special ×1.2 (sharper), horde member ×0.9 (a crowd on the move
  looks ahead).
- **From the player:** ×0.3 for someone fully hidden, rising to ×1 at 0.6 visibility. From 0.6 to
  0.9 (standing or walking in daylight) it's ×1, as without stealth. Only above 0.9 (running or
  sprinting in daylight, just after a shot, a vehicle: the HUD's IN PLAIN SIGHT) does it rise, to
  ×1.2. Prone and still at night gives about ×0.52, crouched and walking at night ×0.63, prone and
  still in daylight ×0.71, walking at night ×0.85.
- **The most visible player counts**, out to 150 m, not just the ones within 60 m. A friend lying
  prone close to them can't hide you while you stand further off.
- With nobody within 60 m, a group goes back to its base. At most 40 infected are touched an
  update; the rest wait their turn.
- **Never lowered for infected that must keep coming:** every horde tied to a mission (the holds
  and waves, and most event and questline hordes), and everything within 1.5 km of the lab during
  the final stand. They keep their own factor. Three hordes are the exception, because staying out
  of sight is the point there: Lead Them Away, The Clockwork Horde and the Horde Migration event.
- With the vanilla `SCR_AICombatComponent` on the infected, the factor is set as its multiplier.
  The AI's own update applies it on top of its alert state, and would overwrite a factor set
  straight on the perception. If that component has no vanilla utility AI behind it (a mod's own
  agent), nothing else updates the perception, so the factor goes straight on the
  `PerceptionComponent`, as it does without the component. Without a `PerceptionComponent` nothing
  changes. Perception that still refuses the factor after 5 updates in a row counts as none. Each
  case is logged once.
- The factor only changes how fast they recognise what they see. Hearing is the engine's own. The
  engine already sees stance and light, so stealth's factor comes on top, which is why it never
  goes below ×0.3.

### Losing them

A group that is after a player can lose them:

1. **Who it has:** the player among its members' engine targets it has recognised as an enemy
   (not just a hint of someone), seen or heard most recently. For infected without engine
   perception: a member touching a player (4 m), a member within 20 m of a player who has just made
   noise, or the group coming at a player within 20 m at a run (2 m/s or more, faster than toward
   where a horde is headed, and not while walking where stealth sent it). It stays on that player
   for up to 53 s (8 + 45) after.
2. **Lost:** that player has stayed hidden (under **0.35**), unseen, unheard and with no noise left
   for **8 s**, counted from when the group last had them. A group that had a player a while ago
   and isn't after them now has nothing to lose, and a contact it has already searched from is
   never lost twice.
3. **Search:** the group walks to where it last had them. A group with nothing else to do then
   looks about for **45 s**, a new spot 5–20 m away every 10–15 s. A group with waypoints of its
   own (a horde on its way) carries on with them once it has been there. If it sees or hears
   anyone in that time, its own AI takes over.
4. **Give up:** stealth's waypoint goes, and the group carries on with its own. One with none left
   goes back: a horde to where it was sent, anything else toward where it spawned.

Stealth's waypoint is put in front of the group's own and taken back afterwards: nothing of the
group's (a zombie mod's wander or patrol, a busy-spot visit) is removed. Never for mission infected
(their mission steers them) or the final stand. It needs a group and the move waypoint (**Zombies
> Move waypoint**); without them only the senses change. The waypoint stealth gives a group is
deleted with the group.

### Stealth in missions

Missions use the score only to go easier on someone lying low, never harder. A player at
visibility 0.6 or more (standing in daylight, even keeping still) is seen exactly as before, by
these checks and by common infected (who only see better than before someone in plain sight).

| Check | With Stealth on |
|---|---|
| Being noticed ([Heat and noise](#heat-and-noise)): puzzles, unarmed missions, the questlines' sneak | The heat limit is unchanged. The "infected close to you for 6–8 s" timer runs at full speed at 0.6 visibility and up, down to 0.35 of it for someone fully hidden: a little over half as fast lying still in the dark |
| People who see anyone within N m: Haven lookouts and the Haven wanderers near them, the storekeeper, the walker, Halcyon's courier, the Long Table's family, Quiet Count's specials (50 m) | N at 0.6 visibility and up, down to 0.6 × N for someone fully hidden: Quiet Count's 50 m is about 36 m lying still in the dark |

Shots nearby still spoil a sneak. In a questline stage, a person who sees anyone within N m checks
`RZ_Stealth.SightRadius(playerId, N)` (see [Writing a questline](#writing-a-questline)).

### The HUD

Above the noise meter, five segments and a word: **HIDDEN** (under 0.35: the infected can lose
you), **LOW**, **SEEN**, **IN PLAIN SIGHT** (0.9 and up: the infected see you better than before).
It's sent only when it changes, and not shown with Stealth off ([UI.md](UI.md#screens)). There's no
cue when a group loses you or gives up: you see them go.

*Simplified:* speed is how far you moved between updates, so a sprint that stops just before an
update reads as still. The stance and the engine's light are read on the server (both on the
VERIFY list). Stealth has no cover input: a wall or a hedge only counts through the engine's own
sight. Infected without engine perception are judged "on you" by distance and by how they move,
so a group that runs at you along its own path (a horde coming straight down the road you're on)
isn't counted as chasing, and never loses you. A lone infected character that isn't in a group
never searches, because it has no waypoints.

## Zombies, bites and infection

Code: `Zombies/RZ_ZombieSpawner.c`, `Zombies/RZ_BiteDetector.c`, `Core/RZ_GameModeComponent.c`.

- **Spawner.** Works with any zombie mod's prefabs. Towns within 600 m of a player are
  populated (4 per size class at 100% infection), at points 120–600 m from a player (so a
  spawn is never removed straight away), and removed beyond 1.1 km. Two roaming infected stay around each player in the countryside. At most
  60 tracked spawns on the server and 4 per tick.
- **Specials** (the stage's chance) drop a cure sample when killed.
- **Zombie-free settlements** while a mission needs one (The Silent Village).

A bite is survivable if you act on it. Code: `Zombies/RZ_BiteDetector.c`, `Cure/RZ_Infection.c`, `Cure/RZ_Meds.c`.

### Bites

Works with any zombie mod: a player's health dropping with an infected within 2.5 m is a hit.
**Most hits are scratches** ("Nails, not teeth", told at most once a minute). Each hit is an
infecting bite with the **Survival > Bite chance** (10%), changed by what's happening:

| Situation | Bite chance |
|---|---|
| Surrounded: 3 or more infected within 5 m | ×2 |
| Grabbed: a second hit within 4 s | ×1.5 |
| Tired: stamina under a quarter (needs the stamina hook) | ×1.5 |
| Hurt: under 40% health | ×1.5 |
| Wearing a jacket / gloves / a helmet (worn prefab names) | ×0.8 / ×0.8 / ×0.85 |
| Never above | 60% |

A careful player in a jacket and gloves who isn't cornered is bitten about one hit in sixteen;
someone pinned by three infected at low health, nearly half the time (60% at most).

### The golden window

A bite **doesn't take for 2 minutes**, and the timer stands still. Cleaning it in that window can
stop it, once per bite:

| How | Stops it |
|---|---|
| "Clean the bite" with alcohol or antiseptic (prefab-name keywords) | 70% |
| "Disinfect and bandage it" (disinfectant or antiseptic and a bandage, or a first-aid kit) | 90% |
| Another player's **Treat their wounds** | 90% |
| Declaring the bite at a safehouse | 90% |

The "Clean the bite" spot (an interaction spot) follows the bitten player's feet and is never
refused by the spot cap; anyone can use it with their own supplies, and the bitten player has to
stay next to them until it's done. Nobody is told whether it worked until the window closes: then
"it didn't take", or the infection starts. Until then the HUD still shows the window, and meds,
an antidote and a safehouse answer the same either way (an antidote used then is spent).

### Infection stages

Once it takes, **60 minutes** to turning (**Backgrounds > Bite minutes**) × the background's
multiplier (Nurse ×1.5, Doctor ×2.5, Ex-Con ×0.8) × the cure's (×1.25 after 25%, ×1.5 after 75%).

| Stage | When | What it does |
|---|---|---|
| **Fever** | the first half | A line every 4 minutes. Nothing else |
| **Weakness** | the second half, until Final | Stamina drains every 30 s (fallback: a little noise, laboured breathing). Players within 15 m are told every 3 minutes that you look sick. Safehouses can catch a hidden bite (half chance) |
| **Final** | the last 10 minutes (at most 30% of the whole) | Stamina drains every 20 s (fallback: noise). A countdown every minute. Others within 15 m see it every 2 minutes; full chance to be caught in a safehouse |

Meds and safehouse slowing add time, so a player can drop back a stage. A player who turns
becomes the **Turned Player** world event: they roam with their gear.

*Simplified:* stamina is drained on the server (whether the player feels it is a VERIFY);
without it the stages make noise instead. "Visible to others" is a message to players nearby, not a change to how you look.

### Treatments

Bite treatment exists from the start of a season. Cure research only changes **supply, price
and odds**; the world-level breakthroughs (spread, timers, the final stand) are in
[Cure research](#cure-research-and-the-seasons-end).

| Treatment | Where | Does |
|---|---|---|
| **Antidote** | Halcyon's quartermaster (Trusted), from the lab's stock; the Red Hand's stolen doses; very rare finds | Cures |
| **Trial dose** | Halcyon's quartermaster, "Volunteer for the trial dose": anyone bitten, free, once per life | Cures 80% of the time; otherwise the time left is halved. +3 Halcyon Fame either way. Refused to people Halcyon hunts |
| **Stabiliser** | Halcyon's quartermaster (Known), from a small stock; rare in pharmacies and hospitals | Pauses the infection for 20 minutes, once per life |
| **Declaring the bite** | Any safehouse | Once per life: time left ×1.15 (Shelter), ×1.25 (Community), ×1.4 (Settlement), ×1.5 (Town). A fresh bite is cleaned instead; one cleaned already uses nothing ([details](REPUTATION.md)) |
| Doctor wanderer, Nurse or Doctor player | On the roads; "Treat their wounds" | ×1.25 / ×1.15 of the time left, once per life each |

**Supply by research:**

| Research | Antidote stock | Stabiliser stock | Stabiliser price | Finds per pharmacy's loot (antidote / stabiliser) |
|---|---|---|---|---|
| 0% | 1 per 30 min, 2 at most | 1 per 90 min, 1 at most | ×2 | 0.4% / 1% |
| 25% | 1 per 20 min, 3 | 1 per 45 min, 2 | ×2 | 0.8% / 1.5% |
| 50% | 1 per 15 min, 4 | 1 per 45 min, 2 | ×1 | 1.2% / 2% |
| 75% | 1 per 10 min, 5 | 1 per 15 min, 5 | ×1 | 1.6% / 2.5% |

Finds are doubled behind a lock. Infected bodies (some were Halcyon couriers) have half the
pharmacy's antidote chance. Power the Lab still doubles antidote production for 2 hours.
Players Known with Halcyon hear when a dose is ready.

### Low-grade meds

Buy time, never a cure (except a lucky cut dose). **Each works once per life**, and between them
they add **30 minutes at most** (a stabiliser doesn't count). Used like the antidote: drop it and
"Take" it, or from the inventory as a consumable. Each has a guidance tip on first use.

From the inventory, the antidote and the meds only work on yourself, and only while your HUD shows
a bite (meds: once it has taken), so a healthy player can't use one up. The server still refuses a
med already taken this life; from the inventory that dose is lost (from the ground it isn't).

| Med | Where | Does | Side effect (10 min) |
|---|---|---|---|
| Expired antibiotics | Common: pharmacies, houses, bodies; every safehouse hands them out for 5 Haven Favor | +10 min; 30% nothing | Nausea: a burst of lost stamina every 90 s (fallback: a little health) |
| Veterinary antiviral | Farms and vet clinics | Pauses 20 min | Coughing fits: 12 heat every 60–90 s |
| Red Hand cut dose | The Red Hand's quartermaster, Known, cheap | Cures 50%; otherwise time left ×0.6 | Fever: 1.5% health every 30 s, never below 30% |
| Fever suppressant | Pharmacies, some shops | Nothing to the timer. Hides the symptoms for 30 min: nobody nearby sees them and a safehouse can't catch the bite | — |
| Home remedy | Brewed on a stove from alcohol and herbs | +5 min | Drunk for 5 min: sway and blur (fallback: stumbling, stamina and noise) |

*Simplified:* there's no known script hook for sway or blur, so the home remedy always uses the
fallback. The side effects use health, stamina and noise, not animations.

## Infection adapts

Code: `Director/RZ_Adaptation.c`.

Small, capped changes that keep the late game interesting without making it unfair.

- **Specials after breakthroughs.** Each cure breakthrough multiplies the stage's special
  chance by 1.05, ×1.15 at most. A relative bump: an Early Outbreak 1% becomes 1.15%.
- **One new trait per stage**, and they add up: Collapse swaps 15% of common spawns for
  **runners**, Overrun puts **helmeted** ones in the same 15% (runners or helmets at random),
  Aftermath makes 10% of specials **screamers**. A screamer within 40 m of a player shrieks:
  +20 heat where the player stands, at most once a minute, heard within 150 m. Each trait is
  a prefab list; an empty list turns it off (screamers then use the ordinary specials).
- **Favourite spots.** The map is split into 500 m squares that count player-minutes
  (fading by a fifth every 10 minutes). Once a square reaches 35 (about an hour of one player
  staying put; a lone player levels off near 50), three infected are spawned
  out of sight 250–350 m away and walk over to look, at most once an hour per square. Never
  within 150 m of a safehouse, and never near a brand-new player in their gentle start.

*Simplified:* "sees" is distance only, with no line of sight. Runners and helmets are
whatever the prefabs are; the code doesn't change speed or armour itself.

## The Perimeter

Code: `World/RZ_Perimeter.c`, `World/RZ_CheckpointMarkerEntity.c`. The story is in [LORE.md](LORE.md), "The Perimeter".

The edge of the playable world is the quarantine line, on any map. It's worked out from the
world's bounds, pulled in to the outermost dry land plus a 200 m sea margin (plus one step of the
coarse land sampling, so the real margin is never less), so nothing is
placed by hand. **Nothing is built along the edge** except the road checkpoints.

- **Warning zone** (300 m from the edge): a drone appears above *you* and follows you, and a
  loudspeaker says *"You are approaching the quarantine line. Turn back."* It leaves when you
  do. Never more than one drone per player, and none anywhere else. On foot it's attached to you;
  in a vehicle or a boat it's moved over you on each tick instead.
- **Kill zone** (100 m): a warning shot (a sound and a near miss), then 8 seconds later you're
  shot by a sniper you never see.
- **Sealed road checkpoints.** Where main roads reach the edge (up to 6 by default, 8 at most;
  island maps may have none), the road ends in concrete blocks and a closed barrier, with 0–4
  guards behind it. Nobody passes in either direction. Within 150 m the loudspeaker warns you;
  closer than 60 m the guards shoot. The guards stand outside the playable area and can't be
  hurt. Checkpoints come from the road network, or from placed `RZ_CheckpointMarkerEntity`s.
- **Blind spots.** The drones run a rotation; The Blind Spot mission opens a 250 m gap for
  ten minutes ([MISSIONS.md](MISSIONS.md), "Perimeter missions").
- **Sea edges.** Where the edge runs out at sea it sits at least the sea margin offshore, so
  the kill zone is water and the warning zone only reaches the beach where the land comes
  within 300 m of the line. Swimming or sailing out gets you warned and shot as on land.
- **Missions at the line** measure from where the land starts, not from the edge. The first
  time one is considered, points every 100 m along the edge (away from the corners, at most
  400) are walked inward in 10 m steps to the first dry ground, up to 600 m; points with no land
  that close are left out. A mission picks among these, preferring spots under the drones;
  one that finds nowhere on a map is never offered there (worked out once per run).
- Other missions, spawn points and quartermasters keep out of the warning zone.
- **After the cure the line stands down** ([LORE.md](LORE.md#endings)): no drone comes down,
  nobody fires a warning shot or a sniper round, at the edge or at a checkpoint. In the warning
  zone the loudspeaker reads the new order instead (*"The quarantine order for this area has been
  lifted. Remain where you are. Processing will begin at the road checkpoints."*), once per
  approach. The checkpoint barriers and their guards stay: the gates open with the wipe. The line
  isn't open either: whoever walks into the kill zone, or up to a barrier, is turned back to the
  warning zone, 40 m clear of it (a voice from past the line, no shot, no drone). The Perimeter
  missions, Learning the Drones, the basement spawn scene and the perimeter tip stop with it
  (`RZ_WorldFacts.PerimeterSealed`); one running when the cure comes out is called off.

## Time and environment

Code: `Director/RZ_Environment.c`, `Puzzles/RZ_GameTime.c`, `World/RZ_PowerGrid.c`, `World/RZ_RadioStation.c`, `Director/RZ_Broadcast.c`.

Time is never skipped or changed. The environment is read every 30 s and nudges other systems.

### Night

The game clock's night is 20:30 to 05:00.

- **They hear further:** a quarter of every bit of heat also reaches the four neighbouring squares.
- **They see less:** you're harder to see in the dark, unless you carry a light ([Stealth](#stealth)).
- **Specials** are ×1.25 as common.
- **Human AI patrols less:** each holding shows one guard fewer (never under one), standing
  closer to its centre; half the hunter-squad rounds are skipped.
- Some missions only run at night ([MISSIONS.md](MISSIONS.md)).

### Weather

- **Rain** (0.3+) dampens noise: heat added ×0.7.
- **Fog** (0.4+) is the engine's own fog; infected may spawn a little closer (×0.75 of the minimum distance).
- **Storms** (the Storm event, or rain 0.6+ with wind 10 m/s+) ground Bastion's helicopter
  (no Last Helicopter contract), and wanderers shelter.
- Players hear one line when rain or fog sets in, at most every 20 minutes each.
- Fog, fog banks, rain and storms also make you harder to see ([Stealth](#stealth)).
- **Fog banks** (the Fog Bank event): one region at a time where heat added is ×0.6 and infected
  may spawn twice as close. The engine's fog is global, so it isn't touched; an optional mist
  prop is the only look.

*Simplified:* weather is read from the time and weather manager (rain and fog 0 to 1, wind in
m/s); on a map without one, only night and the Storm event apply.

### The power grid

One power station per map: a placed `RZ_PowerStationMarkerEntity`, else a real power or
industrial building near a town, else the edge of the biggest town. Its control object
exists while a player is within 300 m. Restoring it takes three steps:

1. **Force the control room door** (loud).
2. **Reset the breakers and start the generator:** a horde is sent at it, and someone has to
   stay within 60 m for 90 s while it spins up (nobody there for 30 s and it dies).
3. **Throw the main switch.**

Then every town within 2.5 km is **Powered** and everyone hears it. Power gives a lights hook
(`RZ_PowerGrid.IsPoweredAt`; no lamps are actually switched on), more radio chatter (every
6 minutes instead of 10), faster radio station broadcasts, and electronics ×1.5 as likely in
loot there. But the generator hums: 10 heat a minute at the station. It **slowly fails**
over about four hours with players online (a warning at 20%). **Refuel and grease** restores
60% with a fuel item or 25% without. When it fails, it's back to step 2. No Fame for any of
it: the power is the reward.

**Power surges** (the Power Surge event): one dark town outside the region gets power for 10
minutes, with the same lights hook and electronics bonus, and no generator to keep running.

*Simplified:* the three steps are actions on one object, not real breakers.

### The radio station

One broadcast point per map: a placed `RZ_RadioStationMarkerEntity`, else a real radio mast
near a town, else high ground near a mid-sized town. Take it over by staying at the console
for 30 s with no infected within 15 m; it's yours until you walk 150 m away. Then send one of
four preset messages to everyone, signed with your name and filled in with real places: a
horde warning, food for the hungriest safehouse, meet here, and "still here". One broadcast
every 10 minutes server-wide (5 with power). **Cosmetic and social only:** no Fame, no Favor.

### The government broadcast

A looping flavour message to everyone every 25 minutes and on each player's first spawn.
Its tone follows the cure and the outbreak ([LORE.md](LORE.md), "The outside world: silent, watching").
Its day count ("Day 14") is 11 plus the real days since the season started, so a season of
about 45 days runs from Day 11 to about Day 56; without the server's clock, a day is 8 hours
played. Notes, journals, the radio station and Bastion's orders use the same count.

## Backgrounds and personal goals

Code: `Backgrounds/RZ_Background.c`, `Backgrounds/RZ_Objectives.c`, `Backgrounds/RZ_PlayerProfile.c`.

You pick a background in the vanilla deploy menu (each is a loadout); a panel there shows its
perk, flaw, starting gear and whether it's locked ([UI.md](UI.md#screens)). Gear comes from the
loadout; the perk and flaw change ReforgerZ systems, so they work with any other mods.
Starting standing is Fame (+) or Infamy (−) ([REPUTATION.md](REPUTATION.md)).

| Background | Perk | Flaw | Starting standing |
|---|---|---|---|
| Nurse | Medical supplies. Treats other players in 8 s, even with a bandage. Infection takes 50% longer | Unarmed | Haven +10 |
| Mechanic | Wrench and jerrycan. Hotwires a car in 15 s and rarely fails | 25% louder | — |
| Hunter | Hunting rifle, 25% quieter | Little ammo | Bastion −10 |
| Police Officer | Sidearm | — | Bastion +20 (Known), Red Hand −25 (Watched) |
| Ex-Con | Knife | Infection kills 20% faster | Red Hand +25 (Known), Bastion −25 (Watched), Haven −10 |
| Lab Technician | Samples worth double research | Unarmed, 15% louder | Halcyon +20 (Known) |
| Radio Operator | Radio; horde warnings and intel map-wide; cracks secure channels | 20% louder | Haven +10 |
| Unemployed | +50% Fame from missions | Starts with nothing | — |
| **Soldier** (Champion with Bastion) | Service rifle and armour | — | Bastion +20, Red Hand −15 (Watched) |
| **Doctor** (Champion with Halcyon) | Infection takes 2.5× as long; treats others in 6 s | Unarmed | Halcyon +20 |
| **Raider** (Champion with the Red Hand) | Black market open from the start | — | Red Hand +40 (Trusted), Haven and Bastion −15 (Watched) |
| **Elder** (Champion with the Haven) | Trusted by the Haven from the start | 30% louder | Haven +40 (Trusted) |
| **Drifter** (10 Legacy points) | Known with every faction | No gear | +15 with all four |

The last five are unlocked through **Legacy**, which carries across lives. The server won't
assign or spawn a locked background (**UI > Lock loadouts**, on by default), whatever the client
asks for, so it holds with a gamepad or a modified client too; the deploy menu also shows it as
unavailable. With the lock off, or if a locked pick gets through another way, the player keeps
its gear for that life but plays as Unemployed, and is told how to unlock it.

**Personal goals.** Each life gets one goal, flavoured by background (the Hunter heads for
their cabin, the Ex-Con goes back for an old stash): reach a town 1.2–4 km away, survive a while,
deliver samples, take part in missions, or become Trusted with a faction. Completing it gives a
**Legacy** point and 10 Haven Fame, never loot, so a fresh life can't farm crates. A player's first life that lasts 10 minutes is always
**Find the Haven**: reach a safehouse ([New-player guidance](#new-player-guidance)).

## Factions in the background

How the four factions hold the map, move supplies and change over a session. What they
offer players, and how morale, needs, truces and feuds work, is in [REPUTATION.md](REPUTATION.md).

### Territory

Code: `Factions/RZ_Territory.c`.

- **Most of the map is abandoned.** Expansion stops once about a fifth of all settlements
  are held (strongholds included). The rest are empty, looted or infested: that's where the
  scavenging, the missions and the danger are.
- **One Stronghold per faction**, at its quartermaster (Halcyon's at the lab). It can never be
  captured or destroyed. At worst it's **Under Pressure** for 25 minutes: while a faction's
  strength is 25 or less, or its morale 20 or less, each territory tick has a 50% chance to
  start it. No supply truck leaves, and the stronghold's Champion shop tier is locked. It ends
  on its own (+5 strength, +5 morale), with a two-hour pause before it can happen again.
- **Outposts, by map size:** 1 per faction under 24 settlements (Arland), 2 under 48, else 3
  (a setting overrides it). Halcyon never has more than one: its field station. Each faction
  can grow one past its starting count.
- **Towns that make sense.** "On the roads" is read from how many towns lie within 1.6 km, so
  it works on any map:

  | Faction | Prefers |
  |---|---|
  | Bastion | towns (size 2+), hubs, names with "air", "base", "field" or "port" |
  | Red Hand | hubs (crossroads) and small places (outskirts), some infection |
  | Haven | quiet villages: small, low infection, few neighbours, away from the Red Hand |
  | Halcyon | quiet, low-infection places |

  Never: a town already held (or within 400 m of a holding), a safehouse town, the lab town,
  a town within 1.5 km of the [Long Table](#the-long-table)'s farm,
  a town at 85+ infection, or anything near the quarantine line. Not under 700 m from the
  faction's own stronghold, and rarely over 6 km.
- **The territory tick**, every 45–75 minutes while players are online (divided by the season
  speed), does at most **one**
  thing. First the floor (a faction with no outpost founds one), then each faction in random
  order by its strength:

  | Strength | Does (first that applies) |
  |---|---|
  | 60+ | win back a town it lost (after 30 min, within 6 h; the Haven and Halcyon only re-found one that is empty again); found an outpost near its holdings if under its count; at 65+ take the nearest outpost of a neighbour at least 10 weaker (any weaker during a feud), within 4 km |
  | 41–59 | under its count: found one (half the time); else 20%: drop an outpost with almost no traffic that has stood 3 hours |
  | 40 or less | 60%: drop its quietest outpost |

  The Haven and Halcyon never take towns. Nobody takes or drops an outpost a player is near,
  a faction's last outpost, a placed `RZ_OutpostMarkerEntity`, or one from a faction it has a
  truce with.
- **Traffic** is player-minutes within 400 m, plus 5 for each contract done for the faction
  within 1.5 km; it fades by a quarter every tick.
- **Faction strength** (10–90, starts at 50): +2 for the giver of a finished contract, −1 for
  a rival it hurt, −1 for a failed one (at most 6 an hour from contracts); +1 per truck
  delivered, −2 per truck lost; +3 / −5 when a town changes hands. It drifts 2 points an hour
  back toward 50.
- **Headlines only.** Every change is a headline, never a server-wide alert.
- **Bodies only near players.** Within 300 m of a holding: its guards, two of its three
  **named people** (the cook, the sentry, the toll man...) and, at an outpost, the faction's
  quartermaster. Named people are saved and come back on every visit.
- **Routines:** shift change at 05:30 and 17:30, the dusk meal at 18:30, night watch from
  20:30. Players near a holding hear the routine change, and its people act it out
  ([Camp life](#camp-life)).

*Simplified:* towns are judged by size, infection, name and neighbours, not real roads or
airfields. A town "taken" changes hands as data while nobody is there: no battle to watch.

Bastion and the Red Hand also put temporary **road posts** on the roads near their holdings, and
their camps warn off the people they watch: [REPUTATION.md](REPUTATION.md#checkpoints-roadblocks-and-camps).

### Camp life

Code: `Factions/RZ_CampLife.c`; settings: **Territory > Camp life**. Holdings, Haven safehouses,
Halcyon's field stations and road posts look lived in, and their people do things. It's
scenery and behaviour only: nobody new decides who is there, and nothing is saved.

**What stands there.** Each faction has a **kit** of compositions (the author builds them from
vanilla props: [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#camp-life-the-compositions-and-kits)):

| Place | Built up with |
|---|---|
| Bastion post | a sandbag nest at the approach, tents, the roster board, a table under a tarp, a fire barrel |
| Red Hand camp | an oil-drum fire, a card table, crates, a tarp, the toll barrier |
| Haven safehouse and outpost | a washing line, benches, a garden bed, a water barrel, a table outside |
| Halcyon field station | field tables, a cold box, an antenna, a generator, a canvas shelter (the lab grounds too, when Halcyon's stronghold sits at the lab) |
| Strongholds | the faction's set, more of it (14 pieces against an outpost's 8) |
| Road posts | 3 pieces beside the barrier: a fire barrel, a nest |

- **Placement, any map.** The layout is seeded from the place's position, so the same pieces
  stand in the same places on every visit. Pieces go on rings round the centre by role:
  **centre** (8–12 m: fire, table, board), **inner** (12–20 m: tents), **approach** (20–30 m
  toward the nearest road, else the next town: the nest, the barrier), **edge** (24–40 m on the
  other sides: the line, the garden, the generator). Strongholds spread 30% wider, safehouses
  20% tighter; road posts put everything 5–10 m beside the road. A spot is used only on flat
  ground (under about 12°, from four ground heights), out of the water and away from the line,
  at least 6 m from a road's centre line, outside every building's footprint (2 m margin), 4 m
  from a quartermaster or the medic, 6 m from a leader's seat and 5.5 m from another piece. A piece with no good
  spot (ten tries) is left out. A place within 60 m of one already dressed (the Haven's
  safehouse at its own stronghold) isn't dressed twice.
- **Nothing to abuse.** `RZ_DressingGuard` checks every piece and its children as it spawns and
  deletes any part with an arsenal, any inventory storage, a pick-up item, a support station
  (resupply, heal, fuel, repair), supplies, fuel, a vehicle or vehicle spawner, a spawn point,
  or any ReforgerZ component or action, with a log line. The interaction scan and the puzzles'
  landmark finder ignore the pieces: a decor fire isn't a stove, a decor generator isn't a
  generator, a barrel isn't a well, an antenna isn't a mission's landmark. The quartermaster and
  a safehouse's own services stay what they are and are never scenery.
- **Only near players.** Pieces spawn and go with the place's bodies (a player within the
  owner's visible radius: 300 m for holdings). At most 60 pieces stand on the server.

**What people do.** The bodies are the ones the built systems spawn (guards, named people,
safehouse occupants, road-post crews); camp life gives them **jobs**. Only at the **3 places
nearest players** (Live places); elsewhere they stand.

| Job | How (vanilla AI) |
|---|---|
| Sentry | the nest's watch point (a smart action), else an observation post toward the approach |
| Patrol | a loop of four points 40–80 m out (patrol waypoints in a cycle) |
| Sit | a free seat at the fire or a table (a sit smart action) |
| Work, lean | a free loiter or lean point at the line, the garden, the cold box, the barrel, the barrier |
| Gather | the roster board, else the centre's table or fire: everyone stands or sits there |
| Stroll, courier | a walk out and back: to the road, or 60–90 m to the edge |
| Off shift | to the tents; with nobody within 80 m they **sleep**: despawned as data, back in the morning |

A piece's smart actions are found by tag (`RZ_Sit`, `RZ_Work`, `RZ_Watch`, `RZ_Gather`, `RZ_Lean`)
and seats are counted, so two people never take the same chair. Without a free seat, the
smart-action waypoint or the piece, they loiter beside it. A lone character (an occupant, a
single guard) is put in an empty AI group of its own faction so it can take waypoints; without
the group prefab it stands.

| Faction | By day | Moments | Night |
|---|---|---|---|
| Bastion | the nest, two on the loop, one at the board, one at the fire, the rest at work | **05:30 and 17:30**: the watch changes hands (someone else takes the nest and the loop). **20:30, the roll**: everyone at the board for 3 minutes, the routine line read out | the nest and one at the fire; the rest asleep |
| Red Hand | two at the barrier taking the toll, one strolling to the road and back, the rest at the fire or the table | — | one lookout, the rest drinking at the fire |
| Haven | the lookout at the door, chores (the line, the garden, the barrel) | **18:30, the dusk meal**: everyone at the table but the lookout; a name is read (the routine line) | the lookout; the rest inside (asleep) |
| Halcyon | a guard pair at the door, researchers at the tables, work at the cold box | the **courier** walks out to the edge and back every 20 minutes | one guard, one at the cold box; the rest asleep |
| Road posts | the crew leaning at the barrier | — | the same |

- **Every few minutes** (Shuffle minutes, 3) anyone idle goes back to work (back from an errand,
  a fight, or a talk) and one person moves to another seat or job.
- **Combat** is the AI's own danger behaviour; hostility is unchanged.
- **Talking** to a named person or an occupant makes the nearest one stop their job for 45 s.
- **Named people** stay within 15 m of the centre and never patrol, keep watch, run errands or
  sleep, so they can always be found and talked to. **Quartermasters, leaders and the medic**
  never get a job.
- **Extras:** each live place may add **1** more person by day (its guard prefab, at most 6 on the
  server), coming out of a tent while nobody is within 60 m. They join the routine like anyone
  else, are the first pick for the courier's walk, and are removed with the place.
- **Routine lines** are the territory's notices at the phase change (subtitles come with
  [Subtitles and the reader](ROADMAP.md#ui), planned).

*Simplified:* a guard prefab that is an AI group gets one job for the whole group. Sleep is a
despawn, not an animation. Props spawned at run time may not cut the navmesh (the VERIFY list).

### Supply trucks

Code: `Factions/RZ_SupplyLines.c`.

- **A real truck does the resupplying.** Outposts and Haven safehouses only restock when a
  real vehicle with an AI crew drives in from the faction's Stronghold and parks within 50 m.
  At most one truck per faction is on the road. Places within 300 m of their stronghold
  never need one.
- **Departures** every 60–90 minutes, divided by the season speed (the first 10–40 minutes after a start), only while
  players are online and not while the stronghold is Under Pressure. The truck goes where it's
  needed most: a short place first, then the least stock, then the longest wait.
- **Boarding:** the truck waits in its yard until its crew group has spawned every member (they
  appear over several frames). Then the crew within 30 m of it get in, the driver first; nobody
  is pulled in from further away. It goes once they're all aboard, or after 90 s with at least a
  driver; anyone not aboard stays in the group and follows on foot, and is removed with the
  truck. With nobody to drive it, it waits on (the log says so once) until it's late. A crew
  killed in the yard counts as robbed.
- **Stuck:** a truck that hasn't moved 8 m in 90 s, with no player within 1 km, is quietly
  moved 80–150 m on. With a player near, it stays stuck.
- **Failure:** destroyed, robbed (its crew killed) or late. The destination is then **short**
  until the next delivery: about 30% of its shop's items are out, prices are +25% to +50%,
  half the time the Champion tier is locked, and an outpost's stock drops to 0. Players within
  600 m are told.
- **Destination lost:** if the outpost changes hands or is abandoned while a truck is on its
  way, the run is called off: no delivery, no reward, no shortage, and the truck is removed like
  a finished one. The outpost's stock and shortage go with its old owner. The roads hear about
  it: a headline and a rumour ("Bastion's truck for Morton turned round on the road"). A mission
  waiting for that truck, or for one asked for and not yet gone, is called off
  (`TRUCK_CALLED_OFF`, [World facts](#world-facts)).
- **Stock:** a delivery fills an outpost to 8 units; it uses one every 45 minutes. Bastion's
  Requisition draws one unit per item. **Haven trucks carry food:** +6 at a safehouse.
- **Players and trucks.** Ride along within 150 m for 40% of the trip and see it arrive: +4
  Fame and +5 Favor with the owner. Its crew dead, drive it in yourself: +8 Fame. Neither pays
  anyone who fired within 150 m of the truck while its crew lived with no infected about, or
  whose shot hit the crew from any range (seen by retaliation, when it's on). Or drive it
  into a rival's stronghold: +6 Fame with the Red Hand, who fence the cargo (with the receiver, for
  a Red Hand truck), and +10 Infamy with the owner. Killing the crew counts as shooting the faction's people ([REPUTATION.md](REPUTATION.md), "What makes AI shoot you").
- **No traces.** A finished truck stays parked while anyone is within 300 m (up to 15
  minutes), then it and its crew are removed. A robbed truck can be taken for 20 minutes.

*Simplified:* trucks drive straight to one waypoint (the AI picks the road) and don't drive
back. A restart takes a truck off the road with no shortage. Without a faction's truck and
crew prefabs, or with territory off, that faction's places count as always supplied.

### Faction life

Code: `Factions/RZ_FactionLife.c`, `Factions/RZ_FactionVoice.c`.

Capped, background and cheap. Needs, morale, truces and feuds are in
[REPUTATION.md](REPUTATION.md), "Morale, needs, truces and feuds"; they feed the following.

- **Radio chatter** with real information, every 10 minutes, one faction at a time, on its
  open channel: the next truck leaving, where a truck is heading, a short place, a shift or
  night watch, the Red Hand's toll hours, Halcyon's samples wanted. The Haven's channel reaches
  everyone with a radio; the others reach players Known with that faction, Radio Operators,
  and anyone with a radio near a running generator.
- **Secure channels**: what Bastion, the Red Hand and Halcyon say where nobody's meant to hear
  (`RZ_FactionLife.GetSecureTraffic`). Radio Operators crack them ([World interactions](#world-interactions)).
- **Life events** in the headlines, one every 35 minutes or so: funerals for leaders and truck
  crews, Bastion promotions, recruiting in a strong faction's town, a researcher walking out of
  Halcyon when its morale is low.
- **Remembered moments**: the night the lab nearly fell, the convoy that burned, the day
  Bastion lost a town, the day the lab got the antidote right (50%). The last 8 are saved. Leaders and named people bring
  one up a quarter of the time ("Not since the night the lab nearly fell.").
- **Rumours**: a leader's news and a wanderer's talk include gossip about another faction
  (rivals twice as often). About a rival it's biased or false 35% of the time; otherwise 10%.
  A false rumour inverts strength, invents a lost town, names the wrong truck destination or
  invents a need.

### The Long Table

Code: `World/RZ_LongTable.c`; settings: **World > Long Table**. One small cannibal family in one
farmhouse, some seasons. Who they are is in [LORE.md](LORE.md#the-long-table); how players deal
with them (no reputation tree, the dealings, the guest rule, shooting them, what each faction does
when they're found out) is in [REPUTATION.md](REPUTATION.md#the-long-table); their two
questlines are in [QUESTLINES.md](QUESTLINES.md#the-long-table) and [MISSIONS.md](MISSIONS.md#questlines).

- **Not a faction.** No territory, strength, morale, truck, leader seat or stronghold, and never a
  stronghold. One house a season at most: **Chance** (0.75) is rolled once, at the season's first
  start (a wipe rolls again). Off, there's no house, nothing is taken and its questlines never start.
- **Where.** The small settlement (size 1) with the fewest towns within 1.6 km, infection under 40,
  at least 2 km from every holding, leader seat, quartermaster, safehouse and the lab, off the
  quarantine line, and not a town a running mission or story is using. Small maps relax it a step
  at a time (1.5 km and infection under 50, then 1 km, then 700 m and any size, then 500 m and 400 m for maps as small as Arland, size 2 at most); if nothing fits,
  there's no house that season. The house is the
  farthest farm building there (barn, farm, mill, stable, granary, cowshed or silo, by prefab
  name), else the building farthest out, else a point on the edge. Saved. Territory never founds or
  takes an outpost within 1.5 km of it, and world events that want a town nobody lives in leave
  its town alone.
- **Data.** One record: 4–7 people by role (the mother, the host, the quiet son, the boy, and up to
  three others; names seeded), **hunger** (0–100, starts at 45, +6 an hour of play × season speed)
  and **suspicion** (0–100, starts at 10). Ticked every 10 minutes of play. Saved with what it took
  last, its tells on the road and what it remembers of each character.
- **Bodies only near players.** Within 300 m (Puzzles > Civilian): the host and one or two others
  outside, the mother at the door, the boy half the time. While a questline's stage mission is at
  the farm, it has the farm: the house's own people stand aside. A body that's a character and is
  killed is one of them gone.
- **The host on the road.** Every 30 s, a player within 1.5 km of the farm (not at it, not in a
  vehicle, nobody else within 150 m) has a 1 in 5 chance to meet him, once a session: he steps out
  35 m ahead with a flask, true road news and an invitation, and goes home after five minutes or
  when they've walked 350 m on. Never to a character who walked away from him or crossed the house.
- **What it takes, capped.** Every 3 hours of play (÷ season speed; twice as long once its buyer
  is gone or it was paid to move), if hunger is over 50: a wanderer as data (no body, no player
  within 600 m, not the Kid, not bitten, held or escorted) whose straight route passes within
  1.5 km of the farm, at most one a real day; else someone who walked out of a hungry safehouse
  alone in the last six hours (upkeep's leavers who found no other house). Never from a safehouse
  or a holding, never a player. H2's exile walked to the crew (`door.exile_sold`) is one more,
  spoken of as "the man the Haven put out for taking bread" (only the flag crosses, not his name;
  no satchel, and C1 doesn't follow his road). Each lowers hunger by
  40, raises suspicion by 10 and leaves its tells: *"never arrived"* in the news (a quiet headline;
  Haven-Known players are told; not a death, though the roster replaces the wanderer a real day
  later as it does the dead), a boot on the verge where they were last seen (a note prop while a
  player is within 150 m, for two real days), and for a courier or a trader (else 30% of the time)
  their satchel at a Red Hand fence (the Red Hand's open channel laughs about it; a rumour).
- **Suspicion** also rises with tells found (+5 a boot, +2 the boots by the door at a first meal,
  +15 the guest rule), +30 once H1 reads the List out (`list.read`), and with other questlines'
  flag `table.suspicion_add`. Wanderers talk about
  anyone who never arrived in the last three real days (half the time); at 30 the rumours start
  too (*"There's smoke past Levie most evenings. Nobody lives out there. Nobody should."*), and
  someone who hears either from a wanderer, or finds a boot, then starts
  [C1](QUESTLINES.md#c1-the-house-that-went-quiet). Past 80 with neither questline running, the
  house moves. At 100 it's found out.
- **Found out** when a questline's proof reaches a faction, at suspicion 100, or by R2's
  `table.bastion_knows` (to Bastion) or `cargo.told_haven` with the farm as the buyer (to the
  Haven): a headline, each faction's reaction
  ([REPUTATION.md](REPUTATION.md#the-long-table)), and no more takings. If nobody ends it within a
  real day (C1's last stage holds that clock), Bastion burns it while Bastion is around (its
  stronghold stands): Missions > Explosion at the farm if a player is within 600 m, as data if not.
  Otherwise it moves, or scatters if it already moved.
- **Moving**, once a season: paid to go, past 80 suspicion with nobody after them, or its town
  overrun (85 infection). Quiet for a real day (no bodies, no takings), then the next place that
  fits 2–5 km away (or the same one if none), with suspicion 20.
- **The fight** (the guest rule's, or a questline's): the family's men as one AI group (Missions >
  Events > Rival group, else Missions > Survivor group) hostile only to the players it's set on, by
  the Haven's bit as Rival Survivors are. Each death is one of them, the host last. All of them
  down: the mother is gone by morning. The players gone for a minute: the group goes, the dead stay
  dead. Without the prefab there's no fight (the guest rule is a narrow escape).
- **Broken up**: without the mother, or with fewer than two grown-ups, or by a questline's ending,
  it's gone for the season. Burnt: its town counts as Burned (town memory) and loses 15 infection.
  The boy, if he got out, becomes the Kid wanderer (or one more at the nearest safehouse if a Kid is
  already on the roads); the host may become a Scavenger wanderer.
- **World flags** (season-wide, for any content): `table.exists`, `table.town` (its settlement's
  index + 1; 0 none), `table.suspicion`; and, written only once true, `table.exposed`,
  `table.exposed_to` (the faction, -1 everyone), `table.broken`, `table.burnt`, `table.moved`;
  `table.supplier_cut` once the Red Hand purges its buyer. What it reads from other lines is in
  [QUESTLINES.md](QUESTLINES.md#crossings). The Haven's The List finds its house through the code
  (`MouthsFarm`, `LivesAt`): it was the farm the mouths were sent to.
- **Next season** is wiped like everything else, and rolls again.

*Simplified:* the takings and the family are data; a taken wanderer just isn't on the roads any
more, and routes are straight lines. The family's people stand where they're spawned. The jar is
any food item in the loot pool.

## Haven safehouses

Code: `World/RZ_Safehouses.c`. Who the Haven are is in [LORE.md](LORE.md); house rules,
exile and what they cost you are in [REPUTATION.md](REPUTATION.md), "Haven house rules and exile".

A safehouse is a single Haven-held spot in an otherwise abandoned town. A rescued survivor
isn't kept as an AI body: they become **+1 population** at the nearest safehouse.

- **Where.** At the first start of a season, up to 3 safehouses (one per 8 settlements, at
  least 1): the first at the Haven quartermaster, the rest in quiet, low-infection villages at
  least 1.2 km apart, away from the lab and the quarantine line. Saved. Placing any
  `RZ_SafehouseMarkerEntity` turns the picking off.
- **Few bodies.** While a player is within 300 m, a safehouse shows 3–6 occupants (never
  more than its population), its guards, its medic and its own quartermaster.
- **Population tiers** (cap 30; past it, new arrivals go to the next safehouse):

  | Population | Becomes | Effects |
  |---|---|---|
  | 0–4 | Shelter | Spawn point; shop sells Known items only; 3 occupants |
  | 5–11 | Community | Trusted stock, one guard, 4 occupants |
  | 12–19 | Settlement | A medic (heals for Favor) and a radio (missions within 3 km); 5 occupants |
  | 20–30 | Town | Champion stock, two guards, 6 occupants |

- **Upkeep.** Every 3 hours of real time with players online (divided by the season speed),
  a safehouse eats one food per 5 people. Players hand food items to the quartermaster (2 Haven Favor each). Short of
  food, 1–3 people leave, to a safehouse that can feed them or onto the roads (one may become
  a named wanderer). Upkeep never takes a safehouse below 2 people.
- **Food in and out.** Haven trucks bring 6. The Water Run brings 2. Bastion's Requisition
  contract takes 4; so do some leader jobs ([MISSIONS.md](MISSIONS.md)).
- **Stragglers.** A fed safehouse under 12 people takes in one person every 4 hours with
  players online (divided by the season speed), so a house reaches Settlement in about four
  days of play even if nobody rescues anyone. Rescues are the fast way.
- **Rescues add population.** Survivors Trapped +3 (+4 with a ★ Nurse or Doctor), Rescue the
  Kids +3, escorting the Kid wanderer +1, and several unarmed and leader jobs. About 1 in 10
  rescued survivors won't stay behind walls and becomes a named wanderer instead.
- **Bites at every safehouse.** Any safehouse takes a bite declaration and hands out expired
  antibiotics for 5 Haven Favor ("Ask for something for the fever"); the bigger the house, the
  more declaring slows the bite ([Treatments](#treatments)). House rules are in
  [REPUTATION.md](REPUTATION.md), "Haven house rules and exile".
- **A notice board** at every safehouse (`World/RZ_NoticeBoard.c`): short notes players leave
  for each other between sessions. Cosmetic and social only.
  - **Read** it with "Read the notice board" at the safehouse's quartermaster. **Write** by typing
    `/note` and the message in chat while at a safehouse (within 60 m); "Leave a note on the
    board" says how. With **UI > Board menus** on, reading opens the board menu, which has a box
    to write in ([UI.md](UI.md)).
  - The newest 8 notes per board, 160 characters at most, one note per player every 10 minutes
    (on any board), the author's name on each, and notes come down by themselves after 3 real
    days. A **note filter** (Haven settings) blanks listed words out with asterisks. Exiles can't
    write. Server admins clear a board with `/noteclear` (the nearest within 1 km) or every board
    with `/noteclear all`. Notes are saved with the safehouse and go with a season wipe.
  - *Simplified:* the filter is a plain word list, matched anywhere in the text; no way for
    players to take a note down.

## Wanderers

Code: `World/RZ_Wanderers.c`.

A small roster of named survivors travels the map: 6–10 per server (6, plus one per 6
settlements above 8). Names come from a seed; roster, positions and memories are saved.

| Role | What they do | Typical reaction to you |
|---|---|---|
| Trader | Moves between safehouses; sells goods for Haven Favor, cheaper if you've helped them, or for a piece of jewellery | Trades if you're not Hunted by the Haven |
| Doctor | Patches you up, or slows an infection (×1.25 of the time left, once per life) for Favor | Refuses the Red Hand's Trusted |
| Scavenger | Sells a rumour pointing at a puzzle mission or a stash, for Favor or one food item | Swaps a rumour for food |
| Deserter | Ex-Bastion, paranoid; tells you what Bastion is up to, and where Struck Off's man sleeps | Friendly if Bastion hates you, refuses if Bastion likes you, draws on Bastion's Champions |
| Preacher | Walks between towns; while he rests, his bells add heat there | Harmless; stops the bells for an hour if asked |
| Courier | Hands out an unarmed delivery job (letters to a safehouse) | Any non-Hunted player |
| Kid | A lost child | Follows you to a safehouse if you show the way; runs from gunfire |
| Mechanic | "Ask about cars": the nearest fixable car and what it needs, and the Car Keys job; sells a part for 12 Haven Favor | Hands over that car's keys if they like you |

- **Travel:** travel → rest (10–20 min at a safehouse, 3–8 min in a town) → somewhere new,
  0.6–3.5 km away. Routes never cross water or the quarantine line.
- **They react to the world** (every 30 s): an overrun, loud or line-side destination is
  dropped; a horde or noise nearby makes them hide; Night of the Dead or a storm makes them
  shelter. Rarely, an infected bites one: players within 1.5 km hear about it, the wanderer
  heads for a safehouse, and an antidote within 20 minutes saves them: a player chooses **Give
  them your antidote** (it takes the place of the Mechanic's "Ask about cars" while bitten).
  Nobody's dose is taken without asking. Otherwise they turn, and a special appears if players
  are near.
- **They react to you.** Attitude is your Haven standing (Bastion, reversed, for the Deserter)
  plus their **memory of you**: *helped me* (better prices, warnings), *robbed or shot at me*
  (they refuse and run, or shoot if armed), *saved my life* (a one-off gift of Favor).
  Standing effects are in [REPUTATION.md](REPUTATION.md).
- **Talking** gives one piece of road news. Actions: the role's special, "Share some food"
  and "Rob".
- **Death is permanent and makes the news.** A replacement shows up a real day later. Who
  killed them is read from the body where the engine records it; otherwise the player who shot
  near them in the last 30 s is blamed, but never while the infected are on them (so nobody is
  framed for a death to the infected).
- **Never arrived.** One whose road passes the [Long Table](#the-long-table)'s farm may be taken
  off it as data: no death in the news, only that they never arrived; replaced a real day later.
- **Performance.** With no player within 400 m, a wanderer is a position moving in a straight
  line on the tick. At most two bodies near each player.

- **A lost outpost.** When a faction loses an outpost (taken or abandoned), wanderers headed
  within 600 m of it pick somewhere else, and nobody heads there for 30 minutes. For an hour the
  loss is the likeliest road news ("Heard Bastion lost Morton to the Red Hand"), and the losers'
  people are in a mood: Haven wanderers open with it and keep twice as close to the safehouses;
  the Deserter gloats when Bastion loses one; Haven people talk low when the Red Hand moves in.

*Simplified:* routes are straight lines, not roads. A body without AI stands still while
someone is near and walks on unseen. Only a wanderer whose body is an armed AI group fights
back. The mood after a lost outpost is one at a time, the latest, and isn't saved.

## Leaders

Code: `Factions/RZ_Leaders.c`, `Factions/RZ_LeaderVoice.c`. Traits, loyalty, memory and how
to talk to them are in [REPUTATION.md](REPUTATION.md), "Faction leaders"; their personal jobs
are in [MISSIONS.md](MISSIONS.md), "Leader missions".

| Seat | Where |
|---|---|
| Stronghold (one per faction) | At the faction's quartermaster; Halcyon's at the lab. Without either, a town that suits the faction: never another stronghold's town, a quartermaster's town or the lab's. Never dies, never replaced |
| Haven safehouse | One council head per safehouse |
| Outpost | One per territory outpost. With territory off, one **interim leader post** per faction for Bastion, the Red Hand and Halcyon instead |

On Arland that's about 9–10 leaders; on Everon about 16–20.

- **Data first.** A leader is a saved record. A body exists only while a player is within
  200 m of their seat. At a stronghold or safehouse the quartermaster *is* the leader unless a
  leader prefab is set.
- **Off-screen deaths** (bites, raids) only with nobody within 400 m, every half hour; a new
  leader takes the chair 2 hours later. Deaths, replacements and loyalty changes are headlines,
  and leader news is also announced to everyone as it happens (territory and faction life stay quiet).
- **Personal jobs:** every two minutes, an Independent or Rogue leader with a player near and
  no job on has a 35% chance to think of one (at most one per 30 minutes per leader).

*Simplified:* interim posts (territory off) are leader seats only, with no guards or shop.
A leader's "people" who can see a rogue job are their faction's other seats within 60 m and,
for the Haven, its wanderers.

## Talking to people

Code: `Dialogue/RZ_Dialogue.c`, `Dialogue/RZ_TalkSpeakers.c`; the screen is `UI/RZ_TalkMenu.c`
([UI.md](UI.md#the-talk-screen)).

Look at a person and the action menu shows one line, **Talk to ...**. It opens the talk screen:
what they say, and your replies as a list. You stay in the conversation until you leave, walk
off (6 m), wait 120 s with no reply, or danger comes (the [screen guard](UI.md#how-the-scripts-fit):
then they say "Not now!" and it closes). You can't move while talking; the world keeps running.

**Who:** stronghold and seat leaders (and the quartermaster's "Talk to whoever's in charge"),
safehouse occupants, named people at holdings, the wanderers, the Long Table's people, and the
people missions and questlines spawn. Any living character with the puzzle components. Bodies,
objects, spots and doors keep their actions. Road posts have no speaker body.

**One engine, built on what's there.** Every one of these people already has a Talk handler and
up to 8 labelled slots. The engine doesn't rewrite them; it wraps them:

- **Greeting:** their Talk handler, run while the engine *captures* the player's notices and
  spoken lines whose title is the speaker's (a leader's is known; otherwise the first title the
  greeting uses). Those lines go into the screen. Anything else (a contract offer, a standing
  change) still comes as a card. A line from the speaker that arrives later (a treatment
  finishing) also goes into the screen while it's open.
- **Replies:** the person's visible choice and use slots (their labels are on the server), plus
  **I'll be going.** A leader's are Ask for work, Heard anything?, What do you make of me?
- **A pick** runs that slot's handler, capturing again. If it said nothing new (the answer was a
  card), what they said stays.
- **Shown before you choose, never rolled.** A reply you can't take stays in the list, dim, with
  what it needs: a price in Favor in its label ("Buy supplies", needs 20 Haven Favor, you have 12),
  "Give them your antidote" (you have none), "Share some food" (you have nothing to eat). There are
  no dice.

So every built rule (Rob's Infamy, the Trader's price, a leader's refusal at attitude −2, an
exile's shut door) works exactly as built. Talk adds no rewards, no items and no memory.

**Safety.** At most 32 conversations (and readers) on the server; a pick is checked for the
current serial, a reply id that was sent and enabled, a living player in range and not in danger,
and at most one every 0.3 s; Talk at most every 2 s. One node at a time goes to that player only.
Nothing is saved.

**The action list.** With the talk screen on (**Dialogue > Talk screen**, and **UI > Talk reply
layout** set), a person's choice and use actions hide, and so do the quartermaster's leader
actions for work, news and standing. Without it, or on a client where the screen can't open,
nothing hides and Talk and the slots work as actions, with notices.

*Simplified:* the quartermasters' shop and service actions stay actions (the shop screen is
planned). Conversation files, topics, persuasion and threats, regard and promises are later
ideas, not built.

## Missions

Two kinds: **world events**, announced to everyone, and **contracts**, offered only to
players who qualify. There's no "accept" button: eligible players who take part get paid.
Every mission, its requirements and its status are in [MISSIONS.md](MISSIONS.md).

### Mission building blocks

Code: `Missions/RZ_MissionBases.c` (base classes), `Missions/Blocks/` (helpers a mission owns
and ticks, so they combine with any base). Limits and optional prefabs are in **Missions >
Contracts** ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)).

| Block | What it does | Used by |
|---|---|---|
| Hold, kill tagged, visit points, destroy, deliver, choice | The base classes: hold an area (optional waves and heat), kill what the mission spawned, stand at points, destroy a prop, carry the mission item to a drop, choose whose drop gets it. The first hand-in wins. If the item vanishes while it's in an inventory (the carrier left the server and took their character with them, or a body was cleared away), a new one is put on the ground where it was, up to 3 times, and everyone involved is told where; lost from the ground, it fails | most contracts |
| **Escort** (`RZ_Escort`) | People who walk a route or follow one player. Data until a player is within 300 m, then an AI group (it walks to move waypoints) or a standing figure. They only move while a player is within 150 m and no infected is within 25 m of them; the mission can stop and resume them. If the player they follow leaves or dies, they follow the nearest player within 20 m instead (or wait for one). An AI body that dies fails the job. At most 3 escorts at once | Refugee Column, Rescue the Pilot, Escort the Scientist, Secure the Shipment, Hunt the Deserter, Struck Off, Bust Out, Desertion Route, Learning the Drones, The Pit's Debtor; questline stages in The Three Weeks and The Other Two |
| **Live capture** (`RZ_LiveCapture`) | A cage trap and a special, both spawned only when a player is near. Lure it into the cage and drop the door within 4 m, or wound it to a third of its health and get within 3 m to tie it. Hold it for 30 s while it fights (heat), then a transport case (the mission item) to deliver before it wakes (12 min). A case lost with a carrier who left is put back where they were, still sedated (up to 3 times). It leaves the world without a kill, so no sample. Killing it fails. At most 2 at once | Live Capture, Private Collection; The Three Weeks (Dr Wolff) |
| **Vehicle zone** (`RZ_VehicleZone`, `RZ_ConvoyWatch`) | A specific vehicle, or any vehicle whose prefab name has a keyword (or is one of a list), driven into a zone by a player. Works with vehicles a mission spawns, fixed cars and supply trucks. The convoy watch asks the supply system for a faction's next truck to an outpost and reports whether it arrived, was robbed, destroyed, stolen or late | Recover the Bus, Car Theft, Mobile Lab, Tied Down and the Cargo questline (one van, `RZ_CargoVan`); Convoy Escort, Hijack the Convoy; questline stages in The Three Weeks and The Other Two |
| **Health cost** (`RZ_HealthCost`) | Takes a share of a player's health (the bite detector's health accessor against the hit zone's maximum), refuses below a floor, once per life per giver. The bite detector forgets its last reading afterwards, so the drop never counts as a bite | Blood Drive |
| **Step chain** (`RZ_StepChain`) | Ordered steps across towns. Only the current step is live: its infected and its clue appear within 250 m of it. A step is done when someone has stood there long enough and read its clue. A clue that disappears before it's read is put back (a few times) | Patient Zero, Hunt the Deserter, The Three Weeks, Tied Down; The Book (questline) |
| **Disguises** (`Factions/RZ_Disguise.c`) | A faction's uniform gets you past its AI; caught, the Infamy doubles ([REPUTATION.md](REPUTATION.md#disguises)) | Bust Out; anywhere a faction shoots from Watched |

*Simplified:* escorted people without an AI body can't be hurt (only a body can die), and
move in a straight line. A caught special is carried as a case, not dragged in its cage.

## Story arcs and headlines

Code: `Director/RZ_StoryArcs.c`, `Director/RZ_Headlines.c`.

The Director strings events into short storylines. Each step only fires if the previous one
really happened (a mission ran, a player heard a rumour, a truck left), so every server lives
through different plots. An arc whose next step can't happen fizzles out quietly.
Longer storylines, over many sessions and with choices, are [questlines](#questlines).

- **When:** one arc at a time. About every hour with players online, 60% of the time one starts:
  a random arc that hasn't run in the last four hours and can run now. The arcs' clock (steps,
  timeouts, the four hours) stands still while nobody is online. Saved; an arc waiting on
  a mission (or The Last Convoy's truck on the road) just ends at a restart; a convoy that
  hadn't left yet is asked for again.
- **The same rules as any mission.** A mission an arc starts goes through the mission manager's
  "can it run now" check (its world gates, its weight if it's also picked at random, one of
  each type, its claims), and an arc holds what it uses while it runs: Fever Town and The Quiet
  Congregation their safehouse and its town, the Preacher's two arcs the Preacher, The Deserter's
  Map its two map holders ([World facts](#world-facts)).

| Arc | Steps (each needs the one before) |
|---|---|
| **The Missing Courier** | A Halcyon courier stops reporting (the Lost Colleague contract, renamed). Once that search ends, wanderers say they saw Red Hand men with lab cases. Once a player has heard it, a headline: that camp sells antidotes in Halcyon cases (if the outpost isn't the Red Hand's any more by then, they've moved to another one). Ten minutes later the Red Hand comes for more: a **Lab Defence**, whose brief says whether stolen doses are still on the market |
| **The Last Convoy** | Bastion announces a convoy to pull an outpost's people back. Its next real supply truck goes there. Red Hand lookouts are seen; the Red Hand's Known hear it's worth robbing, Bastion's Known are asked to ride along. Delivered: the post stays. Lost: it runs dry. The post falls or is abandoned first: the arc ends with a headline ("Morton fell before Bastion's convoy arrived") and a rumour |
| **The Preacher's Flock** | The Preacher rings his bells in every town (twice the heat). After two towns, up to three small hordes come for him while a player is within 600 m. Ends when someone asks him to stop, after an hour, or if he dies |
| **The Deserter's Map** | A dying deserter's map piece names two wanderers. Each holds a piece; with both, the grid of a hidden Bastion bunker, guarded by four infected |
| **Fever Town** | A safehouse's town creeps up in infection. If a player goes near: a Halcyon contract checks the water. If that succeeds: a Haven contract to get the sick out. Done: two people move and the added infection goes; failed: two die |
| **The Quiet Congregation** (the Haven's dark side) | Rare: a safehouse that has lost 6+ people lately (to hunger, raids or worse: `m_iLost`), with the Preacher alive. He walks there and the house prays with him: half portions, bells at the gate, and gossip that it "borrows" from other houses. Players end it by bringing 6 food or asking the Preacher to stop. Otherwise after 90 minutes he walks on and two people go with him |

The missions inside arcs are in [MISSIONS.md](MISSIONS.md), "Story arc missions".

**Headlines.** The last 10 headlines are kept: leader news, territory changes, life events,
arcs, the power grid and burned towns. A player's first spawn of a session gets 3–4 lines of
server news ("Word on the roads"): the latest headline, the last wanderer lost, the biggest
rescue so far, now and then a dead character's journal or a line of Haven gossip, and always
the cure percentage.

*Simplified:* the "camp selling antidotes" is a headline, not a shop. The convoy is the normal
supply truck. The flock's hordes spawn at the Preacher rather than following him from town to
town. The congregation's "borrowing" is talk only; its half portions are real.

## Questlines

Code: `Director/RZ_Questlines.c` (the engine), `Missions/Questlines/` (the author's base class
`RZ_Questline.c`, the approach helpers `RZ_QuestApproach.c`, the registry
`RZ_QuestlineRegistry.c`, and one folder per faction for the questlines themselves). Settings:
**Director > Questlines**. Every questline and its stages are in [MISSIONS.md](MISSIONS.md#questlines).

Long storylines in the New Vegas style, much longer than [story arcs](#story-arcs-and-headlines):
a questline runs over many sessions, through stages that each offer one mission or wait for
time and the world, and how players did each stage decides where it goes and how it ends.

- **Stages.** A *mission stage* offers one mission, a normal `RZ_Mission` started through the
  mission manager, so offers, rough areas, eligibility, payment and cleanup work as for any
  contract. A *wait stage* waits for play time, real time or both ("a day later"), and for the
  world (a leader alive, a town held, morale, a flag set earlier). A stage whose wait is over
  but which can't happen (its world gate is false, or its mission finds no place) keeps
  waiting, then fizzles after its give-up time (3 hours of play unless the stage says
  otherwise) with one line of text. A stage the story keeps coming back to (a retry after a
  failure) fizzles after its visit limit (3).
- **Approaches.** A stage mission can be done several ways: talk, persuade (background and
  standing), sneak (unseen, no shots near), force, bribe (Favor), betray, or side with another
  faction. The approach that decides the stage sets an *outcome*, and the questline picks the
  next stage from it. A side approach can end the mission as failed for its giver (no pay)
  while still moving the story on.
- **Who took part.** Every player who did something at a stage is recorded by identity with
  their approach and a few words ("talked him out of it"), so players can come and go across
  sessions. Personal rewards go at once to whoever did the step: the stage mission's own pay,
  standing from siding, a small extra reward. World consequences at the end are shared.
- **Endings** change the world only through existing systems, each capped per call: faction
  morale (±10), strength (±5), a town's infection (±15), a safehouse's food (±6) and people
  (±3), a leader replaced (never a stronghold's), an outpost handed over (the territory's own
  rules: never a stronghold, never the owner's last, no truce, nobody watching), a headline, a
  rumour wanderers and leaders repeat for some hours, a wanderer who remembers everyone who
  took part, and standing for everyone who took part (Fame ±10, Infamy up to 15; optionally only
  those who took one approach). Players who are offline get their standing on their saved
  profile.
- **How many.** At most **Max active** questlines at once (2), never two for the same faction
  (a group that isn't one of the four factions has its own slot). Every 30 minutes or so with
  players online, half the time, a questline that can start does. One that reached an ending
  doesn't come back this season; one that fizzled can start again after 6 hours of play.
- **Time.** The questlines' clock runs only while someone is online. Real-time waits use the
  server's clock, so "12 real hours" passes overnight with the server empty. **Wait scale**
  multiplies every wait and give-up time (0.1 for a quick test).
- **The journal.** A questline appears in the journal of everyone who took part and of anyone
  its current stage mission was offered to: its title, the stage, the stage's text and "what you
  know so far" (notes added as it goes). Its stage missions show the same type icon
  ([UI.md](UI.md#screens)).
- **Saved.** The stage, flags, texts, notes, timers, branch, visits and every participant are
  saved and restored. A stage mission doesn't survive a restart: its stage stays, and the mission
  is offered again from the start. A season wipe ends every questline.

*Simplified:* the "persuade" check is a fixed score, not a roll: the same player with the same
standing always gets the same answer (once per player per mission, kept by identity, so leaving
and rejoining doesn't give a second try). A stage's people stand
still (props with talk and choice actions), apart from optional AI groups.

### Writing a questline

One class in its own file, `Missions/Questlines/<Faction>/RZ_Quest_<Name>.c`, plus its stage
missions in the same file, registered with one line in `RZ_QuestlineRegistry.RegisterAll`. The
example is `Missions/Questlines/Haven/RZ_Quest_ShortWeight.c`.

1. **Define()**: `Info("haven_short_weight", "Short Weight", RZ_EFaction.SURVIVORS)` (the key is
   saved: never change it once it has run; a questline designed in
   [QUESTLINES.md](QUESTLINES.md#the-questlines) uses the key given there, which content may
   already gate on), then the stages, the first one is where it starts:
   `MissionStage(S_ASK, "Where the food goes", "...{town}...")` or
   `WaitStage(S_WAIT, "The next handover", "...")`. Tune a stage on the object it returns:
   `.Wait(playMinutes, realHours, both)`, `.GiveUp(playHours, "fizzle line")`, `.Visits(n)`.
   A group that isn't an `RZ_EFaction` passes faction -1 and calls `Slot("cannibals")`.
2. **CanStart(gm, run)**: pick the places and people and keep them on the run:
   `run.SetFlag("house", idx)`, `run.SetText("town", name)`, `run.SetPlace(pos, name)`.
   Hold what the story uses so no other story takes it meanwhile:
   `run.Claim(RZ_WorldFacts.Safehouse(idx))` (false if someone else has it; kept with the run,
   held again after a restart, let go when the run ends; a claimed house makes its town busy for
   every other mission). Return false if the world isn't right yet; spawn nothing here. Every
   `{key}` in a stage's text, a note or a line is filled in from the run's texts, then from the
   world's own tokens (`{day}`, `{cure_state}`, `{leader:id}`...).
3. **IsReady(gm, run, stage)**: world gates on top of the wait, with the helpers
   `IsLeaderAlive`, `LeaderNear`, `IsTownHeldBy`, `GetMorale`, `GetHouse`, or a flag. For
   anything else the world knows (the cure, the day, night, who holds a town, a safehouse's
   food, a leader's live name, a fixed fact of the setting) use the [world facts](#world-facts):
   `gm.GetFacts()`. **Crossings between questlines** use the world facts' season flags:
   `gm.GetQuestlines().WorldFlag("key.flag")`, `SetWorldFlag`, `HasWorldFlag` (the same store as
   `gm.GetFacts().Flag`, so contracts, arcs and voice read them too). A flag only your questline
   reads is `<questline key>.<flag>`; a flag another line reads uses its name from the
   [Crossings](QUESTLINES.md#crossings) table (short names such as `list.to_bastion`, reserved for
   those lines). Every ending sets `<key>.ending` by itself. A contract can lead into a
   questline with `GetQuestlines().TryBegin(key)`, which starts it only if the usual limits allow
   (No Reply's tape starts Await Relief). Ordinary contracts read the flags too (Last Helicopter
   stays grounded by The Last Pilot's).
4. **MakeMission(gm, run, stage)**: `return new RZ_QM_MyStage();`. The engine sets its
   `m_iQuestRun`, runs `Configure` and starts it if it can run now (the mission manager's shared
   check: world gates, one of each type, claims; [World facts](#world-facts)). A stage mission may
   use what its questline holds: check its own town with `IsTownBusy(gm, idx)` and pass `this` to
   the shared pickers. Halcyon's stage missions stop after the cure unless `m_Req.m_bAfterCure` is
   set (one running then is called off).
5. **The stage mission** extends `RZ_QuestMission` (a puzzle mission that is never picked at
   random). `Configure` sets the giver and pay as usual (`SetContract`). `SetupPuzzle` reads the
   run (`QFlag`, `QText`, `QFill`) and picks the spot; `SpawnScene` puts people and props down
   when the first player is within 400 m: `SpawnPerson(gm, pos, objIdx, talkClue, "Talk to
   ...", "Ask|Press him|Pay him")` (up to four choices), `SpawnClue`, `SpawnObject`. Approaches
   in `OnChoice`, `OnUse` and `OnClueRead`:
   - talk: `Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked about the stores")`;
   - persuade: `TryPersuade(gm, playerId, faction, difficulty, "police,excon")`: the standing's
     rank with that faction (Stranger 0, Known 1, Trusted 2, Champion 3, mixed tiers one less,
     Watched -1, Hunted -2) plus 1 for a listed background must reach the difficulty; once per
     player (by identity, so a rejoin doesn't reset it);
   - sneak: `m_Quiet.Watch(pos, radius)`, then `m_Quiet.IsQuiet(gm, playerId)` (no shots in the
     radius since, not noticed by heat or the infected), `m_Quiet.Spot(playerId)` when someone in
     the scene sees them; a person who sees anyone within N m checks
     `RZ_Stealth.SightRadius(playerId, N)` ([Stealth](#stealth));
   - force: `RZ_UnitWatch` for a spawned group, `SpawnGuards` and `OnZombiesKilled` for the
     infected, `RZ_Util.IsArmed` for a threat;
   - bribe: `TryBribe(gm, playerId, faction, cost)` (spends Favor, says so if they're short);
   - side with another faction: `RZ_QuestApproach.SideWith(gm, playerId, with, against, fame, infamy)`;
   - the approach that decides the stage: `Resolve(gm, playerId, approach, outcome, "what they
     did", success)`. success = false ends the mission as failed for its giver (no pay), but the
     outcome still goes to the questline.
   Flags for later stages: `SetQFlag(gm, "caught", 1)`; journal notes: `QNote(gm, "...")`;
   a personal extra: `Run(gm).Reward(gm, playerId, faction, fame, favor, "why")` (Fame up to
   15, Favor up to 40).
6. **Next(gm, run, stage, outcome)**: return the next stage's id. The outcome is yours (10 and
   up), or `OUTCOME_DONE` (the mission succeeded without one, or a wait is over) or
   `OUTCOME_FAILED` (it failed or ran out of time). Add notes (`run.Note`), set the branch
   (`run.SetBranch`), check who did what (`run.AnyDid(approach, stage)`, `run.Flag`). End with
   `return Finish(run, ending, "line for everyone who took part")` or
   `return Fizzle(run, "line")`.
7. **OnEnding(gm, run, ending)**: the consequences, through `NudgeMorale`, `NudgeStrength`,
   `NudgeInfection`, `ChangeFood`, `ChangePopulation` (never below 2 people),
   `ReplaceLeader(gm, id, headline, run)` (pass the run to move a leader it claimed itself),
   `HandOutpost` (both return false if they can't happen: then don't claim they did), `Headline`,
   `SpreadRumour`, `RememberWanderer` and `ChangeStanding`.
8. Optional: `OnBegin` and `OnStageBegin` (an opening headline, a first note), `Rumour` (gossip
   while it runs), `OnWandererTalk` and `OnLeaderTalk` (a line when a wanderer or leader is talked
   to, which can also set a flag a wait stage waits for), `JournalText` (a brief that changes
   with the flags).
9. Add it to [MISSIONS.md](MISSIONS.md#questlines) (stages, approaches, endings, ✅ only when
   it's in code) and run `tools/compile_check.sh`. To try it quickly, set **Wait scale** to 0.1
   and call `GetQuestlines().Begin("your_key")` from a test.

Player-facing text follows the writing guide in [LORE.md](LORE.md#writing-guide).

## World facts

Code: `World/RZ_WorldFacts.c`, reached as `gm.GetFacts()` or `RZ_WorldFacts.Get()` (never null:
without a game mode every query gives its default). Server only.

One place where content (missions, events, arcs, questlines, rumours, voice, guidance) reads what
is true in the world right now, so a brief, a rumour or a leader's line never says something the
world contradicts. It adds no simulation: every query reads the system that owns the fact. It
keeps four small things of its own: **season flags**, **canon**, **claims** and **gossip**.

**Queries**

| Fact | Call | From |
|---|---|---|
| The cure is out | `IsCured()` | cure research |
| The season is over and waits for its wipe | `IsEpilogue()` | the cure, or the wipe came due ([Seasons](#seasons)) |
| The final stand is running; research (what the lab has worked through) | `IsFinalStand()`, `CurePercent()` | cure research |
| Research waiting in the lab's backlog; the backlog is full; research waits at 90% for a finale | `LabWaiting()`, `LabFull()`, `AtFinaleGate()` | cure research |
| Outbreak stage; how much of Bastion is still out | `Stage()`, `BastionPresence()` (0–1) | the Director's stage table |
| The day | `Day()` (the broadcast's count), `DaysSince(day)`, `DaysSinceCommand()` (Command's last signal came on Day 3, [LORE.md](LORE.md), "Task Force Bastion") | the broadcast |
| Time of day | `IsNight()` (20:30–05:00), `IsDusk()` (18:30–20:30), `IsDaytime()`, `KnowsTime()` | the game clock |
| The line is up and shooting | `PerimeterSealed()` (not after the cure) | the Perimeter |
| Bastion's helicopter can fly | `HeliAvailable()`; `SetHeliDown(days)` (days < 0: for good) | storms, the season flags `heli.down_until` (a day) and `heli.lost` |
| Who holds a town | `TownOwner(idx)`: faction and kind (lab, stronghold, outpost, safehouse, none); `IsLivedIn(idx)`; `OwnerText(idx)` | the lab, territory, safehouses |
| A town for a job | `PickTown(filter, asker)`, `CollectTowns(filter, result, asker)`: an `RZ_TownFilter` (infection, held by a faction or by nobody, not held by some factions, kind of holder, size, one to leave out), busy towns skipped | the queries above |
| People live near a spot | `IsNearLivedIn(pos, radius)`: a safehouse, a holding or the lab (what shelling keeps away from) | safehouses, territory, the lab |
| Lights | `IsPowered(pos)` | the power grid, surges, generators |
| A safehouse now | `House(idx)`: people, food, short, medic, radio, lost lately (`m_iLost`: left, moved, taken or dead), died lately (`m_iDied`, only the dead), empty, tier | safehouses |
| A leader now | `LeaderName(id)`, `LeaderAlive(id, generation)`, `LeaderGeneration(id)`, `LeaderFree(id)` | leaders: a seat keeps its id when someone new sits down, so write `{leader:id}` in text (filled when it's shown) |
| A wanderer is free | `WandererFree(id)` (alive, not held, not claimed) | wanderers |
| Doses on the black market | `StolenDoses()` | cure research |

**Season flags.** `Flag(key, def)`, `SetFlag`, `HasFlag`, `ClearFlag`: whole numbers for the
season, named `<owner>.<flag>`. The questlines' world flags are these, so any content can read a
questline's outcome (every ending sets `<key>.ending`) and set one a questline reads. Saved in
`state.json`; a new season starts with none.

**Canon.** Fixed facts of the setting, made once a season from its seed and then the same every
time anything asks, so a mission never re-rolls the story: `CanonText(key, rule)`,
`CanonInt(key, min, max)`, `CanonPick(key, options)`, `CanonSettlement(key, rule)`,
`CanonPos(key, rule)`, `HasCanon`, `SetCanon` (the first value wins). Without a rule a key gets
its built-in rule below, else a seeded default (a number in the range, one of the options, a
town nobody lives in, a dry spot just outside it). `CanonInt` always answers inside its range,
even for a built-in or earlier value. A rule is a small `RZ_CanonRule` subclass whose
`Make(gm, facts, rng)` returns the value as text (`RZ_WorldFacts.PosText(pos)` for a spot); a
rule that asks for the fact it is making gets "" back. Saved; a new season makes new ones. A
canon town can change hands during the season: read `TownOwner` when you use it.

| Key | Kind | The fact | Built-in rule |
|---|---|---|---|
| `origin.site` | spot (`CanonPos`) | the closed site in the hills | The Three Weeks' rules: a high dry spot between two towns 1.4–6 km apart, at least 450 m past every settlement, 1.5 km from the lab, not behind a lock and not nearest the lab's town, moved onto a factory, mast or barracks within 400 m if one is as remote; else high ground beyond the town farthest from the lab, as remote as it can be |
| `origin.site.town` | town (`CanonSettlement`) | the town nearest the site | |
| `origin.clinic` | town | the clinic the first patient was brought to | the town or city nearest the site, not the lab's or the site's own |
| `origin.patient` | words | his surname | a surname |
| `origin.told`, `origin.told.month` | number (`CanonInt`) | the date the Institute was told | day 3–24, month 1–12 |
| `bastion.orders` | words | what the last orders said | `expended` (60%) or `relief` |
| `redhand.pit` | spot | where the Red Hand's fight pit is | 60–150 m behind the boss's seat at the Red Hand stronghold, else a crew boss's seat, else its stronghold or an outpost, else outside a quiet town |
| `redhand.pit.town`, `redhand.pit.seat` | town; number | the pit's town; the seat it's behind (a leader id, -1 none: `LeaderName(id)` names whoever sits there now) | made with the pit |

**Claims.** Who is using a subject right now, so two stories don't use the same house, leader or
wanderer at once. `Claim(subject, owner)` (true if it was free or already that owner's),
`IsClaimed`, `IsClaimedByOther`, `ClaimedBy`, `Release(subject, owner)`, `ReleaseAll(owner)`.

| Subjects | Owners |
|---|---|
| `lab`, `heli`, `pit`; `Town(idx)` "town:7", `Safehouse(idx)` "safehouse:2", `Leader(id)` "leader:12", `Wanderer(id)` "wanderer:5", `Truck(faction)` "truck:1" | `MissionOwner(id)` "mission:12", `ArcOwner(kind)` "arc:4", `QuestOwner(key)` "quest:haven_short_weight"; the cure holds `lab` during the final stand |

- **A mission** lists what it needs in `GetClaims(gm, result)`: the manager starts it only while
  they're free, holds them from before `Setup`, and lets go when it ends or `Setup` fails. What
  it picks in `Setup` it holds with `ClaimSubject(gm, subject)`. A mission started by an arc or
  a questline may use what that story holds (`m_sClaimParent`).
- **Story arcs** and **questline runs** (`run.Claim`) hold theirs until they end.
- **Who respects them:** a town is busy (`IsSettlementBusy(idx, asker)`) while another running
  mission uses it or a story holds the town, a safehouse in it, or the lab (the final stand).
  With the asking mission, what it or its own story holds counts as free, so a stage mission can
  use its questline's house. The shared pickers take the mission as their last argument:
  `PickSettlement`, `PickSettlementAway`, `RZ_EventHelp.PickEmptyTown`, `PickFreeSafehouse`
  (also skips claimed houses), `FindForMission` (claimed wanderers) and `PickTown`. A mission
  checks its own town with `IsTownBusy(gm, idx)`. A claimed leader isn't killed off-screen (the
  danger roll, `KillOffscreen`, which returns false then) or moved from the seat by anyone else
  (`RZ_Questline.ReplaceLeader(gm, id, headline, run)` may move one its own run claimed).
- **Runtime only.** Claims aren't saved. After a restart the content that is still running claims
  again: missions are offered again from the start, arcs and questline runs take theirs back.

**Can it run now?** `RZ_MissionManager.CanRunNow(m, players, checkWeight)` (`WhyNot` says why
not, `StartIfCan` checks and starts) is the one check every starter uses: the requirements'
world gates and facts, the season's No Vehicles rule, the mission's weight if the manager also
picks its type at random, one of each type at a time (a leader's personal jobs: one per leader,
so two leaders can each offer the same kind), and its claims free. The random picks, story arcs,
questline stages, leaders' personal jobs and the Mechanic's and Courier's jobs all use it. The
Turned Player event and a leader's Rein In are systems and start directly.

**Called off.** `m.Withdraw(why)` ends a running mission that the world made pointless: at the
next tick, with `why` told to everyone it was offered to, and nobody paid, blamed or penalised
(no Fame, Infamy, strength, morale or leader memory); its arc or questline hears it as a
failure. When the cure comes out, every running mission that stops with it is called off
(`RZ_MissionManager.OnCured`: `m_bNotCured`, and Halcyon's work unless `m_bAfterCure`). When a
supply truck's post changes hands, missions get `RZ_SupplyLines.TRUCK_CALLED_OFF` (with the
holding's id as `driver`, before or after the truck left): Convoy Escort and Hijack the Convoy
are called off with it.

**Requirements' world facts** (`RZ_MissionRequirements`, checked with the stage and cure):

| Field or call | Means |
|---|---|
| `m_bNotCured` | stops once the cure is out. **Halcyon's work does by default** (it's all research); `m_bAfterCure` keeps one going |
| `m_fMinMilitary`, `m_fMaxMilitary` | Bastion presence within this range |
| `m_eTime` | `RZ_ETimeGate`: `ANY`, `NIGHT`, `DUSK_OR_NIGHT`, `DAY` (with no game clock every value passes) |
| `m_iMinDay` | the broadcast's day (`Day()`) is at least this; 0 = any (Struck Off waits for Day 13) |
| `m_bNeedsHeli` | Bastion's helicopter can fly (`HeliAvailable`: no storm, not down) |
| `NeedFlag(key)`, `NeedNoFlag(key)` | a season flag set, or not set |
| `NeedFlagWithin(key, days)`, `NeedFlagNotWithin(key, days)` | a flag that holds a day (what `Changed` sets) set no more than `days` days ago (0 = today), or not |
| `NotWhileQuest(key)`, `NotAfterQuest(key)`, `NotWithQuest(key)` | not while a questline runs, not once it reached an ending, or neither. Keys are fixed in [QUESTLINES.md](QUESTLINES.md#the-questlines); one that is neither registered nor designed is logged once as a warning |

**Text.** `Fill(text)` fills the world's tokens, so no brief, rumour or line hard-codes a fact.
Unknown tokens are left alone; a questline's own `{keys}` are filled first. **Mission briefs are
filled when they're shown** (`RZ_Mission.GetBrief`: offers, the journal, world-event notices,
leaders' and wanderers' jobs), so `m_sBrief` may hold tokens. Any other line (a drop's name, a
notice) is filled only where you call `Fill` on it.

| Token | Gives |
|---|---|
| `{day}`, `{day_text}` | 14, Day 14 |
| `{days_since_command}`, `{days_since_command_words}` | 11, eleven (since Day 3) |
| `{cure_state}`, `{cure_percent}` | "the cure is at 40 percent" (or "is out", "the lab is holding its final stand"), 40 |
| `{stage}`, `{stage_tone}` | Collapse; "the towns are going dark" (one phrase per stage, and one after the cure) |
| `{time_of_day}`, `{lab_town}`, `{stolen_doses}` | night, dusk or day; the lab's town; the black market's doses |
| `{town:N}`, `{owner:N}`, `{flag:key}` | a settlement's name, who holds it ("the Haven", "nobody"), a season flag |
| `{canon:key}`, `{canon_town:key}` | a canon fact as words (for the *words* and *number* kinds), the name of a canon *town*. Text never makes a new fact: only a built-in key or one already made is filled |
| `{leader:id}`, `{leader_short:id}`, `{leader_place:id}` | who sits in that seat now ("Major Weber", "Weber"), and the seat's place; an empty seat reads as its title ("the outpost commander in Morton") |

**World changes and gossip.** `Changed(key, headline, rumour, moment, rumourHours, announce)`
tells the world in one call: the headline goes to the server news (to everyone too with
`announce`), the rumour to wanderers' and leaders' gossip for some hours, the moment (a phrase
that fits "Not since %1.") to what the factions remember, and the season flag `key` is set to
today's day number. A supply truck called off because its post changed hands uses it.
`AddRumour(text, hours)` adds gossip on its own (at most 48 hours, the newest 8 kept; questline
endings use it too). Saved.

**Which call for what** (for authors):

| You want to | Use |
|---|---|
| say what day it is, or how long Command has been silent | `{day}`, `{days_since_command_words}` in `Fill`, or `Day()`, `DaysSinceCommand()` |
| stop a job once the cure is out | `m_Req.m_bNotCured = true` (Halcyon's already stop; `m_bAfterCure` to keep one); a running one is called off by itself |
| a job only at night, or only while Bastion is about | `m_Req.m_eTime = RZ_ETimeGate.NIGHT`; `m_Req.m_fMinMilitary = 0.3` |
| a job only once the season has run a few days | `m_Req.m_iMinDay = 13` |
| a job that needs Bastion's helicopter | `m_Req.m_bNeedsHeli = true`; `SetHeliDown(days)` when it crashes (days < 0: for good) |
| a job only in the days after something changed | `m_Req.NeedFlagWithin("<the Changed key>", 2)` |
| a contract that tells a questline's story | `m_Req.NotWithQuest("<key>")`, with the key from [QUESTLINES.md](QUESTLINES.md#the-questlines) |
| a town that nobody lives in, or one a faction really holds | `RZ_EventHelp.PickEmptyTown(gm, min, max, this)`, or `PickTown` with an `RZ_TownFilter` (`m_iHeldBy`, `m_iNotHeldBy`, `m_iKinds`) |
| keep shells, fire or a horde away from people | `IsNearLivedIn(pos, radius)` |
| name a leader in text | `{leader:id}` in the brief (or `Fill` the line where you show it); `LeaderAlive(id, generation)` before you count on them |
| say how a safehouse is doing | `House(idx)` (`m_iDied` for the dead; `m_iLost` counts everyone who left) |
| mention a fixed fact of the setting | `CanonSettlement("origin.clinic")`, `CanonPos("redhand.pit")`, `CanonText("bastion.orders")`, or `{canon_town:origin.clinic}` in text |
| keep a house, leader or wanderer for your story | `GetClaims`/`ClaimSubject` (missions), `run.Claim` (questlines); pass the mission (`this`) to the pickers and use `IsTownBusy` for its own town |
| start a mission from your own code | `gm.GetMissions().StartIfCan(m, players)` |
| end a mission the world made pointless | `Withdraw("why")` |
| record an outcome other content reads | `SetFlag("<you>.<what>", value)` |
| tell everyone something in the world changed | `Changed(...)` |

*Simplified:* the layer is built, and the epilogue, the Perimeter, guidance, the voice lines named
above and the arcs use it. **Planned:** switching the rest of the content to it (contract town
pickers to `PickTown`, Bastion contracts and road posts to Bastion presence, the hard-coded days
and copied leader names in briefs to tokens, The Three Weeks, Patient Zero, The Real Orders,
Fight Pit, House Rules and The Pit's Debtor to canon, the lab and the helicopter as claims and
`m_bNeedsHeli`, missions that duplicate a questline's premise to `NotWithQuest`, research jobs
outside Halcyon to `m_bNotCured`, Bastion Shelling to `IsNearLivedIn`).

## Town memory

Code: `World/RZ_TownMemory.c`.

- **Town flags:** Looted, Barricaded, Powered, Burned. Plain values per settlement, saved except
  Barricaded and Powered.
  - **Looted:** each town has a loot level (0–1). Every item taken from a house (and every find
    in a car boot) takes a little, and the town counts as Looted under 0.5. It never drops under
    0.2 and comes back over about 2 hours.
  - **Barricaded:** while a player's barricade stands in it, or once Fortify the Safehouse has
    succeeded there. Not kept across a restart (barricades aren't saved).
  - **Powered:** set by the power grid.
  - **Burned:** once half its buildings have burned (a building count guessed by size: 12, 30
    or 60). Each burned building stops counting an hour after it burned (restored ones, an hour
    after the restart), so towns recover.
- **Hooks** for other mods: `GetLootLevel`, `ReportSearched`, `ReportBarricaded`,
  `ReportBuildingBurned`, `ReportBuildingCount` (unused so far).
- **Journals of the dead.** A life that lasted 10 minutes leaves one line (where, background,
  how long, the day, a personal line). The last six are saved; they turn up in the session news
  and in wanderers' talk. Never an object.

## World interactions

Code: `Interactions/`.

Loot lying in houses, car boots and bodies to search, locked doors and keys, cars, crafting,
water, barricades and traps, generators, fire, bells, radios and treating each other. Data until a
player is near, capped and decaying, nothing left behind.

**How it works on any map.**

- **Things are recognised by name.** Every 10 s the server looks at static map objects within
  30 m of each player and sorts them by prefab or mesh name: buildings by type, car wrecks,
  workbenches, wells and pumps, fuel pumps, stoves, bells (every church gets one), sirens, house
  radios and generators. Furniture is where loot lies.
- **Interaction spots, only near players:** an invisible prop with labelled actions ("Search the
  boot"), at most 10 per player and 70 on the server. Buildings and furniture get none.
- **Decided on the server.** Loot lies in houses only while a player is near and is data
  otherwise; a car boot or body is rolled when it's searched. What was searched, opened, stripped
  or burned is remembered as positions and times, and saved.
- **Timed actions** run on the 5 s tick: move more than 3.5 m away and they stop.

### Loot in houses

Walk into a building and a few things lie on its worktops, desks and low cupboards, or on the floor
beside them. Pick them up with vanilla's **Pick up** (or from the inventory's "nearby" list). There
are no search prompts in buildings and no timers.

| Building type (from the prefab name) | What it tends to have | Items at most |
|---|---|---|
| Pharmacy (pharmacy, hospital, clinic) | Bandages, medicine, saline, purification tablets; rare: morphine, antibiotics | 4 |
| Garage (garage, workshop, repair) | Jerrycans, tools, batteries, tyres, spark plugs, crowbars, planks, wire | 4 |
| Farm (farm, barn, stable, shed, vet clinic) | Food, water, axes, rope, planks, matches; rare: hunting guns | 3 |
| Police (police, gendarmerie) | Ammo, batons, torches, radios; rare: pistols, vests | 4 |
| Shop (shop, store, market, kiosk) | Food, water, batteries, torches, bleach | 3 |
| House (anything else) | Food, water, cloth, bandages, knives, torches | 2 |

- **Rolled once, then data.** The first time a player comes near, each of the building's item
  slots holds something 60% of the time at full town loot (scaled by the town's loot level). 10%
  of finds are rare (20% behind a lock), 10% electronics (×1.5 in powered towns). A locked place
  (a seeded lock on outside doors that were found) has one more slot, +25% per slot, and better
  finds. There's also one roll for a treatment (expired antibiotics, suppressant, vet antiviral,
  stabiliser, antidote, from a configurable list: [Treatments](#treatments)), sometimes the town's
  fixable car's keys ([Cars](#cars-found-rare-and-hard-to-fix)), and 12% of the time something to
  read ([Readable things](#readable-things)). Burned buildings and protected places (the same as
  for locks) have none.
- **Where it lies.** On the top of a piece of furniture inside the building, recognised by name
  (cupboards, desks, worktops, counters, shelves no taller than 1.6 m; a trace down at its middle),
  else on the floor beside it toward the middle of the building, always inside the building's
  bounding box. A building with no furniture found gets up to five floor points in its box (traced
  down from 1.4 m above its origin). One item per spot, at most eight spots; items beyond the
  building's spots aren't placed.
- **Only near players.** A building's loot is put out when a player is within 35 m and the
  look-around has seen it, three buildings a tick, the nearest first. With nobody within 60 m for
  a minute, untaken items are deleted and kept as data: they're there again next time. At most
  220 items lie on the server; when it's full, the building farthest from players is put away
  for a nearer one.
- **Taken.** An item that goes into an inventory (or is carried more than 1.5 m off) is taken:
  it leaves the building's list, the town's loot level drops by 0.015, and from then on it's an
  ordinary item (dropped and forgotten, it's cleaned up like any [loose item](#loose-items)).
- **Refill.** An emptied building has loot again after an hour, divided by the town's loot level
  (up to four hours in a picked-over town). One with leftovers keeps them, and rolls fresh after
  three times that.
- **Saves.** Only each building's rolled and emptied times are saved, never the items: after a
  restart a building rolls again once its refill time has passed.
- **A stash someone wrote about** ([Readable things](#readable-things)) adds two good finds to
  the building nearest it, the first time it's put out.

*Simplified:* the furniture names are guesses, and spots are traced once per building, so an item
can float on a high shelf or sit in a corner. Car boots and bodies are still searched (below).

### Car boots and bodies

| What | What it tends to have |
|---|---|
| Body (an infected killed near a player; 4 minutes) | Bandages, ammo, small things; sometimes a key or something to read |
| Car boot (a wreck) | Jerrycans, tools, rope, batteries, tyres |

**Search the boot** or **Search the body** (4 s, 1 heat) finds something 60% of the time (a boot
scaled by the town's loot level), sometimes two things, plus the treatment roll; the things go
into your pack. A searched boot has nothing new for an hour; a body is gone once searched. A car
boot's finds take 0.015 each off the town's loot level.

### Locks and keys

About half of pharmacies and police stations, a third of garages, a quarter of shops and a tenth
of houses are locked, seeded from the building's position so the same ones are locked after a
restart. **The lock is on the building's outside doors**, and the loot is inside. Places people
use are never locked: nothing within 80 m of a safehouse, the lab or an outpost, 100 m of a
quartermaster, or 150 m of a stronghold (the same places that can't burn). **Nothing within 25 m of
a running mission's props is locked either** (its position, its props, items and drops, the current
chain step, a capture's cage): its buildings open while it runs and lock again when it ends. The
one exception is a lock that is part of the mission (the Locked Bunker's keypad).

| State | How it gets there | What it means |
|---|---|---|
| Locked (the default) | | The vanilla Open action on an outside door says **Locked** from outside |
| Unlocked | **Unlock** at the door with its key (quiet), or opening the door from inside (the latch) | Its doors open for anyone |
| Locked again | **Lock** at the door with its key | Locked; the door pulls shut once you step away |
| Forced | **Force the door** with a crowbar: 10 s, 10 heat at the start and 25 when it gives | Open and broken: it can't be locked until the reset |
| Reset | Three hours after it was unlocked, latched or forced | Locked again, its outside doors pulled shut |

- **Which doors.** Only outside doors: a door entity of the building that stands within 1.2 m of
  the building's outline (its bounding box). Interior doors never lock. Doors are data: they're
  looked at only when a player comes within 150 m of a locked place (one look per player every
  30 s, after moving 40 m), or tries one.
- **At the door.** Each outside door of a locked place gets an interaction spot just outside it,
  with Unlock, Lock and Force (at most three per building; a double door shares one). Looking at
  it, or trying the locked door, tells you what the key's tag says.
- **Inside always opens.** Inside means anywhere in the building's bounding box, at least 0.5 m
  in from its sides, or past the door toward the middle of the building and within its width.
  The first covers a shallow closet behind a door near the outline. Shutting a door is never
  refused.
- **Enforced twice.** The client greys the action out from a replicated list of locked buildings
  within 80 m of a player; the server refuses it again. Locked doors found open (AI opened them)
  are pulled shut once nobody stands within 2.5 m.
- **Keys** open one kind of building in one town: "Pharmacy, Lindale", "Shop, back door, Morton";
  houses and farmhouses by the colour of their door, "Blue house, Morton". One lies next to one
  readable thing in five (it was taped inside), and bodies carry one 8% of the time. A key is only
  ever cut for a real building with its own lock on outside doors that were found, in the town
  nearest where it turns up (not burned out); a town with none gives no key. Car keys lie in houses
  in their car's town, or come from the Mechanic. **Keys are real items** (the key prefab): they
  can be dropped, traded and taken off a body, the id stays on the server, and the tag can be read
  on the ground. Without a key prefab there are no keys.
- **Mission spots stay outside.** Missions place their items, props, people and crates on random
  ground points (the shared pickers `LandPointNear` and `HighPointNear`). With locks on, a
  point inside a locked building's bounding box (plus 1.5 m, which covers its doorways) or within
  2.5 m of a doorway blocker is rejected, and the last resort is moved out past the nearest wall.
  The landmark finder skips locked buildings too. A building counts while it's unlocked as well,
  since it locks again at the reset. The one exception is the Locked Bunker event, where the
  lock is the point: it turns a locked building's lock into a keypad (no key fits and no crowbar
  forces it while the event runs; the inside latch still works) and puts its crates inside.
- **Doors only.** A building whose doors aren't found (no door entities, or no usable bounding
  box) isn't locked, and no key is cut for it. With **Search > Locks** off, nothing is locked.

#### Keeping AI and the infected out

AI and the infected don't use the door action: they follow the navmesh and open doors
themselves. Three layers keep them out of a locked place.

| Layer | What it does | Without it |
|---|---|---|
| **AI door hook** (optional file) | The AI's own "open the door" task is refused at a locked outside door, from outside. From inside they're let out, without turning the latch for anyone | Layers 2 and 3 still work |
| **Doorway blocker** | While an outside door is shut and locked and a player is within 150 m, a thin invisible collision box stands just outside the door leaf, and the navmesh around that doorway (a 5 m box) is rebuilt so AI route around it. At most 20 doorways on the server, the nearest to players first | With **Navmesh rebuild** off, blockers only: AI walk into the box and stop there. With no blocker prefab, no blockers |
| **Banging** | A tracked infected pressing within 1.8 m of a locked doorway (blocked or not) makes 4 heat, at most every 15 s per doorway. Players within 30 m are told, at most every 90 s | Silent |

- **Rebuilds are throttled.** Requests are queued, merged per doorway and sent two per 5 s tick.
  A doorway whose blocker came or went can't get one back for 60 s, so lock and unlock spam
  costs at most two rebuilds a minute per door. A key turns (Lock or Unlock) at most once every
  5 s per building, so spamming it can't flood the replicated locked list either.
- **Nobody is trapped.** A blocker appears only while the door is shut and locked, with nobody
  within 2.5 m of the door and no player inside the building. It goes at once on Unlock, Force
  or the inside latch, and on the next tick when a player is inside the building or on the
  inside of an open locked door. Blockers aren't saved; after a restart there are none until
  players come near.

*Limitations:* the AI door hook is a guess at the engine's class; if it's wrong the file is
deleted and only blockers and banging remain. Locked places more than 150 m from every player,
or past the 20 nearest, have no blocker, so AI there still open their doors (the doors are pulled
shut again once a player comes near). Each blocker costs two small navmesh rebuilds (on and off);
how much that costs on a busy server isn't measured yet. A blocker is one fixed size, so a very
wide door may leave a gap. Buildings whose doors are part of the mesh have no lock. Where the
building's box reaches past its walls (a porch, a deep recess), someone standing in front of a
recessed door, 0.5 m or more inside the box, counts as inside and can open it. Key items don't
survive a restart.

### Readable things

12% of buildings' loot (and of car boot searches, 30% of bodies) includes an ID card, letter,
receipt or photo in the Zone's voice ([LORE.md](LORE.md), "Writing guide"). In a building it lies
like the rest of the loot, as a note prop read where it lies, and goes with the building's loot;
one a search turns up is left beside the boot or body for half an hour. One in five had a key
taped inside: in a building the key lies next to it, from a search it goes into your pack. About
one in seven has a stash grid on the back (two good finds in the building nearest it, or a car
boot searched there, within 90 minutes), one in ten a puzzle mission circled on a map.
Reading one opens it in the reader, a sheet of paper you page through, as do puzzle notes, letters
and orders, guidance notes and safehouse flyers ([UI.md](UI.md#subtitles-and-the-reader)).

### Cars: found, rare and hard to fix

Cars are only ever found, never bought. One fixable car per three settlements (at most 6, one per
town); every other wreck is just a wreck, and is never fixable. A fixable car stands near players
as the car it will become (one of the **Car prefabs**, named after it: "the beige saloon"), parked
and locked so nobody can get in, with its prompts in front of its grille.

| Step | How | Settings (defaults) |
|---|---|---|
| **Inspect** | Says what's missing: two or three of fuel, a battery, a tyre, spark plugs, all different | **Min parts** 2, **Max parts** 3 |
| **Fit parts** | Battery, tyre and spark plugs (carryable items) lie in garages, come from car boots, **stripping** wrecks (one part each) and the Mechanic wanderer. Fitting one needs a tool in your pack (a wrench, spanner, toolkit or toolbox), which isn't used up. Without one, Inspect and Fit parts say so | **Tool keywords** (empty = no tool needed); 8 s |
| **Fuel it** | A full can, from siphoning a wreck (60% have some) or a petrol station's hand pump (10 heat). No tool needed | |
| **Hotwire** | Long and loud: 45 s, half its heat when you start and all of it at the end. It fails about one time in three: the starter grinds, the heat is made all the same, and you try again. A Mechanic does it in 15 s and fails one time in ten | 45 s, Mechanic 15 s, 20 heat, fail 0.35, Mechanic 0.1 |
| **Or its keys** | Only a fifth of cars have keys in a house in their town: each building rolled there holds them one time in ten, lying with its loot (picking them up is finding them). A Mechanic wanderer who likes you may hand them over | keys 0.2, find 0.1 |
| **Start the engine** | 15 heat. The parked copy becomes the drivable car with a quarter tank, and from then on it's vanilla | |

A new project turns up elsewhere six hours after one is started. Projects in a save keep the parts
they were missing.

The Car Keys job ([MISSIONS.md](MISSIONS.md#unarmed-missions)) adds one more car for as long as it
runs: it already runs, needs only its keys (always lying in the one house the job names) or hotwiring,
isn't saved and doesn't count toward the replacement timer.

### Light crafting

At a workbench, up to eight built-in recipes (more can be added as data). A recipe whose output
prefab isn't set is hidden.

| Recipe | Takes |
|---|---|
| Improvised bandage | a rag or cloth |
| Molotov | a bottle, fuel, a rag |
| Tin rattle (noisemaker) | 2 empty tins, wire or string |
| Spear | a knife, a pole or plank, wire |
| Tin-can tripwire | 2 empty tins, wire |
| Spike trap | 2 planks, nails |
| Purification tablets | bleach or iodine |
| Purify water | untreated water, a tablet |

The **home remedy** (alcohol and herbs, 20 s) is brewed on a stove ("Brew a home remedy"), since
the workbench's eight slots are full.

### Barricades, traps and generators

| Kit | Per player | Lasts | What it does |
|---|---|---|---|
| Planks (barricade) | 3 | 60 min | Boards up a doorway. Infected against it make noise; 8 hits break it. The town counts as Barricaded |
| Tin-can tripwire | 3 | 60 min | Rattles (20 heat) when something walks through; the owner hears it within 400 m |
| Spike trap | 2 | 60 min | Kills an infected that walks onto it, three times. Players aren't hurt |
| Generator | 1 running | 30 min per fuel item | 6 heat a minute; lights hook within 40 m; a radio within 30 m hears every open channel; standing near it charges your radio and torch |

None of these are saved.

### Fire

Two ways to burn a building:

- **A thrown molotov** (25 heat) kills infected within 3 m and burns the building within 10 m.
  Only a real throw counts (the thrower's grenade-throw event names the bottle); one dropped from
  the inventory is just a bottle, and one held in the hand gives no prompt: throw it.
- **Fuel in your hands.** Hold a full fuel can and walk up to a building: **Douse the doorway and
  set it alight** appears on its nearest outside door (a building with no door entities: just
  outside the side of it nearest you). Five seconds within 3.5 m; the whole can is used up. On the
  5 s tick the server puts one invisible prompt at the nearest building within 8 m of each player
  holding fuel (two players at one building share it) and removes it when nobody with fuel is near.
  The prompt shows only to players whose own hands hold fuel, and the server checks the hands
  again when it starts and when it finishes. With **Fuel in your hands** off, a full can anywhere
  on you will do.

That one building burns: 30 heat, an optional fire effect, the town's infection −1, and for two
hours no loot there and no spawns within 30 m. Strongholds, outposts, safehouses, quartermasters
and the lab can't burn (or be locked), and never show the prompt.

The Gas Station Fire and Burning Town events ([MISSIONS.md](MISSIONS.md#world-events)) use the
same fire, one building at a time; nothing burns a whole town at once.

### Water

Fill a bottle at a well or pump (untreated), then boil it on a stove (30 s) or purify it with a
tablet. What drinking does belongs to the hunger and thirst mod ([MOD_STACK.md](MOD_STACK.md)).

### Treating each other

**Treat their wounds**: a first-aid kit and 20 s next to another player for a full heal. Nurses
(8 s) and Doctors (6 s) can do it with a bandage, and also slow a bite (×1.15 of the time left,
once per life, kept with the profile, so a restart doesn't grant it again). Every 5 minutes per
patient (a new life starts fresh). On a fresh bite (the golden window) a treatment is a
proper clean instead: 90% that it never takes. Anyone can also use the bitten player's **Clean
the bite** spot ([The golden window](#the-golden-window)).

### Bells, sirens and alarms

Ring a church bell (45 heat) or crank a siren (60 heat), once every 15 minutes each, to pull a
horde somewhere else. Players within 1.2 km hear it.

### Radios, open and secure

- **Open channels** are faction chatter and the guidance broadcasts. A **house radio** tunes
  every faction's open channel with power, two of them on weak batteries.
- **Secure channels** (Bastion's command net, the Red Hand boss, Halcyon's lab): only a Radio
  Operator can crack one, on a radio on the ground or a house radio: stand still 60 s with
  nothing loud near. Then 20 minutes of a line every 4 minutes: truck routes, watch times, short
  places, samples needed.

*Simplified:* spots stand on the object's origin (door spots just outside the door). Carrying
the wounded isn't added. Item names are matched by keyword. Fuel cans are full or empty by name:
nothing with "empty" in its name counts as fuel (cars, generators, fires, the power station, the
molotov recipe).
Traps check on the 5 s tick.

## New-player guidance

Code: `Guidance/`.

The world teaches itself through things you find and people you hear, never tutorial pop-ups.
Every line is in a speaker's voice ([LORE.md](LORE.md), "Writing guide").

### Experience levels

Each player has a **guidance level** from lives that lasted at least 10 minutes. It fades but
never disappears.

| Level | Counted lives | What they get |
|---|---|---|
| 0, brand new | 0 | Everything: every tip in full, the gentle start, a wanderer who comes looking, scenarios leaning toward the Haven, the radio every 12 min |
| 1, learning | 1–2 | Every tip in full, full scenario scenes, wanderer gossip every time |
| 2, familiar | 3–7 | Most tips; one-line scenario scenes; gossip now and then |
| 3, veteran | 8+ | The floor: a scenario clue, the open channels (every 42 min), and critical tips as one short line |

A player who comes back after 30+ real days gets two levels more guidance for two lives.

### Contextual tips

When something happens, the nearest plausible person says **one line**: a faction leader within
40 m or someone at a safehouse; else a wanderer within 60 m; else a voice on your radio; else a
plain hint. Each tip comes at most once per life, with a cooldown across lives and 90 s between
tips (critical ones don't wait). The table is data (**Guidance > Tips**, defaults in
`RZ_GuidanceTips.c`); more can be fired with `RZ_Guidance.Report(playerId, id)`.

| Tip | Trigger | Critical |
|---|---|---|
| clean | a fresh bite ("Clean it, now. Before it takes.") | ✅ |
| bitten | the bite takes hold | ✅ |
| med_stabiliser, med_antibiotics, med_vet, med_cutdose, med_suppressant, med_remedy | the first use of each treatment (once per player) | |
| special | a special dies within 80 m of you (not after the cure) | |
| samples | samples in your pack (not after the cure) | |
| tier | your first rise to Known, Trusted or Champion | |
| watched | Watched by a faction | |
| hunted | Hunted or Kill on Sight | ✅ |
| perimeter | near the edge or a checkpoint, while the line is sealed | |
| loud | heat 45+ where you stand | |
| night | the first night | |
| safehouse | your first safehouse | |
| shortage | at a place that's short of food or supplies | |
| clue | your first puzzle clue | |
| channel | a Radio Operator near a faction's chatter or starting to crack a channel | |
| car | your first fixable car | |
| hungry | **hook only:** waits for the hunger mod (or a bridge) to call `Report(playerId, "hungry", place)` | |

### Gentle start and first-life goal

For the first 15 minutes of a brand-new player's life, zombie spawns near them run at half
density. Their first counted life gets the goal **Find the Haven**.

### Spawn scenarios

Every new life starts a few metres from a small clue (brand-new players lean toward the Haven):

| Scenario | The clue | Points to |
|---|---|---|
| A back road | a car in the ditch; the dashboard note: "Gone to the safehouse in *town*." | the nearest safehouse |
| A basement (not after the cure) | scratched into the door: the roads out end in concrete, nobody gets out | the Perimeter |
| Church notice (not after the cure) | Halcyon's flyer, "Infected samples wanted. Reward." | the lab |
| A farmhouse kitchen | a wind-up radio playing the broadcast (after the cure, the new order), then the Haven's open channel | the Haven |
| An attic | a dead raider with a red hand on his jacket and a list of toll roads | the Red Hand's boss |
| Behind the barricade | a dead soldier's orders: "Hold the line. Await relief." | Bastion's stronghold |

Clues are removed after 20 minutes once nobody is near, and after 40 minutes anyway.
*Simplified:* you don't really wake in a basement; the scene says so, and the clue lies next to
wherever the spawn point put you.

### The wind-up radio and faction voices

- Every new life gets a **wind-up radio** (unless they carry one). It picks up the **open
  channels**: the Haven ("If you can hear this, *town* is open. Bring water if you can. Bring
  yourself either way."), Bastion's net and the Red Hand's taunts. About every 12 minutes for a
  brand-new player, up to 42 for a veteran.
- **Bastion loudspeakers** near Bastion seats, **Red Hand taunts** near Red Hand seats (each at
  most every 30 minutes), **Halcyon flyers** in every safehouse, and **wanderer gossip** about a
  faction you haven't met.
- **First contact** with a leader ([REPUTATION.md](REPUTATION.md), "Talking to leaders").

### A wanderer finds you

Two to seven minutes into a brand-new player's life, the nearest Courier or Scavenger comes
looking for them, waves, and waits 5 minutes. Their talk includes faction gossip.

## Cure research and the season's end

Code: `Cure/`, `Core/RZ_GameModeComponent.c`.

```
special infected killed ─▶ sample item drops ─▶ carried to the lab ─▶ "Deliver samples" ─┐
research missions (data, readings, live specimens) ──────────────────────────────────────┤
                                                                                          ▼
              the lab's backlog, at most 4 days of work ─▶ worked through at 3% a real day
                                                                                          │
 25% Field antiviral   spread -15%, bites take 25% longer                                ◀┘
 50% Bite treatment    (antidotes and stabilisers cheaper and more plentiful)
 75% Immunity serum    spread -45% total, bites take 50% longer
(90% the finale's gate: only while a cure finale questline is registered; none is built yet)
100% FINAL STAND       hold the lab 15 min against waves every 2.5 min
       win  ─▶ infection everywhere -70%, Aftermath, season over
       lose ─▶ nobody at the lab for 90 s: the batch is ruined, research drops to 90%
```

- **What a delivery is worth.** Each sample is 1% research (×2 for a Lab Technician, +25% for
  Halcyon's Champions; never more than ×2 together). Research from missions counts ×0.75 of what
  the mission names. Both are multiplied by the **Cure pace** (0.3), the **pace by play** (below)
  and the **season speed**, so on a server of five regulars a sample is about a third of a percent
  ([Season pacing](#season-pacing) has the numbers).
- **Pace by play.** The server measures how much it really plays: the player-hours (players in
  the world × hours) on each real day, averaged over the last 7 whole days. Research counts
  ×(**Pace by play** ÷ that average), between ×0.25 and ×2.5. The setting, 12.5, is the design
  point (5 regulars at 2.5 hours), so two regulars get about ×2.4 and ten about ×0.5, and a season
  takes about as long on either. It follows the server as regulars come and go; days the server
  was down count as unplayed. It's ×1 for the season's first two days (the first is partial).
  Fame and Favor for samples don't change with it. 0 turns it off.
- **The lab's capacity.** Nothing counts at once: what's delivered goes into the lab's
  **backlog**, and the lab works through it at **3% research a real day** (**Lab research per
  day**, × season speed). It works in real time, also while nobody is online or the server is
  down (a restart catches up at once, 30 days at most). The breakthroughs fire when the *worked*
  research crosses 25, 50 and 75%, so a big delivery shows over the next days. Lab research per day
  0 turns the capacity off: research counts at once.
- **Breakthrough news waits for listeners.** The lab often crosses a milestone with nobody online.
  Its effects (spread, bite timers, the antidote stock) start at once; its news ("Breakthrough:
  Field antiviral", and the 90% gate's) is told the first time a player is in the world, in
  order, and is saved across a restart. The broadcast's "Reports of research inside the zone are
  being evaluated" runs for 90 minutes of play from then.
- **The cold store is full.** The backlog holds at most **4 days of the lab's work** (**Lab
  backlog days**; 12% at the defaults), a backstop for spikes: with the pace by play on, the
  simulator never fills it from 2 to 20 regulars. While it's full the lab refuses samples ("The
  cold store is full. We can't take another sample until the lab works through what's waiting.
  Come back tomorrow."): the player keeps them, with no Fame for them, and the action shows "The
  cold store is full". A hand-in that only partly fits takes what fits and the rest stay with the
  player. Mission research beyond the cap is lost, and the mission's result line to everyone says
  so ("The lab's cold store is full: only part of it could be kept."). The Red Hand's dead drop
  still buys samples.
- **An idle lab.** The part of a delivery that fills the backlog up to **half a day of the lab's
  work** (1.5% at the defaults) counts **+25%** (**Empty lab bonus**), so the lab's idle days
  aren't wasted. It's continuous: waiting for the lab to empty and then handing everything in at
  once gains nothing.
- **What players see.** The lab's **Check research progress**, the journal and the first-spawn
  news give the lab's line: "Research 63 percent. Waiting: 14 samples' worth, about 4 days of
  work." (and "The cold store is full" while it is). Every delivery's line to everyone ends with it.
  Halcyon's secure chatter about the samples still needed says when the cold store is full or the
  work has stopped at the gate instead.
- **The finale's gate** (built as a hook; the finale questlines are planned). Research stops at
  **90%** only while a cure finale questline is registered (`RZ_Questline.IsCureFinale`, with
  questlines on). None is yet, so research runs to 100% as before. A finale takes research past
  the gate with `RZ_CureResearch.AddFinaleResearch(amount, faction, reason)`: straight into the
  worked research, not through the backlog, the Cure pace, the pace by play or the season speed,
  and each faction's share is kept (saved) for the ending. Meanwhile the backlog still fills (and
  then refuses). After **a real week at the gate** (÷ season speed) the lab inches up **1% a real
  day** by itself (**Finale fallback**, × season speed; with or without the lab's capacity), so a
  season whose finale nobody finishes still ends, about 17 days after reaching 90%. Once research
  has reached 100% the gate stays open: a lost batch goes back to the lab's normal work.
- **The lab** is a placed prefab, or spawns in a random town and stays there across restarts.
- **Antidotes** are used as a world action or from the inventory; they cure an infection. The
  lab makes doses into a **stock** from the first day, faster with each breakthrough
  ([Treatments](#treatments)): Halcyon's quartermaster sells them (a shop action whose item is
  the antidote prefab), one dose per sale, and refuses when the lab has none. The Red Hand's
  Steal the Antidotes moves up to two doses from the lab to the Red Hand's black market (4 at
  most), and a Red Hand antidote shop action sells only those. Stabilisers are a second, smaller
  stock at the same quartermaster. All three stocks are saved.
- **The final stand** sends one horde every 2.5 minutes, plus one more per 6 players at the lab
  (3 at most), so about five steady players can hold it. Its clock stands still while nobody is
  online; a restart during the stand starts it again from 15 minutes, and players joining are told
  it's running. After the breakthrough or a restart, the stand (its clock, its waves and the 90 s
  "nobody at the lab" rule) only starts once a player reaches the lab.
- **The lab never falls.** It's Halcyon's stronghold; losing the final stand ruins the batch, not the lab.
  The Red Hand's **Take the Lab** can steal the batch during the stand instead (hold the clean room
  for 90 s): the same result, research back to 90 percent, plus two doses on the black market
  (within its cap). It delays the cure; the Red Hand doesn't win it that way.
- **Contracts that touch the lab** (not saved across a restart): Power the Lab makes doses come
  twice as fast for 2 hours; Mobile Lab opens a **field annex**, a second lab prefab in a safer
  town where samples can be handed in for 3 hours, then it packs up. The lab itself never moves.
- **After the cure** (the epilogue, until the wipe): research is over. Nothing drops samples
  (specials or mission rewards), the lab and the Red Hand's dead drop refuse them (and say so),
  the sample tips stop, the lab makes no more doses, and Halcyon gives no more work by default
  ([World facts](#world-facts)). Work that stops with the cure and is running when it comes out is
  called off at once, with no pay and no penalty. The Perimeter stands down
  ([The Perimeter](#the-perimeter)); the broadcast stops its loop, and a first spawn and the
  wind-up radio give the new order instead.
- **The season's end.** When the final stand is won, the cure goes to the faction the server
  gave the most Fame this season (summed over all players). It's chosen before the stand's own
  Fame (Halcyon +50 and the Haven +30 for each player at the lab) is handed out, so the last
  fight can't decide it. The winner is saved to `last_season.json`. What each ending means is in
  [LORE.md](LORE.md), "Endings"; what it leaves the next season, the wipe and the hall of fame
  are in [Seasons](#seasons).

## Season pacing

Code: `Core/RZ_GameModeComponent.c` (**Season speed**, **Cure > Cure pace**, **Cure > Pace by play**
and the lab's settings), `Cure/RZ_CureResearch.c`, `Director/RZ_OutbreakStage.c`. Simulator:
`tools/pacing_sim.py`.

A season is tuned for **about 45 real days (six and a half weeks) to the cure**, with 5 regular
players, each playing 2–3 hours a day, as the design point. The **pace by play**
([Cure research](#cure-research-and-the-seasons-end)) holds that from about 2 to 20 regulars
without any tuning by the admin. Three kinds of clock move a season, and only the first two were
stretched for the longer season:

| Clock | Runs on | Set by |
|---|---|---|
| Research | deliveries, then the lab's work in real time | **Cure pace**, the pace by play, the lab's settings, the season speed |
| Outbreak stages and spread | hours with someone online | the stage list, the season speed |
| Moment to moment: safehouse upkeep, supply trucks, territory, the Long Table, wanderers, spawns | their own minutes of play, as before | the season speed for upkeep, trucks, territory and the Long Table; nothing for wanderers and spawns |

**Assumptions** (the design point; none of these are measured yet; the simulator uses them):

| | Value |
|---|---|
| Regular players | 5, each 2–3 hours a day, at a random time in a 12-hour stretch of the day |
| Player-hours | about 12.5 a day (the **Pace by play** setting: ×1 there) |
| Hours a day with someone online ("hours played", the clock stages run on) | about 8 (4.4 with 2 players, 10 with 10) |
| Infected killed | about 20 per player-hour (travel, looting and missions mixed) |
| Specials (with night and adaptation) | 1.1% of kills in Early Outbreak, 1.6% in Collapse, 2.2% in Overrun, 2.7% in Aftermath |
| Samples that reach the lab | about 60% (deaths, the Red Hand's dead drop, pockets) |
| Research missions finished | 0.14 per player-hour (about 1.7 a day), 2.5 named points each |
| Lab Technicians and Halcyon Champions | about +20% on samples |
| A finale (planned) | about 5 days from 90% to 100% |

**How research adds up for 5 players** (a sample is 1% × 0.3 Cure pace; missions ×0.75 × 0.3;
the pace by play is ×1):

| Stage | From samples | From missions | Delivered a day |
|---|---|---|---|
| Early Outbreak | ~0.6% | ~1% | ~1.6% |
| Collapse | ~0.9% | ~1% | ~1.9% |
| Overrun | ~1.2% | ~1% | ~2.2% |
| Aftermath | ~1.5% | ~1% | ~2.5% |

Plus 25% on the part of each delivery that fills the backlog up to half a day of work. The lab
works through 3% a day, so the backlog is a delay of a day or so and never fills. Other server
sizes deliver about the same a day: the pace by play scales each delivery by how much the server
plays.

**What the simulator gives** (`python3 tools/pacing_sim.py`, the defaults, the median of 60 runs;
day 1 is the season's first day). "Pace" is the average pace by play at a hand-in. The cure comes
on three days: with a finale questline done in 5 days, with one registered that nobody finishes
(the fallback only), and with none registered (no gate: today's build):

| Players | Pace | 25% | 50% | 75% | 90% | 100% with a finale | finale never done | no finale | Collapse | Overrun | Aftermath | Delivered a day | Lab full |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2 | ×2.4 | day 15 | 27 | 39 | 45 | 50 | 62 | 50 | 9 | 27 | after the cure | 2.1% | never |
| 3 | ×1.7 | day 15 | 25 | 36 | 42 | 47 | 59 | 47 | 7 | 21 | 42 | 2.2% | never |
| 5 | ×1.0 | day 13 | 24 | 33 | 39 | **44** | 56 | 43 | 6 | 16 | 31 | 2.4% | never |
| 7 | ×0.7 | day 12 | 23 | 32 | 38 | 43 | 55 | 41 | 5 | 14 | 27 | 2.5% | never |
| 10 | ×0.5 | day 12 | 21 | 31 | 36 | 41 | 53 | 40 | 5 | 13 | 24 | 2.6% | never |
| 20 | ×0.3 | day 9 | 19 | 28 | 34 | 39 | 50 | 37 | 4 | 11 | 22 | 2.8% | never |

Bigger servers are still a little faster: they spend more hours online, so the stages (and with
them the share of specials) come sooner. Without the pace by play (0), the same defaults give two
regulars the cure on about day 90 and ten on about day 36, with 54% of the ten's hand-ins partly
refused at a full cold store.

**How the soft cap works.** Three layers, from the one players should feel to the one they
shouldn't:

1. **The pace by play** sets how much a delivery is worth. It measures the player-hours of the
   last week, so research a day stays near the design point (about 2.4%) whether 2 or 20 people
   play, and it follows the server as regulars come and go. It stops at ×2.5 (one regular is on
   their own about twice as long) and ×0.25 (beyond about 20 regulars the lab starts to fill).
2. **The lab's capacity**, 3% a real day, is a hard ceiling on how fast research can move: no
   server reaches 90% in under 30 days, or the cure in under about 35, however much it delivers.
3. **The cold store** (4 days of the lab's work) is the backstop for spikes the first two miss:
   a weekend event, a new crowd before the week's average catches up. While it's full the lab
   turns samples away (players keep them) and missions lose research.

So day to day the season moves at the pace by play, never faster than the lab, and the cold
store only fills when a server suddenly plays far more than it did the week before.

**The final stand** lasts 15 minutes with one horde every 2.5 minutes, plus one per 6 players at
the lab: six hordes for five defenders, so about five steady players can win it. A lost stand
(back to 90%) costs about three or four days of the lab's work.

**The other clocks** (with players online only, unless it says otherwise):

| Clock | Every | Over a season |
|---|---|---|
| Outbreak stages | 40, 80 and 120 hours played (÷ season speed) | Collapse about day 6, Overrun day 16, Aftermath day 31 |
| Infection creep | a quarter of the stage's spread a minute: 0.15–0.3 a played hour | as much per stage as the old 10-day tuning: +60 or so by Aftermath if nobody pushes back (1.5 per kill) |
| The lab's work | 3% a real day, online or not (× season speed) | 90% in 30 days at the very least |
| The finale fallback | 1% a real day after a week at the 90% gate (only with a finale registered) | the cure about 17 days after 90% if nobody finishes a finale |
| Territory tick | 45–75 min (÷ season speed) | about 8 a day, at most one change each |
| Supply trucks | 60–90 min per faction (÷ season speed) | about 6 a day per faction |
| Safehouse upkeep | 3 h (÷ season speed) | about 2–3 meals a day |
| Safehouse stragglers | 4 h (÷ season speed), up to 12 people | a Settlement (medic, radio) by about day 4 without rescues |

The moment-to-moment clocks keep their pace: a longer season means more of the same days, not
slower ones. Treatment supply (antidotes, stabilisers) is on real time, not the season clock
([Treatments](#treatments)); with the breakthroughs later, the antidote stock stays at its slowest
(a dose every 30 minutes) for about the first two weeks. The stages follow hours played, not the
pace by play, so a small server stays in the earlier stages longer (two regulars never reach
Aftermath before the cure).

**Settings by server** (run the simulator with your own numbers, e.g. `--players 8 --cure-pace 0.25`):

| Regular players | Set | The cure, with a 5-day finale |
|---|---|---|
| 1 | **Season speed** 2 (the pace by play stops at ×2.5) | about day 46 |
| 2–20 | the defaults | day 39–50 (2: 50, 5: 44, 10: 41, 20: 39) |
| More than 20 | **Cure pace** about 6 ÷ the number of regulars (0.2 for 30) | about day 38 |
| Any, for a longer or shorter season | **Cure pace** (research only; the lab's 3% a day still sets the shortest season) or **Season speed** 1.5 (about a month) or 2 (about three and a half weeks), for 5 players | day 31 or 25 with the season speed |
| Pace by play off (0) | fit it by hand: **Season speed** 2 for 2 regulars, **Cure pace** about 1.5 ÷ regulars above 5 (0.15 for 10) | day 42–48 |

The season speed also makes upkeep, trucks, territory and the Long Table faster, as it always did;
on a small server that keeps the world busy. The Cure pace and the pace by play change only the
research. A lower **Lab research per day** lengthens the shortest season; **Lab backlog days** only
changes how much can wait before the lab refuses.

*Simplified:* the simulator counts the stages' clock only (a server that doesn't push back reaches
the infection thresholds sooner), each player hands in once at the end of a session, the same
regulars play every day, and a finale is modelled as 5 days. The pace by play counts whole UTC
days, so a server whose evening crosses midnight UTC splits it over two days (the week's average
evens it out). A kill still takes 1.5 infection off its town while the season is longer, so over a
stage players take off relatively more than creeps in; if towns clean up too easily, lower
**Zombies > Kill infection**. Check real kills, specials and research per player-hour in the first
sessions (the status log shows research, the backlog, the pace by play and the stage every few
minutes) against the assumptions, and adjust the Cure pace.

## Seasons

Code: `Core/RZ_Seasons.c`, `Persistence/RZ_HallOfFame.c`. Settings: **Season** (top level).

A season runs from a fresh outbreak to the cure, then the server wipes and a new outbreak
starts. By default **only the cure ends a season**: there is no timed wipe unless the admin picks
one ([The wipe cycle](#the-wipe-cycle)). Three things carry across: the last winner's starting rules,
the rule sets the admin picks, and the hall of fame. Every effect nudges a system that already
exists, and the combined effects are capped (below), so nothing runs away.

**The season's number and clock.** A fresh season (no state, or after a wipe) numbers itself
after the last one and starts its real-time clock. Both are saved, so a restart continues it.

### Next-season rules from the last winner

A fresh season reads `last_season.json` and starts with what the winning faction left behind
([LORE.md](LORE.md#endings)). **Season > Winner rules** (on) turns it off. A season that ran out
of time without a cure leaves nothing, and neither does a record older than the last season the
hall of fame knows (a wipe by hand after a season without one).

| Last winner | This season |
|---|---|
| The Haven | 2 more safe towns (the quietest start at 5% infection), 1 more safehouse, Haven strength +5 |
| Bastion | 1 more road post, 70% of posts are Bastion checkpoints, a **curfew** (Bastion keeps its full guard at night and its checkpoints go up first after dark), rare finds at police stations and on bodies ×1.3, Bastion strength +5 |
| The Red Hand | 1 more road post, 70% of posts are Red Hand roadblocks, the **black market everywhere** (the fence opens at Known instead of Trusted and holds two more stolen doses), Red Hand strength +5 |
| Halcyon | Starting infection ×0.8, spread ×0.9, specials ×1.15, Halcyon strength +5 |

### Rule sets

**Season > Rule sets** names the rule sets for this season, comma-separated (empty = none). Each
is a list entry with the same fields as the winner's rules, so admins can add their own, or
replace a built-in one by giving theirs the same name (**Season > Rule set list**).

| Rule set | What it does |
|---|---|
| **Winter** | At night they hear further (heat spread ×1.5) and specials come out more (×1.2); safehouses eat 30% more food. Shelter matters more through existing systems: food, the safehouse's walls, the dark |
| **Fog** | Fog counts from thinner fog (threshold ÷1.6), Fog Bank events 2.5 times as often, and infected spawn 15% closer all the time |
| **Military Collapse** | Bastion strength −15 and Red Hand +10 (they start and settle there), 75% of road posts are roadblocks, rare finds at police stations and on bodies ×1.5 |
| **No Vehicles** | No fixable cars, no supply trucks (so no shortages, and the Haven's truck food doesn't come), no contracts that hand players a vehicle (Recover the Bus, Car Theft, Mobile Lab, the car keys job) |

*Simplified:* **Winter** doesn't make anyone cold: the mode has no temperature. Run a weather or
temperature mod alongside it for real cold and snow; Winter only adds the rules above. **Fog**
can't shorten the real view distance: the engine's fog is global and the mode never changes it.
Set the server's weather foggy for that; the rule set only changes how the mode reads fog and
spawns infected. The No Vehicles rule doesn't remove vehicles placed in the world or brought by
other mods.

**Caps** once everything is combined: starting infection ×0.6–1.3, safe towns 3, spread ×0.75–1.25,
specials ×0.8–1.4, night specials up to ×1.3, night hearing up to ×2, upkeep up to ×1.5, fog
threshold up to ÷2, fog banks up to ×3, sight down to ×0.7, safehouses +2 (never more than one per
5 settlements), road posts +2 (6 at most), rare finds up to ×2 (half at most), strength ±20.
Starting infection, safe towns, extra safehouses and starting strength apply only to a fresh
season; the rest apply at once and follow the setting if it's changed.

### The wipe cycle

**Season > Wipe**: *On cure* (the default), *After weeks*, *whichever comes first*, or *Off* (by
hand: **Wipe on start**, or delete `state.json`). Only *On cure* is on by default: the two timed
modes stay available for admins who want a fixed calendar, and are never used unless picked.

- **On cure:** the world stays open for an **epilogue** of one week, 168 real hours (Aftermath, with the
  ending, the line stood down) after the final stand is won. Its warnings and its last line speak
  of processing at the checkpoints; the gates open with the wipe.
- **After weeks** (off unless picked): 10 real weeks after the season started (**Weeks**), cure or
  not; a season to the cure takes six or seven weeks with 5 regulars ([Season pacing](#season-pacing)).
  With no cure, the season record says nobody won, and the next season gets no winner's rules.
- **Warnings:** in the last 24 real hours, the broadcast says so every hour ("Residents are
  advised that the zone will be reprocessed in about 6 hours."), with a plain line under it, and
  first spawns are told.
- **When it's due:** the season record is written (`last_season.json`), the hall of fame archives
  the season, a marker (`wipe.json`) is written, and everyone is told the Zone resets at the next
  restart. Nothing is deleted while the server runs.
- **On the next start:** the old `state.json` is copied to `state_season<N>.json`, and a fresh
  outbreak starts on the same map (new settlements' infection, safehouses, leaders, wanderers,
  territory; profiles, standing and Legacy start over too). The hall of fame and the season
  record stay.
- **By hand:** **Wipe on start** does the same on every start while it's on: the old
  `state.json` is backed up and the new season numbers itself after it, hall of fame or not.
  Deleting `state.json` by hand leaves only the hall of fame and the season record to count from.

*Simplified:* the wipe only happens on a restart; the mode never restarts the server itself.
Real time is the server's clock (`System.GetUnixTime`).

### Hall of fame

Saved to `hall_of_fame.json`, which no wipe touches. **Season > Hall of fame** (on).

| Record | Counted when |
|---|---|
| Most samples delivered | samples handed in at the lab or an annex |
| Held the final stand | at the lab when the final stand is won (everyone there is listed) |
| Most people rescued | a rescue brings people in (to a safehouse or onto the roads), for each rescuer |
| Longest life | a life's clock, checked every minute (lives over 10 minutes) |
| Most missions | each mission a player is paid for |
| Champion of each faction | the most Fame anyone reached with that faction in one life |

It's kept for **this season** and **all time**, plus a few lines for each of the last 10 seasons.
Players get it on their first spawn of a session, after the news, and the notebook's Hall of
fame tab shows it in full ([UI.md](UI.md#the-notebook)).

*Simplified:* players are matched by their backend identity and shown by the name they last
played under; at most 500 records are kept per list (the emptiest go first).

## Loose items

Code: `Core/RZ_LooseItems.c`.

Things the mode spawns for players are tracked: reward crates, dropped samples, key items, shop
and requisition purchases, search finds, loot taken from houses, crafted items and treatments, the wind-up radio, the
Wedding Party's jewellery and the Crashed Helicopter's wreck. One that lies untouched
for **30 minutes** (**World > Loose item minutes**; 0 = off) with no player within 150 m is
removed. Never removed: anything in a pack, a container or a vehicle's storage (its clock starts
again when it's put down), vehicles (a fixed car isn't litter), and anything within 300 m of a
running mission. At most 300 are tracked; past that the oldest carried entry is forgotten, else
the oldest abandoned item goes early. Not saved: after a restart, older items are left alone.

*Simplified:* "in a pack" means the item has a parent entity or sits in an inventory slot. If a
carried item is ever removed, set the minutes to 0.

## Persistence

Code: `Persistence/RZ_SaveData.c`.

Mode state is saved every 2 minutes to `$profile:ReforgerZ/state.json`: the stage, every
settlement's infection and town memory, cure progress, the lab's backlog and clock, breakthrough news nobody has heard yet, the pace by play's player-hours of the last week, antidote and stabiliser stock, player profiles (standing, Legacy,
unlocks, guidance level), quartermaster positions, safehouses, wanderers, leaders, territory,
supply stock, faction life, the Long Table, the power grid and radio station, story arcs, questlines,
headlines, world interactions and the world facts (the season's seed, flags, canon and gossip;
claims aren't saved). A save from another map is ignored. A profile that never spawned and holds nothing
lasting (no lives, Legacy, unlocks or guidance) isn't saved, and an event for a player id that
left before the server knew them (a wound described after a disconnect) gets a blank profile
that is never stored, so no junk `local_` profiles pile up. Player bodies, inventories and vehicles use
vanilla persistence ([MOD_STACK.md](MOD_STACK.md)). The season's number, clock and rules and the
notice boards' notes are in `state.json` too. A season wipe ([Seasons](#seasons)) backs `state.json` up to
`state_season<N>.json` and ignores it; `last_season.json` (the last season's result) and
`hall_of_fame.json` are kept.
