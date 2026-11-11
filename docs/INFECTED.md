# The infected

How the infected live in the Zone: the towns' crawlers, the countryside's loose ones, the heat
hordes, the land they live on, and how their bodies are spawned, counted and removed. Bites and
infection are in [DESIGN.md](DESIGN.md#zombies-bites-and-infection), noise and heat in
[DESIGN.md](DESIGN.md#heat-and-noise), and how hidden a player is in [DESIGN.md](DESIGN.md#stealth).

**Status.** Phase 1 of the tribes design is built: crawlers, loose ones, heat hordes from towns, the
land grid in use, the agent budget and the plumbing tribes will need. **The tribes themselves are
planned** (phases 2 to 5, [below](#tribes-planned)). Nothing here has been play-tested with a game
client yet ([ROADMAP.md](ROADMAP.md)).

Code: `Zombies/RZ_ZombieSpawner.c` (spawning, counting, removing), `Zombies/RZ_Crawlers.c` (towns),
`Zombies/RZ_InfectedBodies.c` (roles, data parts, the sight rule, the agent budget, runtime
groups), `Zombies/RZ_Tribes.c` (the Tribes settings, the active-AI limit, `/tribes`),
`World/RZ_Land.c` (the land grid), `Director/RZ_Adaptation.c` (infection adapts).

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
- [Tribes (planned)](#tribes-planned)

## Two switches

| Setting | On (the default) | Off |
|---|---|---|
| **Zombies > Crawlers > Enabled** | Towns hold crawlers: placed by the time of day, drifting to noise, refilled from the buildings, remembered by the kill ledger, giving up on locked doors | Towns are populated exactly as before |
| **Zombies > Tribes > Enabled** | The land grid is read, the engine's active-AI limit is raised, the countryside's roamers are loose ones walking somewhere, heat hordes come from the nearest town | No land grid; roamers and heat hordes as before |
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
spreads from town to town. *Built as data; what stirs a town comes with the tribes (planned): a hunt
passing through +30, a kill in town +30, a border fight +20, a scout's call +10.* An admin can stir
the nearest town with `/tribes stir` to test it.

**Scavenging.** A kill site within 1 km of a town draws 2–4 of its crawlers out the next night.
They walk out as data at 3.5 m/s, get bodies within 600 m of a player, stand there until dawn, then
walk home. Six kill sites on the server, each gone at its second dawn, saved. *Built as data; kill
sites come with the tribes (planned).* An admin can leave one with `/tribes site`.

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

*Simplified:* crawlers don't give way to a tribe passing through yet (phase 2), and bloated crawlers
are whatever the prefab is: the code doesn't make them burst.

## The countryside: loose ones

With **Tribes** on, the countryside's roamers are **loose ones**: the same count (2 per player ×
the stage's density), each spawned 250–300 m out behind terrain (or just past 300 m in the open), and
**walking somewhere**, so following one leads somewhere:
- **by day** a town within 3 km (a crawler that drifted out, going home: crawler prefabs, 1–2);
- **at night** a lair candidate of the land grid within 3 km if there is one, else a town (Common
  prefabs).

A loose one that reaches its town becomes one of that town's crawlers. Each player keeps 2 within
1.1 km; one that walks out of range is replaced. With no town or lair within 3 km, or for a special,
they spawn as before. *Planned:* splinters, starving walkers and members crossing between ranges
(phases 2 and 3).

## Heat hordes

80 heat in one square within 600 m of a living player sends a horde toward it, at most one every
10 minutes; half the heat goes. A hot square nobody is near draws nothing. Players near the target
are warned; Radio Operators hear it anywhere.
- **With Tribes on,** a horde comes from the nearest town within 1.5 km of the hot square: its own
  crawlers (up to 12 in 4 groups, or a Horde prefab with Crawlers off), spawned 300–450 m out on that
  town's side, out of sight. They're lent out of the town ([above](#towns-the-crawlers)). It waits
  for room in the agent budget rather than come thin. The log says which town.
- **With no town that near, or Tribes off,** as before: a Horde prefab 360–540 m out in any direction.
- *Planned:* by day inside a tribe's range from its day party only, at night from its hunters
  (phase 2).

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
  spots exist (the finale, planned). An overlay, so the cache never needs a rebuild.
- **Reading it.** At the first start on a map, from the game mode's one-second tick: up to 20 land
  squares a second, stopping after 8 ms (about 5 minutes on Everon and Cain; tribes stay dormant
  meanwhile, and the log says so). It's cached in `$profile:ReforgerZ/land_<map>_<number>.json` (the
  number comes from 64 terrain heights) and loads in milliseconds after that. Wild land, fields and
  lair candidates are worked out again on every load, from where people live then. Roads come from
  the world's `SCR_AIWorld`; without one there are no roads and no bridges.
- **Tribe slots** (for the planned tribes): 1 per 5 km² of wild land (**Wild km² per slot**),
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
  hunt (planned) that's an order home through the held waypoint beyond 80 m of every player, then
  back to data out of sight beyond P; the out-of-sight check is built.

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
| `epilogue` | 18 (3 scenes of 6), and room | *Planned:* the epilogue's scenes |
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

### Plumbing (for the tribes)

- **One held waypoint per group** (`Core/RZ_GroupOrders.c`, [DESIGN.md](DESIGN.md#principles)): every
  order (drift, reinforcements, loose ones, hordes, scavengers) moves it.
- **Per-member tracking.** Each tracked unit knows its members. A member missing from its group's
  living agents has died (phase 0: the count drops within a second; the group deletes itself when its
  last member dies; the bodies stay). Each death is a kill **at that member's spot**: a special's
  sample drops where it fell, the body carries its loot ([DESIGN.md](DESIGN.md#bodies-and-car-boots)), the town's infection and ledger and any
  mission's count follow.
- **Roles** on each tracked unit: crawler, loose one, scavenger (and alpha, guard, hunter, scout, young
  and day party for the tribes). Stealth reads them: the alpha and guard as specials, a hunt on its
  trail as a horde that's never lost through stealth's search, the rest as common
  ([DESIGN.md](DESIGN.md#stealth)).
- **Data parts.** A count at a position, going somewhere at 3.5 m/s (phase 0: Bacon runs every member
  to an order, and a group's centre covers about 3.5 m/s). While it has bodies, each tick writes their
  position and their dead back to it, so nothing snaps back when the bodies go. The scavengers use it
  now; tribes will.
- **The silent kill** (`KillSilently`): no samples, no infection change, no ledger, no loot on the body
  and no mission credit. Two ways, both measured in phase 0: **delete** (out of sight: no body, no
  loose item) and **kill and mark** (a body a player can find). Built for the tribes' border fights
  (planned).
- **Death attribution.** The bite detector remembers which unit last hit each player: its role, town
  and tribe, and when. *Planned:* a player-killed event reads it (VERIFY 9 in the design: not measured).
- **Waypoints and stealth:** stealth's search still adds its own waypoint in front of the held one and
  takes it back after.

## Admin visibility

- **The status log** (every 5 minutes, with Crawlers or Tribes on) adds a line: agents against the
  budget, units, crawlers and loose ones, the clusters and their shares, the other bodies in the one
  body budget, hordes from towns, loose ones
  that arrived, the ledgers, stirred towns, scavengers out, kill sites, held doors, the engine's active
  AI against its limit, the land, and the clock. So nothing needs typing.
- **`/tribes`** (server admins, in chat) writes everything to the server log: every town with crawlers,
  a ledger or stir, every kill site and part, every tracked unit, each player's last hit, the slots.
  `/tribes stir` and `/tribes site` are play-test aids ([above](#towns-the-crawlers)).
- **The land** logs what it found when it's read or loaded, and the tribe slots.

The log lines are listed in [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check).

## Tribes (planned)

Designed and approved, not built: packs of infected in the wild land, each with a lair, a range and a
leader. Out by day too, but lighter: a small **day party** of hunters and young walks the tribe's
trails while the leader and guard lie up at the lair; the full hunt only at night. They grow from the
people they take and from the ones who ran when a town near them went quiet, they call at dusk, fight
over borders, split, and succeed one another. Every rule is written so that it shows: a call, a track,
a line someone says, always with the place's name.

The numbers phase 0 set, which the phases below build on:

| What | Value |
|---|---|
| Perception range P | 250 m in the open, 0 behind terrain; no spawn within 300 m in sight (built) |
| Data speed | 3.5 m/s (built) |
| Slots | 1 per 5 km² of wild land: Everon 5, Cain 4, Arland none (built) |
| Day party | a fifth of the tribe (twice that when hungry, never over half), in groups of 2–3, hopping 100–200 m every 4–6 minutes and holding |
| A scout's pack | 150–200 m out, arriving in 40–55 s; by day at most 4, never the leader |
| Agents | 100 on the server, the engine's limit at 160 (built) |
| The clock | about 4 real hours a game day, saved across restarts (built: [DESIGN.md](DESIGN.md#the-clock)) |
| The break-off | scripted: home through the held waypoint beyond 80 m, then back to data out of sight beyond P (the parts and the sight check are built) |

| Phase | What |
|---|---|
| 2 | One tribe end to end on Everon, then all of them: lair, scouts, day parties, the dusk call, hunts by placement, the night's commitment, the alpha's body, kill sites, the vigil, hunger, the taken, the ones who ran. `docs/LORE.md` gains "What the infected do" |
| 3 | The life cycle: succession, splinters, lone ones, borders, splits and clans, lair moves, vacuums, seasonal pull, burned land, memory |
| 4 | People and content: each faction's words, skills, contracts and events, the changed missions, the last week after the cure |
| 5 | The great alpha, signs, props, call sounds, the journal's list |

The tribe simulator (`python3 tools/tribe_sim.py`) runs the tribes as data over a season on the
measured land and sight.
