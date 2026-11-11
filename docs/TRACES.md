# Traces

The world's environmental storytelling. The Zone already remembers a lot as data: who held a town,
who died in a house, where a pack took someone. Traces makes some of that visible, near players
only, and never leaves anything behind. How player text is written is in [WRITING.md](WRITING.md);
the town log and the season marks Traces dresses are in [DESIGN.md](DESIGN.md#world-memory).

**Status.** Phases 1, 2 and 5 of the [Traces design](../designs/TRACES.md) are built: **door marks**
(the grammar of six authors, the first days seeded from canon, live writes from what happens, reading
by level and background, the town's summary, the journal's **Read on doors**, errands' old mark),
**doors you can see** (decals drawn on each client, or boards beside the door, or text only) and
**player traces** (P1 to P5). Phase 0, the Workbench spike that settles decals, is the author's
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#traces-the-workbench-spike-phase-0)). Planned: the first
days' scenes and the line, vignettes and live traces, sound and light, and the epilogue's handover
([designs/TRACES.md](../designs/TRACES.md)). Nothing here has been play-tested.

Code: `World/Traces/` (`RZ_Traces.c` the owner, settings and kit; `RZ_DoorMarks.c` the grammar;
`RZ_DoorSlots.c` doors on the map; `RZ_FirstDays.c` Days 1–10 and the new canon; `RZ_TraceWriters.c`
the hooks and the watch; `RZ_TownHistory.c`; `RZ_TraceText.c` every line; `RZ_TraceDraw.c` decals' list
and boards; `RZ_PlayerTraces.c`), and `UI/RZ_DoorMarkClient.c` (decals on clients). Settings:
**World > Traces** ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#5-settings-objects)).

## Contents

- [Principles](#principles)
- [Door marks](#door-marks)
- [The symbols](#the-symbols)
- [Reading a door](#reading-a-door)
- [Where marks come from](#where-marks-come-from)
- [How marks age](#how-marks-age)
- [Doors on the map](#doors-on-the-map)
- [Doors you can see](#doors-you-can-see)
- [Player traces](#player-traces)
- [Errands' old mark](#errands-old-mark)
- [The Long Table: absence](#the-long-table-absence)
- [Caps](#caps)
- [Admin, saving and the log](#admin-saving-and-the-log)
- [Simplified and not built](#simplified-and-not-built)

## Principles

| Rule | How |
|---|---|
| Data first, props when set, text always | Every trace is a record on the server; its look is optional; its words always come |
| Generated from real history | Every mark comes from a fact a built system holds: the season's seed and canon, a mission's end, a safehouse's dead, a holding changing hands, a town's infection, a player's death |
| Dated, and allowed to be stale | Every written mark carries its day ("D17" on the mark, "Day 17" in prose). In a town nobody holds, marks are history and can be wrong now |
| Show, don't explain | No tutorial. Players learn the symbols by seeing them in the same situations; the backgrounds that would know say more |
| Nothing usable, nothing pays | No mark holds loot, blocks a door or gives a job. Reading pays no Fame, Favor or items (decision 14). Players never write a mark |
| Sparse | At most 6 marked doors a town (8 in a stronghold's town); most towns have none to three |

## Door marks

A door mark is a symbol painted, chalked, stencilled or scratched beside a building's outside door,
with a few written fields. A door carries up to three **layers**, oldest underneath. The look is a
decal or a board when set up; the meaning is always text.

**Who writes.** The season starts on Day 11 (the broadcast's count). Days 1–10 are history, seeded once
a season from its seed and the canon, the same after every restart and never saved. From Day 11 on,
marks are written live, saved, and written only by the server from data.

| Days | Who | What they wrote | Where |
|---|---|---|---|
| 1 | The cordon detail (Bastion, section HQ) | The search X | Towns within 3 km of the road from the closed site (`origin.site`) to the cordon's roadblock (`day1.roadblock`) |
| 1–4 | Residents | Their words ([below](#residents-words)) | 1–2 doors in a third of towns |
| 2–5 | Bastion's sections | The X: the living to the north road (Day 2 before 14:00), to the gym (after 14:00 and Day 3), to the church (Day 4–5); a slash about 1 X in 20 from Day 3; a ring (Day 5) on some X's that found a bitten person or a dead one | Towns within 4 km of `day2.checkpoint`. At most 12 swept towns, 2–5 X's each. No X after Day 5 |
| 2 onward | The Haven | The bear on the gym's doors (Day 2); bears round the gym and the church, half of them ringed with an arrow to a house; at every safehouse the patched bear (opened Day 3–9), plain bears (houses at Community or bigger at the season's start), sometimes a candle (from Day 5), hatching (from Day 6), and the drop at a well within 300 m | Round `day2.gym`, `day4.church` and every safehouse |
| 4 onward | The Red Hand | Tallies (from Day 4), hands (from Day 9) | Towns within 1.5 km of `redhand.crossroads`, and towns the crews held at the season's start |
| 7 onward | Bastion | The post code, ATT, C on a farm | Towns Bastion held at the season's start |
| 7 onward | Halcyon | The beak and its code | Towns within 3 km of the lab |
| 3 onward | The infected | Scratches outside; smears on half of them | Towns nobody held at 60+ infection at the season's start |

**Layers and overpainting.**

| Rule | |
|---|---|
| Three layers a door | A fourth pushes off the most weathered (painted-over layers first) |
| The Red Hand paints over Bastion | A red hand on a door paints over Bastion's marks there, and over Halcyon's in a town the crews hold. What's underneath still reads, as "Under the paint, still readable: ..." |
| Bastion paints grey over the Red Hand | A post code or CLEARED paints the crews' marks grey first |
| The Haven never paints over | It chalks beside |
| Nobody paints over residents' words or a bear | They weather. The crews leave the bears alone (decision 16) |
| Rewrites | The same mark by the same writer on the same door (a candle's number, a hand's strokes, the drop, a bear ringed) updates that layer and resets its age |

## The symbols

| Symbol | Author | Means | Fields |
|---|---|---|---|
| **The X** | Bastion | Searched | Day and time, section (1A, 1B, 2A, 2B, 3C, HQ), hazard (INF, BIT, SEAL, NE, DOG or –), found (*"3L→NR"*, *"1L→GYM 1D"*, *"0"*) |
| **The slash** | Bastion | Went in, never marked it done | Day, section |
| **The ringed X** | Bastion | Checked again on Day 5 | As the X |
| **The post code** | Bastion | A post's street: *"B-2 · D14"* | Post, day |
| **C** | Bastion | Contribution taken: *"C · D9"* | Day |
| **ATT** | Bastion | Attached civilians: *"ATT · DO NOT REQUISITION"* | — |
| **CLEARED** | Bastion | Culled, shelled or retaken: *"CLEARED · 0400 · D27"* | Time, day |
| **The bear** | The Haven | Children inside; **ringed**: moved on, safe, with an arrow; **ticked**: everyone got out | Day, the town the arrow points to |
| **The patched bear** | The Haven | A Haven house; ringed with an arrow when it emptied | Day it opened |
| **The candle** | The Haven | Someone from this house was lost (one meaning only, decision 4) | The number, names under it (the Elder reads them); a player's description shows to all |
| **Hatching** | The Haven | A walk-out (the Haven never writes the words) | Day |
| **Ring and dot** | The Haven | Checked: nobody here | Day |
| **Arrow and letter** | The Haven | Gone to the house at... (*"→ M"*) | The town |
| **The drop** | The Haven | The pump here works | Day |
| **The red hand** | The Red Hand | Paid: under protection; strokes for weeks paid. **Struck**: lapsed | Strokes, day |
| **The tally** | The Red Hand | Tins owed. **PAID** across it: cleared | Strokes, day |
| **The beak** | Halcyon | Sampled: *"S-031 · d17 · 3"*. **Ringed**: please don't burn. **Barred**: finished | Series, day, samples |
| **Scratches outside** | The infected | They wanted in | How many, one tall, age |
| **Scratches inside** | The infected | Someone turned in that room | Age |
| **Smears** | The infected | The slow ones lay against it | Age |
| **Residents' words** | Residents | Whatever they needed to say | The line |

**SEAL is true.** The X's hazard reads *"SEAL"* exactly on buildings the lock system locked, so a Police
Officer reading the code knows the door before trying it. It never says what's inside.

### Residents' words

Picked by the town and its seed, so the same after a restart.

| Line | Day |
|---|---|
| In lipstick on the glass: *"GONE TO MUM'S IN {town}. D1"* | 1 |
| *"WE'RE AT THE GYM. D2"* | 2, within 5 km of the gym |
| *"DON'T COME IN. TELL ANNA I'M SORRY. D2"* | 2; this door may later get scratches inside |
| *"BACK SOON. KEY WHERE IT WAS"* | 1–3 |
| *"THE RADIO SAYS STAY INDOORS. WE ARE INDOORS."* | 1 |
| *"CHURCH OPEN. ASK FOR THE FATHER. D4"* | 4, within 4 km of the church |

## Reading a door

Three levels. **Glance**: what anyone sees. **Read**: what the writer meant, for someone who knows that
writer. **Know**: one more true detail only training or a life gives. Mixed reputation reads as the
access rank does: **Known** or better counts ([REPUTATION.md](REPUTATION.md)).

| Marks | Read | Know |
|---|---|---|
| Bastion's | ★ Police Officer, ★ Soldier, Known with Bastion | ★ Police Officer, ★ Soldier: the ring, the slash, which sweep, what C and ATT mean |
| The Haven's | ★ Nurse, ★ Elder, ★ Police Officer, Known with the Haven | ★ Elder: the names under a candle. ★ Nurse: what the hatching means |
| The Red Hand's | ★ Ex-Con, ★ Raider, ★ Police Officer, Known with the Red Hand | ★ Ex-Con: a struck hand, and PAID on an empty house |
| Halcyon's | ★ Lab Technician, ★ Doctor, ★ Police Officer, Known with Halcyon | ★ Lab Technician: the series is the lab's own |
| The infected's | Everyone at a glance | ★ Hunter: how many, one tall, how old (they grey, they don't fade) |
| Residents' words | Everyone | ★ Police Officer: whether the day fits the rest of the door |
| Any written mark's age | | ★ Hunter: *"The chalk's a day old."* |

**How a reader meets a mark.** No action, no button, no typing: it works the same on a gamepad.

- **The town.** The first time this session a character enters a town with two or more marked doors,
  one glance-level summary, the three biggest things only, on the Ambient lane (a low notice):
  *"Doors in Levie: three bears, two of them ringed. A red hand, one on the shop."*
- **The door.** Stand within 5 m of a marked door for 2 seconds (on foot) and its line comes, at your
  level, newest layer first, once per door per session (a low notice titled **On a door**).
- **Three layers** open in the reader instead (one paragraph a layer) with **UI > Reader** on.
- **The journal.** The last 12 door lines read, with the town and the day, under **Read on doors** in the
  notebook's Journal tab ([UI.md](UI.md#the-notebook)). Lost with the life.

## Where marks come from

**The first days** (`RZ_FirstDays`), at every start, from the season seed and canon made once a season
and saved with the world facts: the season's start (`traces.start.*`: the towns at 60+ infection, the
towns Bastion and the crews held, the bigger safehouses), so a restart seeds the same marks whatever has
changed since; and these spots:

| Key | Rule |
|---|---|
| `day2.gym`, `day2.gym.town` | The school within 3 km of the Haven's stronghold house, preferring one between it and `day2.checkpoint`; else the middle of the nearest town to the north road nobody holds |
| `day4.church`, `day4.church.town` | A church in another town from the gym's, within 4 km of it, in a town nobody holds; else any church within 6 km; else the gym's town's middle |
| `day1.roadblock` | A road point 1.5–3 km from `origin.site` toward the lab, off the line |
| `day2.queue` | The road point about 400 m back from `day2.checkpoint`, inward (for the column's scenes, planned) |
| `day4.hill` | High ground 500–900 m from `day2.checkpoint`, off the line, terrain clear to the barrier; `none` when nothing qualifies (planned scene) |

**Live writes** (`RZ_TraceWriters`, from Day 11). Each event goes into the town's history (8 events a
town, saved) and may write one door. After the cure nothing new is written.

| Event | Who tells Traces | Door write |
|---|---|---|
| A safehouse empties | The world memory's writer | Its patched bear ringed, an arrow to the nearest house with people; one more bear in town ringed |
| A safehouse's dead go up | The watch (30 s) | A candle on a house near it, or its number goes up; a seeded first name for each (the Elder reads them) |
| Walk Him Out done | A mission's end | Hatching and the day |
| People brought out alive: Survivors Trapped, Collapse Shelter, Rescue the Kids, Fetch the Family | A mission's end | The bear ticked on the house nearest the job, an arrow to the nearest house; ring and dot on another (P1) |
| The Red Hand founds or takes an outpost | The watch | Hands on the shop and a house, over Bastion's stencils |
| Protection Racket, A Quiet Arrangement, Rent's Due | A mission's end | A hand on the shop, one more stroke; under `haven.rule` 4 in a safehouse's town, a hand on the safehouse's door |
| Debt Collection, A Debt Gone Quiet | A mission's end | PAID across a tally (Debt Collection, where a tally is owed), else a struck hand |
| Bastion founds or takes an outpost | The watch | The post code on the shop and a house, grey paint over the hands |
| Requisition, Off-Book Requisition | A mission's end | *"C · D{day}"* on a farm or barn |
| Bastion Shelling, Fire Mission, Retake / Take Back a town, Cull | A mission's end | *"CLEARED · {time} · D{day}"* near the job |
| Field Samples, Night Readings, Live Capture, Specimen from the Nest | A mission's end | The beak and its code; ringed for Specimen from the Nest |
| A town turns infested | The world memory's writer | Scratches outside on a door, smears at 65 (once) |
| A town nobody holds goes dark (85) | The watch | Scratches inside one door that has residents' words, no bear, and isn't the gym line |
| A player's death | The world memory's writer | A candle with the description ([P4](#player-traces)) |
| Errands' old mark | Errands | [Below](#errands-old-mark) |

A write asks for a kind of building (a shop, a clinic, a farm or barn, a garage, a house, or the one
nearest a point). It goes on the door that already has the same mark by the same writer (a rewrite),
else a door of the same kind near the same point, else one of the same kind with marks it paints over
(the crews' hand on the shop's X), else a new door; a full town gives up its least salient, most weathered door (a first-days write never pushes out a
door with live marks).

## How marks age

Every mark carries its day; its age is worked out when it's read. Nothing ticks.

| Stage | Paint and stencils | Haven chalk | What changes |
|---|---|---|---|
| Fresh | Under 1 season day | Under 1 | Crisp; the Hunter says *"today"* |
| Weathered | 1–5 | 1–3 | *"Rained on"* (chalk); the decal's alpha drops |
| Old | Over 5 | Over 3 | *"Faded"*, *"Washed almost to nothing"* |
| Gone | Never | At 8 season days with no rewrite (live chalk only) | Removed |

Rain (0.3 or more) ages chalk twice as fast while it lasts. Scratches don't fade, they grey (the
Hunter's read). First-days marks are history: they get old, never gone.

## Doors on the map

A town's doors are resolved once a run, when a player first comes within 400 m of its edge, one town a
tick: one sphere query (at most 64 buildings), each building's first outside door (`RZ_DoorGeometry`),
one slot each, sorted by a hash of the building's position and the season seed, so a slot number is the
same door after every restart and nothing about a building is saved. The building's kind comes from
its prefab name. The mark sits on the door's wall 0.8 m beside the doorway, 1.5 m up: never on the leaf
or in its swing.

No new mark goes on a building inside a lived-in place's ring (50 m of a holding, 80 m of the lab, 40 m
of a safehouse except the building it's in), within 25 m of a running mission, or on the first patient's
clinic (`origin.clinic`'s town's clinics carry nothing); they stay in the list, so slot numbers never
shift. A live mark keeps its slot across restarts. A marked door that finds no slot is words only: it
counts in the town's summary.

## Doors you can see

**World > Traces > Door drawing** (decision 1):

| Mode | How | Needs | Without it |
|---|---|---|---|
| **Decals** (default) | The server keeps a replicated list of the marked doors within 80 m of any player (48 at most, sent only when it changes, every 5 s). Each client draws a decal per layer on the building's wall, from the door atlas (16 symbols) and the overlays (ring, tick, strike, PAID, the infected's three, the thumbprint, grey overpaint), within 70 m of itself, the age as the alpha | **Kit > Door marks atlas** (and **Door marks overlays**) | Boards |
| **Boards** | A board leaned beside each marked door, facing out, by the top layer's author, through the marks spawner's DOORS pool (24 on the server, 8 a cluster, up within 60 m, down past 90 m, 10 minutes up at least) | **Kit > Door board** for the author | That door is text only |
| **Text** | Nothing to see | — | — |

Never both. Every board goes through the dressing guard and counts as scenery. The decal calls are on the
[VERIFY list](WORKBENCH_SETUP.md#6-the-verify-list); [phase 0](WORKBENCH_SETUP.md#traces-the-workbench-spike-phase-0)
settles them.

## Player traces

**World > Traces > Player traces** (on).

| # | Trace | Trigger | Where | Lasts |
|---|---|---|---|---|
| P1 | **A town you emptied**: a ticked bear with an arrow, ring and dot | A rescue job done ([above](#where-marks-come-from)) | The houses nearest the job | As door marks |
| P2 | **Ash**: *"A black patch in the grass, and a count on the post beside it: 9. D27."* (the count seeded by the job) | Burn the Bodies, Burn the Nest or Clear the Trail done | The job's spot | 2 season days |
| P3 | **A grave**: *"Two fence palings and a mound, and a name in biro: VERA. MECHANIC. Day 23."*, or the description | A notable death (a life of 2 hours or more, or a death at a running job's site) with someone to dig: a Haven house with people within 3 km | Where it happened, if 100 m from another grave and 150 m from a safehouse; else the verge of the nearest road 150–300 m on | 2 season days; one per identity, 8 on the server |
| P3b | **The memorial candles**: *"Candles in jars on the step. The house read a name at supper."* | A Haven Champion dies (Remembered) | Every safehouse with people | 1 season day |
| P4 | **A candle near where you died**: chalk on the nearest door, the description small underneath, read by all | A notable death with nobody to dig, in a town | The nearest door within the town | As door marks |
| P5 | **The town log made physical**: the built season mark with a look by its kind | The town log (salience 3+, [DESIGN.md](DESIGN.md#world-memory)) | As built | As built |

**P5's looks.** The words open with what you see, then the built line: rescue *"A sheet on two posts by
the road, in marker: ..."*; burned *"A board by the road, the words burned into it with a hot nail:
..."*; taken *"A stencilled board by the road: ..."*; emptied *"Flowers tied to a post by the road, and a
card: ..."*; other jobs *"A board by the road, painted by hand: ..."*. The prop is the kit's one for that
kind, else **Shared > Season mark prefab**.

**Names** (decision 7) follow **Zombies > Tribes > Your dead run** ([INFECTED.md](INFECTED.md#your-dead-run)):
the description (*"someone who'd been a mechanic"*) by default, the name only on **Named**, with the
player's opt-out and the name filter. P4's candle always uses the description. Nothing names whoever
caused a death.

Graves, ash and candles go through the marks spawner's TRACES pool (**Props on the server** 40, **Props a
cluster** 12): the kit's prefab near players, through the dressing guard; without one, the trace is its
approach line. Graves and ash are saved and come back after a restart.

## Errands' old mark

Errands' found hook *"an old mark"* (decision 15, [ERRANDS.md](ERRANDS.md#strangers)) comes from the
grammar: a marked door within 60 m of the job's site if there is one; else a mark a few days old, written
for two hours (never saved) on the nearest door, by whoever holds the nearest ground within 3 km (Bastion
CLEARED, the Red Hand's struck hand, Halcyon's beak, else the Haven's ring and dot). Whoever passes within
30 m reads it once, at their own level, and it goes in their journal. With no town near, or Traces off,
no old mark is made at that site.

## The Long Table: absence

No marks within 2 km of the Long Table's farm, but residents' words from Days 1–3. Bastion's sweep never
came within 1.5 km. No graves or candles for deaths within 2 km of it (decision 5: absence only).

## Caps

| What | Cap |
|---|---|
| Town history | 8 events a town, saved |
| Marked doors | 6 a town (**Doors a town**), 8 in a stronghold's town |
| Layers | 3 a door (**Layers a door**) |
| Live writes | 2 a town and 20 on the server a season day (**Live writes a season day**); first-days marks and old marks don't count |
| Live layers | 600 on the server, saved; the oldest goes past it |
| Drawn list | 48 doors within 80 m of players |
| Player traces | 16, 8 of them graves |
| Lookups | One town's doors a tick; reading checks only players inside a town, against its marked doors, once a second |

## Admin, saving and the log

- **`/traces`** (server admins): every marked door within 300 m with its layers and their source (first
  days, live, old mark) to the server log, and the status line as a notice. **`/traces town <name>`**: the
  town's history and doors. **`/traces write <symbol>`** (with **Debug** on): a mark at the nearest door for
  an hour. **`/traces clear <town>`**: the town's marks and history (first-days marks come back at the next
  start). A pad player's admin needs no chat command for play: these are play-test aids only.
- **Saved** in `state.json`: the town history, live door marks with their slot, the rain counter and the
  player traces. Not saved: first-days marks, door slots, the drawn list, props, what each player has read.
- **Log lines**, the status line and the play-test checks are in
  [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check).

## Simplified and not built

| What | As built |
|---|---|
| Who sees a town's summary and a door line | A character on foot within the town's radius (+30 m); standing 2 s means two one-second checks within 5 m, not a speed test |
| The first days' sweep | "Within 3 km of the roads into `origin.site`" is within 3 km of the line from the site to the roadblock; at most 12 towns nearest first |
| P3's notable death | A life of 2 hours or more, or dying at a running job's site; the stand and a pack's kill aren't told apart. The Digger stranger doesn't dig graves yet |
| The thumbprint | Not written: no system keeps who the Haven turned out (wanderers have no exile record). It has its overlay cell and words for when one does |
| L4, the young come home (scratches at a ringed bear) | Planned with the live traces ([design](../designs/TRACES.md#live-traces)) |
| Bastion fading (no new Bastion marks under presence 0.25) | Planned with L6 |
| Writes on request for the epilogue, the census | Planned (phase 7) |
