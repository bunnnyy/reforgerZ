# Sites: camps that read as real places

**Designed, not built** (phases 5 and 6). Approved by the author on 2026-11-12. Phases 1 to 4 are
built: each site's ground read on a grid, real footprints, the surroundings and the threat, the
audit, the placement rules and the solver, and building in layers by each faction's doctrine (the
wire and its gates, the boards, the watch, valuables indoors) are in [docs/SITES.md](../docs/SITES.md).
Today's camp life (the jobs, the routines, the people) is in [DESIGN.md](../docs/DESIGN.md#camp-life);
scenes and their ageing are in [TRACES.md](../docs/TRACES.md#scenes). This file says only what
changes. When a phase is built, its part moves into those docs and is cut from here.

## Contents

- [Why](#why)
- [4. The people](#4-the-people)
- [5. The story of each site](#5-the-story-of-each-site)
- [Performance](#performance)
- [What stays as it is](#what-stays-as-it-is)
- [Settings](#settings)
- [For the PC](#for-the-pc)
- [Build order](#build-order)

## Why

The sites now stand right ([docs/SITES.md](../docs/SITES.md): rules, zones, a wire that closes, one
entrance, a watch over the gate). Two things are left from the author's play-tests: the people
don't use the site as it's laid out (they stand where today's jobs put them, and the patrol walks a
loop 40–80 m out that ignores the wire and the roads), and a site doesn't tell what happened to it
(a camp that was attacked last night looks like one that never was). The people (phase 5) take
their posts from the layers and walk the zones; the story (phase 6) puts each site's state in the
world with Traces' own machinery.

## 4. The people

The bodies stay the ones the built systems spawn ([DESIGN.md](../docs/DESIGN.md#camp-life):
guards, named people, occupants, crews, extras up to **In play**); the change is **where they stand
and walk**, driven by the zones and the site's story. Jobs stay today's (`JOB_WATCH`, `JOB_PATROL`,
`JOB_SIT`, `JOB_WORK`, `JOB_GATHER`, `JOB_LEAN`, `JOB_STROLL`, `JOB_COURIER`, `JOB_OFF`, `JOB_COOK`,
`JOB_WASH`, `JOB_CHOP`), done the same way (smart-action seats, loiter and patrol waypoints in a
cycle, the waypoint slots on **Camp life**), and only at the **Live places** (3) nearest players.

### Posts

Each site gets **posts** from its layers: a post is a cell and a facing the job uses.

| Post | From | Job | Who |
|---|---|---|---|
| Gate | the gate zone | `JOB_WATCH` (two at Bastion and Halcyon strongholds: one at the barrier, one in the nest) | guards |
| Lookout | the high point, and each threat sector's watch | `JOB_WATCH` | guards |
| Door | the building's one entrance | `JOB_WATCH` (Halcyon's guard pair, the Haven's lookout) | guards, occupants |
| Counter | the counter | none (the quartermaster never gets a job, as now) | the quartermaster |
| Leader's seat | the inner court | none | the leader |
| Medic | the sick zone | `JOB_WORK` at the ward tent | the medic (the Haven safehouse's stays with the house's services) |
| Kitchen | the cook pot | `JOB_COOK` before the meal, `JOB_CHOP` by day | named people, occupants |
| Work | the work zone's seats | `JOB_WORK`, `JOB_SIT` | everyone off watch |
| Off duty | the fire, the sleep zone | `JOB_SIT`, `JOB_OFF` | the rest |

### Shifts

Today's phases (`RZ_Territory.PHASE_MORNING`, `PHASE_DAY`, `PHASE_EVENING`, `PHASE_MEAL`,
`PHASE_NIGHT`) and each faction's day (`Slots`) stay. What changes:

| Change | Rule |
|---|---|
| Posts first | the slots fill posts by importance: gate, then the lookouts by threat score, then the door; only then chores and seats |
| Night | every lit post keeps its watch; a lookout on the covered side at night; Bastion's curfew (`IsCurfew`) adds the second gate watch as now |
| The handover | at 05:30 and 17:30 the watcher walks from the post to the fire and the relief walks out (today's shuffle, but the relief comes from the sleep zone or the fire, not from nowhere) |

### Patrols

Today: a loop of four points 40–80 m out (`Patrol`). Designed: routes from the site's own lines.

| Route | Path | Who | When |
|---|---|---|---|
| **The wire** | the inside of the wire line, 2 m in, between the gaps and the posts; corners as loiter points | Bastion's loop, Halcyon's guard at night | by day and at night |
| **The approach** | out of the gate along the main road to 80–150 m, a look, back | Bastion, the Red Hand's stroll (`JOB_STROLL`) | by day |
| **The threat side** | out into the kill zone toward the threat front (the hostile holding's road, the lair), 60–100 m, and back | Bastion; Halcyon's courier (`JOB_COURIER`) goes the other way, toward the friendly side | by day; never at night |
| **The road post run** | to the faction's nearest road post within 1.5 km, along the road, and back | Bastion, the Red Hand | once a day, if the AI budget allows (one at a time on the server) |

Every route is built from the grid and the road segments: points on dry, walkable cells (not water,
not steep, not solid), never along a cliff (`BadOutlook`'s drop test), on roads where it can. Routes
vary by the threat sector: the heaviest side gets the wire loop's longest stretch and the threat-side
walk.

### Everyday movement

`Potter` (walking the camp) and the walk-ins follow the zones, so people go where people would go:

| Movement | From → to |
|---|---|
| Meals | the work and posts → the kitchen and the table at `PHASE_MEAL` |
| Water | the kitchen → the water barrel or a well within 60 m, with a bucket look (`JOB_WORK` at the barrel) |
| The latrine | anyone → the latrine, then back (a short `JOB_STROLL`), a few times a day |
| The stores | the quartermaster's runner → the stores door and back (a courier walk inside the wire) |
| Smoke and talk | two off duty → the bench by the back door or the fire; they face each other (seats paired by the Near rule) |
| Idle | the fire, the bench, the table: never the middle of the yard or the gate lane |
| Sleep | through the main door (a building) or to the tents; despawned as data, as today |

### When the site is threatened

A site's **alert** comes from what the game already knows: a horde's heat near it (`RZ_HeatMap`), a
tribe's hunt nearby, a fight with players or another faction at it (the holding's `IsUnderPressure`),
or a recent loss in its story.

| State | Posts | Movement |
|---|---|---|
| Calm | as above | as above |
| Wary (a hostile base's people or heat within 300 m) | every lookout manned; a second at the gate | no walks outside the wire; chores stop at dusk |
| Alert (contact) | everyone with a weapon to the wire on the threat side (loiter waypoints along it) | named people indoors; nobody at the fire |
| Recently hit (the story's *hit* state, below) | the gate doubled for two season days; the threat-side walk every shift | fewer at the fire; the burn pit lit |

Combat stays the AI's own; the alert only moves people to posts first.

### Limits

| Limit | Today | Kept |
|---|---|---|
| Only near players | bodies with a player within 300 m; jobs at 3 Live places | yes; routes and posts are data until then |
| AI budget | extras at most 6 on the server; **In play** counts per site | yes; the road post run counts as one of them |
| Waypoint kinds | patrol, cycle, loiter, observation, smart action | yes; no new waypoint kind is needed |

## 5. The story of each site

Each site gets a **story**: a few facts worked out from real state, and the props and lines that
show them. Worked out when the site is solved, from the seed and the state; nothing new is saved
except what the game already saves.

### Its state

| Fact | From | Values |
|---|---|---|
| How it was settled | the holding's history (`RZ_Territory`: founded, taken, swung back), `RZ_TownHistory` | **held from the start**, **taken from** another faction, **settled** an empty town, **retaken** |
| How it's going | the faction's strength (`GetStrength`), morale (`RZ_FactionLife.GetMorale`), need (`GetNeed`) | **thriving**, **holding on**, **short** (a need), **under pressure** (`IsUnderPressure`) |
| What happened lately | `RZ_TownHistory` within the last 3 season days: a horde, a fight, a fire (`RZ_TownMemory.ReportBuildingBurned`), a death | **quiet**, **hit** (attacked), **burned**, **mourning** |
| What's out there | the surroundings: the threat front's kind | **hostile neighbour**, **the infected**, **the road**, **the sea** |
| The town | its infection band, whether it's the lab's town | **clean**, **fighting it**, **losing it** |

### What it puts in the world

Shown through the same machinery Traces uses ([TRACES.md](../docs/TRACES.md#scenes)): a story element
is a **scene kind** placed by the site's solver into its zone, aged by thirds as scenes are
([How scenes age](../docs/TRACES.md#how-scenes-age)), with its faded twin when old. No second system.

| Where | Element | When |
|---|---|---|
| Outside the wire | **burnt infected piles** (the pyre, `m_sPyre`) at the kill zone's edge | the infected front, or *hit* by the infected |
| | **casings and a broken crate** at the nest | *hit* by people, or a hostile neighbour |
| | **warning signs and stencils** on the approach: the faction's mark | always; more under pressure |
| | **a cleared kill zone**: stumps, a strip of flattened ground (decals where the kit has them) | Bastion and Halcyon on open sides |
| | **wrecks pushed off the road** into the verge | taken or settled on a road |
| | **graves** (`m_sGrave`) on the dead zone | mourning; one more per death in the story (cap 7, as N1) |
| Inside | **the memorial** with names (the Haven), **dog tags on nails** (Bastion), **trophies** (the Red Hand), **the tally board** (Halcyon's samples) | always; one more name or tag per death |
| | **the roster** or the duty board with a gap | after a loss |
| | **scorch** on the building's wall (Traces' L2) | *burned* |
| | **the other faction's mark overpainted** on the gate (door marks' layers) | *taken from* |
| In text | the place's **name** ("the school", "Rudi's", "the church"), from the building's kind and the story | always |
| | one **line** from its people when you come in (the routine line, a subtitle) | by state |
| | a **notebook or radio mention** ("The Haven buried two at the church") | when *hit* or *mourning*, through the existing news lines |

### The library

Each faction has several stories; the state picks one (with the seed breaking ties), and its
elements are added to the site's kit as tier 5 pieces.

| Faction | Story | When | What it shows |
|---|---|---|---|
| Bastion | **The post that held** | held from the start, thriving | neat wire, the roster full, casings old and swept, flag pole straight |
| | **Dug in against the crews** | hostile Red Hand holding within 3 km | double sandbags on that side, a burnt car at the edge of the kill zone, *HALT* sign shot through |
| | **Short of everything** | short | half the sandbags gone to another post, the stores board crossed out, a queue line chalked at the counter |
| | **After the night it came close** | hit by the infected | a pyre at the wire, a gap in the wire patched with a bed frame, two new tags on the nails |
| Red Hand | **Took it off Bastion** | taken from Bastion | Bastion's sandbags reused as a sofa wall, the green paint overpainted red, a soldier's helmet on the trophy wall |
| | **The toll road** | on a through-road | the toll barrier, cars searched and left on the verge, a price board |
| | **Fat and loud** | thriving | crates stacked high (scenery), bottles everywhere, the dog |
| | **Something got in** | hit, mourning | a door boarded from outside, a chair under a sheet, quiet |
| Haven | **The church that opened its doors** | the church, settled | the bear on the door, the long table, names inside |
| | **The farm that feeds three towns** | thriving, a farm | the garden big, carts, the stall |
| | **Burying our own** | mourning | new graves by the church, the table laid with a gap, the candle jars lit (`m_sCandleJars`) |
| | **The night watch** | the infected front, losing the town | the fence doubled with furniture, a bell at the door, everyone sleeping in one room (as data) |
| Halcyon | **The clean room** | the lab, thriving | fence, lights, decon at the door, nothing outside |
| | **The station that lost its courier** | hit by people, short | the courier's bicycle on its side by the gate, a sample case taped shut, the guard doubled |
| | **Burning the samples** | losing the town | a burn barrel by the back door, the plague board's count crossed out and redone |
| | **Under study** | a nest under study nearby (Specimen from the Nest running) | a ringed beak on the gate (L8), a cage look by the sick zone, the notebook bench busy |

### Over the season

- Elements age by thirds like scenes; a site's story is worked out again at each season day's
  change (the same seed, the new state), so a site that was attacked shows it for its scene's life,
  then fades back.
- **Taken** sites show the old owner's traces for 3 season days (overpainted marks, their barriers
  reused), then their own.
- In the epilogue week the story is frozen as it stands (as the hall is, [EPILOGUE.md](../docs/EPILOGUE.md)).

### Lines

Player text follows [WRITING.md](../docs/WRITING.md): plain, short, the faction's voice; never
"you're new". Examples of a routine line by state:

| Faction | Calm | Hit |
|---|---|---|
| Bastion | *"Sign in at the board. Keep your hands where we can see them."* | *"Two on the wire tonight. Don't argue."* |
| Red Hand | *"Toll's on the sign. Read it."* | *"Something came through the back. We're not talking about it."* |
| Haven | *"There's soup. There's always soup."* | *"We buried Marta yesterday. Sit with us."* |
| Halcyon | *"Through the shower first. All of you."* | *"Nobody goes out past the lights."* |

## Performance

| Cost | Budget |
|---|---|
| Spawning | unchanged: 4 pieces a tick, **Pieces max** (200); story pieces count toward it and are the first left out at the cap |
| People | unchanged: Live places (3), extras (6) |

## What stays as it is

- Which building a place moves into (`FindSite`, `PickSite`, `SiteTier`), the entries and the gate
  (`ReadEntries`), the quartermaster's counter and turn (`Counter`, `TurnToServe`).
- `RZ_DressingGuard`: nothing at a camp is usable for gain.
- Pieces spawn and go with the place's bodies, only near players; nothing is saved but what the
  game saves already.
- The solver, the layers and the audit ([docs/SITES.md](../docs/SITES.md)): the people and the story
  read the site's zones, wire, gates and posts as the solver leaves them; story elements are placed
  by the same rules, as tier 5 pieces.

## Settings

New on **Territory > Camp life** (`RZ_CampLifeSettings`; the grid's, the solver's and the layers'
settings are built):

| Setting | Default | What |
|---|---|---|
| **Story** | on | each site's story elements |
| **Alert states** | on | posts and movement change with a threat |
| **Road post run** | on | a walk to the nearest road post once a day |

## For the PC

- Choose the props the story needs (vanilla first, nothing usable): a cage look, a bucket for the
  water run, a bell, and decals for the kill zone if the Traces spike found a way. Then measure
  them (**ReforgerZ: Measure prop footprints**) and run `build.py`.
- After each phase, a screenshot of every site of one map with **ReforgerZ: Fly to the next site**,
  held against `sites_placed.html`.

## Build order

| Phase | Builds | Done when |
|---|---|---|
| 5 | **The people**: posts, shifts by post, patrol routes from the wire and roads, everyday movement by zone, alert states | the jobs log shows posts filled and routes walked |
| 6 | **The story**: the state, the library, its elements as scene kinds, ageing, lines and mentions | each faction's stories seen in play |
