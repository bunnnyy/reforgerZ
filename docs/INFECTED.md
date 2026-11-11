# The infected

How the infected live in the Zone: the towns' crawlers, the countryside's loose ones, the heat
hordes, the land they live on, and how their bodies are spawned, counted and removed. Bites and
infection are in [DESIGN.md](DESIGN.md#zombies-bites-and-infection), noise and heat in
[DESIGN.md](DESIGN.md#heat-and-noise), and how hidden a player is in [DESIGN.md](DESIGN.md#stealth).

**Status.** Phases 1 and 2 of the tribes design are built: crawlers, loose ones, heat hordes from
towns, the land grid, the agent budget and its plumbing, and **the tribes** on every slot of the map
(lairs, scouts, day parties, the dusk call, night hunts by placement, the dawn count, hunger, the taken,
the ones who ran, the vigil), with **the behaviour layer** driving their bodies beyond striking range.
The life cycle and the people's side are planned ([below](#planned)). Nothing here has been play-tested
with a game client yet ([ROADMAP.md](ROADMAP.md)).

Code: `Zombies/RZ_ZombieSpawner.c` (spawning, counting, removing), `Zombies/RZ_Crawlers.c` (towns),
`Zombies/RZ_InfectedBodies.c` (roles, data parts, the sight rule, the agent budget, runtime
groups), `Zombies/RZ_Tribes.c`, `RZ_TribeData.c` and `RZ_TribeBodies.c` (the tribes, `/tribes`),
`Zombies/Behaviour/` (the behaviour layer), `World/RZ_Land.c` (the land grid),
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
- [How they move and hunt](#how-they-move-and-hunt)
- [Planned](#planned)

## Two switches

| Setting | On (the default) | Off |
|---|---|---|
| **Zombies > Crawlers > Enabled** | Towns hold crawlers: placed by the time of day, drifting to noise, refilled from the buildings, remembered by the kill ledger, giving up on locked doors | Towns are populated exactly as before |
| **Zombies > Tribes > Enabled** | The land grid is read, tribes live in the wild land, the engine's active-AI limit is raised, the countryside's roamers are loose ones walking somewhere, heat hordes come from the nearest town | No land grid, no tribes; roamers and heat hordes as before |
| **Zombies > Behaviour > Enabled** | Our code drives the tribes' bodies beyond striking range ([How they move and hunt](#how-they-move-and-hunt)) | Tribe groups take held waypoints, with Bacon's own senses and stealth |
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
player killed in town by a tribe's body +30, a scout's call within 600 m +10 ([Tribes](#tribes));
*planned:* a border fight +20 (phase 3). An admin can stir the nearest town with `/tribes stir`.

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

*Simplified:* crawlers don't give way to a tribe passing through yet (planned), and bloated crawlers
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
  spots exist (the finale, planned). An overlay, so the cache never needs a rebuild.
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

### Plumbing

- **One held waypoint per group** (`Core/RZ_GroupOrders.c`, [DESIGN.md](DESIGN.md#principles)): every
  order (drift, reinforcements, loose ones, hordes, scavengers) moves it.
- **Per-member tracking.** Each tracked unit knows its members. A member missing from its group's
  living agents has died (phase 0: the count drops within a second; the group deletes itself when its
  last member dies; the bodies stay). Each death is a kill **at that member's spot**: a special's
  sample drops where it fell, a body to search lies there, the town's infection and ledger and any
  mission's count follow.
- **Roles** on each tracked unit: crawler, loose one, scavenger, and the tribes' alpha, guard, hunter,
  scout, young and day party. Stealth reads them: the alpha and guard as specials, a hunt on its
  trail as a horde that's never lost through stealth's search, the rest as common
  ([DESIGN.md](DESIGN.md#stealth)).
- **Data parts.** A count at a position, going somewhere at 3.5 m/s (phase 0: Bacon runs every member
  to an order, and a group's centre covers about 3.5 m/s). While it has bodies, each tick writes their
  position and their dead back to it, so nothing snaps back when the bodies go. The scavengers and the
  tribes use it.
- **The silent kill** (`KillSilently`): no samples, no infection change, no ledger, no body to search
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
- **`/tribes`** (server admins, in chat) writes everything to the server log: every town with crawlers,
  a ledger or stir, every kill site and part, every tracked unit, each player's last hit, the slots.
  Play-test aids: `/tribes stir` and `/tribes site` ([above](#towns-the-crawlers)), `/tribes hunt` (the
  nearest tribe within 5 km hunts you, any time of day) and `/tribes call` (every tribe calls now).
- **The status log** also prints one line per tribe (members by role, the alpha's stage and health,
  hunger, temperament, range, lair, hunting, the day party, fed, vigil, samples, young waiting), the
  tribes' phase, counted nights and the town specials' scaling, and the behaviour layer's counts.
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
  Rising alpha 2–5 season days into leading. *Planned (phase 3):* new tribes from the taken, from towns
  gone dark, from lone ones and from splits; a free slot stays free until then.
- **The place** every line carries: *"the woods north of Levie"*, *"the quarry by Morton"* (the lair's
  kind from the land grid, the nearest settlement and the compass). The four factions' own words for a
  tribe are planned (phase 4); player text never says "alpha", "tribe" or "crawler".
- **The range:** 500–1,000 m round the lair by the wild land near it (350–700 m on maps under 25 km²).
- **Hunting grounds** (up to 4): the edges of towns within 2 km that face the lair (not lived-in
  towns), bridges and fords in the range, and the nearest field.
- **Trails** from the lair to each ground: a point every 250 m, each the best of the square on the line
  and the two beside it (dry land, a forest edge or sparse trees preferred, out of towns and no man's
  land, river squares only at a crossing), thinned to 4–6 points.
- **Scout posts:** the two highest wild squares in the range (a ridge first), 300 m apart.
- **Temperament** (25% bold, 20% cautious, cunning, territorial, 15% nomadic; a quarry leans cunning,
  the column nomadic, a barn or silo cautious) and a **call pattern** (*"three short and a long"*) come
  from its seed. *Built:* temperament sets how it hunts (strategies, the core, the commitment). *Planned
  (phase 3):* the rest (home behaviour, fear places, nomads' loops, where each one's day party walks).
- **Roles:** the alpha (alone in its own group), 2 guard, 1 scout (2 from 15 members), hunters in
  groups of 3–5, the young. Members are counted in **parts**: groups as data, each at a position.

### The daily rhythm

| When | The tribe |
|---|---|
| **Day** (06:00–18:30) | The alpha at the lair's centre, the guard 10–25 m round it, the resting ring of hunters and young 20–60 m out. Scouts on their posts. From 07:00 the **day party** walks; from 18:00 it walks back |
| **Dusk** (18:30–20:30) | The day party back in the ring, the new young join, **the call** at 18:45, the **set-out** between 19:30 and 20:30 |
| **Night** (20:30–05:00) | **The hunt** on tonight's trails |
| **Dawn** (05:00–06:00) | **The walk home**, spread over the hour, then **the dawn count** |

Fog counts as dusk (a scout's pack is the dusk pack); in a storm nobody goes out. With no game clock
it's always day: day parties walk, scouts post, nobody hunts.

### Day parties

The author's change: the wild is never empty by day, but the tribe never comes in full until dusk.
- **Who:** hunters and young only, never the alpha, a guard or a scout. **Day party share** of the
  members (0.2; never over 0.25), twice that when **hungry** (never over half); in **groups** of 2–3
  (**Day party group**), each with its own walk. A tribe under 8 sends none.
- **Where:** a day walk of 3–5 points in the range, along forest edges and field margins, never in a
  settlement's radius, no man's land or the line's margin (rain keeps it to the woods). A **hungry**
  tribe's walk goes out to a **field at a town's edge**, outside its radius and outside no man's land,
  never into either.
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
silence** (*"The calling from … stops. Then nothing at all."*). *Planned:* the Radio Operator's relays,
borders answering, two callers when restless (phases 3–4).

### Hunger and the night

- **Counted nights.** A game night counts at 05:00 if someone was online **Counted night minutes** (20)
  of it. Hunger (0–100, from 30) rises 5 a counted night (7 in Decline); each person taken takes 25 off.
  Bands: fed under 40, lean, hungry 60–84, desperate 85–99. *Planned (phase 3):* starving walkers.
- **The set-out.** One trail a night; two when hungry, one of them **testing the wire**: toward the
  nearest lived-in place, ending at its no man's land's edge plus P (250 m). They come to the wire and go
  home; they never go in. Hunters spread over the trails in file, the young ahead, the core about 150 m
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
  taken by the infected on the road to Morton."* *Planned:* supply trucks taken as data (the trucks are
  real vehicles whose loss needs the supply code's own failure path).
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
  home and it won't come for your cluster again tonight.
- **The hide rule.** Lost for 60 s (no group has seen you) and 300 m from all of them, or in no man's
  land: it goes home. *Simplified:* "indoors behind a closed door" isn't read yet; going indoors works
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
  from players. *Planned:* the opt-out, the name filter, the ID card, the journal (phase 4).
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
  Rising, and the next dusk is *"a voice nobody knows"*. *Simplified:* the guard fight, the splinters and
  the two weak days are planned (phase 3); the samples aren't kept from the loose-item cleanup.
- **The alpha's body:** one character in its own runtime group, by day at the lair, at night with the
  core. **Health** is kept as data when its body goes and set again when it comes back (scaled health;
  infected never heal by themselves): it heals 0.1 an hour played while it lies up. An alpha hurt in the
  last 10 minutes isn't removed until every player is 1.5 km away. 3 alpha bodies on the server at most,
  never two within 1 km of one player.
- **Samples** only when a player was within 150 m: the alpha 2 in Prime and 3 in Decline (none while
  Rising), 3 times a season per tribe; a guard 1 under a Prime or older alpha, one a tribe every 2 played
  hours. Hunters, scouts, the young and day parties drop none. Town specials are scaled by the tribes'
  share of samples over the last 10 played hours (never below ×0.6), so specials per player-hour stay
  near the season pacing's. After the cure, none.

### Scouts

**The scout rule**, once a second for a posted scout with a player within 200 m: you're seen after 5
checks running within its sight and in terrain line of sight (within 80 m, also nothing in between). The
sight is stealth's: `RZ_Stealth.SightRadius(player, Scout sight)` (120 m at normal visibility, down to
stealth's floor for someone fully hidden), ×1.5 in a vehicle, ×0.75 for a Hunter. A Hunter gets *"The
birds just went up off the ridge."* at the first check. Kill it inside those 5 seconds and nothing comes.
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
one, the way, how fresh, the alpha, the young, and the wear: 3 nights or more on that ground). Within 60
m of a lair by day: *"No birds in this wood, and the ground's laid flat under the trees. Something lies
up here by day."*

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
  scouts and young to hunters. With Alpha and Special both empty the alpha is data only.
- **Back to data** beyond 700 m of every player and out of sight, as every part; their position and
  dead written back. Stealth reads their roles: the alpha and guard as specials, a hunt on its trail as a
  horde that's never lost through stealth's search.
- **Restarts:** tribes, their members, hunger, alphas (with health), the bank, the taken and post-feed
  sleep are saved in `state.json`; parts, trails, posts and tracks are worked out again, and a hunt that
  was running starts again from the lair the next dusk. A save from before tribes seeds them on load.

## How they move and hunt

The behaviour layer (**Zombies > Behaviour**; code `Zombies/Behaviour/`): our code drives the tribes'
bodies beyond striking range, and Bacon fights at striking range. It's built on the zombie lab's tests
B1–B20 (Bacon 1.8.0; [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#zombie-lab-bacons-real-ai-measured)), and
uses only what the lab found works. Crawlers, loose ones, hordes and missions' infected stay Bacon's own
(with stealth). With **Enabled** off, or another zombie mod's bodies, tribe groups take held waypoints.

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
| In a fight | Run, sprint bursts of 2–4 s every 8–15 s | Run, the lunge | Run | Run | Bacon's heavy run |
| Stance | 0.85–0.95 | 0.72–0.82 (silent while stalking) | 0.9 | Bacon's | Bacon's |
| Sight by day / night | 180 / 156 m | 220 / 190 m (**Hunter sight**) | The scout rule | 200 / 173 m | 160 / 139 m |
| Memory | 30 s | 90 s | 10 s | 60 s | 120 s |
| Reach (released within) | 35 m | 25 m | 15 m | 30 m | 40 m |
| Climbs (grab height) | 1.4 m | 1.4 m | 1.4 m | 1.2 m | Never (goes round: B17) |

Each body also gets a speed factor of 0.88–1 on its caps, ±0.04 on its stance and 1–3 m of slot
jitter, so a group strings out and no two move alike.

**Senses** (steered groups with a player within 250 m, once a second, the leader's eye): a notice meter
per player rises by **Notice rate** (0.15) × stealth's visibility × facing (1 within 70°, else 0.25) ×
(1 − d/R)^1.5 × line of sight, and falls 0.1 a second otherwise; within 12 m in sight it fills in 2 s. At
0.4 the group stops a few seconds and one body makes a sound (heard before seen); at 1 it's aware and the
tribe decides (a day hunt, the lair fighting, or the night's hunt). Line of sight is the terrain between
(script samples) and within 80 m a physics trace (B13: 0.002 and 0.021 ms; 12 a second, **Sight checks a
second**). Hearing: an unsuppressed shot gives a group a lead from 170 m × its kind (suppressed 30 m),
15% of the distance off, and it walks to look; the lair's ring by day is deaf. A hunt's target is
remembered where it was last seen, never where it really is; a night hunt on its trail keeps contact by
sight alone (lying still doesn't shake it), a day group needs to see you.

**Formations** (slots written every second near players, every 3 s beyond 300 m): file on a night trail
and going home, a loose line by day, a wedge for the core, flanking pairs, a ring at the lair, the kill
site and the vigil. Spacing widens as the group nears its target (4 m at 200 m to 20 m within 40 m), and
a strike fans out on an arc round the target, closing. Slots are kept 3 m apart (**Spacing**) and 8 m
off the alpha, enforced in code (B20). Walk-stop takes the group's waypoint off its list so Bacon walks
to slots under 25 m (B7); a march puts the held waypoint at the goal, at most every 5 s.

**Strategies** by temperament (the top two weights, 70/30; a Rising alpha's tribe uses the cautious
weights; **Strategies** off: everyone rushes):

| Strategy | Bold | Cautious | Cunning | Territorial | Nomadic | What it does |
|---|---|---|---|---|---|---|
| **Rush** | 3 | 0 | 0 | 1 | 1 | Fanned out, straight in at a run |
| **Stalk** | 1 | 3 | 2 | 1 | 2 | A shadow point 80–150 m off by day (**Stalk distance**; 50–100 at night), behind the target's line of travel and toward cover (terrain that hides it, then a wooded square); walk-stop, hunters silent; frozen and low for 3–6 s when you look its way within 120 m; strikes at a moment or after 4 (night 6) minutes |
| **Pincer** | 2 | 1 | 2 | 3 | 1 | Two groups: a sidestep first, then flank points 140 m out by day (100 at night) at 70° (**Flank radius**, **Flank angle**), toward cover; both strike when both are set, or 60 s after the first; one group alone stalks |
| **Encircle** | 3 | 0.5 | 1 | 2 | 1 | 5+ bodies, never by day: a ring of 120 m with a 60° gap (always a way out), each group a share of it, closing 0.7 m/s; strikes at 35 m, at a moment, or after 3 minutes |
| **Give up** | | | | | | Lost for its memory time: it searches the last-known spot 45 s, then goes back to what it was doing, and won't take that player again for 10 minutes unless it sees them afresh within 50 m |

A **moment**: you stand still 10 s, crouch or go prone, or face more than 100° away. Every strike starts
with a sound from the leader a second before the release. By day only rush, stalk and a two-group pincer.

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

**Attacks.** Bacon's melee stays the hit and its damage. Built, of the kinds' own moves, only what the
lab says works: the **hunter's lunge and bite** (from 6–10 m, more than 60° out of its target's view, a
1.5 s sprint with SPRINT set every 0.25 s (B7), into Bacon's swing; 20 s apart per hunter). If it lands
(a hit within 3 s, the body whose melee test passed within 1 s and 2.2 m, else the nearest lunger within
2.5 m), the bite chance is ×1.5 (never over 0.6) and a light bleed opens on an arm or a leg (once a
player in 30 s). One special move lands on a player every **Special move gap** (10 s) at most, none in the
gentle start; **Own attacks** off gives plain Bacon melee.

**Performance.** Hooks only read fields (B15: no cost above the noise); the tick is vector maths for the
groups near players; 100 agents with half steered measured p99 11.93 ms on the lab box (the target is
12.5). The status log's behaviour line counts bodies per mode, hunts, releases, take-backs, unsticks and
moves landed.

## Planned

What's left of the design, in [designs/INFECTED.md](../designs/INFECTED.md) and
[designs/BEHAVIOUR.md](../designs/BEHAVIOUR.md):
- **Tribes phase 3, the life cycle:** succession with the guard fight and splinters, lone ones, ageing
  and the Clash, borders and calling contests, Border Nights, splits, the kept slot, clans, lair moves
  and displacement, vacuums, new tribes, seasonal pull, burned land, starvation, habits, fear places,
  marks, echoes, worn trails' effects, bonds, mixed looks.
- **Phase 4, people and content:** the factions' words and marks, skills, the three knowledge levels and
  the journal's names under the place, the contracts and events, the changed missions, truck takings,
  Your dead run's opt-out, name filter and ID card, the last week.
- **Phase 5:** the great alpha, props, call sounds, Red Dusk, the bright moon, the range ring icon.
- **The behaviour layer's rest:** flush, bait, herd, ambush and hand-off; shrieks passed between groups;
  blood scent; the hearing table beyond shots; doors (another way in, weak doors); the reactions table;
  the crawlers' daily life (lying up, waking, herds, sleep with AI off); the alpha's charge, the young's
  leap, the ankle grab and the client effects (hold, stagger, knockdown) after a client test; forest
  edge points in the land cache.
- **Heat hordes and lookers** from a tribe's day party by day and its hunters at night; crawlers giving
  way to a tribe passing through; loose ones from splinters and starving walkers.

The tribe simulator (`python3 tools/tribe_sim.py`) runs the tribes as data over a season on the
measured land and sight.
