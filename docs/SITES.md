# Sites: the ground and the audit

How camp life ([DESIGN.md](DESIGN.md#camp-life)) reads the ground a camp stands on, how big its pieces
really are, what lies round each site, and how every site is measured. Built: the site grid, real
footprints, drop don't squeeze, the surroundings and the threat sectors, and the audit with its
drawing. What comes next (placement rules and a solver in place of the fixed plans, building in
layers by doctrine, the people's posts and patrols, each site's story) is designed in
[designs/SITES.md](../designs/SITES.md) and not built.

Code: `Factions/RZ_SiteGrid.c` (the grid and footprints), `Factions/RZ_SiteSurroundings.c`,
`Factions/RZ_SiteAudit.c`, and the survey in `Factions/RZ_CampLife.c`. Settings: **Territory > Camp
life** (below). Tools: `tools/camp_kit/` (footprints, `build.py`, `render.py --placed`), the runtime
test's site audit (`tools/runtime_test/run.sh sites`) and two Workbench plugins.

## Contents

- [The site grid](#the-site-grid)
- [Real footprints](#real-footprints)
- [Drop, don't squeeze](#drop-dont-squeeze)
- [The surroundings and the threat](#the-surroundings-and-the-threat)
- [The audit](#the-audit)
- [Seeing it](#seeing-it)
- [Settings](#settings)
- [What it costs](#what-it-costs)
- [Simplified](#simplified)

## The site grid

The first time a run dresses a place (a player near a holding, a Haven safehouse or a road post),
its ground is read on a grid of 0.5 m cells, a little each second, one site at a time; then its
layout is worked out on the grid, kept for the rest of the run (positions and turns as data) and the
grid is freed. Every later visit lays the kept layout out at once. Nothing is saved.

| What | Reach |
|---|---|
| A place that moved into a building | round the building: its half-diagonal plus its road-entry ring (50 m at a stronghold, 40 m at an outpost, 28 m at a safehouse) plus 6 m, at least 46 m and at most 72 m each way (a stronghold about 240 × 240 cells) |
| A camp on open ground | 46 m each way (its plan and the shifts tried) |
| A road post | 16 m each way (both verges) |

| Read | How | Cell flag |
|---|---|---|
| Height | the ground every metre (`GetSurfaceY`), between them interpolated | **steep**: a metre square rising over 12°; **bank**: over 0.3 m in a metre |
| The sea | the ground under 0.5 m (`RZ_Util.IsWater`) | **water**, and **shore** within 4 m of it |
| Rivers, ponds, lakes | `TryGetWaterSurface` (river or pond only) every 2 m where the land grid says the 250 m square has inland water, every 4 m before the land grid is read, not at all where it says there's none | **water** |
| Buildings | each building's box (the building query, out to the grid's corners), every cell it touches; a 0.4 m band round it; its outside doors; its window children (by prefab name) | **inside**, **wall**, **door**, **window** |
| Everything else standing | one sphere query over the reach, every entity's turned box: walls, fences, hedges, rocks, props, cars, wrecks. Left out: people, terrain, roads, rivers, decals, lights, shapes, power lines, overhead things (bottom 2 m up) and flat things (under 0.12 m), paving, anything over 60 m across | **solid** |
| Trees and bushes | a tree (prefab under `/tree`) is a 0.5 m trunk disc; a bush is soft (a prop may stand on it, a path may not); a hedge is solid | **solid**, **soft** |
| Big rocks | a rock, boulder, stone or cliff over 4 m² is half buried and round: an ellipse at 0.6 of its box (the traces see the rest) | **solid** |
| Roads | each road segment near (the road network) at its real width (6 m if the road doesn't say) | **road**, **verge** (2 m) |
| Other systems' scenery | anything camp life counts as scenery (Traces' scenes and boards) | **kept** |
| The site's own pieces | each piece's real footprint as it's laid out, one cell wider | **taken** |

A piece may stand where every cell under every part of its footprint (each rectangle grown by
0.2 m) is free of water, steep, bank, inside, solid, road, kept and taken; the spots kept clear (a
quartermaster, the medic, the barrier, a leader's seat, the front door's step, a counter's
customers) are still kept by distance. A quartermaster's counter indoors is fitted by its traces
instead (the grid's building cells don't apply to it). The rest of the fitting (flat enough, off the
line, the traces) is in [DESIGN.md](DESIGN.md#camp-life).

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
so `build.py`'s overlap check passes with the real sizes. The firewood piece's split logs are
`WoodKindling_01` (0.46 × 0.66 m), not the trunk.

## Drop, don't squeeze

A piece that doesn't fit right where its plan puts it is **dropped**, never nudged, pushed or moved
elsewhere (the plan as a whole is still shifted and turned to where most of it fits: DESIGN.md's
"The plan, laid down whole"). The plan's order decides what's dropped first: the most important
pieces come first and take their spots. A smaller site that sits right beats a full one that
clips. Every drop is recorded with why: the flag it met (water, a building, the road, a bank,
steep ground, something standing there, other scenery, another piece, outside the ground read),
not flat, the line, a spot kept clear, something in the way (the traces), or facing water or a
drop.

## The surroundings and the threat

Read once per site with its grid, from data the game already has, and kept as a small record
(`RZ_SiteSurroundings`). Nothing reads it to place pieces yet (the rules will); the audit uses it and
the log shows it.

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

## The audit

Every site laid out is measured once a run, as data, and logged; the runtime test lays out every
site on the map this way with nothing spawned (`RZ_CampLife.AuditAll`).

| Check | How | A hard failure |
|---|---|---|
| Overlaps | every placed rectangle against the other pieces' (exact areas, clipped), the buildings' boxes (not a counter: it may stand indoors) and the grid's solid cells; over 0.1 m² is an overlap | any |
| Floating or buried | the ground under each prop varying over 0.3 m | |
| Posts | the watch pieces (the nest) and the building's lookouts: does one see the gate (a 2 m line on the grid, no solid or building) within 80 m; each side with threat 3 or more (and the front) watched by a post facing within 67.5° | a stronghold with no gate watch |
| Paths | a walkable way (1 m wide, round water, buildings, solid, bushes, scenery and pieces) from the front door (the yard on open ground) to the gate and to the cook pot | |
| Dropped | each dropped piece by tier: 1 security (a counter, a watch, a barricade), 2 what the site is for (the roster and duty boards, the toll, the Haven's table and memorial, the sample table, the decon shower, the cold box), 3 life, 4 character, 5 clutter; read from the piece's name until pieces carry a tier of their own | a tier 1 piece dropped |
| Patrol | the patrol's four points (the first shift): dry, and no 4 m drop within 6 m | |
| Perimeter, outside the wire | not built yet: there's no wire line until the layers ([designs/SITES.md](../designs/SITES.md#3-building-in-layers-by-doctrine)) | |

The **score** (0–100) starts at 100: −15 an overlap (at most −45), −2 a floating prop (−20),
−25 for a stronghold with no gate watch (−10 elsewhere; road posts aren't asked), −5 a post that
can't see the gate (−10), −5 a threatened side unwatched (−15), −10 a blocked path, a dropped
piece −20, −6, −3, −1.5 or −0.5 by its tier 1 to 5, −4 a bad patrol point (−16).

Each site's line, in play too (`[ReforgerZ] Sites: ...`):

*"Sites: The Red Hand stronghold at 25-42: score 53; 13 of 15 pieces; perimeter not built yet; front N
(Task Force Bastion 1.1 km); dropped: SentryNest_RedHand (something standing there), RedHandSheet
(something standing there); 0 overlaps; 3 floating or buried; posts: gate watched, 2 posts (1 can't
see the gate), 1 of 2 threatened sides watched; paths: door to gate clear, door to kitchen blocked;
patrol 4 of 4 points walkable; read 44 ms in 1 steps (most 44 ms), laid out in 16 ms."*

and the summary (the runtime test asks for it once every site is done):
*"Sites audit: 16 sites, mean 42, worst 0 (Task Force Bastion stronghold at 44-107), 0 overlaps; 9 with
a hard failure; 13 under 70: ..."*. **Audit threshold** (70) is the pass mark: the site audit
(`run.sh sites`) fails while any stronghold scores under it.

**Where it stands** (the first runs, 2026-11-12, today's plans): 0 overlaps on Arland (9 sites, three
runs with different holdings) and Everon (16 sites); means 26–40 on Arland and 34–42 on Everon, every
stronghold under 70. The low scores are today's fixed plans meeting real ground: pieces dropped on
banks, steep ground and other buildings, no watch that sees the gate 40–50 m out at the road
entry, and paths blocked by the pieces. That's what the rules and the solver are for.

### The runtime test

`tools/runtime_test/run.sh sites [arland] [everon] [cain]` boots each map's own sub-scene (with the
map's AI world, so its road network, and camp life's kits copied from `RZ_GameMode.et`), waits for
the land grid, measures the props, audits every holding, safehouse and road post, prints the lines
and closes (Arland about 1.5 minutes, Everon about 6). It runs in its own network namespace where
it can, so beside a full test. The full runtime test runs the same audit alongside its route and
prints the summary; its world has no AI world, so no roads, so its scores aren't comparable. How to
run both: [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#runtime-test-without-workbench).

## Seeing it

| Tool | Does |
|---|---|
| `py tools/camp_kit/render.py --placed <console.log>` | draws `tools/camp_kit/sites_placed.html` from a site audit's log ("Sites draw" lines): each site top-down on the ground it read (water, buildings, walls and trees, bushes, roads, steep ground), its building, road entries, gate and door, every piece's real footprint by tier and every drop (dashed, hover for why), the posts and their lines to the gate, the patrol's loop, any overlap, and the threat on each side; and writes `tools/camp_kit/sites_list.txt` |
| **ReforgerZ: Fly to the next site** (Workbench plugin, World Editor) | reads `tools/camp_kit/sites_list.txt`: each run moves the camera above the next site looking down on it, then to its gate at eye height looking in, then on to the next; for a screenshot of every base to hold against `sites_placed.html` |

The committed `sites_placed.html` and `sites_list.txt` are Everon's audit of 2026-11-12 (16 sites; the
holdings move with each season's roll, so a new run may differ).

The audit's drawing lines also name the big solid things the grid read at each site (class, prefab,
size and how many), to tell a rock from a wall when a piece is dropped for "something standing there".

## Settings

On **Territory > Camp life** (`RZ_CampLifeSettings`):

| Setting | Default | What |
|---|---|---|
| **Site grid** | on | read each site's ground on the grid, its surroundings and its audit; off = the older point checks, nothing read or audited |
| **Site read ms** | 2 | milliseconds of server time a second for reading a site's grid (one site at a time); its pieces spawn once it's read and laid out |
| **Audit threshold** | 70 | the audit's pass mark |

The cell (0.5 m) is fixed: the grid's bands and checks are measured in it.

## What it costs

| Cost | Measured (dedicated server, 2026-11-12) |
|---|---|
| Reading a site's grid | 15–160 ms in all (a stronghold's 50–160 ms), read in small parts (a row of heights, a 24 m square's entity query, 16 entities, a building, a row of water or shore), the longest part 1–17 ms; at **Site read ms** 2 a stronghold's first pieces spawn about a minute after a player comes within 300 m (in the full runtime test a field station took 15 steps, 19 s) |
| Laying it out on the grid | 6–67 ms, in one go, once a run (later visits lay the kept layout at once) |
| Memory | a 72 m grid is 288 × 288 cells, two to an int (about 170 KB), with its heights and water marks every metre (about 170 KB), freed once laid out; the kept layout a few hundred bytes a site |
| Spawning, people | unchanged ([DESIGN.md](DESIGN.md#camp-life)) |

## Simplified

- The plans are still fixed drawings, re-spaced for the real sizes: the rules and the solver aren't built.
- Tiers come from the piece's name, not from `pieces.py`.
- Nothing reads the surroundings or the threat to place pieces yet; the window cells are read but
  unused; the perimeter and outside-the-wire checks wait for the wire line.
- A prop's footprint is its box seen from above, its pitch and roll ignored (a pallet stood on edge
  claims its flat size).
- Big rocks are ellipses inside their boxes, not their shapes; the traces catch what sticks out.
- The audit's paths and posts read the grid only: a path or a line of sight off the grid counts as clear.
- The full runtime test's world has no road network, so its audit has no road entries.
- Laying a site out (the plan's frames tried, every piece fitted, the audit) runs in one go, not
  spread over seconds as the reading is: 6–67 ms once per site a run.
- Neither test spawns the mod's own compositions (the server can't load them from the throwaway
  addon), so the drawing and the fly-to screenshots are how to check what stands.
