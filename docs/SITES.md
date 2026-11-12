# Sites: the ground, the rules and the layers

How camp life ([DESIGN.md](DESIGN.md#camp-life)) reads the ground a camp stands on, how big its pieces
really are, what lies round each site, how each site is laid out from its pieces' rules and built in
layers by its faction's doctrine, and how every site is measured. Built: the site grid, real
footprints, the surroundings and the threat sectors, the rules and the solver, building in layers
(the wire and its gates, the boards, the watch, valuables indoors), and the audit with its drawing.
What comes next (the people at their posts and on their rounds, each site's story) is designed in
[designs/SITES.md](../designs/SITES.md) and not built.

Code: `Factions/RZ_SiteGrid.c` (the grid and footprints), `Factions/RZ_SiteSurroundings.c`,
`Factions/RZ_SiteRules.c` (zones, rules, layers, doctrine), `Factions/RZ_SiteSolver.c`,
`Factions/RZ_SiteWire.c` (the wire line), `Factions/RZ_SiteAudit.c`, and the survey in
`Factions/RZ_CampLife.c`. Settings: **Territory > Camp life** (below). Tools: `tools/camp_kit/`
(the pieces with their rules and the doctrine in `pieces.py`, footprints, `build.py`,
`render.py --placed`), the runtime test's site audit (`tools/runtime_test/run.sh sites`) and two
Workbench plugins.

## Contents

- [The site grid](#the-site-grid)
- [Real footprints](#real-footprints)
- [The surroundings and the threat](#the-surroundings-and-the-threat)
- [The rules](#the-rules)
- [The solver](#the-solver)
- [Building in layers](#building-in-layers)
- [The audit](#the-audit)
- [Seeing it](#seeing-it)
- [Settings](#settings)
- [What it costs](#what-it-costs)
- [Simplified](#simplified)

## The site grid

Every site's ground (each holding, Haven safehouse and road post) is read on a grid of 0.5 m cells, a
little each second, one site at a time; then its layout is worked out on the grid, kept for the rest
of the run (positions and turns as data) and the grid is freed. **Pre-read** (on) queues every site
once the land grid is read (at most 10 minutes after the start), strongholds and the lab first, so
camps are laid out before anyone arrives; a place a player comes near goes before the rest (if its
read had begun, it carries on). A visit lays the kept layout out at once; only the pieces and the
people wait for a player near. Nothing is saved.

| What | Reach |
|---|---|
| A place that moved into a building | round the building: its half-diagonal plus its road-entry ring (50 m at a stronghold, 40 m at an outpost, 28 m at a safehouse) plus 6 m, at least 56 m and at most 72 m each way (a stronghold about 240 × 240 cells) |
| A camp on open ground | 56 m each way (its plan, the shifts tried and the wire round it) |
| A road post | 16 m each way (both verges) |

| Read | How | Cell flag |
|---|---|---|
| Height | the ground every metre (`GetSurfaceY`), between them interpolated | **steep**: a metre square rising over 12°; **bank**: over 0.3 m in a metre |
| The sea | the ground under 0.5 m (`RZ_Util.IsWater`) | **water**, and **shore** within 4 m of it |
| Rivers, ponds, lakes | `TryGetWaterSurface` (river or pond only) every 2 m where the land grid says the 250 m square has inland water, every 4 m before the land grid is read, not at all where it says there's none | **water** |
| Buildings | each building's box (the building query, out to the grid's corners), every cell it touches; a 0.4 m band round it; its outside doors; its window children (by prefab name), both kept where they are for the harden layer | **inside**, **wall**, **door**, **window** |
| Everything else standing | one sphere query over the reach, every entity's turned box: walls, fences, hedges, rocks, props, cars, wrecks. Left out: people, terrain, roads, rivers, decals, lights, shapes, power lines, overhead things (bottom 2 m up) and flat things (under 0.12 m), paving, anything over 60 m across | **solid** |
| Trees and bushes | a tree (prefab under `/tree`) is a 0.5 m trunk disc; a bush is soft (a prop may stand on it, a path may not); a hedge is solid | **solid**, **soft** |
| Big rocks | a rock, boulder, stone or cliff over 4 m² is half buried and round: an ellipse at 0.6 of its box (the traces see the rest) | **solid** |
| Roads | each road segment near (the road network) at its real width (6 m if the road doesn't say) | **road**, **verge** (2 m) |
| Other systems' scenery | anything camp life counts as scenery (Traces' scenes and boards) | **kept** |
| The site's own pieces | each piece's real footprint as it's laid out, one cell wider (never onto a path kept clear) | **taken** |
| The paths kept clear | the solver's ways from the entrance to the gate and to the kitchen ([The solver](#the-solver)) | **clear** |

A piece may stand where every cell under every part of its footprint (each rectangle grown by
0.2 m) is free of water, steep, bank, inside, solid, road, kept and taken (and for work and life a path kept clear and the shore: nothing stands within 4 m of water); the spots kept clear (a
quartermaster, the medic, the barrier, a leader's seat, the front door's step, a counter's
customers) are still kept by distance. A quartermaster's counter indoors is fitted by its traces
instead (the grid's building cells don't apply to it). The rest of the fitting (flat enough, off the
line, the traces) is in [The solver](#the-solver).

## Real footprints

Every prop a camp-kit piece uses has its real box in `tools/camp_kit/footprints.json`: width, depth,
height, the box's middle from the prop's origin, and whether its origin is at its feet. Two things
write it:

| Tool | Where |
|---|---|
| **ReforgerZ: Measure prop footprints** (Workbench plugin, World Editor) | spawns each prop in `tools/camp_kit/props.txt` for a moment and reads its box and its children's (not lights, flames or decals) |
| `FOOTPRINTS=1 tools/runtime_test/run.sh sites` | the same on the dedicated server, as part of the site audit (today's file was written this way) |

`tools/camp_kit/build.py` (through `footprints.py`) gives each piece its footprint from it: its props'
boxes, turned and offset as they stand, up to 8 rectangles (the pair whose merged box adds least is
merged until 8 are left), leaving out props standing on another (0.3 m up or more: a lamp on a
table) and nets people stand under (`CamoNet`); and a 0.6 m square for each seat's person. It's
written on each kit entry as **Foot** (`m_sFoot`: `x z width depth turn; ...; s x z`) and its box
as **Foot min** and **Foot max**. A prop with no measured box falls back to the hand-typed
`pieces.FP` and `build.py` lists it (today three: the campfire's flames, a notice and a photo,
which have no mesh box). The openings' and Traces' compositions use the same props, so they're
measured too. `render.py` draws every prop at its measured size and offset.

The first measurement showed why pieces clipped: a `WoodLog_01_A` is a 6 m trunk (drawn as 2 m), the
small camo net 13.6 m across (4 m), a long sandbag 3.5 m (2 m), the big US tent 6.3 × 9.9 m (5 × 7),
`TinWall_01_3m`'s origin at one end. The kits' plans were re-spaced (17 pieces moved 0.5 to 2 m)
so `build.py`'s overlap check passes with the real sizes. The layers' props (the wire, the fences,
the scrap, the sandbags, the floodlight, the checkpoint's sign) were measured the same way: a
concertina coil is 3.45 m long down its own z (so a run turns it 90°), the chain-link, plank fence
and tin sections are 3, 4, 6 and 8 m with their origin at one end.

## The surroundings and the threat

Read once per site with its grid, from data the game already has, and kept as a small record
(`RZ_SiteSurroundings`). The solver reads it (the threat front for what faces the threat and what
stands behind the building, the threatened sides for the boards and the lookouts, the high ground,
the quiet side for the dump) and the audit measures by it; the log shows it.

| Context | Read from | Kept |
|---|---|---|
| Roads | the building's road entries | the main road in (the gate's bearing), each entry's bearing, through-roads (two entries on one road) |
| Water | sixteen ways out to 80 m: the sea (under 0.5 m), rivers and ponds | the sea sides; a drop (8 m lower within 30 m) each way |
| Piers | a pier, jetty, dock or slipway on the grid (by prefab name) | where one is |
| High ground | the grid's heights; sixteen ways out to 200 m | the highest point on the grid and its height over the yard; a ridge: ground 10 m over the yard |
| Cover | the grid's outer ring, each 45° side | the share that's buildings, solid or bushes: **open** under 8%, **covered** over 25% |
| Bases | holdings (`RZ_Territory`), road posts, Haven safehouses within 3 km; hostile by a feud, else not under a truce and enemies by the faction manager's relations, else rivals by Fame | the nearest hostile one (bearing, distance, faction, a feud) and the nearest friendly one |
| The infected | the land grid's nearest lair candidate (else wild land) within 2.5 km; the town's infection | its bearing and distance; the infection (0–100) |
| History | the town's history (Traces) | its hurts in the last 3 days (deaths, pyres, infestations, takings, losses) and whether it ever changed hands |

Each of the eight 45° **sides** gets a threat score: a hostile base that way +3 (+1 within 1 km,
+1 in a feud), the main road in +2, another road entry +1, the infected's land +2 (+1 when the town
is over 60), a ridge +1, open ground +1; a friendly base −1, water −3, else a drop −2. The highest
side is the **front** (the main road's side on a tie).

## The rules

Every camp-kit piece in `tools/camp_kit/pieces.py` has a **tier**, a **zone** and its **rules**
(`pieces.RULES`), written onto each kit entry by `build.py` (**Tier**, **Zone**, **Rules**). The
plan's spot stays as a **preference**: the solver lays the plan where the ground allows and bends it
where it doesn't.

### Zones

Worked out on the grid once the wire line is traced ([Close it off](#close-it-off)). A site with no
wire (a road post, or **Layers** off) has no zones: every piece may stand anywhere its rules allow.

| Zone | Where | Holds |
|---|---|---|
| **Yard** | anywhere inside the wire, 1.5 m in from it (the default) | most pieces |
| **Wire** | on the line, within 1.5 m of it | the plan's barricades |
| **Gate** | within 12 m of the main gate, inside the line or on it | the gate's sign and barrier, the toll, the red hand, the plague board, the kennel, the trophy wall |
| **Kill zone** | outside the wire on the threat sides | nothing ever stands there |
| **Court** | inside the wire within 18 m of the one entrance | the roster and duty boards, the flag, the memorial, the bear, the decon shower, the stall, the stores board |
| **Stores** | inside the wire within 4 m of the building | the cold store |
| **Work** | inside the wire within 15 m of the building | the tables, the radio table, the tarp |
| **Kitchen** | inside the wire within 14 m of the kitchen's door: the building's outside door furthest from the entrance, else the middle of its yard side; by the entrance when no way leads round to it | the cook pot, the woodpile, the water, the wash and shaving stands |
| **Sleep** | inside the wire, 12 m or more from the gate (spots tried first in the two corners furthest from it) | tents (open ground only: in a building they sleep indoors), the washing and drying lines, the children's corner |
| **Sick** | inside the wire, 8 m or more from the gate (the far corners first) | the ward tent |
| **Dump**, **Dead** | outside the wire within 20 m of it, on a side that isn't threatened | nothing yet: the burn pit and the graves come with each site's story ([designs/SITES.md](../designs/SITES.md#5-the-story-of-each-site)) |

### Rule types

Each piece's rules are a line of text, `;` between them (`m_sRules`):

| Rule | Written | Hard or soft | Example |
|---|---|---|---|
| **In zone** | its **Zone** | hard | the cook pot in Kitchen; tents in Sleep |
| **Away from** | `away <what> <m>` (a piece, `A\|B`, `use:Sleep`, or `water`, `gate`, `wire`, `door`, `road`) | hard | the latrine 15 m from the cook pot and the water, 10 m from the table, the fire, the gate; no tent within 6 m of a fire; the generator 10 m from the tents and 6 m from the door |
| **Ground** | `flat <degrees>` (about 12 without it) across the footprint, and the ground under each of its props varying at most 0.28 m (so nothing floats or sinks); `bank` (may stand on a bank or steep ground: up to 20°, a prop's ground up to 0.6 m), `soil` (off the verges and the building's walls) | hard | tables and tents under 5°; the nest on a bank; the garden on soil |
| **Clear** | the paths the solver keeps ([The solver](#the-solver)) | hard | nothing on the way from the entrance to the gate and to the kitchen |
| **Near** | `near <what> <m>` | soft | chairs round the fire, the woodpile by the cook pot, the wash stand by the water barrel, the flag by the roster board |
| **By a building part** | `door <front\|side\|back\|any> <m>`, `wall <m>`, `window <m>`, `corner <m>` | soft | the cook pot by the back or side door; the roster board on the wall by the front door; the decon shower at the entrance; the cold box against the wall |
| **By a building kind** | `kind <garage\|farm\|church\|shop\|clinic\|house\|public\|a word in a prefab's name> <m>` | soft | (none of today's pieces uses it yet) |
| **Over** | `over <what>` | soft | within 2 m of it |
| **Covers** | `covers <gate\|road\|threat\|out>` | soft (+15 or −15) | the nests and guard posts see the gate and 30 m out along the road (a 2 m-high line on the grid) |
| **High** | `high` | soft | the antenna on the highest ground |
| **Faces** | `faces <front\|road\|threat\|out\|in\|gate\|door\|wall\|<what>>` | its turn | nests and gate pieces face out along the road; the sofa faces the fire barrel; boards face out from the wall |
| **Behind** | `behind threat` | soft | the generator and the latrine on the building's far side from the threat |
| **Guarded** | `guarded` | soft | the cold box and the sorting table: far from the wire, near the entrance, against the building |
| **Spacing** | every piece's cells grown by a cell as it's placed (a gap of about 0.7 m between footprints); at most one piece of work and life per 12 m² of the yard inside the wire | hard | |
| **Varied** | `jitter` (turned 0–8° and moved 0–0.5 m by the seed), `noline` (two of a kind not in one line) | soft | clutter jittered; the tables and the Haven's tents never in a row |

A spot's score is −0.6 a metre from the plan's spot (where the plan has one), then each soft rule:
Near, By a building part and By a building kind give their worth (10, 8 or 6) within reach and two
less a metre past it; Covers +15 or −15; Behind +6 or −3; Guarded up to +5 for distance from the
wire, −0.3 a metre from the entrance, +3 against the building; High +2 a metre above the middle
(at most +12); two of a kind in a line −4.

### Importance tiers

| Tier | Pieces | Dropped |
|---|---|---|
| 1. Security | the counter, the nests and guard posts, the barricades, the gate | the audit logs a failure; twice the candidates are tried first |
| 2. Purpose | the roster and duty boards, the toll, the Haven's long table and memorial, the sorting table, the cold store, the decon shower, the stores board | twice the candidates are tried |
| 3. Life | the kitchen, the water, the fire, the tables, the tents, the latrine, the generator | |
| 4. Character | the flag, the trophy wall, the red hand, the kennel, the children's corner, the garden, the stall, the bear, the plague board, the lines, the antenna, the duckboards | |
| 5. Clutter | bags and bottles, the smoking bench | |

The pieces are placed tier by tier, in plan order within a tier, so the most important take their
spots first. A kit can give a piece its own tier (`pieces.KIT_TIER`: the Haven's table is tier 2).

## The solver

`Factions/RZ_SiteSolver.c`, setting **Solver** (on). Once a site's grid is read, it's laid out from
its kit's rules, a little each second (**Site read ms**, the same budget as the reading), one site
at a time, once a run; the result is kept as the layout is (positions and turns as data,
`RZ_SiteLayout`), so every later visit lays the same site again with no solving. Every random
choice comes from the place's seed (`Seed`), so a site is laid the same way after a restart and
differs from its neighbour.

1. **The counters** first, in front of each quartermaster, as before ([DESIGN.md](DESIGN.md#camp-life)).
2. **The plan's frame**: by a building its yard side, on open ground its place facing the road, at a
   road post along the road (`ChooseYard`, `ChooseOpen`: where most of the plan fits). Each piece's
   spot in it is its **preference**.
3. **The layers** in order ([Building in layers](#building-in-layers)): the wire line and its gates,
   the way from the entrance to the gate kept clear, the gate's pieces, the barricades and the
   barrier, the boards, the watch, then work and life by tier.
4. **Each piece**: up to **Candidates a piece** (24; twice that for tiers 1 and 2) spots are scored:
   its plan spot first, then the spots its zone and rules point to (the gate's sides, round the
   entrance, round the kitchen's door, the far corners, along the building's walls, round the piece
   it should be near, the high ground), then rings round its plan spot (or, when that isn't in its
   zone, round the zone's middle) out to 13 m, then spots spread over the yard by the seed. A spot
   whose middle is out of its zone or blocked is passed over cheaply and not counted. Each counted
   spot must meet every hard rule and the ground (every cell under its whole real footprint free:
   [The site grid](#the-site-grid); flat enough; off the line; clear of the spots kept free); the
   best four by score are then traced in turn (nothing standing in its props' boxes) and the first
   clear one wins. The cook pot's must also be reachable on foot from the entrance inside the wire
   (then the way to it is kept clear too). A tier 1 piece that fits nowhere is tried again on
   rougher ground (up to 20°, a prop's ground up to 0.6 m): security goes up even where it floats a
   little.
5. **None fits: dropped**, never squeezed (its whole footprint must fit where it stands), with the
   reason the most spots failed on: a rule (*"Away from: Latrine 15 m"*), the ground (water, a
   building, the road, a bank, steep ground, something standing there, other scenery, another piece,
   a path kept clear, outside the ground read, not flat), the line, a spot kept clear, its zone, or
   something in the way (the traces). The plan's order and the tiers decide what's dropped first: the
   most important pieces come first and take their spots.
6. **Valuables indoors.** At a site with a building, a kit entry marked **Indoors** (`pieces.DOCTRINE`)
   isn't laid out: it's kept as data (the building is where the stores are) and the stores board
   stands by the entrance instead.

With **Solver** off (or **Site grid** off) the plan is laid down whole as before: each piece where
the plan puts it or dropped ([DESIGN.md](DESIGN.md#camp-life)).

## Building in layers

Setting **Layers** (on). Every site is built in the order real people would build it; each layer is
a set of pieces placed by the rules above, and a later layer never takes a cell a former one holds.
Each kit's **doctrine** (`pieces.DOCTRINE`, written onto the kit by `build.py`: **Close**,
**Harden**, **Ring**, **Yard** and its **Layers** list of pieces) says how much of each layer a kind
of site gets. Road posts have none: their plan (the barrier, a nest, sandbags or scrap on both
verges) is their doctrine.

### Close it off

`Factions/RZ_SiteWire.c`. The **wire line** is a ring round the building (or, on open ground, round
the plan), **Ring** metres out on each side (7–9) and on the yard's side as deep as the yard's part
of the plan needs (at least **Yard**, 10–14, at most 32). Each side is then moved 3 m in to 6 m out
(never nearer the building than 4 m) to where more of it is closed already or can take a barrier,
so the line follows the real perimeter, meeting a neighbour's wall, a fence, a hedge or the water.
Every 0.5 m along the sides the doctrine closes, a **station** says what closes it:

| Station | What |
|---|---|
| **Closed already** | a building, a wall, a fence, a hedge, a tree's trunk, a car, a rock, water (the grid's cells on the line and half a metre either side), or a bank falling 1.5 m within 3 m outside the line |
| **Gate** | a road crossing the line, with a step either side: kept open (nothing is ever put on a road, so supply trucks and players' cars pass); a road running along the line is a gate only for 12 m in its middle |
| **Barrier** | the faction's barrier, laid in runs ([below](#each-factions-doctrine)) |
| **Gap** | nothing: an opening (off the grid or on other scenery, a gap nothing can fill) |

The **entrance** is the front door's step (the nearest spot a body can stand within 6 m of it; the
yard's middle on open ground). The **main gate** is a road crossing the entrance can walk to inside
the wire, the nearest the entrance and toward the road in from the next town (the main road entry);
with no such road, an opening 5 m wide on the side toward the road in, where nothing stands and the
entrance can walk to. Then, in order:

1. **The way in kept clear**: a path about 1.5 m wide along the way found on the grid, inside the
   wire, from the entrance to the main gate; the gate's pieces, the barrier, the boards and work and
   life keep off it, and no piece's gap closes it.
2. **The gate**: the kit's gate pieces (Zone **Gate**: Bastion's checkpoint, the Red Hand's toll and
   red hand, the Haven's gate swung open, Halcyon's plague board) beside the opening, facing out
   along the road.
3. **The barricades** (Zone **Wire**): beside the other gates first, then over the widest gaps on
   the threat front, each slid a metre or two along the line to fit, else anywhere within a metre of
   the line.
4. **The barrier**: each side's gaps filled with the faction's runs, the longest that fits first
   (about 10, 6, 3 and 2 m), each nudged half a metre in or out where the line itself is blocked. A
   run may stand on steep ground and banks but not on water, a building, something standing, a road,
   other scenery or a path kept clear; it's **tilted** to follow the ground along it (up to 20°, no
   point of the ground more than 0.25 m off its line; it doesn't bend). The site's log and audit line
   give the **share closed** and the widest gap.

### Harden the building

At a site with a building (**Harden**: none, doors, threat, all):

- **One entrance**: the outside door nearest the front door's spot. Every other outside door of the
  building (the grid's door marks) is **blocked** with the kit's door board, at most 4.
- **Windows boarded** (setting **Board windows**, on): every ground-floor window (the building's
  children named *window*, under 3 m up and within 2.5 m of its box's sides), or with **threat** only
  those on a side facing a threatened sector or the front, at most 14, with the kit's window board at
  the window's own height, not those within 1.6 m of a door.
- **Who holds it**: the faction's own door board from **World > Traces > Kit** (Traces' door marks,
  [TRACES.md](TRACES.md#doors-you-can-see)) leaned beside the entrance, if the kit has one.

Boards stand on the building's wall: the grid isn't asked (the audit doesn't count them against the
building).

### Watch

1. **The nest over the gate**: the plan's watch pieces (Bastion's and the Red Hand's nests, the
   Haven's lookout, Halcyon's guard post) by their rules: inside the wire, beside the gate, seeing it
   and the road out (Covers), facing out along the road, on a bank if need be.
2. **A lookout toward each threatened side** that no watch faces yet (threat 3 or more, or the
   front: as the audit), the most threatened first, at most three: the kit's lookout piece just
   inside the line that way, facing out.
3. **Lights** (Halcyon's lab): a floodlight each side of the main gate and in each corner of the
   wire, facing out.
4. **The building's lookouts** (where its sentries stand, [DESIGN.md](DESIGN.md#camp-life)): of five
   spots along each of its sides, 2 and 4 m out, the two (a safehouse's one) that see the gate and
   stand nearest it; the best one only if none sees it.

### Work and life

Everything else, tier by tier, inside the wire only ([The rules](#the-rules)). At a site with a
building, **anything valuable or delicate goes indoors** (kit entries marked **Indoors**): kept as
data, not laid out, and the **stores board** stands by the entrance once anything did.

### Each faction's doctrine

| Kit | Close | Harden | Barrier | Gate, watch, lights |
|---|---|---|---|---|
| Bastion stronghold | all round | all | concertina wire (three coils, two, one; a hedgehog in a gap) | the checkpoint (a stop sign, a knife rest pulled aside), the nest, sandbag barricades, a nest as lookout; side doors sandbagged; the stores (the tarp's crates) indoors with the stores board |
| Bastion outpost | the road's side and the most threatened other | all | sandbags chest high, a coil, a hedgehog | the nest, a sandbag barricade, a nest as lookout; side doors sandbagged |
| Red Hand stronghold | all round | doors | rusted tin sheets, a steel road block, tyres | the toll, the red hand, the trophy wall and the kennel at the gate, the nest, scrap barricades; the shutter down on every other door |
| Red Hand outpost | the road's side | none | the same | the toll, the red hand, the nest |
| Haven stronghold | all round | threat | plank fences, an old panel, crates | the gate swung open, a lookout's chair at the gate, fence barricades; doors shut with pallets |
| Haven outpost | the road's side | doors | the same | a lookout's chair, a fence barricade |
| Haven safehouse | the road's side | all | the same | a lookout's chair |
| Halcyon's lab (stronghold) | all round | all | tall chain-link, a coil in a gap | the plague board, the guard post, fence barricades, floodlights on the gate and the corners; the tables, the ward tent, the cold store, the sorting table, the notebooks, the radio, the tarp and the chairs indoors with the stores board; outside the decon shower at the entrance, the generators behind sandbags against the wall on the far side from the threat, the antenna on the high ground, a bench by the back door |
| Halcyon field station | the road's side and the most threatened other | doors | the same | the plague board, the guard post |

**The lab, worked through.** The grid reads the lab and its yard; the threat front is the road in
or the lair. The wire is traced 9 m off the building (deeper on its yard side), each side moved to
meet the walls, hedges and water already there; the main gate is where the road crosses it, the
way from the entrance to the gate is kept clear, the plague board and the guard post stand beside
the gate. Chain-link fills the gaps. Every outside door but the entrance is shut with pallets,
every ground-floor window boarded. A floodlight each side of the gate and in each corner; a guard
post toward the lair if no watch faces it. Then the decon shower at the entrance, the generators
behind sandbags against the back wall, the antenna on the high ground, the bench by the back door;
the tables, the ward, the cold store and the samples are indoors (as data), the stores board
by the entrance. What a player sees: a fenced, lit, boarded building with one way in through a
shower, and nothing left lying about.

## The audit

Every site laid out is measured once a run, as data, and logged; the runtime test lays out every
site on the map this way with nothing spawned (`RZ_CampLife.AuditAll`).

| Check | How | A hard failure |
|---|---|---|
| Overlaps | every placed rectangle against the other pieces' (exact areas, clipped), the buildings' boxes (not a counter, which may stand indoors, nor a board on the wall) and the grid's solid cells; over 0.1 m² is an overlap | any |
| Floating or buried | the ground under each prop varying over 0.3 m (not a board on the wall) | |
| Posts | the watch pieces (nests, lookouts, guard posts) and the building's lookouts: does one see the gate (a 2 m line on the grid, no solid or building) within 80 m; each side with threat 3 or more (and the front) watched by a post facing within 67.5° (a lookout placed toward a side isn't asked to see the gate) | a stronghold with no gate watch |
| Paths | a walkable way (1 m wide, round water, buildings, solid, bushes, scenery and pieces) from the entrance (the solver's step; from the nearest square a body can stand on within 2 m) to the gate (the wire's main gate) and to the spot in front of the cook pot | |
| Dropped | each dropped piece by its tier ([Importance tiers](#importance-tiers)) | a tier 1 piece dropped |
| Patrol | the patrol's four points (the first shift): dry, and no 4 m drop within 6 m | |
| Perimeter | the share of the wire line closed (the ground, the barrier, the gates) and its widest gap by bearing and width | under 80% at a stronghold, 60% at an outpost, a field station or a safehouse |
| Outside the wire | a plan piece of tier 2–4 (not the gate's or the wire's own, not a dump's or the dead's) standing outside the line | any |
| Over water | the building the place moved into (its middle), the yard where the people come out, or any piece (its spot) over the sea, a river or a pond | any |

The **score** (0–100) starts at 100: −15 an overlap (at most −45), −2 a floating prop (−20),
−25 for a stronghold with no gate watch (−10 elsewhere; road posts aren't asked), −5 a gate post (a
nest, a guard post, a building's lookout) that can't see the gate (−10), −5 a threatened side unwatched (−15), −10 a blocked path, a dropped
piece −20, −6, −3, −1.5 or −0.5 by its tier 1 to 5, −4 a bad patrol point (−16), half a point for
each percent the perimeter is under its bar (−15), −3 a piece outside the wire (−12).

Each site's line, in play too (`[ReforgerZ] Sites: ...`):

*"Sites: Halcyon stronghold at 27-17: score 86; 38 of 40 pieces; perimeter 91% (gap 3 m at 216), 2
gates; 9 indoors, 4 doors blocked, 5 windows boarded; front NW (road in, a lair); dropped: Antenna
(steep ground); 0 overlaps; 2 floating or buried; posts: gate watched, 5 posts (2 can't see the
gate), 2 of 2 threatened sides watched; paths: door to gate clear; patrol 4 of 4 points walkable;
read 85 ms in 2 steps (most 60 ms, the longest part 5 ms), laid out in 95 ms."*

and the summary (the runtime test asks for it once every site is done):
*"Sites audit: 16 sites, mean 83, worst 61 (...), 0 overlaps; 2 with a hard failure; 2 under 70: ..."*
and *"Sites audit perimeters: The Haven stronghold at 34-44 92%, ...; 0 tier 1 pieces dropped, 0
pieces outside the wire, 0 sites with something over water."* **Audit threshold** (70) is the pass mark: the site audit (`run.sh sites`)
fails while any stronghold scores under it.

**Where it stands** (2026-11-12, the runtime test's site audit): Everon's 16 sites score a mean of 81
(80–86 over four rolls of the holdings; the fixed plans scored 35), with 0 overlaps, no tier 1 piece
dropped, nothing over water and the strongholds' wire 87–96% closed (the lab 92–96%); Arland's 9 a
mean of 85–86 (24–40 before). What's left is mostly ground: pieces dropped on banks, on roads through
a yard and against neighbouring buildings, and a stronghold on a hillside whose watch can't see its
gate (the Red Hand at 106-118 on one roll, 53).

### The runtime test

`tools/runtime_test/run.sh sites [arland] [everon] [cain]` boots each map's own sub-scene (with the
map's AI world, so its road network, and camp life's kits copied from `RZ_GameMode.et`), waits for
the land grid, measures the props (and prints `RZS roll`: which way a roll tilts a piece, for the
barrier runs on slopes), audits every holding, safehouse and road post, prints the lines and closes
(Arland about 1.5 minutes, Everon about 6). Last, the **pier probe** tries every town as each
faction's stronghold and prints, where they differ or either is over water, the building picked
before the waterside was ruled out and now (`RZS pier: ...`). The site audit queues every site itself,
with its drawing, so it turns **Pre-read** off. It runs in its own network namespace where
it can, so beside a full test. The full runtime test runs the same audit alongside its route and
prints the summary; its world has no AI world, so no roads, so its scores aren't comparable. How to
run both: [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#runtime-test-without-workbench).

## Seeing it

| Tool | Does |
|---|---|
| `py tools/camp_kit/render.py --placed <console.log>` | draws `tools/camp_kit/sites_placed.html` from a site audit's log ("Sites draw" lines): each site top-down on the ground it read (water, buildings, walls and trees, bushes, roads, steep ground, the paths kept clear), its building, road entries, gate and door, the wire line (what closes each stretch: the ground, the barrier, a gate, or a gap) and the ring it was traced on, every piece's real footprint by tier and every drop (dashed, hover for why), what went indoors, the posts and their lines to the gate, the patrol's loop, any overlap, and the threat on each side; and writes `tools/camp_kit/sites_list.txt` |
| **ReforgerZ: Fly to the next site** (Workbench plugin, World Editor) | reads `tools/camp_kit/sites_list.txt`: each run moves the camera above the next site looking down on it, then to its gate at eye height looking in, then on to the next; for a screenshot of every base to hold against `sites_placed.html` |

The committed `sites_placed.html` and `sites_list.txt` are Everon's audit of 2026-11-12 with the solver and the layers (16 sites, mean 81; the
holdings move with each season's roll, so a new run may differ).

The audit's drawing lines also name the big solid things the grid read at each site (class, prefab,
size and how many), to tell a rock from a wall when a piece is dropped for "something standing there".

## Settings

On **Territory > Camp life** (`RZ_CampLifeSettings`):

| Setting | Default | What |
|---|---|---|
| **Site grid** | on | read each site's ground on the grid, its surroundings and its audit; off = the older point checks, nothing read, solved or audited |
| **Site read ms** | 2 | milliseconds of server time a second for reading a site's grid and then solving it (one site at a time); its pieces spawn once it's laid out |
| **Pre-read** | on | read and lay out every site in the background from the start, strongholds first; off = a site is read the first time a player comes near |
| **Audit threshold** | 70 | the audit's pass mark |
| **Solver** | on | lay sites out by their pieces' rules ([The solver](#the-solver)); off = the plans laid down whole |
| **Candidates a piece** | 24 | spots scored before a piece is dropped (twice that for tiers 1 and 2) |
| **Layers** | on | build in layers by each faction's doctrine (the wire and its gates, the boards, the lookouts, the lights); off = work and life only, no wire |
| **Board windows** | on | harden: board ground-floor windows (the doctrine says which); off = only side doors are blocked |
| **Layer pieces** | 60 | the layers' pieces at a site at most, on top of its piece count |

Each kit's doctrine is on the kit itself (**Close**, **Harden**, **Ring**, **Yard**, **Layers**,
written by `build.py`); **Pieces max** (200) caps camp life's pieces on the server, the layers'
with them. The cell (0.5 m) is fixed: the grid's bands and checks are measured in it.

## What it costs

| Cost | Measured (dedicated server, 2026-11-12) |
|---|---|
| Reading a site's grid | 15–160 ms in all (a stronghold's 50–160 ms), read in small parts (a row of heights, a 24 m square's entity query, 16 entities, a building, a row of water or shore), the longest part 1–17 ms; at **Site read ms** 2 a stronghold's first pieces spawn about a minute after a player comes within 300 m (in the full runtime test a field station took 15 steps, 19 s) |
| Solving it | 40–300 ms in all (a stronghold's 140–300 ms), in parts: the start (the counters, the plan's frame, tracing the wire, where the entrance can walk), each piece, each side of the wire; the longest part 14–110 ms (the lab's start, or a tier 1 piece's three passes), the rest a few ms. At **Site read ms** 2 a site is read and solved in about 1–4 minutes of the background, once a run (pre-read: all of Everon's 16 in about half an hour after the land grid) |
| Memory | a 72 m grid is 288 × 288 cells, two to an int (about 170 KB), with its heights and water marks every metre (about 170 KB), freed once laid out; the kept layout a few hundred bytes a piece (a stronghold with its layers about 60 pieces) |
| Spawning | 4 pieces a tick for all places together, as before; a stronghold's 20 pieces and its layers' 30 to 50 are up in about a minute and a half |
| People | unchanged ([DESIGN.md](DESIGN.md#camp-life)) |

## Simplified

- The wire is a rectangle round the building (or the plan), each side moved in or out to meet what
  closes already; it doesn't bend round a yard's corner or follow a curving road, and a road running
  along it stays open (but for 12 m in its middle, a gate).
- A barrier run doesn't bend: it's tilted to follow a slope along it (up to 20°, the ground within
  0.25 m of its line) and stands level across it; a gap shorter than the shortest run (about 2 m),
  ground over 20° or more uneven than that, or a corner where two runs would cross stays open. A bank falling 1.5 m
  within 3 m outside the line counts as closed.
- A gate is an opening with nothing across it (no barrier pole), so the roads stay open for the
  supply trucks and players' cars.
- Windows are found by prefab name (the VERIFY); a board is a pallet at the window's own height,
  the same for every faction (Bastion's firing gaps aren't drawn); a door board stands at the door's
  origin. Only the building the place moved into is hardened.
- Valuables indoors are data only: nothing is placed inside, and nobody guards the door for them
  yet (the people, [designs/SITES.md](../designs/SITES.md#4-the-people)).
- **By a building kind** is used by one piece (the water barrel near a well); the **Dump** and
  **Dead** zones are worked out but hold nothing yet (the story); the kill zone is kept by placing
  work and life inside the wire, not by clearing anything outside it.
- **Over** is "within 2 m of it" (the nets are overhead in their own pieces); the latrine's
  "downwind" is "behind the building from the threat" (there's no wind); **soil** is "off the verges
  and the building's walls" (the grid reads no surface).
- The building's lookouts stand on the ground 2–4 m out from its sides, not on its roof or in a bell
  tower; a lookout piece and the lights stand just inside the wire.
- A part of a site's solving runs in one go, before the budget is checked: the start (the counters,
  the plan's frame, tracing the wire, choosing its gates where the entrance can walk) or one piece
  (a tier 1 piece's three passes), up to about 110 ms once per site a run, in the background.
- A prop's footprint is its box seen from above, its pitch and roll ignored (a pallet stood on edge
  claims its flat size).
- Big rocks are ellipses inside their boxes, not their shapes; the traces catch what sticks out.
- The audit's paths and posts read the grid only: a path or a line of sight off the grid counts as clear.
- The full runtime test's world has no road network, so its audit has no road entries.
- The waterside is ruled out by prefab names and by water under a building's box: a quay that's
  terrain-high concrete under an ordinary building's name isn't caught by name (its cells read as
  dry ground, so pieces may stand there).
- Neither test spawns the mod's own compositions (the server can't load them from the throwaway
  addon), so the drawing and the fly-to screenshots are how to check what stands.
