# Sites: camps that read as real places

**Designed, not built.** Approved by the author on 2026-11-12. Today's camp life (the building a
place moves into, the plans, the fit rules, the jobs) is in [DESIGN.md](../docs/DESIGN.md#camp-life);
the pieces and kits and how they're built are in
[WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#camp-life-the-compositions-and-kits); scenes and their
ageing are in [TRACES.md](../docs/TRACES.md#scenes); the land grid is in
[INFECTED.md](../docs/INFECTED.md#the-land). This file says only what changes. When a phase is built,
its part moves into those docs (camp life's section of DESIGN.md grows, or becomes its own doc,
`docs/SITES.md`, if it outgrows it) and is cut from here.

## Contents

- [Why](#why)
- [1. Reading the ground](#1-reading-the-ground)
- [2. The rules: where things belong](#2-the-rules-where-things-belong)
- [3. Building in layers, by doctrine](#3-building-in-layers-by-doctrine)
- [4. The people](#4-the-people)
- [5. The story of each site](#5-the-story-of-each-site)
- [6. The audit](#6-the-audit)
- [Performance](#performance)
- [What stays as it is](#what-stays-as-it-is)
- [Settings](#settings)
- [For the PC](#for-the-pc)
- [Build order](#build-order)

## Why

From the author's play-tests: pieces clip through each other and through walls, layouts don't
always make sense, and a site doesn't read as what it is: in a sealed, dying Zone the lab keeps its
things out in the open and nothing guards its building. The causes in today's code:

| Cause | Today (`Factions/RZ_CampLife.c`) |
|---|---|
| A fixed drawing | Each kit is one plan (`tools/camp_kit/pieces.py`, `KITS`) laid down whole; a piece that doesn't fit is nudged up to 1.6 m or left out (`Slot`, `PlaceFitted`) |
| Hand-typed sizes | Footprints come from `pieces.FP`, typed by hand; a prop drawn too small clips |
| A thin view of the ground | `Fits` samples nine points for height and flat traces at two heights; it can't see a low wall, a kerb, a bush, a slope between samples or a fence post |
| No purpose | Nothing knows that a generator is noisy, that stores must be guarded, or that a perimeter must close; roles are four bands (`RZ_EDressRole`: Centre, Inner, Approach, Edge) |
| No surroundings | The front faces the nearest road, landward (`ReadGround`, `Landward`); nothing reads hostile neighbours, high ground, the infected's land or what happened here |

The fix stays automatic and works on any map: every site is **built from rules** against a
**reading of its ground and surroundings**, in the order real people would build it, with a
**story** of its own, and every result is **measured** by an audit.

## 1. Reading the ground

### The site grid

Read once per site per run when it's first dressed (as `RZ_CampSite` is today), kept, never saved.

| What | How it's read | Cells |
|---|---|---|
| The grid | 0.5 m cells over the site's reach (a stronghold's 60 m ring plus its entry ring: about 240 × 240 cells; an outpost about 180 × 180; a road post 40 × 40) | one byte each: flags below; one byte height step |
| Height and slope | `GetSurfaceY` at every cell corner, sampled 1 in 2 (1 m) and interpolated; slope from neighbours | **steep** over 12°, **step** a jump over 0.3 m between cells (a kerb, a bank) |
| Water | `RZ_Util.IsWater` per 2 m; the land grid's sea, river and lake shares (`RZ_Land`) for the cells' 250 m square | **water**, **shore** (within 4 m) |
| Buildings | Each building's real box (`RZ_DoorGeometry.LocalBox`, as `AddBox` today), plus its doors (`RZ_DoorSlots`) and ground-floor windows (the building's window children, by name, where the prefab has them) | **inside**, **wall** (within 0.4 m), **door**, **window** |
| Walls, fences, hedges, kerbs | One `QueryEntitiesBySphere` over the reach, every entity; each static entity's world box rasterised into the grid (oriented, not axis-aligned) | **solid** |
| Trees and bushes | The same query; trees by prefab name ("tree", as `RZ_Land` reads them) with a 1 m trunk disc, bushes as soft cells | **solid**, **soft** (a prop may stand on it, a path may not) |
| Cars and wrecks | The same query (vehicles, `Wrecks` prefabs) | **solid** |
| Roads | The road segments `RZ_DressPlacer.ReadGroundWithin` already reads, painted with their real width (from the land grid's road width, else 6 m) | **road**, **verge** (2 m) |
| Other systems' scenery | Traces' scenes and boards (`s_aPieces`, the TRACES pool), loot spots, the quartermaster, the medic, leader seats (today's `m_aKeep`) | **kept** |
| Our own pieces | Each piece's real footprint rasterised as it's placed | **taken** |

A cell a piece may use is dry, not steep, not solid, not inside, not road, not kept and not taken.
A piece's footprint is a set of cells (its props' real boxes, plus a person round each seat), so
**every** part of it is tested, not nine points. The flat traces stay as a last check on the
chosen spot only (one placement, not every try).

### Real footprints

| Today | Designed |
|---|---|
| `pieces.FP`: width and depth typed by hand | A Workbench plugin, **ReforgerZ: Measure prop footprints**, spawns each prop `pieces.py` uses, reads its mesh bounds (`GetBounds` on the spawned entity) and writes `tools/camp_kit/footprints.json`: width, depth, height, the box's offset from the origin, and whether its origin is at its feet |
| A piece's footprint is a rectangle round its props | `build.py` writes each piece's footprint as its props' boxes (up to 8 rectangles) plus a 0.6 m disc for each seat's person, so an L-shaped piece doesn't claim its empty corner |
| A prop missing from `FP` is drawn 0.5 m | `build.py` refuses a prop with no measured footprint |

### Drop, don't squeeze

A piece that fits nowhere its rules allow is **dropped**, never pushed into a worse spot. The plan
order (importance) decides what's dropped first ([tiers](#importance-tiers)). A smaller site that
sits right beats a full one that clips. The log and the audit say what was dropped and why.

### The surroundings

Read once per site from data the game already has; each is a small record on the site, so the
rules below can ask "where is the threat?" without a query.

| Context | Read from | What the site keeps | Used by |
|---|---|---|---|
| **Roads** | `RZ_DressPlacer.ReadGroundWithin` (as now) and the entries (`ReadEntries`) | the main road in (toward the nearest other town, `m_fTownBearing`), each entry, through-roads (a road that passes and leaves again) | the gate on the main road; barricades on the others; a **through-road is never closed** (a vehicle lane is kept), only watched |
| **Sea, shore, rivers** | `RZ_Util.IsWater`, `BadOutlook`, the land grid's water shares | the **sea side** (bearings with water within 80 m), a jetty or pier within 60 m (by prefab name, as `SiteTier` refuses them today) | no wall on the sea side (water is the wall), a watch on a jetty or slipway (boats are a way in); nothing within 4 m of water |
| **High ground** | The land grid's ridges (`RZ_Land`, ridge posts), and the site grid's own heights within 150 m | the **high point** near the site, and any ridge that overlooks it (ground 10 m above the yard within 200 m) | a watch post wants the highest free cell with a view; a camp under a ridge puts a watch facing the ridge and its sandbags on that side |
| **Open ground and cover** | The land grid's fields and tree counts; the site grid's soft and solid cells | **open** sides (fields of fire) and **covered** sides (trees, buildings close) | open sides get wire and a clear strip; covered sides get the most watch (that's where the infected come from unseen) |
| **Hostile and friendly bases** | `RZ_Territory` (holdings by faction), `RZ_Safehouses`, the road posts (`RZ_Checkpoints`); hostility from `RZ_FactionLife.HasFeud`/`HasTruce` and the FactionManager's relations | the bearing and distance to the **nearest hostile holding** and to the road from it; the bearing to the nearest **friendly** one | defences face the hostile side; the friendly side is lighter (a fence, not sandbags); a feud makes it heavier |
| **The infected** | The land grid's lairs and wild land (`RZ_Land.IsWild`, `NearestCellWith`), the heat map (`RZ_HeatMap`), the town's infection (`RZ_Settlement.m_fInfection`) | the bearing to the nearest **lair** or wild land, and the town's infection band (under 30, 30–60, over 60) | burn pits and kill zones face it; infection over 60 adds a second line of wire and a watch at night |
| **History** | `RZ_TownHistory` (the town's events), `RZ_TownMemory` (burned buildings, barricades), `RZ_WorldFacts` (records), the holding's own changes of hands (`RZ_Territory` losses) | the site's **state** and **story** ([below](#5-the-story-of-each-site)) | the story's props and how worn the site is |

Each **side** of the site (eight 45° sectors from its middle) gets a **threat score** from these:
hostile base that way, road in, open approach, lair or wild land, ridge over it; minus sea, a
friendly base, a cliff. The highest sector is the **front**; the rules read the score of the
sector a piece is in.

## 2. The rules: where things belong

Each piece in `pieces.py` gets a few **rules** instead of only a spot. The plan's spots stay as
**preferences** (a rule like any other, worth a little): the solver lays the plan where the
ground allows and bends it where it doesn't.

### Zones

The solver first divides the site into zones on the grid; pieces then go into their zone.

| Zone | Where | Holds |
|---|---|---|
| **Wire** | the closed perimeter line ([layer 1](#3-building-in-layers-by-doctrine)) | barricades, wire, fences, scrap walls, cars as walls |
| **Gate** | where the main road crosses the wire | the barrier, the nest or tower, lights, the sign, the search spot |
| **Kill zone** | 10–40 m outside the wire on the threat sides, open ground only | nothing standing; bodies burned at its edge; stakes, a cleared strip |
| **Inner court** | the yard nearest the main door, most guarded | the quartermaster's counter, the roster, the leader's seat |
| **Stores** | inside the building, else against its wall at the inner court, the cell furthest from the wire | stores, fuel, ammunition (as scenery), samples |
| **Work** | the yard, under cover where it can be | the work tables, the workbench look, the radio table, sorting tables |
| **Kitchen** | by the building's back or side door, upwind of the latrine | the cook pot, firewood, water, the wash stand |
| **Sleep** | indoors at a building; else the yard's quietest corner (furthest from the gate and the generator) | tents, bedrolls, the drying line |
| **Sick** | the yard's far corner, screened, upwind, 10 m from the kitchen and sleep | the ward tent, the decon shower (Halcyon), the bitten chair (the Haven) |
| **Dump** | outside the wire on the down side, away from the threat front | the burn pit, the rubbish pile, broken furniture |
| **Dead** | outside the wire, on a quiet side, by a tree or a wall where one is | graves, crosses, the memorial's names (the Haven keeps theirs inside) |

### Rule types

| Type | Says | Examples |
|---|---|---|
| **In zone** | which zone a piece belongs to | the cook pot in Kitchen; tents in Sleep; the burn pit in Dump |
| **By a building part** | next to a door, window, wall or corner of the site's building | the cook pot within 4 m of the building's back or side door; the bear board beside the front door; the counter in front of the main door; crates against a wall, never alone in the open; the roster board on the wall by the main door |
| **By a building kind** | near another building of a kind (`RZ_DoorSlots.KindOf`) | the motor pool by a garage or barn; the water by a well or pump; the graves by the church if there is one |
| **Near a piece** | within a distance of another piece | chairs round the fire (2–3 m, facing it); the wash stand within 3 m of the water barrel; firewood within 4 m of the cook pot; the chopping block by the woodpile; a lamp within 4 m of every table used at night |
| **Away from** | at least a distance from a piece or a zone | the latrine 15 m from the kitchen and the water, and downwind of both; no tent within 6 m of a fire; the generator 10 m from Sleep; the burn pit 25 m from the wire's inside; nothing within 4 m of water |
| **Over** | covers another piece | a tarp or camo net over the work table or the stores' crates; a tin roof over the cold box |
| **Guarded** | the most guarded cell: furthest from the wire, nearest the main door, inside if it can be | fuel, stores, samples, the armoury look, the quartermaster, the leader's seat; the lab's sample table indoors |
| **Covers** | in sight of a target (a 2 m-high line on the grid, no solid cells) | the nest sees the gate and the main road out 80 m; a light covers the gate and a gap; a lookout sees the covered side |
| **High** | the highest free cell within reach, with a view | the lookout, the antenna, the flag |
| **Faces** | which way its front turns | barricades and nests face the threat front; seats face the fire or the table; a counter faces its customers; backs to walls where they can be |
| **Behind** | on the far side of something from the threat | the generator behind sandbags against a wall, on the side away from the front; tents behind the building |
| **Clear** | cells kept free of anything | a path 1.5 m wide from the main door to the gate and to the kitchen; a vehicle lane 4 m wide on a through-road and to a garage; 2 m in front of every door; the customers' spot at the counter |
| **Ground** | slope and footing | tents, tables and the garden flat (under 5°); the nest may sit on a bank; nothing on a step; a garden on soft ground, never on concrete (by the cell's surface) |
| **Spacing** | density and gaps | at most one piece per 12 m² in the yard; 1 m between pieces people walk round; clutter (bottles, cans) only near seats |
| **Varied** | breaks symmetry | mirror by seed (as today); jitter yaw 0–8° and position 0–0.5 m on clutter; two of a kind never in a straight line unless the plan says (Bastion's tents are) |

Each rule is **hard** (must hold: Away from, Clear, Ground, In zone) or **soft** (scored: Near, Covers,
Faces, the plan's spot). A piece is placed at the free cell with the best soft score that meets every
hard rule.

### Importance tiers

The plan's order already ranks pieces; tiers make "what goes first" explicit:

| Tier | Pieces | Dropped |
|---|---|---|
| 1. Security | the gate, the nest, wire on the threat front, the counter | never (the site logs a failure instead) |
| 2. Purpose | what the site is for: the roster and stores (Bastion), the toll (the Red Hand), the table and the memorial (the Haven), the sample table and decon (Halcyon) | only if tier 1 took the room |
| 3. Life | kitchen, water, sleep, the fire, the latrine | before purpose |
| 4. Character | the faction's marks, the trophy wall, the children's corner, the garden | before life |
| 5. Clutter and story | bottles, cans, extra crates, story props | first |

### The solver

Server-side, once per site per run, the result cached in `RZ_CampSite` (positions and yaws as data),
so the next visit lays the same site with no solving.

1. Read the grid and the surroundings (above).
2. **Zones**: the wire line first ([layer 1](#3-building-in-layers-by-doctrine)), then the gate on the
   main road, the kill zone outside, then the inside zones by distance from the wire and the door.
3. **Pieces in tier order**, plan order within a tier. For each: the candidate cells are those in
   its zone meeting every hard rule; at most **24 candidates** are scored (its plan spot first, then
   the nearest cells to it, then a spread by seed); the best wins; none = dropped, with the rule that
   failed.
4. **Deterministic**: every random choice comes from the place's seed (`Seed`, as today), so the same
   site is laid the same way after a restart, and a site differs from its neighbour.
5. **Budget**: at most 2 ms of server time per step of the game mode's tick, spread over ticks
   (a stronghold takes 3–5 ticks to solve before its first piece spawns); never more than one site
   solving at once.

## 3. Building in layers, by doctrine

Every site is built in this order; each layer is a set of pieces with the rules above, and a
later layer never takes a cell a former one needs.

1. **Close it off**: a perimeter that closes: the wire zone traced round the yard and the building,
   gaps filled by the faction's barrier pieces, the building's own walls and neighbouring walls,
   fences and hedges counted as closed; water counts as closed. The log and audit give the share
   closed.
2. **Harden the building**: one entrance; side doors blocked; ground-floor windows boarded
   (Traces' door boards, [TRACES.md](../docs/TRACES.md#doors-you-can-see), reused on windows by the
   same kit, **World > Traces > Kit > Door board** for each faction).
3. **Watch**: the nest or tower covering the gate, and a watch for every threat sector over the
   threshold (the covered side, the ridge, the jetty).
4. **Work and life**: inside the wire only, under cover where it can be; **anything valuable or
   delicate goes indoors** (as data: the building is where the stores are, shown by a guard at its
   door and a stores board, not by props inside).

### Bastion: soldiers who occupy

| Kind | Close it off | Harden | Watch | Work and life |
|---|---|---|---|---|
| Stronghold | sandbags and wire on every road in, a gate on the main road, wire across the yard's open sides, a cleared strip 20 m deep on the threat front | windows boarded by plank with firing gaps; one door; the stores' window | the nest at the gate, a lookout on the high point, a second nest toward a hostile holding within 3 km | roster board and flag at the inner court, stores indoors with a guard, the motor pool by a garage, the mess by the kitchen door, tents only if no building |
| Outpost | a sandbag run on the road, wire on the threat front | the farmhouse's door; windows boarded | the nest on the road; a lookout toward the covered side | the fire and the roll board, two tents squared |
| Road post | the barrier, sandbags on both verges | — | the nest down the road (toward the threat front if it's the other way) | a fire barrel and a net |

### The Red Hand: crews who squat

| Kind | Close it off | Harden | Watch | Work and life |
|---|---|---|---|---|
| Stronghold | scrap walls, tyres, a bus or car wrecks as walls where they stand (cars on the site grid count as wall when within 3 m of the line), the toll barrier on the main road | the garage shutter down; one door | a scrappy nest at the toll and one watching the road from the nearest hostile holding | the fire barrel and the sofa in the forecourt, the trophy wall facing out at the gate, the kennel by the gate, tents behind scrap |
| Outpost | scrap and tyres on the road, the rest open | — | a lookout behind tyres | the card table, the fire, two tents |
| Road post | the toll barrier, scrap on the verge | — | one leaning at the barrier | the card table |

### The Haven: people keeping a village going

| Kind | Close it off | Harden | Watch | Work and life |
|---|---|---|---|---|
| Stronghold | fences and gates, hedges kept, a cart across a side lane | the church doors; windows boarded on the threat side only | the bell tower or high point; the door | the long table and the memorial inside the fence, the garden, the washing, the children's corner furthest from the gate; graves by the church |
| Outpost | a fence on the road side | the door | the door | the table, the garden, the memorial |
| Safehouse | the garden gate, a fence on the road side | ground-floor windows boarded, the bear on the door | one at the door | the table outside the door, the pot and barrel by the kitchen |

### Halcyon: a clean zone in a dirty world

| Kind | Close it off | Harden | Watch | Work and life |
|---|---|---|---|---|
| The lab (stronghold) | chain-link or fence round the grounds, one gate, floodlights on the gate and the corners, a cleared strip all round | every ground-floor window boarded; one entrance; the decon tent **at** that entrance | guards at the gate and the entrance; a light on each corner; a watch toward the nearest lair | samples, the cold store and the work **indoors** (as data: guarded doors, a stores board); outside only the decon tent, two generators behind sandbags against the back wall away from the beds, the antenna on the high point, a smoking bench by the back door |
| Field station | a fence on two sides | the house's door with the decon screen | the guard pair at the door | the field table under a tarp, the cold box against the wall behind a tarp, the generator behind it |

**The lab, worked through.** The site grid reads the lab and its yard; the threat front is the road
in from the nearest town and, if it's nearer, the bearing to the nearest lair. Layer 1 traces a fence
line 8–12 m off the building, closing on the building's own walls and any wall or hedge on the grid;
the gate goes where the road crosses it, with two lights and the plague board. Layer 2 boards every
ground-floor window it can find and marks every door but the main one blocked (a board), and the
decon tent goes on the cells outside the main door, the path through it kept clear. Layer 3 puts a
guard post at the gate and one at the entrance, lights on the corners that see the most fence, a
watch on the side toward the lair. Layer 4: the generators behind sandbags against the back wall,
10 m from where people sleep; the antenna on the highest free cell; the sample table and the cold
box are no longer in the yard (indoors, as data); a bench and a tin of butts by the back door for the
night shift. What a player sees: a fenced, lit, boarded building with one way in through a shower,
and nothing left lying about.

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

## 6. The audit

Headless, in the runtime test (`tools/runtime_test`), and as a log summary in play.

### What it checks

The test solves **every site on the map** as data (no props spawned: the solver's output is
positions and footprints), so it runs on the dedicated server with no players.

| Check | How | Fails |
|---|---|---|
| Overlaps | each placed footprint against the others, the buildings' boxes and the grid's solid cells | any overlap over 0.1 m² |
| Floating or buried | height spread under each footprint against the prop's measured height | spread over 0.3 m on a flat piece |
| Outside the wire | a tier 2–4 piece outside the wire zone | any |
| Perimeter | the share of the wire line closed (pieces, walls, fences, water) | under 80% at a stronghold, 60% at an outpost; each gap named by bearing and width |
| Posts | gate and each threat sector's watch filled; their **Covers** lines clear | a stronghold without a gate watch |
| Paths | the Clear rules: door to gate, door to kitchen | a path blocked |
| Dropped | pieces dropped by tier, with the rule that failed | a tier 1 piece dropped |
| Routes | each patrol route's points walkable, none in water or over a drop | any |

Each site gets a **score** (0–100) from the checks; the run's summary lists every site under
**Sites > Audit threshold** (70) and the test fails if any stronghold is under it.

Log lines: *"Sites: <faction> <kind> at <grid>: score 84; 18 of 20 pieces; perimeter 91% (gap 4 m
at 230°); dropped: Latrine (Away from: kitchen 15 m)."* and one summary *"Sites audit: 31 sites,
mean 82, worst 64 (Red Hand outpost at 041-077), 0 overlaps."*

### Seeing it

| Tool | Does |
|---|---|
| `render.py --placed <log>` | draws the layouts **actually placed** on the map from the audit's log lines: each site top-down with its building, roads, water, wire, zones, pieces, posts, routes and gaps, with its score, in `tools/camp_kit/sites_placed.html` |
| **ReforgerZ: Fly to the next site** (Workbench plugin) | in Workbench play or the World Editor, moves the camera above the next site in the audit's list, then to its gate at eye height, so the author can take screenshots of every base in a few minutes |

## Performance

| Cost | Budget |
|---|---|
| Reading a site's grid | one sphere query, one height sample per metre, under 20 ms spread over ticks (2 ms a step); once a run per site |
| Solving | at most 24 candidates a piece, 30 pieces: under 10 ms a site, spread over ticks; cached |
| Memory | about 60 KB a stronghold's grid while solving, freed after; the cached layout under 2 KB a site |
| Spawning | unchanged: 4 pieces a tick, **Pieces max** (90); story pieces count toward it and are the first left out at the cap |
| People | unchanged: Live places (3), extras (6) |

## What stays as it is

- Which building a place moves into (`FindSite`, `PickSite`, `SiteTier`), the entries and the gate
  (`ReadEntries`), the quartermaster's counter and turn (`Counter`, `TurnToServe`).
- `RZ_DressingGuard`: nothing at a camp is usable for gain.
- Pieces spawn and go with the place's bodies, only near players; nothing is saved but what the
  game saves already.
- The kits stay in `pieces.py`; `build.py` writes them; plans stay as the preference.
- Today's open-ground layout (no building) is the same solver with no building parts.

## Settings

New on **Territory > Camp life** (`RZ_CampLifeSettings`):

| Setting | Default | What |
|---|---|---|
| **Solver** | on | build sites by the rules; off = today's plans |
| **Grid cell** | 0.5 | metres a cell |
| **Candidates a piece** | 24 | cells scored before a piece is dropped |
| **Layers** | on | close off, harden, watch before work and life |
| **Story** | on | each site's story elements |
| **Board windows** | on | harden: windows boarded with the faction's door board |
| **Alert states** | on | posts and movement change with a threat |
| **Road post run** | on | a walk to the nearest road post once a day |
| **Audit threshold** | 70 | the runtime test fails a stronghold under this score |

## For the PC

- Run **ReforgerZ: Measure prop footprints** once (and after adding a prop): it writes
  `tools/camp_kit/footprints.json`; then `py tools/camp_kit/build.py`.
- Choose the new props each layer needs (vanilla first, nothing usable): wire coils and pickets,
  chain-link or security fence sections, floodlights on poles (look only), a tin roof, a bench, a bucket
  for the water run, a cage look; and decals for the kill zone if the Traces spike found a way.
- With **ReforgerZ: Fly to the next site**, take a screenshot of every site of one map after each
  phase and compare it with `sites_placed.html`.

## Build order

| Phase | Builds | Done when |
|---|---|---|
| 1 | **The ground**: the site grid, real footprints (the plugin, `footprints.json`, `build.py`), drop don't squeeze, the surroundings record and the threat sectors | no overlaps in the audit's overlap check on Arland and Everon |
| 2 | **The audit**: the headless solve in the runtime test, the log lines, the score and threshold, `render.py --placed`, the fly-to plugin | every site on two maps scored and drawn |
| 3 | **The rules and the solver**: zones, rule types, tiers, the solver; pieces in `pieces.py` get their rules | mean score 80 on Everon; no tier 1 drops |
| 4 | **The layers**: close off, harden (windows and doors), watch; each faction's doctrine table; the lab as the first site to pass | the lab and every stronghold perimeter at 80% |
| 5 | **The people**: posts, shifts by post, patrol routes from the wire and roads, everyday movement by zone, alert states | the jobs log shows posts filled and routes walked |
| 6 | **The story**: the state, the library, its elements as scene kinds, ageing, lines and mentions | each faction's stories seen in play |
