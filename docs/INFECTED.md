# The infected

How the infected live in the Zone: the towns' crawlers, the countryside's loose ones, the heat
hordes, the land they live on, and how their bodies are spawned, counted and removed. Bites and
infection are in [DESIGN.md](DESIGN.md#zombies-bites-and-infection), noise and heat in
[DESIGN.md](DESIGN.md#heat-and-noise), and how hidden a player is in [DESIGN.md](DESIGN.md#stealth).

**Status.** Phases 1 to 5 of the tribes design are built: crawlers, loose ones, heat hordes from
towns, the land grid, the agent budget and its plumbing; **the tribes** on every slot of the map
(lairs, scouts, day parties, the dusk call, night hunts by placement, the dawn count, hunger, the taken,
the ones who ran, the vigil); **their life cycle** (successions, lone ones and splinters, ageing and
the Clash, borders, splits and clans, lair moves, vacuums, new tribes, burned land, starvation,
memory and who they are); and **the behaviour layer** driving their bodies beyond striking range,
with every hunt strategy and the crawlers' daily life; and **the people's side** ([People](#people):
each faction's words, voices and marks, knowing and the ways to learn, skills, your dead run, the
dark corners, the moat, the packs' contracts and events, the cure's milestones, the final stand's
waves and the last week); and **[the great one](#the-great-one)** and **[signs and
sounds](#signs-and-sounds)** (props, call sounds, the bright moon, the map's range rings). What's left
is [planned](#planned). Nothing here has been play-tested
with a game client yet ([ROADMAP.md](ROADMAP.md)).

Code: `Zombies/RZ_ZombieSpawner.c` (spawning, counting, removing), `Zombies/RZ_Crawlers.c` (towns),
`Zombies/RZ_InfectedBodies.c` (roles, data parts, the sight rule, the agent budget, runtime
groups), `Zombies/RZ_Tribes.c`, `RZ_TribeData.c` and `RZ_TribeBodies.c` (the tribes, `/tribes`),
`Zombies/RZ_TribeLife.c`, `RZ_TribeBorders.c` and `RZ_TribeWays.c` (the life cycle),
`Zombies/RZ_TribePeople.c` and `RZ_TribeItems.c` (the people's side), `Zombies/RZ_TribeGreat.c` (the
great one), `Zombies/RZ_TribeProps.c` (props, call sounds, the moon), `Missions/Tribes/` and
`Missions/Events/RZ_TribeEvents.c` (the packs' contracts and events),
`Zombies/Behaviour/` (the behaviour layer and the crawlers' life), `World/RZ_Land.c` (the land grid),
`Director/RZ_Adaptation.c` (infection adapts).

## Contents

- [Two switches](#two-switches)
- [The spawner](#the-spawner)
- [Towns: the crawlers](#towns-the-crawlers)
- [The countryside: loose ones](#the-countryside-loose-ones)
- [Heat hordes](#heat-hordes)
- [Infection adapts](#infection-adapts)
- [The land](#the-land)
- [Bodies](#bodies)
- [Admin visibility](#admin-visibility)
- [Tribes](#tribes)
- [The life cycle](#the-life-cycle)
- [Borders](#borders)
- [Memory](#memory)
- [Who they are](#who-they-are)
- [The signs](#the-signs)
- [How they move and hunt](#how-they-move-and-hunt)
- [People](#people)
- [The great one](#the-great-one)
- [Signs and sounds](#signs-and-sounds)
- [Planned](#planned)

## Two switches

| Setting | On (the default) | Off |
|---|---|---|
| **Zombies > Crawlers > Enabled** | Towns hold crawlers: placed by the time of day, drifting to noise, refilled from the buildings, remembered by the kill ledger, giving up on locked doors | Towns are populated exactly as before |
| **Zombies > Tribes > Enabled** | The land grid is read, tribes live in the wild land, the engine's active-AI limit is raised, the countryside's roamers are loose ones walking somewhere, heat hordes come from the nearest town | No land grid, no tribes; roamers and heat hordes as before |
| **Zombies > Tribes > Life cycle** | Successions, borders, splits, lair moves, new tribes, memory ([The life cycle](#the-life-cycle)) | Tribes as in phase 2: one of the guard takes the call at dawn, and a tribe that dies out leaves its slot free |
| **Zombies > Tribes > People** | Each faction's words, voices and marks, knowing, skills, your dead run's named young, the moat, the packs' contracts and events, the call box, the stand's waves from the lab's neighbours, the last week ([People](#people)) | Tribes as in phase 3; Every Ridge Calls and The Tribe Moves fall back to the old events |
| **Zombies > Behaviour > Enabled** | Our code drives the tribes' bodies beyond striking range, and the crawlers lie up, wake and herd ([How they move and hunt](#how-they-move-and-hunt)) | Tribe groups take held waypoints, with Bacon's own senses and stealth; crawlers are Bacon's own |
| Either on | Bodies are counted in agents inside a budget shared by player clusters, tracked member by member, and never spawned in a player's sight within 300 m | |
| Both off | The spawner is exactly as it was before phase 1 | |

The player-facing words never say "crawler", "tribe" or "alpha" (the writing guide in
[LORE.md](LORE.md)); these are the code's and the docs' words.

## The spawner

The infected come from whichever zombie mod the server runs (Bacon Zombies by default,
[MOD_STACK.md](MOD_STACK.md)): **Zombies > Common, Special and Horde prefabs**. They exist only near
players.

- **Towns** within 600 m of a player are populated, at points 120–600 m from a player (so a spawn is
  never removed straight away), and removed beyond 1.1 km. The count is 4 spawns per size class at
  100% infection, times the stage's density, world events, heat (+1 per 25 heat in the town's
  square) and a focus town (×1.5, the rest ×0.4); a brand-new player nearby gets a gentler few
  minutes. With Crawlers on, that count is turned into crawlers ([below](#towns-the-crawlers)).
- **The countryside:** 2 infected per player (× the stage's density), within 300 m of them. With
  Tribes on they're [loose ones](#the-countryside-loose-ones).
- **Specials** (the stage's chance, ×1.25 at night, more after breakthroughs) drop a cure sample
  when killed: with Crawlers or Tribes on, where that special fell.
- **Caps:** 60 tracked units on the server; before phase 1, 4 spawns a tick. With either switch on,
  the [agent budget](#the-agent-budget) counts every member instead.
- **Zombie-free settlements** while a mission needs one (The Silent Village).
- **Kills:** each one takes 1.5 infection off its town ([DESIGN.md](DESIGN.md#the-director-outbreak-stages-and-spread)).

## Towns: the crawlers

A town's own infected: the ones who turned lying still. They're slow, many and easy to see, they
lie up by day and come out at night, and they're why you don't loot a street with a shotgun.

**Prefabs.** **Zombies > Crawlers > Prefabs**: Bacon's civilian characters, each spawn a runtime
group of 1–3 ([Bodies](#runtime-groups)). Empty: **Common**. From Overrun, 5% of the crawlers in
towns at 70+ infection come from **Bloated prefabs** (Bacon's exploder by default), if set.
*Honest note:* phase 0 measured every Bacon 1.8.0 character at about 3.7 m/s on an order and in a
chase. There's no slow prefab in the stack, so crawlers read as many and loud, not slow, until one is
found ([MOD_STACK.md](MOD_STACK.md)).

**How many.** The old count of spawns (above) times the average group (2), less the **kill
ledger** and those out at a kill site as data. Counted in agents, inside the
[agent budget](#the-agent-budget), at most 8 a tick.
- **The kill ledger.** Each infected killed in a town adds 1 to its ledger (and still takes 1.5
  infection off). The ledger halves every 30 played minutes and is saved, so walking 1.1 km away and
  back never refills a town. After the cure it stops halving: a cleared town stays cleared.
- **Reinforcements.** The first fill when players arrive is placed as below. Once a town has been
  full, what refills it spawns at a building 200 m or more from every player and walks into the
  street. A cleared street stays clear for a while, then fills again.
- **Lent out:** a heat horde from the town and its scavengers with bodies count as its own, so the
  town doesn't refill behind them, and their kills come off its infection and go in its ledger.

**Where they stand** (every spot is also dry, not suppressed or scorched, not by a held door, and out
of every player's sight within 300 m, buildings and trees included: [Bodies](#the-sight-rule))

| When | Building rests | Streets and yards | Elsewhere |
|---|---|---|---|
| Day (and no game clock) | 70% | 30% | — |
| Dusk (18:30–20:30) | 50% | 50% | — |
| Night (20:30–05:00) | 30% | 60% | 10% in the fields at the town's edge (a field square of the land grid, else open ground just outside) |
| Stirred | 20% | 50% | 30% toward its kill site, else toward the last noise |
| Storm, reinforcements | 100% | — | — |

A **building rest** is 2–6 m outside one of a building's outer doors (the door system's geometry),
else just past the building's box in its yard; never at a building's origin. Crawlers don't spawn
inside.

**Drift to noise.** A crawler group within 300 m of a remembered noise (8+ heat at once, the last
16, under 5 minutes old) walks to it once, through its held waypoint, stays 90 s, then walks back.
Rain ×0.7 on the reach; a stirred town's crawlers come from 600 m. Not while a player is within 250 m
of the group: by then the zombie mod's own senses have them.

**Stir** (0–100 a town, falling 20 an hour played, saved). At 50 or more the town is stirred: its
crawlers come out into the streets and toward the last noise, and its square makes 5 heat a minute,
never past 60 (below the horde's 80, so a stirred town alone never brings a horde). Stir never
spreads from town to town. What stirs a town: a tribe's hunting group passing through at night +30, a
player killed in town by a tribe's body +30, a scout's call within 600 m +10 ([Tribes](#tribes)), a
border fight within 1 km +20 ([Borders](#borders)). An admin can stir the nearest town with
`/tribes stir`.

**Scavenging.** A kill site within 1 km of a town draws 2–4 of its crawlers out the next night.
They walk out as data at 3.5 m/s, get bodies within 600 m of a player, stand there until dawn, then
walk home. Six kill sites on the server, each gone at its second dawn, saved. Kill sites come from the
tribes' takings ([The taken](#the-taken-and-the-ones-who-ran)); an admin can leave one with
`/tribes site`.

**Doors and sieges.** Infected at a locked door still bang on it and make heat, as before
([DESIGN.md](DESIGN.md#keeping-ai-and-the-infected-out)). After 10 minutes (**Siege minutes**) they
give up and the door holds: players within 40 m hear the banging stop. Out of every player's sight
they go back to the town as data; in sight they're sent away. No crawler spawns within 150 m of that
door for another 10 minutes. So hiding in a house is a real escape, not the start of a siege until
dawn.

**Who a crawler makes.** Someone killed or turned in a town becomes one of the town's infected,
as before (infection).

**Burn the Nest** ([MISSIONS.md](MISSIONS.md)) is where a town's crawlers lie up thickest. Its brief
now reads *"The slow ones lie up thick in Levie, all in one place. Find it and burn it out."*

How their bodies behave (lying up, waking, herds, giving way to a pack) is the
[crawlers' life](#crawler-life). *Simplified:* placement doesn't shift indoors when a pack passes
(planned), and bloated crawlers are whatever the prefab is: the code doesn't make them burst.

## The countryside: loose ones

With **Tribes** on, the countryside's roamers are **loose ones**: the same count (2 per player ×
the stage's density), each spawned 250–300 m out behind terrain (or just past 300 m in the open), and
**walking somewhere**, so following one leads somewhere:
- **by day** a town within 3 km (a crawler that drifted out, going home: crawler prefabs, 1–2);
- **at night** a lair candidate of the land grid within 3 km if there is one, else a town (Common
  prefabs).

A loose one that reaches its town becomes one of that town's crawlers. Each player keeps 2 within
1.1 km; one that walks out of range is replaced. With no town or lair within 3 km, or for a special,
they spawn as before. Splinters and lone ones walk the wild with bodies of their own
([Lone ones and splinters](#lone-ones-and-splinters)); starving walkers go into a town as data.
*Planned:* members crossing between ranges.

## Heat hordes

80 heat in one square within 600 m of a living player sends a horde toward it, at most one every
10 minutes; half the heat goes. A hot square nobody is near draws nothing. Players near the target
are warned; Radio Operators hear it anywhere.
- **With Tribes on,** a horde comes from the nearest town within 1.5 km of the hot square: its own
  crawlers (up to 12 in 4 groups, or a Horde prefab with Crawlers off), spawned 300–450 m out on that
  town's side, out of sight. They're lent out of the town ([above](#towns-the-crawlers)). It waits
  for room in the agent budget rather than come thin. The log says which town.
- **With no town that near, or Tribes off,** as before: a Horde prefab 360–540 m out in any direction.
- *Planned:* by day inside a tribe's range from its day party only, at night from its hunters.

Remembered noises (8+ heat at once, the last 16) also lead infected that a mission controls: they walk
to a nearby noise and stay about 90 s. The unarmed missions are built on that
([MISSIONS.md](MISSIONS.md)).

## Infection adapts

Code: `Director/RZ_Adaptation.c`. Small, capped changes that keep the late game interesting
without making it unfair.

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

*Simplified:* "sees" is distance only, with no line of sight. Runners and helmets are whatever the
prefabs are; the code doesn't change speed or armour itself. *Planned:* favourite-spot lookers from
the nearest town's crawlers by day, and from a tribe's young at night inside its range.

## The land

Code: `World/RZ_Land.c`. On with **Zombies > Tribes > Enabled**. The land is read, never placed by
hand, so it works on any map.

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
  and near a town: crawlers stand in them at night), lair candidates (forest, or one of the landmarks
  above, 600 m from every town centre and 800 m from where people live; 400 and 600 m on maps under
  25 km² of land: loose ones walk to them at night), and **wild land**: dry squares outside every
  settlement's radius + 100 m, 300 m round every lived-in place (a lived-in town's edge, or 100 m
  round a holding, safehouse or the lab), 700 m round the Long Table's farm, and the Perimeter's
  warning zone + 200 m.
- **No-lair land:** 300 m round the finale's lower store and Bastion's held store, once those canon
  spots exist (they're seeded at the 90% gate, [FINALE.md](FINALE.md#items-and-people); the overlay
  reading them is planned). An overlay, so the cache never needs a rebuild.
- **Reading it.** At the first start on a map, from the game mode's one-second tick: up to 20 land
  squares a second, stopping after 8 ms (about 5 minutes on Everon and Cain; tribes stay dormant
  meanwhile, and the log says so). It's cached in `$profile:ReforgerZ/land_<map>_<number>.json` (the
  number comes from 64 terrain heights) and loads in milliseconds after that. Wild land, fields and
  lair candidates are worked out again on every load, from where people live then. Roads come from
  the world's `SCR_AIWorld`; without one there are no roads and no bridges.
- **Tribe slots** ([Tribes](#seeds-and-places)): 1 per 5 km² of wild land (**Wild km² per slot**),
  rounded, 1 to 15; none under 1.5 km² of wild land or with no lair candidate. **Slots** overrides
  it on any map. One slot is kept for a split from 3 slots, 2 from 9, 3 from 13. The log line:
  *"Tribes: 24.4 km2 of wild land, 5 slots (1 kept for a split), 1 clan possible. Override: none."*

**Measured** (`tools/land_survey/`, four to six starts on each map; the wild land changes a little
with the holdings, safehouses and Long Table of the season):

| | Arland | Everon | Cain |
|---|---|---|---|
| Squares | 289 (17 × 17) | 2,652 (51 × 52) | 2,756 (52 × 53) |
| Land / wild | 6.3 / 0.4–1.1 km² | 51.0 / 23.8–25.8 km² | 39.0 / 19.7–21.3 km² |
| Tribe slots at 1 per 5 km² | 0 (under the floor) | 5 (1 kept) | 4 (1 kept) |
| First reading | 39 s (0.3 s of work) | 4.5 min (2.0 s of work) | 4.8 min (2.4 s of work) |
| Longest second | 12 ms | 14 ms | 23 ms (one dense square) |
| Cache | 6 KB, loads in 1 ms | 51 KB, 3 ms | 53 KB, 3 ms |
| Rivers, lakes | 3 (0.9 km), none | 28 (14.9 km), 2 | 7 (4.3 km), none |
| Bridges inferred (real), fords | 4 (4), 0 | 22 (19), 2 | 15 (14), 3 |
| Ridge posts, valley lines | 3, 1 | 71, 10 | 51, 6 |
| Lair candidates | 2–8 | 34–35 | 77–81 |

- **Arland has almost no wild land**, so it gets no tribe without **Slots**: only crawlers. The
  margins of its 14 settlements leave 2.6 of its 5.9 km² of dry land, lived-in places 1.0–1.3, and
  the Long Table's farm and the line take most of the rest.
- **What eats the wild land** is the settlements, not the line: on Everon their margins take 19 of
  its 49 km² of dry land, lived-in places 5 more, the Long Table and the line under 1 each.
- **The server's memory** doesn't move measurably: the grid is about 75 KB.

**What it can't see:** ponds and streams smaller than about 100 m between the samples; rivers made
any other way than as river entities; bridges whose deck is under 3 m above the ground, or that no
road crosses (castle and pier bridges). Of the bridge prefabs a road crosses it finds 4 of 5 on
Arland, 20 of 33 on Everon and 14 of 22 on Cain (some misses are other parts of a bridge it found).
Three 4 m tracks over gullies on Everon read as bridges, and one low castle bridge as a ford. Nor
does it see cliffs, banks and forest edges inside a square; trees whose prefab path doesn't say
"tree" (a Workshop map's own trees read as open land); or a map update that adds objects but leaves
the terrain alone (delete the cache).

## Bodies

Everything is data on the server, and bodies exist only near players. The zombie mod's AI is
untouched: fighting, chasing, banging on doors and animations are all its own. ReforgerZ chooses
where groups spawn, what's in them, their waypoints and when they're removed; stealth sets their
perception factor.

### The sight rule

Phase 0 measured Bacon's sight with the zombie lab: it's a chance, not a radius (a walker passing
one in the open is seen across about 210 m by day and 190 m at night), a hill blocks it completely,
and **spawn order is the real switch**: an infected spawned after a player is already in its sight
charges at once, from 300 m. So **P = 250 m in the open, 0 behind terrain**, and:

- **No body spawns within 300 m of a player who could see the spot.** The check is the terrain
  between their eyes (a few dozen height samples), and for town spawns also buildings and trees (a
  physics trace from the player's eyes, 16 a tick; once they're used up the spot counts as seen).
- **A data part goes back to data** (its bodies removed, its survivors kept where they were) once
  every player is 700 m away: they appear within 600 m, and the margin stops them flickering.
- **The break-off** must be scripted, because the zombie mod's own perception never forgets a target
  (phase 0: a chaser kept walking toward the target's real position for 2 minutes and 380 m). For a
  tribe's hunt it's the hide rule and the take-back ([How they move and hunt](#how-they-move-and-hunt)),
  then back to data out of sight beyond P.

### The agent budget

Settings under **Zombies > Tribes**; they apply with either switch on.

| Budget | Value |
|---|---|
| Living infected on the server | **100 agents**, counting every group member (phase 0: 100 agents at a p99 frame of 11.6 ms on a shared 4-vCPU box, 140 at 14.5 ms). Before phase 1 it was 60 tracked units, which could be 200+ AI with group prefabs |
| Per player cluster (players within 600 m share one) | **28**, or 100 ÷ the number of clusters if that's less |
| Spawned a tick | **8 agents** (not units), so a town or a horde arrives over a few ticks |
| Tracked units | 60, as before |
| The engine's active-AI limit | It covers **all** AI (faction AI, wanderers and trucks share it). 128 by default; raised to **160** at start with Tribes on |

Mission infected count toward the budget but are never removed for it. Nothing is trimmed to fit: the
budget only stops new spawns. Group prefabs count the members they had the last time one spawned (4
until then).

### The one body budget

The agent budget is the one body budget for everything the shared engine spawns, not only the
infected ([DESIGN.md](DESIGN.md#the-shared-engine)). Such a body asks
`RZ_Shared.CanSpawnBody(kind, pos)` first and is entered with `RZ_Shared.AddBody(kind, entity)`
(`RZ_BodyBudget`, `Zombies/RZ_InfectedBodies.c`). Each tick, every body still alive in the ledger is
counted in agents on its nearest player's cluster, so the infected near it leave room; a body that's
deleted or dead drops out. With both switches off there's no agent budget, and only the kinds' own
caps apply.

| Kind | Rule | Callers |
|---|---|---|
| `story` | A mission's people: always allowed, so no job stalls, but counted | The escort block (`Missions/Blocks/RZ_Escort.c`): every escort, rescue and walk-along |
| `wanderer` | Needs room in the server's 100 agents; refused, the wanderer stays data and tries again next tick | Wanderers (`World/RZ_Wanderers.c`) |
| `scene`, `asker` | 16 agents together, and room in the budget | *Planned:* dialogue's scenes and askers |
| `errand` | 10, and room | *Planned:* errand scenes |
| `stranger` | 6, and room | *Planned:* strangers |
| `epilogue` | 18 (3 scenes of 6), and room | The epilogue's people: the queue, the route's table, the lab gate, the barrier, Loose Ends' people ([EPILOGUE.md](EPILOGUE.md#places-and-scenes)) |
| `speaker` | Only within 100 m of a player, and room | *Planned:* post speakers |

Faction AI, leaders, camp life, supply trucks and the Long Table keep their own limits; they share only
the engine's active-AI limit ([above](#the-agent-budget)). The status line shows the ledger as
`other bodies N agents`.

### Runtime groups

A Bacon character on its own takes no waypoints and stays inactive. Crawlers and loose ones made
from a character prefab are put in a **runtime group**: an empty group prefab (**Zombies > Empty
group**, Bacon's `Group_Zombies_Base`) filled with `AddAIEntityToGroup` (phase 0, VERIFY 1: passed
with Bacon 1.8.0, members join active, a one-member group takes waypoints). Without the empty group
they stand on their own.

### Plumbing

- **One held waypoint per group** (`Core/RZ_GroupOrders.c`, [DESIGN.md](DESIGN.md#principles)): every
  order (drift, reinforcements, loose ones, hordes, scavengers) moves it.
- **Per-member tracking.** Each tracked unit knows its members. A member missing from its group's
  living agents has died (phase 0: the count drops within a second; the group deletes itself when its
  last member dies; the bodies stay). Each death is a kill **at that member's spot**: a special's
  sample drops where it fell, the body carries its loot ([DESIGN.md](DESIGN.md#bodies-and-car-boots)), the town's infection and ledger and any
  mission's count follow.
- **Roles** on each tracked unit: crawler, loose one, scavenger, and the tribes' alpha, guard, hunter,
  scout, young and day party. Stealth reads them: the alpha and guard as specials, a hunt on its
  trail as a horde that's never lost through stealth's search, the rest as common
  ([DESIGN.md](DESIGN.md#stealth)).
- **Data parts.** A count at a position, going somewhere at 3.5 m/s (phase 0: Bacon runs every member
  to an order, and a group's centre covers about 3.5 m/s). While it has bodies, each tick writes their
  position and their dead back to it, so nothing snaps back when the bodies go. The scavengers and the
  tribes use it.
- **The silent kill** (`KillSilently`): no samples, no infection change, no ledger, no loot on the body
  and no mission credit. Two ways, both measured in phase 0: **delete** (out of sight: no body, no
  loose item) and **kill and mark** (a body a player can find). For the tribes' border fights
  (planned).
- **Death attribution.** The bite detector remembers which unit last hit each player: its role, town
  and tribe, and when. A player's death (the perks' death check compares the living each tick, so no
  engine event is needed) reads it: within 60 s of a tribe's hit, that tribe took them.
- **Waypoints and stealth:** stealth's search still adds its own waypoint in front of the held one and
  takes it back after.

## Admin visibility

- **The status log** (every 5 minutes, with Crawlers or Tribes on) adds a line: agents against the
  budget, units, crawlers and loose ones, the clusters and their shares, the other bodies in the one
  body budget, hordes from towns, loose ones
  that arrived, the ledgers, stirred towns, scavengers out, kill sites, held doors, the engine's active
  AI against its limit, the land, and the clock. So nothing needs typing.
- **`/tribes`** (server admins, in chat, or the **[Admin] The infected, to the server log** reply
  when an admin talks to anyone) writes everything to the server log: every town with crawlers,
  a ledger or stir, every kill site and part, every tracked unit, each player's last hit, the slots.
  Play-test aids: `/tribes stir` and `/tribes site` ([above](#towns-the-crawlers)), `/tribes hunt` (the
  nearest tribe within 5 km hunts you, any time of day) and `/tribes call` (every tribe calls now).
- **The status log** also prints one line per tribe (members by role, the alpha's stage and health,
  hunger, temperament, range, lair, hunting, the day party, fed, vigil, samples, young waiting; with the
  life cycle its alpha's name and kind, look, disturbance, restless, clan, weak days, fear places,
  habits, marks, bonds, drift and history), the tribes' phase, counted nights and the town specials'
  scaling, a `tribes' life` line (lone ones, splinters, vacuums, burned land, abandoned lairs, the taken
  in the open, borders and fights), the great one's line (who, the clan's days, gatherings, falls) with
  the props and call sounds, and the behaviour layer's counts. `/tribes` adds the rovers.
- **The land** logs what it found when it's read or loaded, and the tribe slots.

The log lines are listed in [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check).

## Tribes

Packs of the ones who turned running, in the wild land: each with a lair, a range, trails and a
leader (the **alpha**). Out by day too, but lighter: a small **day party** of hunters and young walks
the tribe's paths while the alpha and guard lie up at the lair; the full hunt comes only at night.
Every rule shows: a call, a track, a line, always with the place's name. Code:
`Zombies/RZ_Tribes.c` (the tribes as data), `Zombies/RZ_TribeData.c` (records, save),
`Zombies/RZ_TribeBodies.c` (their bodies, scouts, hunts).

### Seeds and places

- **Slots and seeds.** At a season's start, once the land grid is read: seeds = slots ÷ 3, rounded up,
  at most 4 and never more than the slots not kept for a split (Everon 2, Cain 2, Arland none without
  **Slots**). In order: **the column** (the canon spot `origin.column`, 1–3 km in from the northernmost
  sealed checkpoint toward the largest town, at the nearest lair candidate within 1.5 km), **the site
  tribe** (the lair candidate within 1 km of `origin.site`, if that's wild), then the lair candidates in
  the most wild land, far from the lab and 3 km from each other. Each starts with 10–14 members and a
  Rising alpha 2–5 season days into leading. More come later: from the taken, from towns gone dark,
  round lone ones and from splits ([New tribes](#new-tribes)).
- **The place** every line carries: *"the woods north of Levie"*, *"the quarry by Morton"* (the lair's
  kind from the land grid, the nearest settlement and the compass). Each faction has its own word for
  it, always with the place ([People](#the-words)); player text never says "alpha", "tribe" or "crawler".
- **The range:** 500–1,000 m round the lair by the wild land near it (350–700 m on maps under 25 km²).
- **Hunting grounds** (up to 4): the edges of towns within 2 km that face the lair (not lived-in
  towns), bridges and fords in the range, and the nearest field.
- **Trails** from the lair to each ground: a point every 250 m, each the best of the square on the line
  and the two beside it (dry land, a forest edge or sparse trees preferred, out of towns and no man's
  land, river squares only at a crossing), thinned to 4–6 points.
- **Scout posts:** the two highest wild squares in the range (a ridge first), 300 m apart.
- **Temperament** (25% bold, 20% cautious, cunning, territorial, 15% nomadic; a quarry leans cunning,
  the column nomadic, a barn or silo cautious) and a **call pattern** (*"three short and a long"*) come
  from its seed. Temperament sets how it hunts (strategies, the core, the commitment) and how it lives:
  a bold tribe is disturbed easily, hunts on in rain and its day party turns to noise from 1.5 times
  as far; a cautious one's day party keeps to the woods and its fear places last 5 nights; a cunning
  one learns habits faster and baits; a territorial one never hunts beyond its range (hunger doesn't
  widen it), never tests the wire, walks its range's edge by day and is hard to disturb; a nomad
  loops round 3-4 lairs and its day party walks the next leg ([The life cycle](#the-life-cycle)).
- **Roles:** the alpha (alone in its own group), 2 guard, 1 scout (2 from 15 members), hunters in
  groups of 3–5, the young. Members are counted in **parts**: groups as data, each at a position.

### The daily rhythm

| When | The tribe |
|---|---|
| **Day** (06:00–18:30) | The alpha at the lair's centre, the guard 10–25 m round it, the resting ring of hunters and young 20–60 m out. Scouts on their posts. From 07:00 the **day party** walks; from 18:00 it walks back |
| **Dusk** (18:30–20:30) | The day party back in the ring, the new young join, **the call** at 18:45, the **set-out** between 19:30 and 20:30 |
| **Night** (20:30–05:00) | **The hunt** on tonight's trails |
| **Dawn** (05:00–06:00) | **The walk home**, spread over the hour, then **the dawn count** |

Fog counts as dusk (a scout's pack is the dusk pack); in a storm nobody goes out. Under a [bright
moon](#signs-and-sounds) every pack hunts further and stays out until 05:30. With no game clock it's
always day: day parties walk, scouts post, nobody hunts.

### Day parties

The author's change: the wild is never empty by day, but the tribe never comes in full until dusk.
- **Who:** hunters and young only, never the alpha, a guard or a scout. **Day party share** of the
  members (0.2; never over 0.25), twice that when **hungry** (never over half); in **groups** of 2–3
  (**Day party group**), each with its own walk. A tribe under 8 sends none.
- **Where:** a day walk of 3–5 points in the range, along forest edges and field margins, never in a
  settlement's radius, no man's land or the line's margin (rain keeps it to the woods). A **hungry**
  tribe's walk goes out to a **field at a town's edge**, outside its radius and outside no man's land,
  never into either, and players within 1.5 km of its range hear the Haven's warning that morning:
  *"The Quarry lot are hungry. Their walkers will come right down to the fields today..."*
- **Hop and hold:** legs of 100–200 m at walk-stop (0.7 m/s as data), then 4–6 minutes still.
- **Bodies:** at most 6 day-party bodies near one player cluster (**Day party bodies**), from every
  tribe together; never within 400 m of a player in their gentle start. When one appears within 350 m
  of a player, once a tribe an hour: *"The crows went up off the hedge line and didn't come back down."*
- **Day noise draws only the day party:** a remembered noise within 700 m (400 for the young, and for a
  cautious tribe) turns it to look, 3 minutes, once per noise. The resting ring and the core never turn.
- **Nobody out** the day after a feed, in a storm, the morning after the alpha fell, while weakened.
- **A scout's day call brings at most 4** (below), never the alpha or a guard. Putting the whole day
  party down brings nothing else.

### The call

At 18:45 each tribe calls, and **each player hears one line at most**: the nearest tribe within 3 km.
It tells how the tribe is:

| Line (the place filled in) | When |
|---|---|
| *"Nothing from the quarry by Morton tonight. Whatever lies up there ate well last night."* | Post-feed sleep |
| *"A short call from …, and only a few voices answering."* | Weakened (under 8) |
| *"Many voices from …, and none of them leading."* | No alpha |
| *"A voice nobody knows, calling from …"* | An alpha in its first season day |
| *"A ragged call from …, an old voice."* | An alpha in Decline |
| *"A long call from …, and … answering. They'll be out early tonight."* | Hungry or worse |
| *"A call from … at dusk, three short and a long, and about ten answering."* | Otherwise |

Other calls: **the long call** when a hunt breaks off (*"One long call from …, and then they pull
back."*, within 1.5 km of the lair and of its target), **the dawn count** (*"A call from … at first
light, and fewer answers than at dusk."* or *"The same answers"*, within 2 km), and **the vigil's
silence** (*"The calling from … stops. Then nothing at all."*). A dusk call also carries its border's
answer, two voices when it's restless, and its drift toward people late in the season
([The life cycle](#the-life-cycle)); a vacuum's silence reaches players who heard no call.
A Radio Operator hears the calls pass ridge to ridge ([Skills](#skills)). After the cure the call
thins out, and on the last dusk nothing answers ([The last week](#the-last-week)).

### Hunger and the night

- **Counted nights.** A game night counts at 05:00 if someone was online **Counted night minutes** (20)
  of it. Hunger (0–100, from 30) rises **Hunger a counted night** (3; 2 more in Decline, 2 more while
  restless); each person taken takes 25 off. Bands: fed under 40, lean, hungry 60–84, desperate 85–99,
  starving 100 ([Starvation](#starvation)). The design said 5 (7); on the measured land the simulator
  had Everon's tribes hungry 60–70% of the time and starving 8–13 times a season at that.
- **The set-out.** One trail a night; two when hungry, one of them **testing the wire**: toward the
  nearest lived-in place, ending at its no man's land's edge plus P (250 m). They come to the wire and go
  home; they never go in. In its two weak days, or the night after a border fight, one trail only. Hunters spread over the trails in file, the young ahead, the core about 150 m
  behind (a bold alpha in front). A wounded alpha (under half health) stays home with its guard, a
  cautious one unless hungry, a Declining one half the time and home by 02:00. In rain every part is
  home by 03:00.
- **On the trail** (as data at 3.5 m/s): out to the hunting ground, 10 minutes there, back, 5 minutes at
  the lair end, out again, until dawn. A hunting part passing through a town stirs it (+30, once a
  night).
- **Hearing as data:** a hunting part with no bodies turns to a remembered noise within 700 m (the young
  and a cautious tribe 400) and waits there 3 minutes, once per noise.
- **Takings as data:** at night, with no player within 1 km, a hunting part takes a wanderer it passes
  (within 150 m, 1 time in 10), at most once a tribe every 3 counted nights: *"Marta the runner was
  taken by the infected on the road to Morton."* Supply trucks too ([Trucks taken as
  data](#trucks-taken-as-data)).
- **Post-feed sleep.** After a counted night in which it took someone, it sleeps through the next day:
  no day party, no scouts, no dusk call (silence is the sign), and the lair's bodies appear only within
  150 m. It hunts again that night.
- **Weakened** (under 8 members): it hunts as data only and its trails keep 600 m from players, its
  scouts call but bring no pack, its call is short, and it sends no day party.

### How a hunt comes for you

A hunt only runs at night, once the tribe has set out. It gets a target when **a scout calls** on you,
when you make **30 heat** inside its range (×1.25 hungry, ×1.5 desperate), or when you're within 120 m
of one of its groups as it passes. Never a player in their gentle start, in no man's land, or in a
cluster a hunt broke off from tonight; never while it's weakened or at its vigil; one target at a time,
and it never switches.

- **By placement:** the first wave is up to 3 young and up to 2 hunter groups, at most 8 (less when the
  server binds). Its parts close on you as data, then appear out of sight: **the young** 200–260 m out on
  the trail side, **the hunter groups** 130–180 m out at 60–90° either side, **the core** (alpha and
  guard) 150–250 m back toward the lair, only once the first wave is down to 2 (a bold alpha comes at
  once). How each group then closes is the behaviour layer's ([below](#how-they-move-and-hunt)).
- **The commitment.** It breaks off with **the long call** at the first of: a third of what it set out
  with lost (a quarter for a cautious tribe, half for a bold one), two waves spent (one for cautious;
  a second wave comes when the first is down to 2), or **10 minutes** within 150 m of you. Its groups go
  home and it won't come for your cluster again tonight. No wave, and no core, goes in that could take
  the night's losses past half of who set out (the first wave is capped by it too): without that cap
  the simulator's worst first hunt lost 64–75% of a tribe, and 1–2 tribes a season were wiped.
- **The hide rule.** Lost for 60 s (no group has seen you) and 300 m from all of them, or in no man's
  land: it goes home, and for the rest of that night heat within 300 m of where it lost you never sets
  it on anyone (a siege's banging doesn't bring it back).
- **Logged out.** If its target logs out, the hunt waits where it is for 10 minutes (the tribe's
  clock); the same character back in that time is hunted again, from there.
- **Empty nights.** A pack that sets out three nights running and takes nobody drops its least-worn
  trail (until its places are worked out again), and the log says so.
- **The reckless young.** A hunting group of young goes to any loud noise (8 heat or more) within 400 m
  ahead of it, leaving the hunt to the others. *Simplified:* "indoors behind a closed door" isn't read yet; going indoors works
  through sight (the layer's senses never see through walls).
- **Taken:** a player its bodies kill (below) ends the hunt, and the tribe feeds.

### The taken and the ones who ran

- **The taken** (rule 4). A player who dies within 60 s of a hit from a tribe's body (the bite detector's
  attribution, read when the player dies) in the open is taken by that tribe; inside a town (radius +
  100 m) they're the town's, and the town stirs (+30). A player who turns, and a wanderer bitten or
  turned (or taken as data), in the open within 1.5 × a tribe's range radius of its lair, is taken by
  it. Each taking: −25 hunger, the alpha's count +1, the name or description kept (12, newest first), a
  **kill site** (the crawlers' scavenging), and **a young at the next dusk**.
- **Your dead run** (Described by default): a player's character counts as a young only after a life of
  10 minutes or more, once per player a season day, 3 players' young per tribe at most; public text
  describes them (*"someone who'd been a hunter"*), or names them with **Named**; **Off** makes no young
  from players. The opt-out, the name filter, the ID card and the journal: [Your dead run](#your-dead-run).
  From the 75% serum a person taken comes back as a young only half the time; after the cure, never.
- **The ones who ran** (rule 5): each played minute, a town's rise in infection is banked to the nearest
  tribe whose range comes within 1.5 km; every 4 points is a young at the next dusk, 2 a season day per
  tribe at most, 8 points banked at most; halved from the 75% serum, none after the cure.
  *"Someone from Levie ran, and joins the woods north of Levie at dusk"* is the log's line.

### Kill sites, the vigil and the alpha

- **Feeding order.** When a tribe takes someone at night, its groups within 1 km come to the spot and
  feed for 20 minutes; bodies near players stand in order: the alpha at the site, the guard 8–15 m out,
  the hunters 20–40 m, the young 40–70 m. Walk up and you meet the young first. The kill site draws the
  nearest town's crawlers the next night ([Scavenging](#towns-the-crawlers)).
- **The vigil.** When the alpha dies, there's no hunt that night: every group goes to where it fell (to
  the lair if that was outside the range) and holds there until dawn; bodies stand 10–30 m round the spot
  for 30 real minutes at most. At dawn one of the guard takes up the call (a hunter if no guard is left),
  Rising, after the guard fight ([Succession](#succession)), and the next dusk is *"a voice nobody
  knows"*. *Simplified:* the samples aren't kept from the loose-item cleanup.
- **The alpha's body:** one character in its own runtime group, by day at the lair, at night with the
  core. **Health** is kept as data when its body goes and set again when it comes back (scaled health;
  infected never heal by themselves): it heals 0.1 an hour played while it lies up. An alpha hurt in the
  last 10 minutes isn't removed until every player is 1.5 km away. 3 alpha bodies on the server at most,
  never two within 1 km of one player.
- **Samples** only when a player was within 150 m: the alpha 2 in Prime and 3 in Decline (none while
  Rising), 3 times a season per tribe; a guard 1 under a Prime or older alpha, one a tribe every 2 played
  hours and at most 4 a played hour on the whole server; a lone one 2, once. Hunters, scouts, the young, day parties and splinters drop none, and
  nothing does in a staged fight's silent kills. Town specials are scaled by the tribes' share of
  samples over the last 10 played hours (never below ×0.6), so specials per player-hour stay near the
  season pacing's. After the cure, none. The status log prints samples a player-hour (packs, guards).

### Scouts

**The scout rule**, once a second for a posted scout with a player within 200 m: you're seen after 5
checks running within its sight and in terrain line of sight (within 80 m, also nothing in between). The
sight is stealth's: `RZ_Stealth.SightRadius(player, Scout sight)` (120 m at normal visibility, down to
stealth's floor for someone fully hidden), ×1.5 in a vehicle, ×0.75 for a Hunter, ×1.5 for a character
the tribe [knows](#memory). A Hunter gets *"The birds just went up off the ridge."* at the first check. Kill it inside those 5 seconds and nothing comes.
- **The call** (one a tribe every 10 minutes): +20 heat at the scout, *"A shriek from the ridge, and an
  answer from …"* to players within 600 m, +10 stir in a town within 600 m, and the scout comes for you.
- **The pack** (one at a player cluster every 10 minutes): **by day** at most 4, the day party's groups
  within 600 m plus hunters from the resting ring, 2 day packs a tribe a day; **at dusk and in fog** the
  nearest hunter group, and a bold Prime alpha with its guard if the lair is within 600 m; **at night**
  the hunt. A pack appears 150–200 m out on the lair's side, out of sight, and rushes only the seen
  player; when it loses them it goes home. A weakened tribe's scouts bring nobody. No scouts during
  post-feed sleep, and no call on a player in their gentle start.

### Tracks

Every group out and moving leaves a track every 150 m or so (40 a tribe, 12 played hours). A **Hunter**
within 40 m of one reads it, once every 3 minutes: *"Four or five, heading north-east, last night. A big
one with them. Young ones all over the place. They come this way most nights."* (the count give or take
one, the way, how fresh, the alpha, the young, and how worn the trail is: [Worn trails](#memory)).
Within 60 m of a lair by day: *"No birds in this wood, and the ground's laid flat under the trees.
Something lies up here by day."* At a fear place, the Hunter reads that instead ([Memory](#memory)).

### Tribe bodies

- **When:** by day scouts and day parties within 600 m of a player, the lair (core and resting ring)
  within 300 m (150 in post-feed sleep); at night hunting groups within 600 m and the core within 300 m;
  a pack at once.
- **Where:** at the group's data spot if no player could see it within 300 m (terrain) and it's 120 m
  from everyone (×0.75 in fog); else pushed back toward the lair 40 m at a time, or round to the lair's
  far side. Hunts and packs by placement ([above](#how-a-hunt-comes-for-you)).
- **Caps:** a tribe's presence at one cluster 14 (×share ÷ 28), the first wave 8, day parties 6, alpha
  bodies 3, the agent budget and spawns a tick as for every infected ([The agent budget](#the-agent-budget)).
- **Prefabs** (**Zombies > Tribes**): Alpha (Bacon's heavy), Guard (military), Hunter (civilian and
  military), Scout, Young; empty lists fall back: the alpha and guard to Special, hunters to Common,
  scouts and young to hunters. With Alpha and Special both empty the alpha is data only. The looks'
  lists mix in by where it grew up ([Who they are](#who-they-are)).
- **Rovers** (lone ones, splinters) get bodies the same way: a lone one within 300 m, a splinter
  within 600 m.
- **Back to data** beyond 700 m of every player and out of sight, as every part; their position and
  dead written back. Stealth reads their roles: the alpha and guard as specials, a hunt on its trail as a
  horde that's never lost through stealth's search.
- **Restarts:** tribes, their members, hunger, alphas (with health, name and history), the bank, the
  taken, post-feed sleep and the life cycle (rovers, borders, fear places, habits, marks, wear, drift,
  loops, burned land, vacuums, the taken in the open, abandoned lairs, towns gone dark, displacements,
  counted nights) are saved in `state.json`; parts, trails, posts and tracks are worked out again, a
  hunt that was running starts again from the lair the next dusk, and a Clash or a Border Night that was
  running is settled out of sight. A save from before tribes seeds them on load; one from before the
  life cycle reads with its defaults.

## The life cycle

Tribes are born, grow, age, quarrel, split and die, and every rule shows itself to players
([the signs](#the-signs)). On with **Zombies > Tribes > Life cycle** (off: tribes as in phase 2,
where one of the guard simply takes the call at dawn and a tribe that dies out leaves its slot free).
Code: `Zombies/RZ_TribeLife.c` (successions, lone ones and splinters, ageing, new tribes, splits and
clans, lair moves, vacuums, the season's pull, burned land, starvation, trucks),
`Zombies/RZ_TribeBorders.c` (borders) and `Zombies/RZ_TribeWays.c` (memory, who they are).

Slow rules turn on the **season day** (about 8 hours played at Season speed 1), nightly ones on
**counted nights**. A tribe's **range centre** is its lair plus its drift (vacuums and the season's
pull move it): range checks (30 heat, the taken in the open, the ones who ran, borders, day walks)
use it.

### Succession

The vigil runs as before ([The vigil](#kill-sites-the-vigil-and-the-alpha)). At dawn comes **the
guard fight**:
- players within 2 km hear *"Shrieks from the quarry by Morton at first light, a fight by the sound
  of it. Then one voice, and the rest go quiet."*;
- one of the guard leads, Rising, with a name ([Who they are](#who-they-are)); a hunter if no guard
  is free;
- one other guard dies, or a quarter of the time leaves as a [lone one](#lone-ones-and-splinters);
- 30-40% of the rest (12 at most) break off as 1-2 splinters of 3-6;
- no day party that morning: the splinters are on the roads instead;
- **two weak season days**: one trail a night, no pincer, ring or herd, half a day party;
- its [borders](#borders) reopen, and a headline names the new leader with the place.

With no guard left the whole tribe splinters and its slot frees. **Break-up:** a tribe under 4 (never
in its first 3 season days) breaks up the same way: its alpha leaves as a lone one, the rest splinter,
and its range is a [vacuum](#vacuums). Players near hear *"Nothing calls from ... any more. What was
left of it scattered."*

### Lone ones and splinters

Both are **rovers**: records of their own, with bodies near players like a tribe's (2 lone ones and
4 splinters on the server; past that, members end up with a town's crawlers).
- **A lone one** lies up alone by day on the highest wild ground within 400 m (its body within
  300 m of a player: a big one alone on a ridge). By night it walks: into a vacuum within 4 km,
  toward splinters, else along a neighbour's range edge. It dies after 6 season days alone, unless it
  comes within 1 km of 6 or more splinter members with a free slot: then they're a [new
  tribe](#new-tribes), with it as alpha. Killed with a player within 150 m it drops 2 samples (never
  after the cure).
- **A splinter** walks to a road 1-2 km off, then another; it fights alone if it notices you. After 1-2
  season days it rejoins its tribe (half the time), joins its old tribe's clan or the nearest tribe
  within 4 km (a quarter), or ends up with the nearest town's crawlers: +2 infection each, +10 a town
  a season day at most. After the cure it dies out.

### Ageing and the Clash

The stages are as before (Rising, Prime, Decline). Each season day in Decline, a guard challenges
its alpha 15% of the time (30% while [restless](#splits-and-clans)).
- **The Clash.** With a player within 1 km of the lair, it's at the next dusk: the tribe stands in a
  ring 20-40 m round the lair while the two meet at its front, for up to 3 minutes, and the set-out
  waits. Players within 1 km: *"Two voices at the quarry by Morton, at each other, not calling. The
  rest have gone quiet round them."*
- **Who wins:** the challenger 50% of the time, +5% a season day of Decline, up to 85%. The loser
  walks off alone as a lone one (*"One of them walks off from ... alone. The rest follow the
  other."*); an old alpha under 30% health dies instead. When the old one loses, its own body walks
  away as the lone one.
- **Stepping in:** shoot the challenger and the old one keeps the tribe a while longer; shoot the old
  one and the challenger leads with no vigil and no splinters.
- With nobody near it's settled out of sight: the lone one, the headline and *"a voice nobody knows"*
  at the next dusk are the signs.
- **Death of age:** never challenged 15 season days into Decline, the alpha dies at the lair (killed
  where it stands, by the silent kill: a body to find), and the vigil and a succession follow.

### Growing up

Each season day about a quarter of a tribe's young (at least one) come of age as hunters, and the
guard's place goes to a hunter, one a season day, up to 2 under a Rising alpha and 3 after.

### New tribes

At most one a season day on the server, into a free slot that isn't kept for a split, at a free lair
candidate (outside every range by 300 m, not burned):
- **from a town gone dark:** 70 or more infection, not lived in, and no range within 2 km for 3 season
  days: a tribe of 6 at the nearest candidate within 4 km, led by someone from that town. The
  headline: *"Half of Levie went quiet this week. Not all of them stayed."*
- **from the taken:** 5 people who died to the infected in the open with no range near (players,
  wanderers, a player who turned) within 1.5 km of each other in 3 season days (3 in a vacuum): a
  tribe of that many at the nearest candidate within 2.5 km, led by the first of them;
- **round a lone one** ([above](#lone-ones-and-splinters)), and **splits** ([below](#splits-and-clans)).

**Room kept for a split:** while a kept slot is free, no new tribe founds within 3 km of a tribe of
18 or more. A new tribe's first dusk is *"A voice nobody knows"*. The design said 5 season days and
3 km for a town gone dark; at those numbers the simulator left Everon at 3 tribes.

### Splits and clans

- **A split.** A tribe at **Split at** (24) splits when there's a free slot (a kept one first) and
  room: a free lair candidate 1.5-3 km away, or the wood of a bordering tribe under 10 (which is
  pushed out: a [lair move](#lair-moves-and-displacement)). One a season day on the server, at the
  next dusk: one of its guard (if it has two) leads a scout (if it has two) and 40% of its hunters and
  young to the new lair. The daughter is Rising, keeps the mother's look and temperament, its call
  *"one note higher"* and its habits at half. Players within 3 km: *"Two calls from the quarry by
  Morton at dusk, the same call, and one of them moves off toward the woods north of Levie with some
  of the rest."* The headline: *"There's two of them up by ... now. Same call, one note different."*
  Both sides' borders reopen.
- **Restless.** At Split at with no slot or no room: *"Two voices calling from ... tonight, one over
  the other"*, hunger +2 a counted night, border fights and challenges twice as likely.
- **Clans.** A daughter and its mother, lairs within 4 km, both 8 or more for 3 season days, are one
  clan: they never fight, each answers the other's dusk call (*"The same call answers from ..., one
  note different."*), they go round each other's fear places, and a splinter joins its old tribe's
  clan first. A clan ends when one is left or they're 4 km apart. A clan of three that holds raises
  [the great one](#the-great-one); after it dies there's no clan for 8 season days.

### Lair moves and displacement

**Disturbance** (0-100; x1.2 for a bold tribe, x0.6 territorial; -10 a season day):

| Cause | Disturbance |
|---|---|
| A player within 150 m of the lair by day | +2 a minute |
| A new remembered noise (8+ heat) within 200 m | +5 |
| A guard killed within 150 m of the lair | +15 |
| Shells within 200 m (the Fire Mission perk and contract: Bastion's) | +50 |
| Fire within 50 m (the fire system) | +60 |
| The lair burned (fire within 30 m; it marks whoever set it) | +100 |

At 100 the lair moves (once a tribe and twice on the server a season day): to another lair candidate
in its range 400 m or more away, else to a new range 1.5-4 km off in free wild land. **Displaced:**
when one faction caused more than half of it, the new lair is 2 km or more from that faction's
holdings and never within 1.2 km of a safehouse or a holding, 2 a season per faction (then it just
moves within its range). Bastion's shells and its Cull and Clear the Trail
([MISSIONS.md](MISSIONS.md#the-packs-contracts)) are the faction causes; a pack a job holds never
moves. A move out of its range starts **The Tribe Moves** ([MISSIONS.md](MISSIONS.md#world-events)). Players within 2.5 km of the new lair: *"Something big is moving through the
trees from the quarry by Morton toward the woods north of Levie, a lot of them."* Its parts walk
there as data (bodies near players walk too), and its place, trails and posts are worked out again.

- **Nomads** keep a loop of 3-4 lairs within about 2 km and move on every 2-4 season days. They leave
  no vacuum and no fear places.
- **Abandoned lairs** stay a find for 2 season days. A player within 25 m finds it once: *"Somebody's
  things at the back of it: what's left of Marta the runner."* (the first of the people it took), and
  a sample if a courier was among them (once). A nomad's empty old lair reads *"... Whatever lay up
  here has moved on. Never the same place twice."* *Simplified:* the find is a line, not items.

### Vacuums

A range left empty (its tribe moved out, broke up, was absorbed or died out) is a vacuum for 3 season
days: tribes within 3 km drift their range centre 200 m into it (500 m from the lair at most), a new
tribe forms there from 3 taken instead of 5, and lone ones head for it. A player within 2.5 km of it
who heard no call at dusk gets *"Nothing calls from the quarry by Morton tonight. Nothing has for a
while."*

### The season's pull and Winter

- **From Overrun** free lair candidates nearer a lived-in place score higher (up to +2 within 3 km),
  so moves and new tribes come closer to people.
- **From Aftermath** each range centre drifts 100 m a season day toward the nearest lived-in place,
  stopping at its no man's land plus 350 m; the dusk call adds *"They come closer to Morton every
  night."*
- **Winter** (Season > Rule sets), from Overrun: a tribe within 2.5 km of an empty village (40+
  infection, not lived in) moves into the barns at its edge. Its place becomes *"the barns by ..."*,
  and it shares the village with the crawlers at night.

### Burned land

The fire system (`Interactions/RZ_Traps.c`) tells the tribes where things burn. An event's fire
(Burning Town, a gas station) is no lair for 3 season days and no trail for 6 counted nights (about
3 season days) within 150 m; a player's fire (a molotov, a doused doorway) is no lair for 3 season
days and no trail for one counted night, so nobody keeps a corridor open with matches. Tribes whose
range is near have their trails worked out again.

### Starvation

A tribe at hunger 100 for two counted nights running loses a fifth (at least one) to the nearest town:
they end up as its crawlers (+2 infection each, +10 a town a season day), and its hunger drops to 70.
Players within 1.5 km of the town: *"Some of the quick ones from ... came down into Levie in the
night, and stayed with the slow ones."*

### Trucks taken as data

At night, a hunting part as data within 200 m of a supply truck on the road, with no player within
1 km, takes it 15% of the time, one on the server every 3 counted nights. The truck fails the way any
truck does (`RZ_SupplyLines`: a shortage where it was going, its people told, the world memory's
signal), it and its crew are gone, its driver counts as taken, and the tribe learns the road hour.

## Borders

Two tribes **border** where their ranges come within 150 m of each other with no river or valley floor
between (sound carries badly over them), or where a crossing lies within 300 m of both ranges (the
bridge is the border). Worked out each dusk.

- **The calling contest**, each dusk. **Strength** = (hunters + guard) x the alpha's stage (Rising
  0.8, Prime 1.2, Decline 0.9, none 0.6), x1.3 territorial. The border moves 50 m toward the weaker,
  300 m at most. A Radio Operator within 3 km: *"Calls across the bridge at dusk. The woods north of
  Levie answered weaker tonight."* Every dusk call adds its border: *"The woods north of Levie answer
  from across the bridge."*; *"answer quietly"* on an old border; *"From ..., a new voice is testing
  the bridge."* when it reopened; a clan answers in the same call ([Clans](#splits-and-clans)).
- **Old and reopened.** A border with no fight for 5 season days is old: fights half as likely. A
  succession, a challenge won or a split on either side reopens it for 2 counted nights: fights twice
  as likely.
- **Fights.** Once a counted night per border, when both tribes have set out and either is hungry or
  worse, it's a crossing, or it reopened: 20% (half on an old border, twice reopened, twice with a
  restless tribe, 80% at most). The weaker side (strength x0.8-1.2) loses 2-4 members and won't hunt
  toward the winner for 3 counted nights; the winner's hunger falls 10; both hunt one trail the next
  night; a town within 1 km stirs (+20).
- **A Border Night.** With a player within 800 m (2 on the server a night), the fight is staged: both
  sides' hunter groups walk to the border point and wait there, 5-20 minutes. Players within 1.5 km:
  *"Calls from both sides of the bridge, near the quarry by Morton. Then a great deal of shrieking."*
  The side players thinned by 2 or more than the other loses (else strength decides). If nobody is
  within 150 m of the point, the loser's bodies there are killed where they stand (the silent kill's
  kill-and-mark: bodies to find, no samples, no credit); the rest of its losses just don't come home.
  The groups go home. Bacon's infected share one faction, so the fight itself is heard, then found.
- **Absorbing and merging** (the next season day, with no bodies out). A loser under 8 with no alpha,
  or a Declining one, joins the winner (30 at most; 30% splinter off, its alpha leaves alone). Two
  bordering tribes under 10, one with no alpha, join under the other's alpha. Players within 3 km:
  *"One call from ... tonight, where there used to be two."* The slot frees. Clans never fight.

## Memory

Code: `Zombies/RZ_TribeWays.c`.

- **Fear places.** 8 or more of a tribe killed within 300 m in one counted night make a fear place
  250 m across: 3 counted nights (5 cautious or cunning, 1 bold; nomads none; 4 a tribe). Its trails
  and day walks go round it, and its clan's. Kills inside don't extend it; once it's over, no new one
  within 500 m for 3 counted nights (they get used to it). A Hunter within 125 m: *"Old tracks, all of
  them going round this place, none through it. They don't come through here any more."*, and after:
  *"Fresh tracks through here again, where there were none. They're coming through here again."*
- **Habits** (3 a tribe). Each person it takes teaches it the place (+40, +60 cunning):

  | Habit | Learned when it took someone | At 50 or more |
  |---|---|---|
  | **The crossing** | within 150 m of a bridge or a ford | A hunter group walks there at set-out and waits until it's sent home |
  | **The road hour** | a driver, on a road (a supply truck's crew among them) | When its hour is within 2 hours of the set-out, a hunter group waits on that road from then |
  | **The back door** | within 400 m of a lived-in place's wire | The young wait at that wire, outside no man's land |
  | **The shots** | with shots within 300 m in the last 5 minutes | It hears from 1.5 times as far: noises as data, and shots for its steered groups |
  | **The bells** | with a loud noise within 500 m in the last 10 minutes | The same |

  Every habit falls 10 a counted night, and 30 more on a night when 2 or more of the tribe die within
  300 m of it: two such nights at its place break it. Gone under 10. A habit place ahead of a target
  makes an [ambush](#how-they-move-and-hunt) likelier.
- **Marks.** Kill a tribe's alpha or a guard (the nearest player within 100 m), or burn its lair, and
  it knows your character (this life): strength 100, -25 a counted night, 4 a tribe, only inside its
  range. You're told once: *"They know you at the quarry by Morton now."* Its scouts see you from 1.5
  times as far, and at night its hunt goes for you first, heat or not (never into one of its fear
  places); it never switches.
- **Worn trails.** Each trail wears +1 a counted night it's walked and -1 a night it isn't (0-10), and
  tonight's first trail is picked by wear. A Hunter reading a night's track: *"A path worn through the
  bracken. Most nights."* (6 or more), *"They come this way most nights."* (3), *"Faint: new ground for
  them."* (a new one).
- **Bonds.** Two taken at one kill site within 10 minutes are a bond among its young (3 at most). When
  a young of the tribe is put down near a player, a young of a group within 120 m runs reckless
  (released with a 4 s sprint), and a Hunter within 150 m: *"That one came straight at us, after we
  put down the other."* *Simplified:* the young aren't individuals, so the bond is a count and the
  reckless one is the nearest young.

## Who they are

- **Names.** Every alpha has a name (the notes' name lists, seeded) and who they were: the column's
  were drivers of the Day 2 column; a military tribe's are mostly soldiers, a farm valley's mostly
  farmers; else a resident, a farmer, a driver, a runner or a soldier.
- **Echoes**, 3 season days into its lead:

  | Who it was | The tribe |
  |---|---|
  | **Soldier** | Sets out at 17:30 and comes home at 05:30; a guard left home stands at the lair's edge 20:30-21:30. A Soldier or a Police Officer within 2 km: *"They set out from ... on the shift change, half five on the dot. Somebody taught them that."* |
  | **Farmer** | Comes home at first light the long way round, by a field near the lair; prefers a barn when its lair moves |
  | **Driver** | Its trails to roads and crossings weigh more |
  | **Runner** | Its young wait at the nearest house's wire at dusk |
  | **The column** | At dawn a hunter group walks toward the line, stops 600 m short of the warning zone, and goes home. Players near: *"At first light some of them walk out toward the line, stop short of it, and stand there a while."* |
  | **Resident** | No echo |

- **Looks.** Where it grew up sets its look: military within 1.5 km of a Bastion holding or a
  checkpoint, dust-grey at a quarry, work clothes at a barn, silo or mill, outdoor in the woods, else
  civilian. Its hunters, scouts and young come 70% from that list (**Zombies > Tribes > Military,
  Work clothes, Dust-grey, Outdoor prefabs**, if set). Soldiers it takes (a Soldier's or a Police
  Officer's character, a soldier, a patrol, a deserter) raise its share in military clothes, so its
  look mixes as it hunts. Empty lists: as before.
- **Histories.** Each alpha keeps its last 6 events (took Marta the runner, won the bridge off the
  woods north of Levie, moved, split, starved, took a supply truck, how it took the lead), shown by
  `/tribes`. The headlines (never broadcast: the latest are read on joining; never a player's name)
  carry them, always with the place: a leader falls (*"The calling at the quarry by Morton stopped
  tonight. Josef Hruby led them, and doesn't any more. Josef Hruby won the bridge off the woods north
  of Levie."*), a guard fight, a challenge won, a death of age, a split, a town gone dark, a truck
  taken. The factions speak of them in their own words ([People](#people)).

## The signs

Every rule above was checked against the design's signs table: each shows within one session.

| Rule | What players meet |
|---|---|
| Succession | The vigil's silence, shrieks at first light, nobody on its paths that morning, splinters on the roads, a voice nobody knows, a headline |
| Lone ones | A big one alone on high ground by day; one walking at night |
| Ageing, the Clash | The ragged call; the Clash's ring and lines; a lone one; a headline |
| New tribes | A voice nobody knows where there was none; the dark town's headline |
| Splits, restless, clans | Two voices; two calls and one moving off; the same call answering one note different |
| Lair moves, displacement, nomads | The move's line, a new place in the call, the old lair's find |
| Vacuums | *"Nothing calls from ... tonight"*; neighbours' day walks closer |
| The season's pull, Winter | *"They come closer to Morton every night"*; a call from the barns by a village |
| Burned land | Trails off the burned ground (tracks), a lair moved |
| Starvation | Quick ones lying down with the slow ones in a town; its infection up |
| Borders | The answer in every dusk call, the Radio Operator's contest, Border Nights, the dead found at a bridge, one call where there were two |
| Fear places, getting used to them | The Hunter's two lines; nobody coming through, then somebody again |
| Habits | A group waiting at the bridge or on the road at its hour, the young at the wire; worn tracks |
| Marks | *"They know you at ..."*; a hunt that comes for you |
| Worn trails | The Hunter's wear lines |
| Bonds | One that comes straight at you, and the Hunter's line |
| Echoes | The shift change and its line, the column at the line at dawn, a farmer's tribe in the fields at first light |
| Looks | Military clothes among a tribe that took soldiers |
| Histories | The headlines on joining |
| Trucks | A truck that never arrives, and the shortage |

**Waiting** (no sign of their own yet, or another phase): the clans' relay along the ridges, a range
that moves as its border does (the border's offset is data only), hunting grounds following the prey,
the stragglers after a bad night, tracks always fresh in snow. The people's side shows through its
own lines, marks and the journal ([People](#people)); the great one through its call, the Gathering
and the Night of Many Calls ([The great one](#the-great-one)).

## How they move and hunt

The behaviour layer (**Zombies > Behaviour**; code `Zombies/Behaviour/`): our code drives the tribes'
bodies beyond striking range, and Bacon fights at striking range. It's built on the zombie lab's tests
B1–B20 (Bacon 1.8.0; [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#zombie-lab-bacons-real-ai-measured)), and
uses only what the lab found works. Loose ones, hordes and missions' infected stay Bacon's own (with
stealth); crawlers too, but for their [daily life](#crawler-life). With **Enabled** off, or another
zombie mod's bodies, tribe groups take held waypoints.

**The hooks** (`RZ_BaconHooks.c`, the only Bacon-specific file): Bacon's loiter point returns our slot
(B3); our dynamic stance (B7); a steered body ignores gunfire, shared targets and doors (B2, B18) and
never shares a target, and an engaged one shares only a target it saw itself in the last 2 s (B5); the
climb gate refuses a grab point above the kind's limit (B8); a mute switch (B3); and a stamp when Bacon's
melee test passes (for hit attribution; B14 found the melee read itself doesn't work).

**Modes.** Each body is in one:

| Mode | Who drives | Perception | How it moves |
|---|---|---|---|
| **Steer** | Our code | 0, set in the spawn frame (B1: 0 of 24 engaged at 6–150 m) | Walks to its slot |
| **Engage** | Bacon | 1, then stealth's | Bacon's chase and swing; RUN wanted (B6: else it walks in) |
| **Recover** | Our code, waiting | 0 | Its slot and Bacon's loiter point at the last-known spot or home; steered again after 15 s (B5) |

- **Released** to Bacon by any of: within its kind's reach of its target on a strike; any player within
  8 m in terrain line of sight (**Contact radius**) or 3 m without; a hit; an unsuppressed shot within
  30 m; a member of its group released within 15 m that second.
- **Taken back** after 10 s engaged by any of: its target beyond 2.5 × its reach for 8 s; its target
  lost for its memory time and 20 m away; its group gone home; its target dead or gone. Never with a
  player within 15 m in sight. 5 s at least before it can be released again.
- **Stealth** sets factors only on engaged bodies; its search never runs on a group the layer drives
  (our memory loses a player instead).

**The kinds** (speed caps are fractions of the sprint speed, about 4.4 m/s, B7):

| | Young | Hunter | Scout | Guard | Alpha |
|---|---|---|---|---|---|
| Walking (walk-stop: 0.45–0.8 m/s on average) | Bacon's walk | Bacon's walk | Still on its post | Bacon's walk | Capped about 1.4 m/s |
| Marching (held waypoint) | Bacon's run | Capped about 2 m/s | — | About 2 m/s | About 2 m/s |
| In a fight | **Sprint** (about 6 m/s) for 6–8 s, then run (about 3.7 m/s) for 4–6 s, and again (**Young sprint**) | Run, the lunge | Run | Run | Bacon's heavy run |
| Stance | 0.85–0.95 | 0.72–0.82 (silent while stalking) | 0.9 | Bacon's | Bacon's |
| Sight by day / night | 180 / 156 m | 220 / 190 m (**Hunter sight**) | The scout rule | 200 / 173 m | 160 / 139 m |
| Memory | 30 s | 90 s | 10 s | 60 s | 120 s |
| Reach (released within) | 35 m | 25 m | 15 m | 30 m | 40 m |
| Climbs (grab height) | 1.4 m | 1.4 m | 1.4 m | 1.2 m | Never (goes round: B17) |

Each body also gets a speed factor of 0.88–1 on its caps, ±0.04 on its stance and 1–3 m of slot
jitter, so a group strings out and no two move alike.

**The young are a little faster than you.** A player runs at about 4 m/s and sprints at about 5.5 m/s
(the base game's soldier; VERIFY in play); Bacon's bodies run at about 3.7 m/s, and a sprint set every
0.25 s gives about 6 m/s (lab B7). So with **Young sprint** on, an engaged young within its reach (35 m)
of its target chases at a sprint for **Young sprint seconds** (7, ±1), then runs for **Young rest
seconds** (5, ±1): it catches a sprinting player in the open within about 10 s, and a door, a fence
over 1.4 m, a car or a broken line of sight still loses it. Never in the gentle start. Its speed factor
scales the sprint's length rather than its speed (a cap above 0.7 doesn't slow a body, B7), so a group
strings out. Off: bursts of 2–4 s every 8–15 s. Hunters, guards and scouts run at Bacon's run or their
caps, the alpha's only sprint is its charge, and crawlers are unchanged.

**Senses** (steered groups with a player within 250 m, once a second, the leader's eye): a notice meter
per player rises by **Notice rate** (0.15) × stealth's visibility × facing (1 within 70°, else 0.25) ×
(1 − d/R)^1.5 × line of sight, and falls 0.1 a second otherwise; within 12 m in sight it fills in 2 s. At
0.4 the group stops a few seconds and one body makes a sound (heard before seen); at 1 it's aware and the
tribe decides (a day hunt, the lair fighting, or the night's hunt). Line of sight is the terrain between
(script samples) and within 80 m a physics trace (B13: 0.002 and 0.021 ms; 12 a second, **Sight checks a
second**), each check kept 2 s. Hearing: an unsuppressed shot gives a group a lead from 170 m × its kind
(suppressed 30 m), 15% of the distance off; other sounds, blood and shrieks below. The lair's ring by day
is deaf. A hunt's target is
remembered where it was last seen, never where it really is; a night hunt on its trail keeps contact by
sight alone (lying still doesn't shake it), a day group needs to see you.

**Hearing, blood and shrieks** (`Zombies/Behaviour/RZ_Hearing.c`). Every sound gives a steered group
(not hunting, not the deaf ring) a **lead**: a spot off by 15% of the distance, walked to look, never a
fix on you. A fresh lead (under 30 s old) is replaced only by a nearer one. Each kind multiplies the
radius (crawlers ×1.5, the young ×1.2, alphas ×0.8; a pack's shots habit ×1.5).

| Sound | Radius |
|---|---|
| A player sprinting / running / walking / crouched (their speed over the last second; still or prone: nothing) | 30 / 20 / 8 / 3 m, once a second |
| A player in a moving vehicle | 200 m, every 5 s |
| A heat of 8 or more at once (bells, noisemakers, sirens, a banged door, a fire): every heat the map is given is heard (a modded `RZ_HeatMap.AddHeat`) | heat × 10 m, at most 400 m |
| A heat of 2 to 8 at once: a door, a forced lock, a search | 40 m |
| Smaller heat, and trickles (presence, a generator) | not a sound |

A sound within 25 m of one heard in the last 10 s, no louder, isn't heard again.

**Blood.** A bleeding player (the damage manager's bleeding state) drops a point of scent every 10 s:
the last 6, kept 60 s after the bleeding stops. Every 10 s a group within its kind's blood radius of
the newest point (hunters 250 m, the young and alphas 200, scouts 150, guards 100, crawlers 60) gets a
lead there, ±20 m. It pulls them along your trail; it never gives them you.

**Shrieks.** When a group with young in it becomes aware of a player, half the time one shrieks (a
sound): its own pack's groups within 200 m by day (300 m at night) get a lead at the shrieker, ±20 m,
and players within 300 m hear *"A shriek, close by, and an answer from further off. They know."* (one
line in 90 s). A group shrieks at most once in 30 s, and a shriek heard is never passed on, so it never
chains across the map. *Simplified:* scouts don't shriek (their call is the scout rule's). The built
Aftermath screamers keep their heat; they're the town version.

**Reactions** (to a shot, fire, losses, wounds and vehicles):

| Event | Crawler | Young | Hunter, scout, guard | Alpha (by temperament) |
|---|---|---|---|---|
| **Gunfire heard** (a lead) | Bacon's own, the built drift | Rushes the lead at a run | Freezes 3 s, then marches to a point a third of the way off to one side of the shot (it comes round, not straight in) | Bold: comes at a run, with a roar. Cautious: holds. Others: as hunters |
| **Fire** (a building burning or smouldering: traps' scorch radius) | — | No slot inside it; an engaged body in it is taken back and moved 20 m off | Same | Same |
| **Losses** (the group down by half in a hunt) | — | Keeps coming | The regroup, once a target: 60–100 m back toward cover at a run, then the hunt goes on; halved again, it goes home | Bold: presses. Cautious: goes home. Others: as hunters |
| **Wounded** | — | Keeps coming | A hunter under 40% health breaks off to 50–80 m toward cover and holds 2 minutes, then comes back | Under 50%: the group goes home |
| **Player in a vehicle** | Lets it go beyond 20 m | Chases it to 60 m | Lets it go beyond 30 m | As hunters; it charges a vehicle within 30 m only at foot speed (under 6 m/s) |

At most **3 elements** join one hunt, and at most **2 strategies** run near one player cluster (150 m):
a third hunt there takes one already running.

**Doors** (`Zombies/Behaviour/RZ_Doors.c`, on top of the built doorway blockers, banging and the siege
cap; Bacon opens shut, unlocked doors itself, B10):
- **Another way in.** After 30 s banging at a locked building's door, the infected there go round to
  another of its outside doors within 20 m that stands open: a group the layer drives runs there as one
  (its engaged bodies taken back), any other is sent with its held waypoint (VERIFY that Bacon follows
  it with a target inside). With none, they wait out the siege cap and leave.
- **A weak door breaks.** Only for a pack's leader, or a bold pack's night hunt with 3 or more at the
  door, and only a door whose building's or door's prefab name has a **Weak door words** word (Shed,
  Barn, Garage, Wood). Never within 150 m of a holding, a safehouse or the lab (their outposts and
  quartermasters with them). At 20 s of banging a tell (*"The door is starting to give..."*, more heat);
  at 45 s the lock is forced (the built Forced state: open and broken until the reset) and the door
  swings open. Hiding behind a locked door stays a real escape, but not always behind a shed's.

**Formations** (slots written every second near players, every 3 s beyond 300 m): file on a night trail
and going home, a loose line by day, a wedge for the core, flanking pairs, a ring at the lair, the kill
site and the vigil. Spacing widens as the group nears its target (4 m at 200 m to 20 m within 40 m), and
a strike fans out on an arc round the target, closing. Slots are kept 3 m apart (**Spacing**) and 8 m
off the alpha, enforced in code (B20). Walk-stop takes the group's waypoint off its list so Bacon walks
to slots under 25 m (B7); a march puts the held waypoint at the goal, at most every **Order gap** (5 s) unless the goal moved 15 m, and the
whole layer moves at most **Orders per 5 s** (40) waypoints in 5 s (the rest wait a tick).

**Strategies** by temperament (the top two weights, 70/30; a Rising alpha's tribe uses the cautious
weights; **Strategies** off: everyone rushes):

| Strategy | Bold | Cautious | Cunning | Territorial | Nomadic | What it does |
|---|---|---|---|---|---|---|
| **Rush** | 3 | 0 | 0 | 1 | 1 | Fanned out, straight in at a run |
| **Stalk** | 1 | 3 | 2 | 1 | 2 | A shadow point 80–150 m off by day (**Stalk distance**; 50–100 at night), behind the target's line of travel and toward cover (terrain that hides it, then a wooded square); walk-stop, hunters silent; frozen and low for 3–6 s when you look its way within 120 m; strikes at a moment or after 4 (night 6) minutes |
| **Pincer** | 2 | 1 | 2 | 3 | 1 | Two groups: a sidestep first, then flank points 140 m out by day (100 at night) at 70° (**Flank radius**, **Flank angle**), toward cover; both strike when both are set, or 60 s after the first; one group alone stalks |
| **Encircle** | 3 | 0.5 | 1 | 2 | 1 | 5+ bodies, never by day: a ring of 120 m with a 60° gap (always a way out), each group a share of it, closing 0.7 m/s; strikes at 35 m, at a moment, or after 3 minutes |
| **Flush** | 2 | 1 | 1 | 2 | 1 | The target in a wood or a building, 2 groups, at night: the first pushes in at a walk, groaning every 6 s; the others wait 40–60 m off on the far side; all strike when the target moves 8 m or breaks cover, or after 90 s |
| **Bait** | 0 | 0 | 3 | 0 | 1 | With young in the hunt, at night: the young rush in loud (their own rush); the hunters come up the target's back trail at a walk, quiet, and strike when the young have it in a fight, when it turns to them, or after 90 s |
| **Herd** | 1 | 1 | 2 | 0 | 2 | A crawler town within 500 m, 5+ bodies, at night: an encircle whose gap faces the town, closing; strikes at the town's edge, at 35 m or after 4 minutes, and the town stirs (+20) |
| **Ambush** | 0 | 2 | 3 | 2 | 0 | A crossing or one of its habit places within 300 m ahead (+2 for a habit place), at night: the first group lies still and quiet 20–40 m off it, toward cover, and strikes when the target passes within 30 m; after 5 minutes it stalks instead. Other groups stalk |
| **Give up** | | | | | | Lost for its memory time: it searches the last-known spot 45 s, then goes back to what it was doing, and won't take that player again for 10 minutes unless it sees them afresh within 50 m. **Hand-off:** first it passes the last-known spot and heading to the nearest group of its tribe that's closer to it (within 600 m) and not hunting, which walks to look there, primed to notice them (the same player only) |

A **moment**: you stand still 10 s, crouch or go prone, or face more than 100° away. Every strike starts
with a sound from the leader a second before the release. By day only rush, stalk and a two-group pincer;
in a tribe's two weak days no pincer, encircle or herd. Lone ones and splinters hunt alone, by day's rules.

**Navigation.**
- **Water** (B16): a slot, and the straight line to it, is pulled back short of any water; the land grid
  says where that check can be skipped. Trails cross rivers only at the grid's crossings.
- **Fences** (B8): the young, hunters and scouts vault grab points up to 1.4 m (**Climb limit**), the
  guard 1.2 m, the alpha nothing; a refused climb counts as stuck at once.
- **Stairs** (B11: they don't climb them reliably): a target more than 2.5 m up within 15 m across, not
  reached for 20 s, and the body waits below until the target comes down. A roof or an upper floor is a
  fair escape.
- **The unstick ladder** (a steered body that moved under 1.5 m in 10 s with its slot 8 m off, or
  refused a climb): a 4 m sidestep, back off 10–20 m, another way 45° off, then (**Unstick out of
  sight**) moved up to 15 m ahead only where no player is within 60 m or has terrain line of sight within
  150 m, then give up. A 10 m square stuck in twice is avoided for 30 minutes. An engaged body that
  closes under 1 m in 8 s on a target within 40 m is taken back with a sidestep.
- **LOD** (B12): bodies think every loop within 60 m of a player, then at LOD 2, 4 and 6 beyond 150 and
  300 m (**LOD bands**).
- **The resting ring asleep** (L14, as the crawlers lie up; with **Sleep lying crawlers** on): the lair's
  deaf ring by day, holding, with no player within 250 m, has its steered bodies' AI off; a player
  within 220 m, a hunt or any other order wakes it.

**Releases and tells.** A release comes 0.2–1.2 s after its cause, each body its own (a hit or a player
within 3 m: at once). Not hunting, hunters freeze 3–5 s when a player within 120 m looks their way
(at most every 20 s), and the young jump between places in their group every 6–12 s (3–7 m).

**Attacks.** Bacon's melee stays the hit and its damage; each kind adds one move, and its effect is
status (a hold, a stagger, a knockdown, a light bleed), never extra damage. One special move lands on a
player every **Special move gap** (10 s) at most, from all the infected together, none in the gentle
start; **Own attacks** off gives plain Bacon melee.

| Move | Kind | How | Effect on the player | Caps |
|---|---|---|---|---|
| **The lunge** | Hunter | From 6–10 m, more than 60° out of its target's view, a 1.5 s sprint (SPRINT set every 0.25 s, B7) into Bacon's swing | If it lands (a hit within 3 s: the body whose melee test passed within 1 s and 2.2 m, else the nearest lunger within 2.5 m): the bite chance ×1.5 (never over 0.6) and a light bleed on an arm or a leg (once a player in 30 s) | 20 s per hunter |
| **The ankle grab** | Crawler | Any hit from a town's own within 2 m while it's hunched (stance 0.7 or less, as they lie up) or the player is crouched or prone | **Held:** speed capped at 0.25 for up to **Hold seconds** (3). **Hold kicks** (3) presses of Jump kick you free; it ends too if the one holding dies or is left 3 m behind. A hit while held counts as grabbed (×1.5 bite chance). *"Something has your ankle."* | 20 s per crawler (and after you kick free); one hold on a player at a time; never two within 5 s |
| **The leap and stagger** | Young | Running at 3 m/s or more with its target 3–6 m ahead (within 25°), it sprints the last of it into Bacon's swing (B14: a scripted jump goes up, not forward) | A hit within 3 s of the leap, from it within 2.5 m: **staggered**, speed capped at 0.6 for 0.8 s | 15 s per young; one stagger a player per 6 s |
| **The charge and knockdown** | Alpha | Its target 12–30 m away, on foot and in its sight, never with 3 or more other infected within 10 m of them: a roar, then a straight sprint of up to 4 s (about 6 m/s) | Reaching within 1.6 m: **knocked down**, forced prone (the stance's own animation) and movement off for 1.2 s, then Bacon's swing. *"It hit you like a car door..."* Missing it: it overruns and stumbles, capped at 0.2 for 2.5 s, the window to hurt it | 30 s per alpha and per player; the knockdown never with 3 or more others within 10 m |

**The effects run on the player's machine** (`Zombies/Behaviour/RZ_AttackEffects.c` decides on the
server; `Zombies/RZ_PlayerEffects.c`, `RZ_PlayerEffectsComponent` on the player controller, plays
it), because a player's movement is predicted there: owner-only RPCs for a hold, a stagger, a
knockdown or clear, and one back, "kicked free", which the server checks against the hold it gave.
Every effect ends on a timer on the client and is cleared on death or a respawn. Breaking a hold is
the base game's Jump (Space, the pad's jump button), so it needs no new binding, and its prompt is the
base game's hint with a timer ([UI.md](UI.md#controllers)). Each effect has one line, at most one
every 45 s a player. **Client effects** off, or no component on the player controller: no hold,
stagger or knockdown, and the lines and the lunge stay. The effects' calls are VERIFY items until a
client has seen them ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#6-the-verify-list)).

### Crawler life

Code: `Zombies/Behaviour/RZ_CrawlerLife.c`, with **Zombies > Behaviour > Crawler life** (on) and the
built placement ([Towns](#towns-the-crawlers)). Only their walking slots, their sleep and an optional
cap: their senses and their fight stay Bacon's, and a group that's drifting to a noise, walking
somewhere, in a stirred town or with a player within 150 m is left alone.

| When | The town's crawlers |
|---|---|
| **Day** | Lying up where they were placed, hunched (stance 0.6). With **Sleep lying crawlers** on, a group with no player within 80 m, no shot within 255 m and no noise within 300 m in the last minute has its AI switched off (lab B12: awake in 0.2 s, moving in 6.5 s; a prone pose didn't hold, so they're hunched, not flat) |
| **Waking** | A player within 40 m, a shot or a noise in their hearing, the dusk or a stir wakes them: the AI on, standing, and one groan, the player's warning |
| **Dusk** | A herd round each group's doorway, 6 m across |
| **Night** | Each town's herd follows an anchor that walks a loop of 3–4 points round its streets at 0.5 m/s, each group in its own spot of it (8 m across); a stirred town's anchor heads for its last noise; with a pack's group out in the town, the anchor steps 60 m aside (they give way). Slots are never more than 20 m from where a body stands, so Bacon walks them there (lab B7: further, it runs) |
| **In a fight** | Bacon's. **Crawler speed cap** (0 = off) caps them, e.g. 0.4 for about 1.8 m/s; the lab found the cap works but the body probably runs in slow motion, so it waits for a client to see it |

**Performance.** Hooks only read fields (B15: no cost above the noise); the tick is vector maths for the

groups near players; 100 agents with half steered measured p99 11.93 ms on the lab box (the target is
12.5). The status log's behaviour line counts bodies per mode, hunts, releases, take-backs, unsticks,
moves landed, hand-offs, leaps and charges (with stumbles), young sprints, holds (kicked free),
staggers and knockdowns, sound and blood leads, shrieks, doors gone round and broken, regroups, rings
asleep and woken, and the crawlers asleep, woken and herded.

## People

How the Zone's people live with the packs (**Zombies > Tribes > People**, on by default). Code:
`Zombies/RZ_TribePeople.c` (words, voices, marks, knowing, skills, your dead run, the dark corners,
the moat, frenzies, the cure, the last week), `Zombies/RZ_TribeItems.c` (the tag, the call box,
the notebook's opt-out), `Missions/Tribes/RZ_TribeContracts.c` and
`Missions/Events/RZ_TribeEvents.c` ([MISSIONS.md](MISSIONS.md#the-packs-contracts)). How the lines
are written: [WRITING.md](WRITING.md#writing-the-infected); who the factions are:
[LORE.md](LORE.md#what-the-infected-do).

### The words

Every speaker has its own word, and every line carries the place, so players can tell four names
are one pack.

| Speaker | A pack | With the place | The leader | The young | The towns' slow ones | A lone one |
|---|---|---|---|---|---|---|
| **The Haven** | "the Quarry lot" (the lair's kind), or "Josef's lot" when it knew the leader | "the Quarry lot, up at the quarry by Morton" | by name when it knew them (about 1 in 3, never a soldier or the column), else "the big one" | by name | stay-at-homes | "the one on his own" |
| **Bastion** | "Contact Papa-two", numbered as found on its contact board | "Contact Papa-two, the quarry by Morton" | the heavy | new contacts | statics | a single |
| **The Red Hand** | a crew's nickname, after something it would rather forget: the Commuters (the column), the Postmen (a soldier's echo: always on time), the Farmhands, the Choir (bold or restless: loud at dusk), else one of eight seeded | "the Postmen up the quarry by Morton" | the boss | new hires | the furniture | a boss who lost |
| **Halcyon** | "Pack P-02", in the order the packs formed | "Pack P-02 (the quarry by Morton)" | the index | recent conversions | sessile cases | an isolate |
| **The Long Table** | the neighbours | — | — | — | — | — |

The journal keeps, under the place, every name the character has heard: *"The quarry by Morton: the
Quarry lot (Haven), Contact Papa-two (Bastion), the Postmen (Red Hand), Pack P-02 (Halcyon)."*

**Tokens** for any filled text (`RZ_WorldFacts.Fill`): `{tribe:N}` is pack N in a speaker's word with
the place, `{alpha:N}` its leader in that speaker's word (the Haven's name when it knew them), and
`{young:N}` its newest named young (described where a name isn't allowed), else the speaker's word for
the young. The speaker follows a dot, `{tribe:3.bastion}` (`haven`, `bastion`, `redhand`, `halcyon`,
`table`); without one it's the Haven's words. A pack that's gone leaves the token as it is.

### Voices

- **At dusk**, once a game evening, each faction tells its Known one line about one pack, in its own
  word, through the voice bank (kind `infected`, [WRITING.md](WRITING.md#the-voice-bank)) when it has a
  line for the manner, else the built one. They've Heard of it, in that word.

  | Faction | Which pack | Line |
  |---|---|---|
  | Bastion | one it has just put on its board: a pack whose ground lies within 3 km of a Bastion holding, while its presence is 0.2 or more | *"Contact Papa-two, the quarry by Morton, twenty plus, one heavy. Cull pending. Bastion out."* (*"No cull. No reply."* under 0.2) |
  | The Haven | the hungriest hungry pack within 3 km of a house | *"Nobody on the roads by the quarry by Morton after seven. The Quarry lot are out."* |
  | Halcyon | a tagged pack | *"Pack P-02 (the quarry by Morton): the index is in its third week as lead. Please don't shoot it."* |
  | The Red Hand | its fed crew, 3 evenings in 10 | *"The Postmen up the quarry by Morton. Same as us: the boss is whoever's got something on the rest."* |

- **Other lines:** Halcyon's *"P-02 took Marta. The index is in good condition. We're sorry."* (a tagged
  pack takes someone named, half the time) and *"Its tag has stopped moving. That was a year of data."*;
  the Haven's *"Whatever they took from …, they want it back."* (Furious); Bastion's *"… culled. Not our
  sector now."* (Cull); wanderers' road news ([below](#the-factions-sides)).
- **The Red Hand's night rate.** At night, within 1.5 km of a road a pack hunts at its hour (a road
  habit), the Red Hand's toll is one Favor more: *"Night rate. The Postmen are out."*
  ([REPUTATION.md](REPUTATION.md#checkpoints-roadblocks-and-camps)).

### Marks

Each faction writes its mark where a road enters a range it knows: the road square on the range's
edge nearest the nearest town, else the edge toward it. The Haven marks packs within 3 km of a house,
Bastion the ones on its board, the Red Hand its fed crew and packs within 3 km of its camps, Halcyon
tagged ones. A mark is rewritten every 3 season days while the faction still knows the pack, so an old
mark can be wrong.

| Faction | Mark |
|---|---|
| Haven | *Haven chalk on a post: QUARRY LOT. NOT AFTER 7.* |
| Bastion | *A Bastion stencil: P-2 · QUARRY · 20+ · HVY · 1930* (count, the leader, its set-out hour) |
| Red Hand | *A red hand with a line through it, and POSTMEN.* |
| Halcyon | *A Halcyon plate with the plague doctor stencilled on it: P-02 · I d19* (days its index has led) |

A player within 25 m reads each mark once a version: *"… Written on day 12."*, and the range is Read.
With a faction's **mark prefab** set, the mark also stands there as a prop ([Signs and
sounds](#signs-and-sounds)); its words are always the text.

### Knowing

Per character, like the outposts you've seen: lost with the life, saved in `state.json`.

| Level | How you get it | What the journal shows |
|---|---|---|
| **Heard of** | A faction's line or service, a wanderer's road news, a call heard within 3 km | The place, the names heard, a rough area (a point 250 m off its range's centre: half a kilometre either way) |
| **Read** | Its call heard from within its range plus 300 m, a mark read, the contact board, the map of the crews, Halcyon's range map, a Radio Operator's relays, a Lab Technician's sample | Its range (centre and radius by grid) |
| **Mapped** | Seeing its lair by day (150 m) or its leader (80 m), Contact Report, Track Them Home, a tag you planted | Its lair by grid, its size and leader's stage, its temperament |

**Ways to learn** (each a service on a quartermaster, [REPUTATION.md](REPUTATION.md#what-each-faction-gives-you)):

| Service | Where, who | What |
|---|---|---|
| **Ask about the roads** | The Haven, Known | The three nearest packs within 4 km, Heard of with the Haven's names and their mood (*"hungry. Nobody out after seven."*) |
| **The contact board** | Bastion, Known | Every pack on the board, Read (four read out) |
| **The map of the crews** | The Red Hand, Known, 15 Favor | Every pack, Read, with its crew name; which crew is fed |
| **The tag frequencies** | The Red Hand, Trusted, 25 Favor | Every tagged leader's rough area, and a stolen tag when the server has the tag item |
| **The range map** | Halcyon, Known, from 25% | The ranges Halcyon has read (within 4 km of the lab) or tagged, Read; also given to its Known at the breakthrough and on spawn |

**In the journal and on the map.** Each pack is a journal row with the range ring icon, Mapped first,
then Read, then Heard of: the place as its title, the level where a job shows its time left, a
direction to its rough area, and the detail on select. On the map each is a **range ring**
(`marker_range`) sized to its ground: Heard of, half a kilometre round the rough area; Read, its
range; Mapped, its range, with where it lies up in the label ([UI.md](UI.md#the-map)).

### Skills

| Background | Reads |
|---|---|
| ★ **Hunter** | Tracks and the scout's warning ([Tracks](#tracks), [Scouts](#scouts)) |
| ★ **Radio Operator** | **The relays:** at dusk, with two packs calling within 5 km, *"Calls passing east, ridge to ridge, from the quarry by Morton toward the woods north of Levie."*; both Read. The calling contests are the call's own ([Borders](#borders)) |
| ★ **Lab Technician** | **Tags:** a tagged leader's rough area (300 m), stage and lean or fed in the journal. **Samples** (from 50%): a pack body's samples dropped within 150 m: which pack, and how long its index has led; Read |
| ★ **Soldier** | **The contact board** from spawn: every pack on it, Read |
| ★ **Police Officer** | **The column**, Read from spawn |
| ★ **Elder** | **Who they were:** the Haven-known leaders Heard of from spawn; the journal gives the leader's name and who they were, and the named young (not players) running with each pack |
| ★ **Ex-Con** (Raider) | **The crews:** every pack Heard of in the Red Hand's word from spawn, and which crew is fed |

Halcyon's Trusted read tags like a Lab Technician.

### Your dead run

The setting is **Zombies > Tribes > Your dead run** (Off, Described, Named; Described by default), and
the taking rules are [above](#the-taken-and-the-ones-who-ran).
- **A named young.** A player's character taken as a young is kept by name (3 players' characters and
  6 named in all a pack, the oldest going first), and so is a named wanderer taken, or a walk-out.
- **The name** shows on the ID card, in the dead player's own journal (*"Your last character, …, runs
  with the Quarry lot now."*) and to their group. Public text (headlines, the Haven's lines, road news,
  Bring Them Home's brief) uses the description (*"someone who'd been a hunter"*) unless the server is on
  **Named**, the player hasn't opted out, and the name passes the **Name filter** (words; a name
  containing one is described instead).
- **The opt-out**, without typing: the notebook item's action *"If I die: say my name, or don't"*
  (a toggle, with a gamepad from the action menu); on PC also `/mydead hidden` or `/mydead shown`. Saved
  by identity.
- **Put down.** When one of a pack's young is killed with a player within 100 m, it's a named one about
  as often as they are a share of its young (always the one a running Bring Them Home wants): the ID card
  (**ID card prefab** if set, a loose item) and its text to everyone within 100 m: *"In a pocket, an ID
  card: Pavel Novak. Driver. Died on day 12, near Morton."*
- **Road news.** Wanderers say *"… runs with the Quarry lot now."* Bring Them Home
  ([MISSIONS.md](MISSIONS.md#the-packs-contracts)) carries the card home, with its exclusions.

### The dark corners

All said sideways.
- **The fat moat** (rule 19). Once a season day, the pack nearest a Red Hand camp whose ground lies
  between the camp (within 3 km) and Bastion's nearest holding is the Red Hand's: its hunger rises at most
  3 a counted night and never past 40, it gains a member a season day up to 22, it stays settled and
  never splits. Only an Ex-Con's journal and the map of the crews say so.
- **The old wall.** A pack whose ground comes within 1 km of the Long Table's farm gains at most 3
  hunger a counted night; its trails stop 1 km short of the farm, and its day walks keep 1 km off it.
- **The walk-outs.** A walk-out (Walk Him Out's "walk on", within 2 km of a pack) is a named young. Some
  evenings (3 in 10) a pack fed walk-outs sends its young to wait at the nearest house's wire, and the
  Haven's Known hear only gossip: *"Somebody saw … at the wire last night. Nobody went out."*
- **Good condition.** Halcyon lets a tagged index hunt, and apologises ([Voices](#voices)).

### The factions' sides

- **The moat rule.** When the territory tick would take an outpost whose straight line from the taker's
  nearest holding crosses the ground of a pack whose leader is in its prime or older, the take waits;
  the third time it's held up, it goes through (*"they found a way round"*, in the log). The Red Hand
  scores a camp town ×1.3 when its fed pack lies between the town and Bastion.
- **A tagged leader** in its prime or older killed: +2 Halcyon Infamy to the nearest player within
  150 m, *"a year of data"* ([REPUTATION.md](REPUTATION.md)).
- **The Haven goes round.** At dusk and night, Haven-allied wanderers don't pick a destination whose
  straight way crosses a hungry pack's ground, and the Haven truck waits for dawn when its road does
  (it tries again at its next departure).
- **Hunter squads** stop at the edge of a pack's ground at night: *"Nobody follows anyone in there
  after dark."*
- **Spawn points** never go within 1 km of a lair. The gentle start's rules are the packs' own
  ([Day parties](#day-parties), [How a hunt comes for you](#how-a-hunt-comes-for-you)).
- **Road news.** A wanderer's news can name the nearest pack within 4 km in the Haven's words, with the
  place, and whoever hears it has Heard of it.

### Contracts and events

The packs' thirteen contracts, The Lone One, the changed missions and the events are in
[MISSIONS.md](MISSIONS.md#the-packs-contracts). The rules they share:
- **Claims.** A job holds its pack (`tribe:N`): no lair move, nomad's move, split, merge or absorption
  while it runs. A succession still happens. HC1's site stages hold the site's pack, and its parts keep
  150 m off the store ([QUESTLINES.md](QUESTLINES.md)).
- **Frenzy nights** (Every Ridge Calls everywhere, Red Dusk within 3 km of one point): one at a time,
  never in the first 2 season days, one every 3 at most (Red Dusk one every 7). A frenzied pack hunts with
  its range ×2 (×1.75) whatever its temperament, on three trails (the wire first), until dawn. It
  ignores fear places (its trails are worked out again through them at dusk, and back round them at
  dawn) and borders: a border with a frenzied side always has a cause to fight, twice as likely, still
  2 Border Nights staged a night at most. The great one's clan gathers on a frenzy night ([The great
  one](#the-great-one)).
- **A pack on the move** (Lead Them Away, with **Life cycle** on): a free pack of 8 or more whose ground
  is within 3 km of the safehouse (always a nomad, else half the time) is the crowd itself, moving
  house, held for the job. Every body killed comes off the pack (hunters first, then young). Led to the
  quarry, it lies up at a free lair within 1.2 km of it; reaching the house, at one within 2 km of the
  house's side; all of it killed, it stays where it was.
- **Furious** (its leader caught alive, Capture the Index): for 2 counted nights its one trail runs
  toward the lab and stops at the lab's no man's land plus P (250 m). The most recently Furious pack
  still Furious comes with the final stand's last wave.

### The cure

| Research | The packs |
|---|---|
| 25% | **Halcyon's range map** for its Known ([Knowing](#knowing)) |
| 50% | **The call box** (Halcyon's quartermaster, Trusted; 2 a real day on the server, one per player a real day; **Call box prefab**): put down and played, 90 s of a recorded dusk call, +25 heat, and the nearest pack within 1.5 km sends its hunters and young to it at dusk and night (as data, so it works with no bodies near; never its leader or guard), its walkers by day (within 1.5 km). Never in no man's land, or within 150 m of another player who isn't in your group. Lab Technicians read samples |
| 75% | Each person taken comes back as a young only half the time; the ones who ran are halved (built before) |
| 90% | The finale's live tissue: each route's Act III is a hunt for a leader taken alive (Capture the Index's rules, +2 on the route's track at once) or one of its guard or young (Live Capture's, +2 after 12 real hours) ([FINALE.md](FINALE.md#live-tissue)) |
| 100%, the final stand | Each wave's hordes come from the crawler towns within 3 km of the lab and the nearest pack within 3 km (its hunters drawn too), alternating, 350–450 m out on the line they come from. Their number is the built waves' × 0.7–1.3 by the neighbours' average infection (carried over between waves, at least one), so clearing them helps but never trivialises it. The last wave adds the most recently Furious pack. With neither near: the built waves |
| Lost | The packs that came go home, and the lab's nearest pack is kept Hungry (hunger 60 or more) for a season day |

### The last week

From the cure to the wipe (Seasons > Epilogue hours, 168 by default), on real hours, so a quiet
server's ridges go quiet too:
- **Nobody new turns.** No young (taken or ran), no new packs, no splits, no named young; a pack still
  eats (−25 hunger).
- **Hunger** rises 15 every 12 real hours (counted nights add nothing). A starving pack loses a fifth
  (at least 1) every 12 real hours: the young first, then hunters and scouts, the guard last; they lie
  down where they are and never join the crawlers.
- **No break-ups and no lone ones.** However small, a pack keeps its lair and its leader. A leader killed
  after the cure ends its pack: the vigil until dawn, then *"Nothing calls from … this morning. What was
  left of it has gone off into the trees to lie down."*
- **The calls thin:** *"The quarry by Morton still calls. There's hardly anyone left to come."* Under 8
  it sends no walkers (the weakened rule).
- **The last call.** At the dusk before the wipe falls due (when it's under a game day away), every
  leader calls once: *"A call from …, long and slow. Nothing answers it."* At dawn every pack lies down
  at its lair and is gone.
- **Towns stay cleared.** The kill ledger stops halving (built before), and nothing raises a town's
  infection in the epilogue.
- **Beats for the epilogue:** still calling (each leader alive at the cure), brought home, taken alive
  for the lab, culled, burned out, the great one risen and fallen: kept in `state.json` (40 at most) and shown by `/tribes`.
  The epilogue reads the packs calling at the cure for its beats and Quiet the Ridges, and "taken alive"
  for the director's grey choice ([EPILOGUE.md](EPILOGUE.md#the-closing-work)).

## The great one

Code: `Zombies/RZ_TribeGreat.c`; **Zombies > Tribes > Great one** (on; needs the life cycle). Player
text never names it: it's *"the deep voice"*, the call every ridge answers.

- **How it rises.** A [clan](#splits-and-clans) of 3 or more packs that holds together 8 season days,
  from Overrun, makes its oldest leader in its prime or older the great one. One on the map at a time;
  none for 8 season days after one dies. The headline: *"Every call along the ridges answers one voice
  now: Josef Hruby's, at the quarry by Morton."* On Everon (a clan in about half of seasons, usually of
  two) expect it rarely; on a Chernarus-size map, some seasons.
- **What changes.** Its guard is 4 (hunters lying up step up; the Guard prefabs are the helmeted ones).
  It doesn't die of age while its clan holds 3 packs, though it can still be challenged. Its dusk call is
  **the server's one dusk line** that evening: everyone online hears *"One call from the quarry by Morton
  at dusk, deeper than the rest, and every ridge from there to the woods north of Levie answers it."*
  instead of their nearest pack's, and within 1.5 km it's a sound (**Great call sound prefab**).
- **The Gathering**, every 4 season days at its dusk call and on a frenzy night: each pack of the clan
  (not one holding a vigil or weakened) sends one hunter group along the great one's way, by its lair, to
  the wire of the lived-in place nearest it (no man's land's edge plus P), where they wait until they're
  sent home at dawn. The bodies stay inside the per-cluster shares, so it comes in waves. It's a world
  event everyone is told (*The Gathering*, [MISSIONS.md](MISSIONS.md#world-events)); 10 or more of them
  put down within 300 m of the wire leave what they carried there at dawn.
- **When it dies:** every pack of its clan holds the vigil at its own lair, so nothing in the clan hunts
  that night (*"Every call along the ridges stops at once"*); the clan breaks up, with no clan and no
  great one for 8 season days; and the next dusk is **the Night of Many Calls**: every border on the map
  reopens for 2 counted nights (border fights twice as likely), told as a world event. Its fall is an
  epilogue beat.
- **Losing its clan** (a break-up, a merge) makes it an ordinary leader again.
- **Caught alive** it would count +3 on a finale route's track: planned ([designs/FINALE.md](../designs/FINALE.md#live-tissues-remaining-rules));
  today a leader counts +2 ([FINALE.md](FINALE.md#live-tissue)).

## Signs and sounds

Code: `Zombies/RZ_TribeProps.c`. Every prop and sound is an optional prefab under **Zombies > Tribes**
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)); without one, its line is all there is, and the lines are
always told.

| Sign | Prefab | Where and when |
|---|---|---|
| **Kill-site things** | **Kill-site prefabs** (a list: a bicycle on its side, a rucksack with its zip open, one shoe; never gore) | One at each [kill site](#kill-sites-the-vigil-and-the-alpha), picked by the spot, so it's the same one each time |
| **An abandoned lair** | **Abandoned lair prefab** (flattened bedding, somebody's things at the back of a culvert) | Where a pack lay up before it [moved](#lair-moves-and-displacement), as long as the lair is remembered |
| **The marks** | **Haven / Bastion / Red Hand / Halcyon mark prefab** | Each faction's [mark](#marks) that's written, a step apart where the road enters the range; a rewritten mark is a new prop |
| **The ID card** | **ID card prefab** (built with [your dead run](#your-dead-run)) | Where a named young is put down |
| **Call sounds** | **Call sound prefab**, **Great call sound prefab** | At a lair when it calls at dusk with a player within 1.5 km (not after it fed), at a call box when it plays, and at the last call; removed after 40 s, 6 at once at most. The prefab carries `RZ_CallSoundComponent`, which plays its sound event once on each client |

- **Props** appear only while a player is within 200 m and go once nobody is within 250 m (checked
  every 5 s): 6 a pack and 30 on the server at most, never saved. They're looks only; reading them is
  the lines' job.
- **The bright moon** (**Bright moon**, from phase 0.85): on a clear night (no rain, fog or storm) with
  the engine's moon at least that full, every pack hunts with its range ×1.25 (on top of hunger; a
  frenzy's own) and stays out until 05:30, its hunt too. Players near a pack are told at nightfall:
  *"A full moon over the quarry by Morton, and not a cloud. They'll come further tonight, and they'll
  stay out late."* *Simplified:* the moon doesn't change what anyone sees (the engine's light does).

## Planned

What's left is in [designs/INFECTED.md](../designs/INFECTED.md) and
[designs/BEHAVIOUR.md](../designs/BEHAVIOUR.md): the model's small rules (prey-scored trails, the
border moving the ranges, snow tracks, the clans' relay), the call's and the hunt's rest (stragglers,
two callers, a cunning pack on your back trail), crawlers giving way in their placement and heat hordes
from a pack's own groups, scouts' posts over the night's trails, the jobs' rest and the questline hooks,
the epilogue's beats from the other tribe beats (brought home, culled, burned out) and the factions'
jobs on the ridges; the behaviour layer's few small rules (the notice rate's
calibration, the formations' re-plan, forest edge points) and its untested levers. The finale's live
tissue's rest is [designs/FINALE.md](../designs/FINALE.md#live-tissues-remaining-rules)'s.

## The simulator

`python3 tools/tribe_sim.py` runs the tribes as data over a season on the measured land and sight, on
the rules and numbers the code runs (`--design` for the design's own, `--proposed` for phase 0's other
retunes, `--set` for anything). Phase 3's run (60 seasons a row; Arland with **Slots** 2):

| Map, players | Tribes over the season | 4-6 tribes | Clan | Members | Hungry | Starved a season | Successions | Lone ones | Splits | Broken up | Worst first hunt | Wiped |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Arland, 2 / 5 / 10 | 0 (seeded in half the seasons) | — | — | 6-7 | 0-1% | 0 | 0.1-0.2 | 0.5-0.7 | 0 | 0.3-0.6 | 36-38% | 0 |
| Everon, 2 | 4 | 82% | 60% | 12 | 36% | 2.1 | 1.7 | 1.7 | 0.7 | 0.2 | 46% | 0 |
| Everon, 5 | 4 | 73% | 48% | 11 | 38% | 3.6 | 2.6 | 3.6 | 0.7 | 0.7 | 50% | 0 |
| Everon, 10 | 2 | 42% | 12% | 10 | 36% | 3.1 | 2.4 | 3.7 | 0.1 | 1.0 | 46% | 0 |
| Cain, 2 / 5 / 10 | 2 / 2.5 / 1 | 0-17% | 3-60% | 9-13 | 30-34% | 1-2 | 1.9-2.1 | 1.7-3.0 | 0-0.9 | 0.2-1.0 | 45-46% | 0 |
| Chernarus-size, 2 / 5 / 10 | 15 | — | 98-100% | 12-13 | 72-79% | 28-59 | 2.2-5.7 | 2.6-9.2 | 3.5-3.6 | 0.3-1.0 | 46-50% | 0 |

Tuned against the design's numbers on the code's rules: **Hunger a counted night** 3 (at 5: hungry
60-70%, 8-13 starvations a season on Everon), the commitment's cap at half (without it 1-2 tribes a
season wiped), a town gone dark after 3 season days with no range within 2 km (at 5 and 3 km Everon sat
at 3 tribes), **Split at** 24 (26: a clan in a quarter of Everon seasons at 5 players). A busy server
keeps its towns from going dark, so Everon and Cain at 10 players hold fewer tribes; Arland's one tribe
(with **Slots**) seeds in only half the seasons and breaks up; Chernarus-size's tribes, far from
prey, go hungry and starve a few times each. Day crossings stay lighter than night ones (1.0-1.5 fights
against 1.2-1.7 hunts).
