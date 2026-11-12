# Task places and things

Code: `World/RZ_PlaceNeeds.c` (the needs, the checks, the finder), `World/RZ_PlaceAudit.c` (the
audit), and the pickers that use them (below). Server only.

A task's words must be true where it puts you. "Search the house over the way" needs a house over
the way; "what's left of a big one by the road" needs a road, and the remains lying by it; "the well
in Tisy" needs a well, in Tisy. Every kind of task that names its place or a thing at it has a
**need**: place flags with their ranges, and the things its words name. Its picker tries candidates
against the need and takes the first that fits, or offers nothing; it never puts a task where its
words are false. The place it settles on is checked again in play (the live check, a warning in the
log), and a headless audit asks the same pickers for places across a map and checks them with the
same code.

## Contents

- [Why: the forest by Camurac](#why-the-forest-by-camurac)
- [The mainland](#the-mainland)
- [The flags](#the-flags)
- [The things](#the-things)
- [The needs](#the-needs)
- [How a picker uses a need](#how-a-picker-uses-a-need)
- [The live check](#the-live-check)
- [The audit](#the-audit)
- [Results](#results)
- [Not covered](#not-covered)

## Why: the forest by Camurac

On Everon the opening's first step, **Across the Road** ("Search the houses over the way, quietly"),
put the player in a forest on a rocky slope by Camurac. The root, found by the audit below:

1. **Step 1 made the place up.** With no building 50–250 m from the contact, it took a random point
   80–200 m out and called it "the houses over the way": no check for houses, water, the line or a
   slope. With a building, it took any building (a shed, a garage, a ruin) and called it "the house".
2. **The start was often alone.** The openings' place roll read only the first 48 buildings the
   engine returned round a town's middle (walls, sheds and fences fill that fast) and took a fitting
   one with a door among those, so a start was often a farm or a lone house with nothing 50–250 m off.
   On Arland every start was (39 of 39); on Everon 16 of 39.
3. **The fallback start stood anywhere.** When the roll finds nowhere (a territory roll that holds
   most towns, the line, a pack's lair), the life starts at a spawn point, which was a random point
   within 80% of a town's radius: woods and fields as often as a street (Everon: 53 of 88 had no
   house within 60 m).

So step 1 from a lone start or a spawn point sent you into the trees: before the fix, step 1 failed
its need 12 times in 39 on Everon and 24 in 39 on Arland (in water 16 times there, at the line 6).

Fixed at each step: step 1 is a real house (or the background's kind) checked against its need, or
no step 1 at all; the roll reads the town's houses themselves, ranks buildings with houses over the
way first, tries 12 towns and has a second pass at half the distances before the spawn point; spawn
points stand just outside a house. The audit found the same fault in other pickers (a random point
called "the farmhouse", "the well in ...", "the police station", "by the road", "the house with the
owing in it"; A Sample's remains missing altogether, the sample alone in a field), fixed the same way.

## The mainland

Code: `World/RZ_Mainland.c`. Islands are close to impossible to reach on foot, so nothing is put on
them. At the start the server reads the map once on squares of about 100 m (bigger on a big map, at
most 320 a side): a square is land if any of five samples (its middle and 35 m each way) is above
sea level (a beach or a spit you can walk across counts), or a road runs through it (so a bridge
joins what it crosses). A flood fill over land
squares by their sides gives each piece of land; the biggest is the **mainland**. A point is on the
mainland if its square is, or it's a sea square beside one (the shore). Water inside the land (a
lake, a river) doesn't cut it: islands are cut off by the sea. It takes 3 ms on Arland and about
30 ms on Everon and Cain.

| Who reads it | How |
|---|---|
| Every place need | the `MAINLAND` flag, on every need |
| The settlement scan | a fresh scan keeps no town whose middle is on an island, so holdings, safehouses, the lab, the Long Table, missions, quartermasters, spawn points and the infected's towns are never there; a saved season's island towns are marked and skipped (`PickSettlement`, `PickTown`, the openings, errand towns, spawn points) until the next fresh season |
| The openings' roll | a door on an island is refused (`an island`) |
| The land grid | an island square is neither field nor wild land: no pack lives there |
| The audit | lists the islands and their towns (`Places mainland: ...`) and checks every town, holding, safehouse, the lab, the Long Table's farm and every wanderer (`world.place`) |

Log at the start: `Mainland: mainland N km2 (100 m squares, N ms); K islands: <size> km2 (<towns>),
...`, and on a fresh scan `Mainland: <town> at <grid> is on an island: not a settlement` for each.
A fresh scan also moves a town whose middle came out in the water (a shore cluster's average) to
its nearest dry house (`Settlements: <town>'s middle was in the water: moved N m to <grid>`), so
nothing that walks to a town's middle (a wanderer, an errand's town) walks into the sea.

## The flags

| Flag | True when | How it's read |
|---|---|---|
| `DRY` | not in the sea, a lake, a pond or a river | the ground's height (the sea), the engine's water surfaces (the rest) |
| `NOT_AT_LINE` | off the quarantine line | `RZ_Perimeter.IsNearLineAt` |
| `MAINLAND` | on the map's main landmass, not an island | `RZ_Mainland.OnMainland` ([the mainland](#the-mainland)) |
| `NOT_STEEP(deg)` | the ground under it is less steep | the steeper of two 6 m slopes across the point |
| `HOUSES(n/m)` | at least n houses within m | buildings whose prefab names a house, villa, cottage, flat, pub, shop, hotel or office, not a shed, garage, barn, ruin, church or military or industrial block (`RZ_PlaceCheck.IsHouse`) |
| `BUILDING(kind/m)` | a building or landmark of that kind within m | `any`, `house`, `orchard` (three fruit trees), the door slots' kinds by building (clinic, shop, garage, farm, public), else the landmark finder's words (fuel, well, watertower, pumphouse, police, school, radio, military...) |
| `ROAD(m)` | a road's centre line within m | the AI world's road network |
| `WATER_SOURCE(m)` | a well, a hand pump or a water tower within m | the landmark finder |
| `OPEN_GROUND(m)` | no building and few trees within m | buildings, then trees |
| `TREES(n/m)` | at least n trees (not bushes) within m | a sphere query of trees |
| `IN_TOWN(+m)` | within the town its words name (else the nearest), its radius + m | the settlements |
| `OUTSIDE_TOWN` | outside every town's radius | the settlements |
| `NEAR_FACTION(f/m)`, `AWAY_FROM_FACTION(f/m)` | a holding or road post of faction f within m, or none | territory and road posts |
| `NOT_LOCKED` | not behind a lock | `RZ_LockedSpots.IsBlocked` |
| `HIGH_GROUND(+m)` | at least m above the ground 100 m round it | eight heights round it |

Every need has `DRY`, `NOT_AT_LINE` and `MAINLAND` (`world.place` all but the line).

## The things

What the words name at the place, each with the prefab slot it's made from and what the game does
when that slot is empty. An empty slot is never silent: either the task isn't offered, or the stated
fallback happens, and the audit prints which.

| Kind | Checked | How |
|---|---|---|
| `SLOT` | the slot is set | the setting (`RZ_PlaceThings.Slot`) |
| `ITEM`, `PROP` | made from the slot, within range of the place once the step starts | a sphere query for that prefab |
| `NPC` | a living person (not a player) within range | a sphere query of characters |
| `REMAINS` | a dead body within range | the same, dead |

| Slot key | Where it's set | Used by |
|---|---|---|
| `sample` | Cure > Sample prefab | A Sample's sample |
| `remains` | Zombies > Special prefabs: the first that doesn't burst when it dies (no exploder, bomber, bloater, spitter or screamer; today the Shambler), else a single common character | A Sample's remains |
| `contact` | Guidance > Contact prefab, else Puzzles > Civilian | the opening's contact |
| `key` | Interactions > Search > Key prefab | The Station Keys |
| `spawnpoint` | World > Spawn point prefab | spawn points |
| `prop.effects`, `prop.crate`, `prop.papers`, `prop.parts` | Missions > Errands > Props | an errand way's prop |
| `prop.car` | Guidance > Car wreck prefab | Shift Change's car at the verge |

## The needs

The table `RZ_PlaceNeeds.Build` holds. "Not offered" means the picker returns nothing and the task
or way isn't offered here.

| Need | The words | Flags | Things | Picked by |
|---|---|---|---|---|
| `opening.start` | outside the door you came out of, someone waiting | BUILDING(any/12) NOT_STEEP(30) (houses over the way are the roll's ranking, not its words) | NPC the contact within 15 m (empty: nobody stands there, the scene's note points at the nearest work) | the openings' place roll |
| `opening.search` | the house (the chemist's, the garage...) over the way | BUILDING(house or the background's kind/15) NOT_STEEP(30) | | Across the Road (step 1) |
| `opening.water` | the well | WATER_SOURCE(8) | | Water, Water for the Fever |
| `opening.pump` | the petrol station | BUILDING(fuel/20) | | The Pump |
| `opening.downwind` | the pack's ground | NOT_STEEP(35) | | Downwind, by a lair |
| `opening.treeline` | the tree line where they cross | TREES(4/40) NOT_STEEP(35) | | Downwind, with no pack near |
| `opening.keys` | the locked house over the way, and its key | BUILDING(any/6) | SLOT the key (not offered) | The Station Keys |
| `opening.sample` | what's left of a big one by the road, and the sample by it | ROAD(12) NOT_STEEP(25) NOT_LOCKED | REMAINS within 4 m, ITEM the sample within 4 m (either slot empty: not offered) | A Sample |
| `opening.sample.yard` | what's left of a big one in a yard over the way, and the sample by it | HOUSES(1/20) NOT_STEEP(25) NOT_LOCKED | the same | A Sample, with no road near |
| `opening.gate` | the gate | NOT_STEEP(30) | | Hold the Gate |
| `opening.debt` | the house with the owing in it | HOUSES(1/15) | | The Debt |
| `opening.toll` | the Red Hand's toll or gate | NEAR_FACTION(Red Hand/120) | | The Toll |
| `opening.swap` | the wanderer on the road | | | A Fair Swap |
| `spawn.point` | a spawn point named after its town | IN_TOWN(+0) HOUSES(1/60) NOT_STEEP(30) | SLOT the spawn point (empty: none spawn, the map's own) | `RZ_GameModeComponent.SpawnPointSpot` |
| `errand.here` | {here}, {house}: the giver's place | | | a way at the giver's place or gate |
| `errand.out` | a direction and a distance out | | | a way 200–400 m or 300–500 m out |
| `errand.subject` | the card's own place | | | a card's subject (checked live only) |
| `errand.town` | in {place}, a town by name | IN_TOWN(+0) HOUSES(2/60) | | the nearest empty village, the next town |
| `errand.gardens` | the gardens and sheds at {place} | IN_TOWN(+100) HOUSES(2/60) | | The Pot |
| `errand.well` | the well in {place} | IN_TOWN(+150) WATER_SOURCE(10) | | Water for {house}, the night water |
| `errand.pumphouse` | the pump house at {place} | IN_TOWN(+300) BUILDING(pumphouse/20) | | Filter Parts |
| `errand.car` | a car out on the road, {place} | ROAD(25) | | Shift Change (its car: `errand.prop.car`) |
| `errand.sensor` | a sensor out in the open | NOT_STEEP(30) | | Night Readings |
| `errand.ridge` | up {place}: the ridge | HIGH_GROUND(+8) | | Weather, Write It Down |
| `errand.farmhouse` | the farmhouse {place} | BUILDING(farm/25) | | Not Past the Chapel |
| `errand.orchard` | the orchard at {place} | IN_TOWN(+300) BUILDING(orchard/40) | | The Last Apples |
| `errand.police`, `errand.school`, `errand.hall` | the police station, the school, the hall in {place} | IN_TOWN(+300) BUILDING(police, school, public/30) | | Clean Rifles, Clean Paper, the hall's chairs |
| `errand.prop.effects`, `.crate`, `.papers`, `.parts`, `.car` | the way's prop at the place | | PROP within 6 m (the car 8 m; empty: words only, and a warning) | a way with a prop |
| `mission` | at {town}: the town its brief names | IN_TOWN(+1500) | | every contract, event and stage, when it starts (live only) |
| `world.place` | a town, a holding, a safehouse, the lab, the Long Table, a wanderer | DRY MAINLAND only (a town by the map's edge is by the line) | | the audit's check of what stands all season |

An errand way names its need in `RZ_ErrandWay.m_sNeed`; without one, its place kind's (`NeedKey`).
A routine whose ask names `{next_town}` names the town its next-town way goes to, and isn't true
(not dealt) when no town near has the place.

## How a picker uses a need

`RZ_PlaceFind.Near(centre, min, max, need)` draws candidates from what the need asks for, shuffled,
and returns the first that passes the checks; zero when none does:

| The need has | Candidates |
|---|---|
| BUILDING of a kind | 3 m outside the side, facing the asker, of each building of that kind in the ring; the landmarks of that kind; an orchard's fruit trees |
| WATER_SOURCE | by each well, pump or water tower in the ring |
| HOUSES | 3 m outside each house in the ring |
| ROAD | 2–4 m off the road's centre line, on its verge |
| HIGH_GROUND | the highest of a few samples |
| none of these | random dry points in the ring |

`NearKept` keeps a result for 10 minutes by need, ring and where it was asked (the errands ask a
giver's places each time a talk menu is built). The pickers:

- **The openings' roll** (`RZ_OpeningPlaces.c`): each candidate building ranked (a fitting building
  with houses 50–250 m off, a house with them, a lone fitting one, a lone house), the best kept;
  12 towns; a second pass at half the clear distances (without the last starts) before the spawn
  point. Why doors were refused is counted (`WhyText`) and logged with a spawn-point fallback.
- **Step 1** (`RZ_Openings.PickSearch`): a house 50–250 m from the contact, the background's kind
  first, the spot outside its side facing the contact; none, and the chain skips step 1.
- **Step 2** (`RZ_OpeningSteps.c`): the well and the pump at a dry spot by them; the tree line, the
  gate and the debt through `Near`; A Sample by a road's verge, else in a yard.
- **Errand ways** (`RZ_Errands.PlaceFor`): out from the giver through `Near`; a town by name: the
  nearest of four towns with such a place, nearer that town than any other.
- **Spawn points** (`SpawnPointSpot`): just outside a house in the town.

**A Sample's things.** When the step starts, a special that doesn't burst is spawned at the place
and killed where it stands, kept from the engine's corpse cleanup while the opening runs, and the
sample lies beside it; when the opening ends the body goes back to the engine's cleanup. Both are
made only for a player taking the step, 80–250 m off.

## The live check

`RZ_PlaceNeeds.Verify(need, place, text)` checks a settled place's flags and its things' slots;
`VerifyThings` checks the things stand there once they're spawned. A failure logs:

```
[ReforgerZ] Place check: <need> at <grid> ("<text>") fails: <why>; <why>
```

Checked: an opening's start, its contact, step 1, step 2's place and A Sample's things; every
errand step taken, and its prop's slot; every mission as it starts. The full runtime test prints
how many failed (`Place checks failed (live, docs/PLACES.md): N`).

## The audit

```bash
SERVER_DIR=... EXTRA_ADDONS_DIR=... tools/runtime_test/run.sh tasks [arland] [everon] [cain]
python3 tools/runtime_test/places.py draw <console.log> [out.html]
```

It boots the site audit's worlds (the map's own world and its roads) with the mod's own game mode
settings (`RZ_GameMode.et`'s, `places.py prepare`), waits for the land grid and 30 s more, and asks
the game's own pickers:

| Asks | How many |
|---|---|
| The openings' roll | 3 per background (13 backgrounds), as a brand-new player |
| From each start's contact | step 1, and every step 2 that has a place (water, pump, downwind, keys, sample, gate, debt, swap, toll) |
| Errand ways | every way with a step out of the giver's place, from up to 6 givers of its anchor (safehouses, holdings, the lab), one per need, place kind and giver |
| Spawn points | one per town |
| What stands all season | every town, holding, safehouse, the lab, the Long Table's farm and every wanderer, against `world.place` |
| Spawn tests | the contact, A Sample's remains and sample (by the step's own `SpawnSampleScene`), each errand prop: made at the place, looked for with the live check's code, deleted |

The spawn tests use the mod's own slots and prefabs (the server logs `Wrong GUID/name` for each of
the mod's prefabs in a loose addon folder, but finds them by their path). The specials come from
Special Zombies, which is left out on Linux, so `places.py` gives the remains a vanilla civilian as
a stand-in special.

Log lines (also what `places.py summary` prints and `draw` reads):

| Line | Means |
|---|---|
| `Places: <need>: P of A placed, N pass, F fail, score S[ UNDER]; fails: <why xN>; fallback: ...` | per need: asked, placed (the rest declined), passed; `<need> things` for the spawn tests |
| `Places need: <need>: <flags>; <things> \| <words>` | the need it was checked against |
| `Places fail: <need> at <grid> ("<text>"): <why>` | the first 8 failing places a need |
| `Places roll: <background>: <kind>: rank R, first pass \| second pass; doors refused: ...` | each roll |
| `Places declined: opening.start for <background>: <why>`, then `Places roll: <background>: the spawn point at <grid>` | a roll that found nothing: its steps are asked from a spawn point, as in play |
| `Places declined: <need> from <grid> ("<text>"): N candidates; <why> xN` | the first 8 asks a need declined, and why its candidates failed (declining is allowed: nothing there would make the words true) |
| `Places mainland: ...` | the islands found and their towns |
| `Places draw: ...`, `Places town: ...` | for the drawing |
| `Places audit: K kinds, P places, F fail, score S; U under T: <needs>` | the summary |

A need that places and scores under the threshold (`RZ_PlaceAudit.s_iThreshold`, 95) fails the run.
The drawing (`task_places_<map>.html` in the work folder) puts each place on the map by need, the
failed ones red and bigger, with a line back to where it was asked from; a need can be hidden.

## Results

`run.sh tasks`: the pickers before this change, checked against the same needs by the same audit (a
roll that finds nothing goes on from a spawn point, as in play), and after (with the mainland and
the season's stands). "Pass" is passed of placed; the rest of the asks declined (nothing there would
make the words true: the step or way isn't offered there).

| Need | Arland before | Arland after | Everon before | Everon after |
|---|---|---|---|---|
| Step 1, `opening.search` | 15 of 39 (24 failed: in water 16, no house within 15 m 15, at the line 6) | 29 of 29 (9 skipped: no house near the start) | 27 of 39 (12 failed: no house 12) | 39 of 39 |
| Its start, `opening.start` | 39 lone starts (no other house within 250 m) | 24 of 24; 15 rolls found no free town with a door off the line or a lair, those lives start at a spawn point by a house | 16 of 39 lone | 39 of 39 |
| A Sample's remains and sample | 0 of 39 (no remains) | 38 of 38 | 0 of 39 | 39 of 39 |
| The Debt, `opening.debt` | 0 of 39 | 11 of 11 | 13 of 39 | 36 of 36 |
| The tree line | 38 of 39 | 38 of 38 | 25 of 31 | 37 of 37 |
| The Pump | 39 of 39 | 33 of 33 | 13 of 15 (in water 2) | 12 of 12 |
| Spawn points | 0 of 12 (no house within 60 m) | 3 of 3 (11 towns have no house: none there) | 35 of 88 | 43 of 43 |
| Errand ways and props | 51 of 86 | 52 of 52 | 93 of 129 | 111 of 111 |
| What stands all season (`world.place`) | — | 32 of 32 | — | 114 of 115 |
| **All** | **358 of 550 (score 65)** | **497 of 497 (100)** | **436 of 628 (69)** | **667 of 668 (100)** |

Cain after: 506 of 506. The one left on Everon is a town's middle in the water ("Outskirts 37-85"),
a riverside cluster with no dry building in its radius to move it to: nothing is put at it. In an
earlier run the Red Hand camp at Gorey had its middle in a river, so The Toll's gate there now
stands at a dry spot by the camp.

The territory roll changes from run to run (on Arland it holds 11 to 14 of the 14 towns), so
Arland's starts vary: one run rolled 39 of 39 starts in free towns, every one with houses over the
way; another none. Errand ways that decline most are the ones that name a rare building: the
police station (none near any giver tried on any map), a hall, an orchard, a pump house.

**The islands found** (a fresh scan, the `Mainland:` lines):

| Map | Mainland | Islands | Towns that are no longer settlements |
|---|---|---|---|
| Arland | 6.8 km² | 1 (0.1 km², no town) | none (4 shore towns' middles moved out of the water) |
| Everon | 51.5 km² | 17: the north-east island (1.2 km²), the north-west one (0.6 km²), Thollevast's (0.5 km²) and islets | 10: Erquy and 4 outskirts on the north-east island, 2 outskirts on the north-west one, Thollevast, The Scythe, Outskirts 108-87 |
| Cain | 38.7 km² | 17 (the biggest 1.6 km²) | 10: Gravel Spit and 9 outskirts |

On a fresh scan no holding, safehouse, the lab or the Long Table can stand on an island (their towns
aren't settlements). A saved season keeps its towns: the start's warning counts any on an island,
and the audit's `world.place` lines name any holding, safehouse or wanderer there.

## Not covered

- **Missions' own places** are checked live only (a mission's Setup spawns its people and props, so
  the audit doesn't run it); the full runtime test starts every one and counts the live warnings.
- **Card subjects** (a wreck, an impact, another house) are each card's own; checked live.
- **Words-only objects**: Night Readings' "sensor four" and The Receiver's "mast" have no prop yet.
- **People on the move** (a wanderer, a road post's crew, a holding's guards) are data until a player
  is near, so the audit checks where they stand, not that a body is there.
- **Loot** lies in any building a player comes near, an island's too: only a player who gets there
  finds it.
