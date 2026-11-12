# Traces

The world's environmental storytelling. The Zone already remembers a lot as data: who held a town,
who died in a house, where a pack took someone. Traces makes some of that visible, near players
only, and never leaves anything behind. How player text is written is in [WRITING.md](WRITING.md);
the town log and the season marks Traces dresses are in [DESIGN.md](DESIGN.md#world-memory).

**Status.** The whole Traces design is built, simplified where marked: **door marks**
(the grammar of six authors, the first days seeded from canon, live writes from what happens, reading
by level and background, the town's summary, the journal's **Read on doors**, errands' old mark, the
thumbprint, the young come home, Bastion fading), **doors you can see** (decals drawn on each client,
or boards beside the door, or text only), **player traces** (P1 to P5), **scenes** (the first days
F1–F8 and the line N1–N2, vignettes V1–V8, live traces L1–L8), **sound and light** (S1–S8) and the
**handover to the epilogue**, with its rest: one placer (its ground reading shared with camp life), pieces that age, scenes
on their buildings, indoor scenes (behind a setting), camp life's own lamp, generator and loudspeaker,
the clue registry, the finer triggers and one kit with the epilogue. Every prop and sound is an optional kit prefab; with the kit empty every
trace still tells its story as text. Phase 0, the Workbench spike that settles decals, lights, loops
and wrecks, is the author's ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#traces-the-workbench-spike-phase-0)).
Errands' cards anchor on standing scenes (`RZ_Scenes.SceneNear`; Traces never deals an errand itself):
Boarded Up on V2, Is She Gone? near L1, His Effects on L5 or V7, and a stranger's fresh event at L1 or L2
([ERRANDS.md](ERRANDS.md#the-cards)). Nothing here has been play-tested.

Code: `World/Traces/` (`RZ_Traces.c` the owner, settings and kit, the epilogue's asks; `RZ_DoorMarks.c`
the grammar; `RZ_DoorSlots.c` doors on the map; `RZ_FirstDays.c` Days 1–10 and the new canon;
`RZ_TraceWriters.c` the hooks and the watch; `RZ_TownHistory.c`; `RZ_TraceText.c` every door line;
`RZ_DressPlacer.c` the placer camp life shares; `RZ_TraceClues.c` the clue registry;
`RZ_TraceDraw.c` decals' list and boards; `RZ_PlayerTraces.c`; `RZ_Scenes.c` and `RZ_SceneText.c` the
scenes and their lines; `RZ_TraceSounds.c` sound and light), and `UI/RZ_DoorMarkClient.c` (decals on
clients), `UI/RZ_TraceLightsClient.c` (lit windows on clients). Settings: **World > Traces**
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#5-settings-objects)).

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
- [Scenes](#scenes)
- [Sound and light](#sound-and-light)
- [The cure and the epilogue](#the-cure-and-the-epilogue)
- [Errands' old mark](#errands-old-mark)
- [The Long Table: absence](#the-long-table-absence)
- [Caps](#caps)
- [Admin, saving and the log](#admin-saving-and-the-log)
- [Decisions](#decisions)
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
| **The thumbprint** | The Red Hand | Someone the Haven put out sleeps here | Day |
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
| The Red Hand's | ★ Ex-Con, ★ Raider, ★ Police Officer, Known with the Red Hand | ★ Ex-Con: a struck hand, PAID on an empty house, and what the thumbprint is |
| Halcyon's | ★ Lab Technician, ★ Doctor, ★ Police Officer, Known with Halcyon | ★ Lab Technician: the series is the lab's own |
| The infected's | Everyone at a glance | ★ Hunter: how many, one tall, how old (they grey, they don't fade); at a ringed bear, *"One of them. Not tall. It stood here a long time."* and never who |
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
| `day2.queue` | The road point about 400 m back from `day2.checkpoint`, inward (the column, F1 and F2) |
| `day4.hill` | High ground 500–900 m from `day2.checkpoint`, off the line, terrain clear to the barrier; `none` when nothing qualifies (the hill, F8) |
| `traces.v6` | The waiting room's clinic (V6): a pharmacy or clinic in a town nobody holds, never the first patient's town; `none` when there's none |

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
| Red hands spreading (L7) | The watch, once a season day | Red Hand strength 50 or more: a hand on a shop or garage in a town within 1.5 km of a crew's holding or road post. Under 35: one hand struck through (nobody's collecting) |
| The young come home (L4, decision 6) | The watch, at dusk | 3 evenings in 10 a pack with named young: fresh scratches outside a ringed bear in the town one of them came from (its range within 1.5 km), never at a lived-in house and only with room beside the bear; a season day, never saved. Reading them makes the pack **Heard of**, the one thing reading a trace gives |
| An exile sleeps here | The watch | A player Hunted by the Haven (the door stays shut) who spends 20 minutes in a town nobody holds: 2 real hours later, if they aren't within 300 m, the thumbprint low on a house's frame there. One a season day an exile |
| The epilogue asks | The epilogue | [Below](#the-cure-and-the-epilogue) |
| Errands' old mark | Errands | [Below](#errands-old-mark) |

**Bastion fading** (with **Live traces** on): while Bastion's presence on the roads is under 0.25 (the
stage table's), no new Bastion mark goes up outside the towns it holds.

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

## Scenes

A scene is a record: a kind, a place, a made day, how long it lasts and a few fields. Its words always
come; its props, when the kit has them, come through the marks spawner's TRACES pool near players only.
At most 3 scenes have props within 300 m of each other; the rest are text.

### The first days and the line

Seeded at every start from the season's seed and the canon spots (made once a season), the same after
a restart, never saved (**World > Traces > First days' scenes**).

| # | Scene | Where | Its words (and the paper) | Reads more |
|---|---|---|---|---|
| F1 | **The column's tail**: 4–8 cars nose to tail on the verge, all facing the barrier | Back from `day2.queue`, 400 m in from the north barrier | *"Cars on the verge, all pointing the same way, all the doors open."* Collapse: one burnt out; Overrun: grass through the wheels. The dashboard sheet: *"NUMBER 31. PROCEED TO NORTH ROAD CHECKPOINT..."* | ★ Police Officer: the ticket numbers. ★ Hunter: old scratches on the last car |
| F2 | **Where they turned back**: suitcases across the verge | 360 m in from the barrier, outside the warning zone | Set down neatly; from Collapse, two opened | ★ Soldier: *"...the men behind it, in our badge."* |
| F3 | **The gym**: camp beds, blankets, GYM OPEN, the bus | By the gym's own door at `day2.gym` (the public building within 60 m; inside it with **Indoor scenes**), else beside the spot (no props if its town is lived in now: camp life owns it) | The bear with a patch on its belly; the list: eleven first names, ticked, *"All at the house. Lock up after you."* | ★ Elder: *"They all made it."* |
| F4 | **The Day 4 church**: a chair, a blanket, the board | By the church's own door at `day4.church` (within 60 m), else beside the spot | The board: *"CHURCH OPEN. ALL WELCOME. DECLARE YOUR BITE."* | ★ Nurse: whoever sat here watched somebody inside |
| F5 | **The cordon's Day 1 roadblock**: sandbags, the pole, a field radio | `day1.roadblock` | The log sheet, ending *"Leaving the pole down."* The radio hisses until Overrun (S3) | ★ Soldier, ★ Radio Operator |
| F6 | **The square's notices**, pasted over each other | One town in five nobody lives in, at its square's landmark (the monument, the bank, the station), else near its centre | The paper, newest on top: *"AID IS ON ITS WAY"*, the crews' diesel (3 km of the crossroads), Halcyon's flyer (4 km of the lab), GYM OPEN (5 km of the gym), the Day 2 order (*"TURNED BACK 1400"* within 4 km of the north road), the Day 1 order | ★ Police Officer: signed Lt H. Sedlak |
| F7 | **Rudi's shutter** | On the shutter of a garage within 60 m of `redhand.crossroads` (the kit's Red Hand door board flush on it); the shutter board beside the spot only where no garage stands | *"...The crews call it Rudi's."* Nothing more, ever | ★ Ex-Con: *"Nobody parks in front of it."* |
| F8 | **The hill over the north road** | `day4.hill` (none when no point qualifies) | *"On the post, cut with a knife: 05:41."* | ★ Soldier; Known with Bastion |
| N1 | **Crosses** of fence palings, up to 7 | Just outside the warning zone, 320–420 m in from a barrier | Two at the north road from the start (D4, D6); one more for each death in the kill zone within 1 km of a barrier. Saved | — |
| N2 | **The Day 1 sheets** in the hedges | 350–700 m in from every barrier (decision 3) | *"BY ORDER. THIS AREA IS UNDER QUARANTINE..."* | — |

The new canon keys they stand on are in [Where marks come from](#where-marks-come-from).

### Vignettes

Dealt where the data says something happened (**Vignettes**: 12 at once, 3 dealt a season day, one a
town). Once placed a vignette lives its days even if the town changes: it's history. Saved.

| # | Scene | Trigger | Where | Lasts |
|---|---|---|---|---|
| V1 | **The last meal**: a table laid for four | A town nobody holds goes dark (85) | A garden 6 m in front of a house's door (the town's middle if its doors aren't known) | 3 days |
| V2 | **Someone held out**: planks, casings, a tally of 19 on the door; the note on the step | Survivors Trapped or Collapse Shelter failed, or a town went infested with a player's barricade standing | Before the house nearest the job | 3 days |
| V3 | **The chair at the edge**, facing out | A bite declared at a safehouse (`house.bitten`), or Walk Him Out's *walk on* (its record `walk.out`: "walked on") | 150 m out from the house, toward the nearest pack | 2 days; its jars lit its first two evenings (S8) |
| V4 | **The honest shop**: IOUs on the door | A town nobody holds whose doors are known, with a shop, a Haven house within 2 km and no Red Hand holding within 1.5 km (tried once a town a run, about a third deal) | Before the shop's door | The season; when the crews take ground within 1.5 km, *"PAID"* across them in red, then 3 days |
| V5 | **The dropped run**: an empty sample case in the ditch; the notebook page | A Halcyon sample or escort job failed, or a Halcyon truck lost | The verge where it happened | 2 days |
| V6 | **The waiting room**: six chairs, *"DOCTOR BACK 1400"*, ticket 31 | Once a season (canon `traces.v6`) | A clinic in a town nobody holds, never the first patient's town | The season |
| V7 | **The roll at a fallen post**: the roster board | A Bastion outpost lost or taken | Where its centre was | 3 days |
| V8 | **The water queue**: buckets with names | A safehouse short of food with a pump within 400 m | Beside the pump | Until the house is fed, then 1 day |

### Live traces

Following the simulation as it runs (**Live traces**: 24 at once, the oldest goes). A player's deed
makes at most one of a kind a town a season day.

| # | Trace | Trigger | Where | Lasts |
|---|---|---|---|---|
| L1 | **After a kill site**: a candle chalked on a gatepost with who (the Haven), or *"CONTACT P-n · 1 LOST · Dn"* (Bastion, n its board number) | A tribes' kill site ends (its second dawn) within 3 km of a Haven house with people, else when its pack (the nearest within 3 km) is on Bastion's contact board; with nobody knowing, nothing | The verge by the site | 2 days |
| L2 | **Scorch** from the doorway, the Haven's *"WE CHECKED"* chalk when a Haven job was running there | A building burned (the fire system's report) | The doorway | 2 days |
| L3 | **Overpainted** | A holding taken or founded | The town's doors ([layers](#door-marks)) | As door marks |
| L4 | **The young come home** | [Above](#where-marks-come-from) | A ringed bear's door | A season day |
| L5 | **The truck**: a wreck on its side with the destination stencilled, a red hand on the door if robbed, gouges on the cab if a pack took it | A supply truck lost (robbed, destroyed or taken) | The verge where it was lost | 2 days; at the cure the last four stay for the week |
| L6 | **Bastion fading**: sandbags and a HALT sign on its side | A Bastion road post pulled; and no new Bastion marks under 0.25 presence | Where the post stood | 2 days |
| L7 | **Red hands spreading** | [Above](#where-marks-come-from) | Shops and garages first | As door marks |
| L8 | **Halcyon's beaks** | Halcyon's sample jobs ([above](#where-marks-come-from)), and a ringed beak by a nest under study: while Specimen from the Nest runs, once a job, on the door nearest the nest | As door marks | As door marks |

### How a reader meets a scene

No action, no button, no typing: the same with a pad.

- **The approach line**: within 40 m (on foot or in a car), once a session a scene, a low notice on the
  Ambient lane titled by the place (**On the verge**, **At the church**...), with the one more line the
  reader's background or standing gives (the ★ reads above).
- **The paper**: stand 2 s within 6 m on foot and it opens in the reader (**UI > Reader**), else a notice.
  Once a session.
- **The radio** (S3): within 12 m of the last meal or the Day 1 roadblock (until Overrun), the
  broadcast's current line as a subtitle, once a session; a Radio Operator gets one line more.

### Where scenes stand

Never in the warning zone or the kill zone, within 2 km of the Long Table's farm, within 300 m of the
closed site (`origin.site`) or within 60 m of a lived-in place (a camp measured from its yard out to
its farthest piece, and the road posts: [DESIGN.md](DESIGN.md#camp-life)). Vignettes and live traces also keep
300 m off a running job, 600 m off a lair (the chair and the kill site's chalk aside: they're about
packs), off the evening's set piece (`IsQuietAt`), and 1 km off an active questline clue of the same
faction (`World/Traces/RZ_TraceClues.c`: each running questline's place and its stage job's spot; the
Haven's V2, V3, V4 and V8, Halcyon's V5, Bastion's V7 and L6): *"Traces: <kind> keeps off a
questline's clue near <grid>"*. A point is dry, flat (under about 12°) and out of
every building's footprint (2 m); verge points are 6–10 m off the road's centre line, so nothing blocks
a truck (decision 11). When nothing fits: `Traces: no anchor for <kind> in <town>`, and nothing is dealt.

**One placer** (`World/Traces/RZ_DressPlacer.c`): the scenes' ground rules (dry, off the line, flat,
out of footprints by 2 m, 6 m off a road's centre line, apart from each other, clear of a quartermaster
or the medic), so a scene can be several pieces round its centre ([How scenes age](#how-scenes-age)).
Its reading of the buildings and roads near a place is camp life's too; camp life then fits each
piece's whole footprint by its own rules ([DESIGN.md](DESIGN.md#camp-life)).

**Indoor scenes** (decision 10; **World > Traces > Indoor scenes**, off until a play-test of the
outdoor ones): the last meal (V1) is laid inside its house and the gym's beds (F3) stand inside the
gym, on one of the loot's ground-floor spots, when the building has one; else they stay outside. An
indoor scene is its one composition.

### How scenes age

Worked out when shown; nothing ticks. The first days by the outbreak's stage (Early, Collapse, Overrun,
Aftermath: the column, the suitcases and the radio change their words); vignettes and live traces by
thirds of their life (fresh, weathered, old: the last meal's jug knocked over, then its chairs
fallen); a season-long one by its days. A scene whose stage changes is shown again.

**Pieces that age** (**Kit > Pieces that age**, each entry a scene's kind, a piece, an optional faded
twin, and when it drops): a scene's extra pieces stand 2–6 m round its centre by the shared placer (the
same spots after a restart, 9 pieces at most), each gone from its age on (weathered: a dealt scene's
middle third, the first days from Collapse; old: the last third, from Overrun), and shown as its faded
twin when old. An entry with no piece and a faded twin swaps the scene's own piece (a board's fresh and
faded looks) once it's old.

## Sound and light

**World > Traces > Sound and light** (on). Each needs its kit prefab; the lines come without.

| # | What | Where and when | Text |
|---|---|---|---|
| S1 | **Lanterns where people live**: a lit lantern (light on spawn) | The 4 lived-in places (holdings, safehouses with people) nearest players within 200 m, 20:30–05:00, from 18:30 in rain or fog: exactly where camp life's own table lamp stands (its lit twin over it), else near the place's middle. Never in a town nobody holds | — |
| S2 | **Lit windows**: a small warm light 1.5 m inside the outside doors of lived-in buildings, drawn on each client | **Lit windows** (off until the spike shows it at the windows, not through the walls; decision 9). As S1, 12 at most | — |
| S3 | **A radio in an empty house**: the hiss loops | The last meal and the Day 1 roadblock (until Overrun), 2 at most | The broadcast's line, [above](#how-a-reader-meets-a-scene) |
| S4 | **Dogs, then nothing**: a few barks that stop dead | 5 real minutes before a pack's set-out hour, at the town nearest its lair (within 1.5 km), one a pack a night, players within 800 m | ★ Hunter: *"The dogs in Levie have stopped."* |
| S5 | **A generator's hum** | The lab, and Halcyon's outposts on camp life's own generator piece when the station has one; after dark the kit's quieter night hum (**Generator hum, night**) when it's set | — |
| S6 | **One stroke of the bell** (decision 8) | At the Day 4 church, the next 18:30 after a Haven Champion dies (P3b); 10 heat there; never at night | *"One stroke of a church bell, from the direction of ..."* to everyone within 1.2 km |
| S7 | **The town loudspeaker**: a chime and static | Bastion's holdings, 05:30 and 17:30, within 300 m, from camp life's loudspeaker pole (`RZ_Dress_Loudspeaker` in Bastion's kit) when the holding has one, else its middle; silent after the cure | A subtitle: presence 0.25 or more *"Remain indoors. Relief is on its way."*, under it *"Remain indoors."* ★ Soldier: one line more |
| S8 | **Jars on the step** (the kit's candle jars) | The Day 4 church on the bell's evening, and a walk-out's chair its first two evenings, dusk to 22:00 | — |

Lights: 12 on the server within 200 m of a player. Loops (`RZ_AmbientLoopComponent` on the prefab): 4,
up within 80 m, down past 120 m. One-shots (`RZ_CallSoundComponent`): 4 at once, gone after 40 s. Every
prop goes through the dressing guard, which lets these two look-only components through. The tribes
keep their own calls ([INFECTED.md](INFECTED.md#signs-and-sounds)); Traces adds no infected sound.

## The cure and the epilogue

[EPILOGUE.md](EPILOGUE.md) owns the week; Traces owns how its marks look.

- **The handover**, once, when the cure is out: Traces' in-season graves come down (the epilogue places
  its own from the record), the last four lost-truck wrecks and every fallen post's roll (V7) still
  standing stay for the week (where the epilogue's outpost boards are), and nothing new is dealt or
  written but what the epilogue asks for. Running vignettes and live traces finish their
  days; the first days stay. Log: `Traces: handed over to the epilogue (...)`.
- **One kit:** an epilogue mark whose own slot is empty takes Traces' piece for it: a grave, a truck
  wreck, the sheet on posts, the stencilled board, tape between posts, a board, the candle jars
  (`RZ_Traces.EpilogueKit`).
- **The census**: each of the epilogue's town beats ends with its doors counted, at a glance:
  *"Four candles on the doors in Brod, and one bear."* (candles by their number, bears ringed or not,
  hands and tallies, beaks, gouged doors; painted-over layers don't count).
- **Writes on request**, through the grammar and its door caps, after the cure and past the daily caps:
  a wanderer's name chalked as a candle on the last door they rested at (their safehouse), and *"GONE
  HOME: n"* by a safehouse's door in processing (written only when the number changes).

## Errands' old mark

Errands' found hook *"an old mark"* (decision 15, [ERRANDS.md](ERRANDS.md#strangers)) comes from the
grammar: a marked door within 60 m of the job's site if there is one; else a mark a few days old, written
for two hours (never saved) on the nearest door, by whoever holds the nearest ground within 3 km (Bastion
CLEARED, the Red Hand's struck hand, Halcyon's beak, else the Haven's ring and dot). Whoever passes within
30 m reads it once, at their own level, and it goes in their journal. With no town near, or Traces off,
no old mark is made at that site.

## The Long Table: absence

No marks within 2 km of the Long Table's farm, but residents' words from Days 1–3. Bastion's sweep never
came within 1.5 km. No graves or candles for deaths within 2 km of it, and no scenes, vignettes or live
traces (decision 5: absence only). A player who walks every village notices one stretch of country with
no writing on any door. Nothing says why.

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
| Scenes | The first days and the line about 10 plus one town in five's notices, not saved; vignettes 12 at once, 3 dealt a season day, one a town; live traces 24 |
| Scene props | The TRACES pool (40 on the server, 12 a cluster, shared with the player traces); 3 scenes with props within 300 m |
| Lights, loops, one-shots | 12 lights within 200 m of a player; 4 loops within 80 m; 4 one-shots at once |
| Lookups | One town's doors a tick; reading checks only players inside a town, against its marked doors, once a second; scenes once a second within 40 m |

## Admin, saving and the log

- **`/traces`** (server admins): every marked door and every scene within 300 m (its kind, stage and
  whether it has props) to the server log, and the status line as a notice. **`/traces town <name>`**: the
  town's history and doors. **`/traces write <symbol>`** (with **Debug** on): a mark at the nearest door for
  an hour. **`/traces clear <town>`**: the town's marks and history (first-days marks come back at the next
  start). A pad player's admin needs no chat command for play: these are play-test aids only.
- **Saved** in `state.json`: the town history, live door marks with their slot, the rain counter, the
  player traces, the vignettes, live traces and crosses, the day's deal count and whether the cure's
  handover is done. Not saved: first-days marks and scenes, door slots, the drawn list, props, lights,
  sounds, the young come home's scratches, the exiles' minutes, what each player has read.
- **Log lines**, the status line and the play-test checks are in
  [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check).

## Decisions

The author approved all 16 as recommended (2026-11-11).

| # | Decision | Decided |
|---|---|---|
| 1 | How door marks look | Text first, spike decals, then decals; boards only if the spike fails |
| 2 | Where the design lives | Its own doc, this one |
| 3 | Where the Day 1 sheets near the line came from | Bastion's sheets with Command's words: the outside stays silent but for the broadcast |
| 4 | Candles where a town was rescued | One meaning a symbol: the candle is the dead; a rescue is ticked bears, arrows and the sheet |
| 5 | The Long Table | Absence only |
| 6 | Scratches at a ringed bear | Yes: only outside, only at a ringed bear, never at a lived-in house; the Hunter never says who |
| 7 | Names on in-season graves and candles | Follow Your dead run (described by default) |
| 8 | The bell at the Day 4 church | One stroke, 10 heat, at 18:30, never at night |
| 9 | Lit windows and the map's own lamps | Lanterns first; lit windows behind a setting after the spike; the map's lamps left alone |
| 10 | Scenes indoors | Outdoor and doorway by default; indoor later, behind a setting, after a play-test |
| 11 | The column's cars | On the verge, 6 m off the centre line |
| 12 | What the radio in an empty house plays | The hiss and the broadcast's line, never music |
| 13 | First-days places each season | Re-seeded each season |
| 14 | Does reading pay | Nothing; only a pack Heard of from L4 |
| 15 | Errands' old mark | Through the grammar |
| 16 | The Red Hand and the bears | They leave the bears |

## Simplified and not built

| What | As built |
|---|---|
| Who sees a town's summary and a door line | A character on foot within the town's radius (+30 m); standing 2 s means two one-second checks within 5 m, not a speed test |
| The first days' sweep | "Within 3 km of the roads into `origin.site`" is within 3 km of the line from the site to the roadblock; at most 12 towns nearest first |
| P3's notable death | A life of 2 hours or more, or dying at a running job's site; the stand and a pack's kill aren't told apart. The Digger stranger's grave, once you've dug it with her, stays by the road with whoever she was told it was (or SOMEONE) |
| The thumbprint | Exiles are players Hunted by the Haven; their minutes in a town, and a thumbprint that's due, are saved across a restart. The Long Table never makes a player its exile, so it has none to count |
| Scene placement | A scene's own point by Traces' ground test, its extra pieces by the shared placer; with no **Pieces that age** set, each scene is its one composition and ages in its words. A prop never comes up while a player or a vehicle is within 8 m of its spot |
| The column (F1) | 4–8 cars along the road from the queue's point (400 m back from the barrier) to 1.2 km back, never within 25 m of a junction (three road ends meeting); where the road can't be followed, the rest 9 m apart back from the queue's point |
| The gym, the church, the shutter (F3, F4, F7) | On the building's own door when one stands within 60 m; F7's red hand is the kit's Red Hand door board flush on the garage's shutter, not a decal layer (the door marks are a town's, and the crossroads lies outside one). F4's jars are only the lit ones (S8) |
| The square's notices (F6) | One town in five nobody lives in, at its monument, bank or station when one is near the centre (no square as such) |
| Vignettes' triggers | V5 from failed Halcyon jobs and trucks, and a pack taking a runner as data within 3 km of the lab or a Halcyon field station; V4 tried once a town a run; "not the same player's trigger twice a day" is one a town |
| L1 after a kill site | In a town (within 60 m of it), the Haven's candle goes on the nearest door; elsewhere its chalk is a scene on the verge by the site. The site's pack is the nearest within 3 km |
| L3, L8 | The door writes built in phase 1 (overpaint on a take, beaks from sample jobs); a nest under study is a running Specimen from the Nest |
| L6 | From the road posts that go between two looks (30 s), not a saved list of the last six |
| S1, S5, S7 | The lit lantern stands over camp life's own lamp (the unlit one stays under it); quieter at night is a second hum prefab; the loudspeaker pole is a composition the PC builds (`RZ_Dress_Loudspeaker`) |
| S4 | Before the pack's set-out hour on a night it will set out (not weakened, holding a vigil or sleeping off a feed), at the town the night's first trail passes (else the town nearest the lair); the Hunter's line comes with the barking |
| The handover | Fallen posts (V7) stand for the week near the epilogue's outpost boards rather than becoming them; Traces' kit only fills an epilogue slot that's empty |
