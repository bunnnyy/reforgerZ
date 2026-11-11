# Environmental storytelling: the world's traces

**Status: designed, not built.** Ideas approved in principle by the author; this is the full design to read
before anything is built. Working name for the system and its doc: **Traces** (`docs/TRACES.md` when it
ships, code in `scripts/Game/ReforgerZ/World/Traces/`). `RZ_Environment` is already the weather, so the
classes are `RZ_Traces...`. Links are written as if this file sits in `designs/`.

## Contents

- [In short](#in-short)
- [Principles](#principles)
- [One owner per thing](#one-owner-per-thing)
- [The door-mark grammar](#the-door-mark-grammar)
- [The catalogue](#the-catalogue)
- [How things age](#how-things-age)
- [Placement on any map](#placement-on-any-map)
- [Caps and performance budget](#caps-and-performance-budget)
- [Data model and owners](#data-model-and-owners)
- [How it meets the other systems](#how-it-meets-the-other-systems)
- [Abuse rules](#abuse-rules)
- [Sound and light: feasibility](#sound-and-light-feasibility)
- [What the author builds in Workbench](#what-the-author-builds-in-workbench)
- [Settings, commands and log lines](#settings-commands-and-log-lines)
- [Writing the traces](#writing-the-traces)
- [Phases and rough sizes](#phases-and-rough-sizes)
- [Risks](#risks)
- [Decisions for the author](#decisions-for-the-author)

---

## In short

The Zone already remembers a lot as data: who held a town, who died in a house, where a truck burned, where a
pack took someone. Traces makes some of that visible, near players only, and never leaves anything behind.

What players will see:
- **Doors that talk.** Chalk, paint and stencils on a few doors in each town, written by the people who were
  there: Bastion's search code from the first days, the Haven's bear and candle, the Red Hand's red hand and
  tally, Halcyon's beak, residents' last words, and the infected's scratches. Each mark is generated from that
  town's real history. Anyone sees the gist; a Police Officer reads what Bastion wrote, a Hunter reads how old
  the scratches are and how many made them.
- **The first days, frozen in place.** The tail of the Day 2 column on the north road, suitcases on the verge
  where people turned back, the gym with its bear on the doors, the Day 4 church, the cordon's Day 1
  roadblock with its radio still hissing, Rudi's shutter, the hill where two men watched the relief go by,
  and a town square's notices pasted over each other, day by day.
- **Small scenes where the data says something happened.** The last meal in a town that went quiet, a house
  that held out, the chair at the edge of a Haven village, the shop with IOUs on the door, a Halcyon courier's
  dropped case, a waiting room outside a clinic, the roster board at a fallen Bastion post, buckets queued at a
  pump.
- **Traces that follow the simulation.** Scorch at a burned doorway, a wrecked supply truck where it was lost,
  a stencil painted over when a town changes hands, Bastion's relics on the roads as it fades, red hands
  spreading round the crossroads, a candle chalked where a pack took someone.
- **Your own traces.** The bears ticked in a town you emptied of survivors, ash where you burned the bodies, a
  grave someone dug, a candle chalked near where your character died.
- **Sound and light.** Lanterns only where people live, a radio hissing in an empty house, dogs barking and
  then going quiet before a hunt, a generator's hum at a field station, one stroke of the church bell, the
  town loudspeaker at shift change.
- **The line.** Crosses of fence palings outside the warning zone, Day 1 sheets blown about near the north
  road, and later the epilogue's queue in front of the column's cars.
- **The Long Table: nothing.** Its only trace is an absence.

No cut-scenes, no markers, no new screen. Nothing pays. A player who stops at a door learns something; a
player who doesn't loses nothing.

## Principles

1. **Data first, props when set, text always.** Every trace is a record on the server. Its prop is optional,
   spawned near players only, and its words are always there (an approach line or a door line). With every
   prefab slot empty the system still tells its stories in text.
2. **Generated from real history.** No mark is random dressing. Each one comes from a fact a built system
   holds (a seeded first-days canon fact, a town log entry, a safehouse count, a territory change, a kill
   site). If the fact can't be named in the data model, the trace isn't built.
3. **Dated, and allowed to be stale.** Every written mark carries its day. In an abandoned town marks are
   history and can be wrong now; in a lived-in place they're kept current. A reader always sees the day.
4. **One owner per thing.** Each trace has exactly one owning system at a time. The epilogue, tribes, camp
   life, errands and the majors keep what they own ([below](#one-owner-per-thing)).
5. **Show, don't explain.** The grammar is never explained in a tutorial. Players learn it the way people in
   the Zone did: by seeing the same symbol in the same situations. The backgrounds that would know say more.
6. **Bleak but human.** Most traces are about ordinary people looking after each other, badly, under
   pressure. No gore. No child hurt. The dark side said sideways ([WRITING.md](../docs/WRITING.md)).
7. **Nothing usable.** No trace holds loot, blocks a road or a door, gives a mission, or marks where a player
   is. Every prop goes through the dressing guard.
8. **Performance first.** No bodies at all. Props within a few hundred metres of a player, capped per cluster
   and per server; decals and lights drawn on clients from a short replicated list.
9. **Any map.** Everything is placed relative to settlements, buildings, doors, roads and canon spots, with a
   fallback for each when the map lacks it.
10. **Sparse.** Most of the map stays abandoned and quiet. At most six marked doors in a town, one vignette in a
    town, and three scenes with props within 300 m.

## One owner per thing

Traces owns how a thing looks and where it stands. The system that owns a fact owns whether it's true.

| Thing | Owner in the season | Owner after the cure | Note |
|---|---|---|---|
| Door marks (all six authors) | Traces | Traces, frozen: no new writes except those the epilogue asks for | The epilogue's door chalk (*"GONE HOME: {n}"*) is written through the grammar's API: the epilogue owns the fact, Traces the drawing |
| The first days (F1–F8) | Traces | Traces (unchanged; the epilogue's human queue stands in front of the column's cars) | |
| Vignettes (V1–V8) | Traces | Traces: running ones finish their life, no new ones | |
| Live traces (L1–L8) | Traces | Traces until each expires; the epilogue's snapshot reads them | Lost-truck wrecks: the epilogue picks the last 4 at the commit and owns those from then |
| Kill-site things, abandoned lairs, faction marks at a pack's range, call sounds | Tribes ([INFECTED.md](../docs/INFECTED.md#signs-and-sounds)) | Tribes, then the epilogue's tribe beats | Traces adds only the after-trace once a kill site is gone (L1) |
| Season marks from the town log (built) | World memory (whether one stands, and its text) | The epilogue's town beats | Traces supplies the look by kind (P5) |
| In-season graves, the Remembered candles | Traces (the epilogue design's "All season" memorials, built here) | The epilogue's "Their dead" and safehouse candles | At the commit Traces' graves come down; the epilogue places its own from the record |
| Fallen-post roll call (V7) | Traces | The epilogue's outpost boards | |
| Camp life's pieces, routines and the lantern on its table | Camp life | Camp life | Traces lights the lantern after dusk (S1) |
| Ambient notice prop at holdings' gates and safehouse gates | Tone ([designs/TONE.md](../designs/TONE.md)) | Tone | Traces' layered notices stand only in towns nobody lives in |
| Route marks from 90% | The finale/epilogue design | Epilogue | |
| Rudi's crossroads lorry, bonfire and book | The majors | The majors' codas | Traces owns only Rudi's shutter (F7) |
| The lower store, the relay, the Day 3 page, questline clues | Their questlines | Their questlines | Traces never repeats a clue's text and keeps 1 km off an active clue of the same faction |
| Errands' found hook "an old mark" | Errands decides when and where | — | Its words and look come from the grammar ([Errands](#errands)) |

## The door-mark grammar

A door mark is a symbol painted, chalked, stencilled or scratched beside an outside door, with a few written
fields. A door can carry up to three layers (an old stencil, chalk beside it, paint over it). The look is a
decal or a small board; the meaning is always in text.

### Who writes on doors, and when

The season starts on Day 11 (the broadcast's count). Days 1–10 are history, seeded once a season from its seed
and the canon spots, the same after every restart and never saved. From Day 11 on, marks are written live.

| Days | Who | What they wrote | Where |
|---|---|---|---|
| 1 | The cordon detail (Bastion) | The search X on doors near the Day 1 roadblocks; residents' notes go up | Roads into the hills round `origin.site` |
| 1–3 | Residents | *"GONE TO..."*, *"DON'T COME IN"*, *"WE'RE AT THE GYM"* (from the evening of Day 2) | Any town |
| 2 (06:00–14:00) | Bastion's sections walking civilians to the checkpoints | The X with *"→NR"*: living sent to the north road | Towns within 4 km of `day2.checkpoint` |
| 2 (after 14:00)–5 | Bastion, looking for stragglers | The X with *"→GYM"*, later *"→CH"* (Day 4) | Same towns, plus the gym's and the church's |
| 2 onward | The Haven | The bear on the gym's doors (Day 2), then on houses with children; the ring-and-dot, arrows, the water drop | Round the gym and the church, then round every safehouse |
| 4 onward | The Red Hand | Tallies near the crossroads (the first toll was Day 4); red hands from Day 7 (Rudi's shutter), protection from Day 9 | Within 1.5 km of `redhand.crossroads`, then round every crew |
| 5 onward | The Haven | Candles (the first dead read at supper) and cross-hatching (the first walk-outs, from Day 6) | Round its houses |
| 6 | Bastion | The searches stop with the CO's call: no X is ever dated after Day 5 | — |
| 7 onward | Bastion | *"C · D7"* (contribution) on barns near posts; *"ATT"* on attached families' houses; *"CLEARED"* after a cull | Round its holdings |
| 7 onward | Halcyon | The beak and a sample code (the trial was Day 6) | Round the lab and its field station |
| 1 onward | The infected | Scratches | Towns as they go quiet |

### The symbols

| Symbol | Author | Material and colour | Means | Fields |
|---|---|---|---|---|
| **The X** | Bastion | Orange spray, sometimes black | Searched | Four quadrants: day and time, unit, hazards, found ([below](#bastions-search-code)) |
| **The slash** | Bastion | Orange spray | Went in, never came out to finish it | Day, unit |
| **The ringed X** | Bastion | Orange, a second colour for the ring | Searched, then checked again by someone else | As the X |
| **The post code** | Bastion | Black stencil | A Bastion holding's street | *"B-2 · D14"*: post, day |
| **C** | Bastion | Black stencil, small | Contribution taken | *"C · D9"*: day |
| **ATT** | Bastion | Black stencil | Civilians, attached, live here; don't requisition | *"ATT · DO NOT REQUISITION"* |
| **CLEARED** | Bastion | Black stencil, sometimes over grey paint | Culled, shelled or retaken | *"CLEARED · 0400 · D27"* |
| **The bear** | The Haven | White paint, stencilled; copies in chalk | Children inside | Day |
| **The patched bear** | The Haven | White paint, a patch on its belly, one eye stitched | A Haven house | Day the house opened |
| **The ringed bear** | The Haven | Chalk ring round the bear | Children were here; moved on, safe | Arrow to where |
| **The ticked bear** | The Haven | Chalk tick | Everyone got out (a rescue) | Day |
| **The candle** | The Haven | White chalk | Someone from this house was lost | A number under it; names under that, small |
| **Cross-hatching** | The Haven | White chalk, like a gate | Someone here declared a bite and was walked out | Day |
| **Ring and dot** | The Haven | White chalk | Checked: nobody here | Day |
| **Arrow and letter** | The Haven | White chalk | Gone to the house at... | *"→ M"* |
| **The drop** | The Haven | White chalk on a post | Water: the pump or well here works | Day |
| **The red hand** | The Red Hand | Red paint, a flat palm | Paid: under protection | Day; tally strokes beside it for weeks paid |
| **The tally** | The Red Hand | Red paint, gates of five | Owed: tins | Strokes |
| **The struck hand** | The Red Hand | A red hand with a stroke through it | Lapsed: owes | Day |
| **PAID** | The Red Hand | Red, in the book's hand, across a tally | The line is cleared | Day. Nobody asks who paid it |
| **The thumbprint** | The Red Hand | A small red print low on the frame | Someone the Haven put out sleeps here | None. Only an Ex-Con sees what it is |
| **The beak** | Halcyon | Black stencil on a square of white primer | Sampled here | *"S-112 · d19 · 2"*: series, day, samples |
| **The ringed beak** | Halcyon | As the beak, with a painted ring | Sessile cases under study: please don't burn | Day |
| **The barred beak** | Halcyon | As the beak, a bar through it | Finished here | Day |
| **Scratches outside** | The infected | Gouges at hand height on the outside | They wanted in | Fresh or old, how many (Hunter) |
| **Scratches inside** | The infected | Gouges on the inside of an open door | Someone turned in there | Fresh or old |
| **Smears at the sill** | The infected | Low grey smears | Slow ones lay against it | Fresh or old |
| **Residents' words** | Residents | Pencil, marker, lipstick on glass, chalk | Whatever they needed to say | Usually a day |

### Bastion's search code

Bastion's manual has a search code, and the first days left it on hundreds of doors. It's inspired by real
search-and-rescue marks, with fields of its own. Glance level sees an orange X with writing in its corners.

| Quadrant | Field | Values |
|---|---|---|
| Top | Day and time | *"D2 0740"* |
| Left | Unit | *"1A"*, *"1B"*, *"2A"*, *"2B"*, *"3C"*, *"HQ"* (seeded per sweep) |
| Right | Hazards | *"INF"* infected inside; *"BIT"* a bitten person inside; *"SEAL"* sealed (the building the lock system locked); *"NE"* no entry, barricaded from inside; *"DOG"*; blank for none |
| Bottom | Found, and where they were sent | *"3L→NR"* (three living to the north road, Day 2 before 14:00); *"1L→GYM"* (after 14:00 on Day 2); *"1L→CH"* (Day 4 on); *"1D"* (one dead); *"0"* (nobody) |

- **The slash.** The first stroke goes on as the section goes in; the second as it comes out. A slash with no
  second stroke means it never finished. About 1 X in 20, never before Day 3.
- **The ring.** A second sweep (Day 5) circled what it checked again. Rare, and only where the first sweep
  found a bitten person or a dead one.
- **SEAL is true.** The right quadrant says *"SEAL"* exactly on buildings the lock system has locked (its
  seed), so a Police Officer reading the code knows the door before trying it. It never says what's inside.
- **No X after Day 5.** After the CO's call on Day 6, Bastion stops looking for anyone but its own.

Examples:
- *"D2 0740 · 1A · – · 3L→NR"*: Day 2, before the column left. Three people walked to the north road and
  turned back that afternoon.
- *"D3 1420 · 2B · INF · 1L→GYM 1D"*: infected inside, one living sent to the gym, one dead.
- *"D4 / 1B"* and nothing else: the slash.

### Haven chalk

The Haven chalks for the next runner, never for the record. It never writes blame, never writes "taken", and
never paints over anyone else's mark: it chalks beside it.

- **The bear** is the gym's, from Day 2 ([LORE.md](../docs/LORE.md#the-haven)). A safehouse's door carries the
  patched bear. A house with children nearby carries a plain bear while they're there, a ring round it when
  they've moved, and an arrow to where.
- **The candle** is the dead. The number under it grows. The Elder reads the names chalked small underneath
  (seeded residents, a safehouse's named dead, a player's description, by [the naming rules](#writing-the-traces)).
- **Cross-hatching** is the walk-out. The Haven never writes the words; the hatching looks like a gate.
- **The drop** marks a working pump or well within 50 m (the interaction system's own wells and pumps), so
  it's always true when it's fresh.

### Red Hand paint

The Red Hand paints so everyone knows what's paid. It leaves the bears alone.

- **The hand** on a shop, a garage shutter or a house says the crews take a cut here and nobody else does. The
  tally beside it counts weeks paid. A **struck hand** has lapsed. The text never says what happens next.
- **Tallies** without a hand count tins owed, mostly near the crossroads and the tolls.
- **PAID** across a tally clears the line. On a house nobody lives in any more, an Ex-Con reads it for what it
  is: *"Paid, in the book's hand. Nobody lives here now."*
- **Over other marks.** When the crews take a town they paint hands over Bastion's stencils. They never paint
  over a bear.
- **The thumbprint** marks where a stray sleeps ([LORE.md](../docs/LORE.md), "Strays"). It comes from the
  data of the Haven's exiles (wanderers turned out), never from a player.

### Halcyon stencils

The beak started as a joke on a sample case ([LORE.md](../docs/LORE.md#halcyon)). On a door it's procedure.
- **The beak** and its code: a sample series, the day, how many were taken. A Lab Technician reads the series
  as Halcyon's.
- **The ringed beak** asks people not to burn a nest of the slow ones under study. Burning it anyway scorches
  the mark (L2); Halcyon never repaints it.
- **The barred beak** closes a site.

### The infected's marks

Not a language: damage. A Hunter reads them like tracks.
- **Outside**, at hand height: they wanted in. Fresh ones are pale wood; old ones grey.
- **Inside**, on an open door: someone turned in that room. Never on a door with a bear, never in a lived-in
  place, never beside residents' words about a child.
- **Smears at the sill**: the slow ones lay against it. Common in towns at 65+ infection.

### Residents' writing

Plain words, on the door or the glass, in what was to hand. Most are from Days 1–3. A seeded pool (each line
picked by the town and slot, so it's the same after a restart):

| Line | When |
|---|---|
| *"GONE TO MUM'S IN {town}. D1"* | Day 1 |
| *"WE'RE AT THE GYM. D2"* | from the evening of Day 2, within 5 km of the gym |
| *"DON'T COME IN. TELL ANNA I'M SORRY. D2"* | Day 2–3; this door may later get inside scratches |
| *"BACK SOON. KEY WHERE IT WAS"* | any |
| *"THE RADIO SAYS STAY INDOORS. WE ARE INDOORS."* | Day 1 |
| *"CHURCH OPEN. ASK FOR THE FATHER. D4"* | Day 4, within 4 km of the church |

### Layers and overpainting

| Rule | |
|---|---|
| Three layers a door | A fourth write removes the most weathered layer |
| The Red Hand paints over Bastion | When it takes or works a street: a hand over the X or the post code. The X shows through in the text |
| Bastion paints grey over the Red Hand | When it takes or clears a town: grey paint, then its stencil on top |
| The Haven never paints over | It chalks beside |
| Halcyon never paints over, and is painted over | Its beak is left alone by the Haven, painted over by the crews in their towns |
| Nobody paints over residents' words or the bear | They weather |
| Chalk washes | Rain ages chalk twice as fast ([How things age](#how-things-age)) |

### Reading: who reads what

Three levels. **Glance**: what anyone can see. **Read**: what the writer meant, for someone who knows that
writer. **Know**: one more true detail that only training or a life gives.

| Marks | Read | Know |
|---|---|---|
| Bastion's X and stencils | ★ Police Officer, ★ Soldier, Known with Bastion | ★ Police Officer, ★ Soldier: the slash, the ring, what *"C"* and *"ATT"* mean, which sweep |
| Haven chalk | ★ Nurse, ★ Elder, ★ Police Officer, Known with the Haven | ★ Elder: the names under a candle. ★ Nurse: what the hatching means |
| Red Hand paint | ★ Ex-Con, ★ Raider, ★ Police Officer, Known with the Red Hand | ★ Ex-Con: the thumbprint, and what PAID means on an empty house |
| Halcyon stencils | ★ Lab Technician, ★ Doctor, ★ Police Officer, Known with Halcyon | ★ Lab Technician: the series is the lab's own, and which pack the samples came from |
| The infected's marks | ★ Hunter (and everyone at glance) | ★ Hunter: fresh or old (in days), how many, one tall |
| Residents' words | Everyone | ★ Police Officer: whether the day fits the rest of the door |
| Any written mark's age | ★ Hunter: *"The chalk's a day old."* | |

The Police Officer reads every faction's marks at Read level (scenes are the job); the Hunter reads when and
how many. Mixed reputation reads as the access rank does ([REPUTATION.md](../docs/REPUTATION.md)).

**Reading never pays.** No Fame, no Favor. Two small ties only: reading scratches left by a pack's young who
came home makes that pack **Heard of** ([Knowing](../docs/INFECTED.md#knowing)); and every door line you read
goes in the journal.

### How a reader meets a mark

No action, no button, no typing: it works the same on a pad.
- **The town.** The first time a character enters a town with two or more marked doors this session, one
  glance-level summary on the Ambient lane: *"Doors in Levie: three bears, two of them ringed. A red hand on the
  shop."*
- **The door.** Stop within 5 m of a marked door for 2 seconds and its line comes, at your level, once per
  door per session (the Ambient lane, keyed by the door). A door with layers reads newest first.
- **The journal.** The last 12 door lines read, under **Read on doors**, in the notebook's Journal tab, with
  the town and the day you read them. Lost with the life.
- With **UI > Reader** on, a door with three layers opens in the reader instead of a notice (each layer a
  paragraph).

### Generation: from a town's history to its doors

1. **Season start.** `RZ_FirstDays` seeds Days 1–10 from the season seed and the canon spots: which towns
   Bastion swept (towns within 4 km of `day2.checkpoint` and within 3 km of the roads into `origin.site`, at most
   12, at most 5 doors each), residents' words (1–2 doors in a third of towns), the Haven's early chalk round
   the gym and the church, the Red Hand's tallies round the crossroads, Halcyon's first beaks round the lab, and
   scratches in towns already at 60+ infection. These are never saved: they're made again, the same, at every
   start.
2. **Live writes.** Built systems tell Traces what happened through one-line writers (`RZ_TraceWriters`, the
   same pattern as `RZ_WorldWriters`). Each event goes into the town's history and, through the table below, may
   write one door.
3. **Which door.** A town's doors are its buildings with an outside door (`RZ_BuildingQuery`,
   `RZ_DoorGeometry.FindDoors`, `IsOuterDoor`), outside camp life's rings, sorted by a hash of position and
   season seed. A write asks for a kind (a shop, a clinic, a farm or barn, a house, the building nearest a point)
   and takes the first free slot of that kind; with none, any house. Slots are numbers, so nothing about a
   building is saved.
4. **Limits.** At most 6 marked doors a town (8 in a stronghold's town), 3 layers a door, 2 live writes a town
   a season day, 20 a season day on the server. A full town gives up its least salient, most weathered door.

| Event (writer) | Door write |
|---|---|
| A safehouse stands in the town (safehouses; at start and when one opens) | The patched bear on the building nearest it (within 30 m; else text only); a plain bear on 1–2 houses at Community or bigger |
| A safehouse empties (world writers' `emptied`) | Its bears ringed, an arrow to the nearest house |
| A house's dead go up (`m_iDied`) | A candle on a house near it, or its number goes up |
| A bite declared and walked out (`house.bitten`, Walk Him Out) | Cross-hatching and the day, on the house on the side of town the walk went |
| People brought in alive from a town (the town log's `rescue`; Survivors Trapped, Collapse Shelter) | The bear ticked or a ring and dot on the house they came out of; an arrow to the house they went to |
| The Red Hand founds or takes an outpost (territory) | Hands on 1–3 doors, the shop first, over Bastion's stencils |
| Protection Racket, A Quiet Arrangement, toll work done for the crews | Hand and tally on the house or shop. Under `haven.rule` 4, a small hand on a safehouse's back door |
| Debt Collection, a debt gone quiet | A struck hand, or PAID across a tally |
| Bastion founds or takes an outpost | The post code on 1–2 doors; grey paint over hands |
| Requisition, Off-Book Requisition | *"C · D{day}"* on a farm or barn |
| A cull: Bastion Shelling, Fire Mission, Retake the Town, a nest cleared | *"CLEARED · D{day}"* |
| Halcyon's sample jobs, Specimen from the Nest, a Lab Technician's sample | The beak and its code; the ringed beak at a nest Halcyon wants left |
| A town turns infested (`town.infested`, 65) | Scratches outside on 1–2 doors, smears at a sill |
| A town goes dark (85, or the tribes' "town gone dark") | Inside scratches on one door that has residents' words and no bear |
| A notable death of a player's character in the town (P4) | A candle with the description under it |
| A pack's named young who came from this town, with the pack's range within 1.5 km | Outside scratches at a ringed bear (L4, [decision 6](#decisions-for-the-author)) |
| An exile (a wanderer the Haven turned out) sleeps in a town nobody holds | The thumbprint |
| Errands' found hook, *an old mark* | An existing mark near the site, else a two-hour chalk or stencil ([Errands](#errands)) |

### Three towns over a season

The map's names are examples. Day 11 is the season's first day; the cure comes on Day 52.

**Morton**: a crossroads town. A Bastion outpost from the start; the Red Hand takes it on Day 24.

| Door | Day 11 | Day 20 | Day 32 | Day 48 |
|---|---|---|---|---|
| The shop | X: *"D2 0740 · 1A · – · 3L→NR"* | The same, faded | A red hand over the X, two strokes | The hand, six strokes |
| House A | Lipstick on the glass: *"GONE TO MUM'S IN BROD. D1"* | The same | The same | The same; a candle chalked beside it, *"someone who'd been a mechanic. D41"* (a player's character died nearby, P4) |
| House B | X: *"D4 1130 · 2B · BIT · 1L→CH"*, ringed on Day 5 | *"ATT · DO NOT REQUISITION"* stencilled beside it (Bastion's post) | A red hand beside ATT, struck through: the family won't pay | Struck hand, tally of four: owed |
| The garage | Blank | Post code *"B-2 · D14"* | A red hand on the shutter, over the post code | The same |

A Police Officer stopping at House B on Day 32 reads: Bastion found a bitten person here on Day 5 and sent them
to the church; Bastion later attached the family to the post; the crews now say they owe. Nobody tells that
story; the door does.

**Levie**: a quiet village with a Haven safehouse; two walk-outs and a pack nearby.

| Door | Day 11 | Day 20 | Day 32 | Day 48 |
|---|---|---|---|---|
| The safehouse | The patched bear, *"D4"* | The same | The same; a small red hand low on the back door (`haven.rule` 4 from Day 27) | The same |
| House A | A bear, *"D6"* | A candle, *"1"* | Cross-hatching, *"D29"*; candle *"2"* | Candle *"3"*; the bear ringed, arrow *"→ L"* |
| The pump | The drop, *"D6"* | The same, washed faint | Re-chalked *"D30"* | The same |
| House B | X: *"D3 1510 · 2A · – · 0"* | The same, faded | The same | The same |
| House C, at the edge toward the woods | A bear, *"D8"* | The same | The bear ringed, arrow *"→ L"* (the children moved into the house, D30) | Fresh scratches outside the ringed bear (a young from Levie came home, D44) |

A Hunter at House C reads: *"Fresh. One of them, not tall. It stood here a long time."* The Haven's Known hear
gossip; nobody writes it down.

**Brod**: a small town near the closed site. Halcyon samples it; it goes dark on Day 30.

| Door | Day 11 | Day 20 | Day 32 | Day 48 |
|---|---|---|---|---|
| The clinic | X: *"D1 0350 · HQ · INF · 2D"*; a card in the window: *"DOCTOR BACK 1400"* | The beak, *"S-031 · d17 · 3"* | Scratches outside | The barred beak, *"d46"* |
| House A | The slash: *"D4 / 1B"* | The same | Old scratches over it | The same |
| The barn | Blank | Blank | The ringed beak, *"d22"* | The ringed beak, blackened: the barn was burned on Day 35 (L2) |
| House B | *"DON'T COME IN. TELL ANNA I'M SORRY. D2"* | The same | Scratches inside | The same |

Brod isn't `origin.clinic`: Patient Zero's clinic gets no marks at all, so no door can be read as a clue. Brod's
doors say only what LORE says out loud: the cordon was in the hills on Day 1.

## The catalogue

Each entry: what you see; the props by type; the trigger in the data; where it stands (any map); how long it
lasts and how it ages; its text; who reads more. "Approach line" means the line a player gets once a session
within the scene's radius (40 m unless noted), on the Ambient lane. Every prop is optional.

### The first days, frozen

Seeded once a season from canon (new keys below), made again the same at every start, never saved.

**F1. The column's tail** (the Day 2 car queue)

| | |
|---|---|
| You see | Four to eight cars pushed onto the verge nose to tail, doors open, a suitcase split on a roof, a coat on the grass |
| Props | Static car wrecks (no vehicle component), suitcases, a coat, a blanket; one car with a sun-faded sheet on its dashboard |
| Trigger | Always, once a season: the road from `day2.checkpoint` inward |
| Where | On the verge of the road leading to `day2.checkpoint`, from 400 m to 1.2 km back from the barrier (outside the warning zone), at least 25 m from a junction, every car 6 m or more from the road's centre line. Island maps: the road to the slipway the canon spot picks. No road network: none |
| Lasts, ages | All season. Early Outbreak: suitcases on the verge. Collapse: suitcases gone, one car burnt out. Overrun: grass through the wheels (text). After the cure it stays; the epilogue's queue forms 150–250 m from the barrier, in front of it |
| Text | Approach: *"Cars on the verge, all pointing the same way, all the doors open. Nobody came back for them."* The dashboard sheet (reader): *"NUMBER 31. PROCEED TO NORTH ROAD CHECKPOINT FOR PROCESSING. BRING IDENTIFICATION. D2."* In biro: *"Mirror shot off the bus. Turned round 1400. Walking to the gym."* |
| Reads more | ★ Police Officer: *"The numbers on the dashboards run 18 to 40. Somebody was handing out tickets."* ★ Hunter: *"Old scratches down the side of the last car, low. Day 2 or 3."* (the column's pack, [INFECTED.md](../docs/INFECTED.md#who-they-are)) |

**F2. Where they turned back**

| | |
|---|---|
| You see | Suitcases and bags dropped in a line across the verge, as if people put them down to turn round faster |
| Props | Three suitcases, a holdall, a child's coat folded on a suitcase (never a child's body or shoe alone) |
| Trigger | Always |
| Where | 320–400 m from the barrier on the same road, just outside the warning zone (the column turned at 150 m; what people dropped carried on back). Fallback: none |
| Lasts, ages | All season. Collapse: two suitcases opened, empty (text: *"Somebody's been through them. Nothing worth taking."*) |
| Text | Approach: *"Suitcases across the verge, set down neatly, then left."* |
| Reads more | ★ Soldier: *"From here you'd see the barrier. And the men behind it, in our badge."* |

**F3. The gym** (canon `day2.gym`)

| | |
|---|---|
| You see | A school gym's double doors with the original bear stencilled on them, faded and touched up; a list on the door; folded camp beds and blankets stacked beside it; the bus in the car park |
| Props | The bear (the door-mark layer), a stack of camp beds, blankets, a hand-painted board *"GYM OPEN"*, a static bus wreck |
| Trigger | Always. If the gym's town is lived-in now, only the bear and the list (camp life owns the rest) |
| Where | `day2.gym`: the school (landmark *school*) within 3 km of the Haven stronghold's safehouse, preferring one between it and `day2.checkpoint`; else the largest building in the nearest town to the north road with no holding. The bus on flat ground 10–30 m from the gym's door, 6 m off any road |
| Lasts, ages | All season. The list never changes |
| Text | Approach: *"A bear on the gym doors. It's been painted over so many times it has a patch on its belly."* The list (reader): eleven first names in a teacher's hand, each ticked; at the bottom: *"All at the house. Lock up after you."* |
| Reads more | ★ Elder: *"That's the teacher's writing. They all made it."* |

**F4. The Day 4 church** (canon `day4.church`)

| | |
|---|---|
| You see | A church door with a board leaning on the wall: *"CHURCH OPEN. ALL WELCOME. DECLARE YOUR BITE."*; a kitchen chair beside the door with a blanket folded on it; jam jars on the step |
| Props | The board, a kitchen chair, a blanket, 3–5 candle jars (lit at dusk if lights are on, S8) |
| Trigger | Always |
| Where | `day4.church`: a church (landmark *church*) in a different town from the gym, within 4 km of it, not a holding's town; else any church within 6 km; else the gym's town hall (the gym's second door) |
| Lasts, ages | All season. The jars are lit at dusk on evenings a name is read (S6) |
| Text | Approach: *"A chair by the church door, a blanket folded on it. Somebody sat up here all night once."* The board: *"Declare your bite"* is the first time players see the Haven's rule written down |
| Reads more | ★ Nurse: *"The chair faces the door, not the road. Whoever sat here was watching somebody inside."* (the lodger, never named) |

**F5. The cordon's Day 1 roadblock** (canon `day1.roadblock`)

| | |
|---|---|
| You see | A sandbag half ring across a verge, a barrier pole knocked into the ditch, a camp chair, empty ration tins, a field radio hissing on a crate |
| Props | Sandbags, a barrier pole, a camp chair, tins, a field radio (static mesh, no radio actions), a plain crate (no storage) |
| Trigger | Always |
| Where | `day1.roadblock`: a road point 1.5–3 km from `origin.site` on the road toward the lab's town, outside every questline's stage area (HC1's site stages and the lower store keep their 150 m and 1 km), off the line |
| Lasts, ages | All season. The radio hisses (S3) until Overrun, then the batteries are dead |
| Text | The log sheet (reader): *"D1 0410. Incident roadblock established. 0600 traffic stopped, 14 vehicles turned. 2210 two civilians attempted to pass on foot. Returned. D2 0600 ordered to the north road. Leaving the pole down."* Nothing after |
| Reads more | ★ Soldier: *"'Leaving the pole down' means they meant to come back."* ★ Radio Operator: *"Still on the cordon net. Nothing on it but carrier."* |

**F6. The town square's notices** (one town in five with a square)

| | |
|---|---|
| You see | A board, a bus shelter or a wall by the square with sheets pasted over each other |
| Props | A board on two posts or the shelter's wall, 4–6 sheets (`Notice_A4_Vertical_V1.et`) |
| Trigger | Towns nobody lives in, with a square (a landmark *monument*, *bank*, *station* or the town's centre) |
| Where | Within 40 m of the town's centre, on a wall face or a free-standing board; never at a holding's or safehouse's gate (that's Tone's ambient notice) |
| Lasts, ages | All season. Layers are worked out from the town's data each time it's read, so a layer appears when its fact becomes true |
| Text | Read top to bottom, oldest underneath: (1) *"INCIDENT IN THE HILLS. REMAIN INDOORS. D1."* (2) *"ALL RESIDENTS TO THE NORTH ROAD CHECKPOINT FOR PROCESSING. 0600. D2."* Across it in marker: *"TURNED BACK 1400"* (towns within 4 km of the north road). (3) *"GYM OPEN. BRING BLANKETS."* (the Haven, within 5 km of the gym). (4) Halcyon's printed flyer, *"Infected samples wanted"* (within 4 km of the lab). (5) *"DIESEL. ASK AT THE CROSSROADS."* in red (within 3 km of the Red Hand). (6) The government sheet on top, *"Aid is on its way"*, with *"AID"* underlined twice in somebody else's pen |
| Reads more | ★ Police Officer: the order in (2) is signed *"Lt H. Sedlak"*. The name means something at Bastion's stronghold |

**F7. Rudi's shutter** (at `redhand.crossroads`)

| | |
|---|---|
| You see | A garage shutter with one red hand painted over smeared prints |
| Props | The door-mark layer on the garage's shutter; without a garage within 60 m, a shutter-sized board on the verge (a static garage door prop) |
| Trigger | Always |
| Where | The building nearest `redhand.crossroads` within 60 m whose name says garage or workshop; else the board on the verge |
| Lasts, ages | All season. Never repainted, never washed |
| Text | *"A garage shutter with one red hand on it. Under the paint, smears, a lot of them, at hand height. The crews call it Rudi's."* Nothing more, ever |
| Reads more | ★ Ex-Con: *"Nobody parks in front of it."* |

**F8. The hill over the north road** (Day 4–5)

| | |
|---|---|
| You see | A scrape in the ground behind a low wall or rocks, a ration tin, a fence post with *"05:41"* cut into it |
| Props | A ration tin, a groundsheet, a post |
| Trigger | Always, if the map has high ground with a view |
| Where | High ground 500–900 m from `day2.checkpoint`, outside the warning zone, with terrain line of sight to the barrier (the sight rule's height samples) |
| Lasts, ages | All season |
| Text | *"Somebody lay up here for a long time watching the road. On the post, cut with a knife: 05:41."* |
| Reads more | ★ Soldier: *"An observation post. Two men, two days, by the tins."* ★ Known with Bastion: *"Every post knows what happened at 05:41."* |

**New canon keys** (rules as `RZ_CanonRule` subclasses, made once a season, same after restarts):

| Key | Kind | Rule |
|---|---|---|
| `day1.roadblock` | spot | As F5 |
| `day2.gym`, `day2.gym.town` | spot, town | As F3 |
| `day4.church`, `day4.church.town` | spot, town | As F4 |
| `day2.queue` | spot | The point on the road 400 m back from `day2.checkpoint` (F1, F2) |
| `day4.hill` | spot | As F8; empty when no point qualifies |

### Vignettes

Small scenes dealt where the data says something happened. Each needs its trigger to stay true when it's
placed; once placed it lives its lifespan even if the town changes (it's history). At most 12 on the server,
one a town, 3 dealt a season day.

| # | Scene | You see; props | Trigger | Where | Lasts, ages | Text | Reads more |
|---|---|---|---|---|---|---|---|
| V1 | **The last meal** | A garden table laid for four, plates, a jug, a radio; chairs pushed back. Garden table, 4 chairs, plates, a jug, a static radio | A town nobody holds goes dark (85, or the tribes' "town gone dark") | In the garden of a house: 4–10 m from an outside door, flat ground, inside the town radius, away from the road | 3 season days: fresh (cloth on), settled (the jug knocked over), old (chairs fallen) | *"A table laid for four in a garden. Nobody got as far as the soup."* The radio plays its hiss (S3) | ★ Hunter: *"They left in a hurry, all at once, and nobody came back for the coats."* |
| V2 | **Someone held out** | Planks across the ground-floor windows, a hanging plank off the door frame (the door stays clear), shell casings by the step, a tally of days chalked on the door. Planks (static), casings | A Survivors Trapped or Collapse Shelter failed in a town (the town log's worst `mission`), or a town nobody holds turned infested with a player's barricade standing (the town's Barricaded flag) | The house nearest the job's site, else nearest the town's centre | 3 season days | The door's tally: 19 strokes. A note on the step (reader): *"D19. Out of water. Going for the pump. Back by dark."* | ★ Police Officer: *"The casings are all one calibre. One gun, one person."* |
| V3 | **The chair at the edge** | A kitchen chair at the edge of a village facing out, a blanket, a jam jar. Chair, blanket, jar (lit at dusk on its first two evenings) | A walk-out: `house.bitten` at a safehouse, or Walk Him Out finished with *walk on* | 100–200 m out from the safehouse, on the side toward the nearest pack's range, on a field edge or a verge, never on a road | 2 season days | *"A chair at the edge of the village, facing out. Somebody sat with somebody here."* | ★ Nurse: *"They sit with you until the end, in one house. In another, they walk you here at dusk."* |
| V4 | **The honest shop** | IOUs pinned to a shop door, a tin on a crate by the step. Sheets, a tin, a crate (no storage) | A shop in a town nobody holds, a Haven house within 2 km, no Red Hand holding within 1.5 km | The shop's outside door | All season while true. When the Red Hand takes a town within 1.5 km, *"PAID"* in red across the IOUs, then 3 season days, then gone | *"IOUs on the shop door, a dozen hands: 'Two tins, bleach. Will pay when this is over. — M., Brod.'"* After: *"Across them all, in red: PAID."* | ★ Ex-Con: *"That's the book's hand. The shop's in the book now."* |
| V5 | **The dropped run** | A steel sample case on its side, a respirator with a long filter, a bicycle in the ditch. Case (static, closed, no storage), respirator, bicycle | A Halcyon courier or sample job failed on the road, a truck lost on a lab route, or a pack took a runner within 3 km of the lab or field station | On the verge where it happened (else the road between the field station and the lab) | 2 season days | *"A sample case in the ditch, with the plague doctor on its lid. It's empty. The cold packs are warm."* A notebook page (reader): *"Courier 4. Two samples, Pack P-02. Left 1700."* | ★ Lab Technician: *"They'll have written the case off by now. They'll have written the courier up."* |
| V6 | **The waiting room** | Six chairs in a line outside a clinic, a ticket roll, a sign on an easel. Chairs, a sign | Always, once a season, at one clinic (landmark *pharmacy*) in a town nobody holds; **never at `origin.clinic`** (Patient Zero's) | Along the clinic's front wall, 2 m out, beside the door (never in front of it) | All season | The sign: *"WAIT HERE. DOCTOR BACK 1400."* The last ticket on the roll: *"31"* | ★ Doctor: *"1400 on which day, it doesn't say."* |
| V7 | **The roll at a fallen post** | A roster board left at a lost Bastion outpost, grease pencil names, tags on nails, a cold fire barrel. Board, barrel, sandbags | A Bastion outpost lost or abandoned (territory) | Where the outpost's centre was, in its approach ring (camp life's pieces are gone) | 3 season days; then the epilogue owns lost posts after the cure | *"A roster board, left behind. The names are still on it. Every one has a mark against it but six."* | ★ Soldier: *"They took the tags with them. They'll read the names tonight anyway."* |
| V8 | **The water queue** | Buckets and bottles in a line at a pump, each with a name on tape. Buckets, plastic bottles (static meshes, never jerrycans) | A Haven house within 1.5 km is short (`house.hungry`) and a working well or pump stands within 400 m of it | Beside the pump, in a line away from it | While the house is short, then 1 season day | *"Buckets in a line at the pump, each with a name on it. Nobody's queueing. The names are."* | ★ Elder: one or two of the names, seeded |

### Live traces

Traces that follow the simulation as it runs. At most 24 on the server.

| # | Trace | You see; props | Trigger (the owner of the fact) | Where | Lasts, ages | Text | Reads more |
|---|---|---|---|---|---|---|---|
| L1 | **After a kill site** | Haven chalk on the nearest post or door: a candle and a description or a name; or Bastion's stencil | A tribes' kill site ends (its second dawn) in a place the Haven or Bastion knows: a Haven house within 3 km, or a pack on Bastion's board | The nearest door within 60 m, else the nearest post on a verge | 2 season days | Haven: *"A candle in chalk on the gatepost, and under it: 'someone who'd been a driver. D17.'"* Bastion: *"CONTACT P-2 · 1 LOST · D17"*. With nobody knowing: nothing | ★ Hunter: the tracks, as built ([Tracks](../docs/INFECTED.md#tracks)) |
| L2 | **Scorch** | A black fan of scorch up the wall from a doorway, ash on the step; Haven chalk *"WE CHECKED"* if the Haven burned it | A building burned (the fire system's record) | The doorway the fire started at | Fresh 1 hour (the built fire effect), scorch 2 season days | *"Burned from the doorway out. Somebody chalked on the step: BURNED D33. NOBODY INSIDE (WE CHECKED)."* (only when the burn came from a Haven job) | ★ Police Officer: *"Started at the door, with fuel. Not an accident."* |
| L3 | **Overpainted** | The loser's marks painted over by the taker (door-mark layers) | Territory takes or abandons an outpost | That town's doors | Until painted over again | *"A red hand over a Bastion stencil. The stencil still shows through."* | As the grammar |
| L4 | **The young come home** | Fresh scratches outside a ringed bear | A pack's named young from this town, the pack's range within 1.5 km, at dusk 3 evenings in 10 | A door with a ringed bear, never a lived-in one | 1 season day | Glance: *"Scratches on the door with the bear. Fresh."* Nothing more, from anyone | ★ Hunter: *"One of them. Not tall. It stood here a long time."* Makes the pack Heard of |
| L5 | **The truck** | A burnt-out supply truck on the verge, crates spilled empty; a red hand on the door if crews robbed it; scratches on the cab if a pack took it | A supply truck lost (supply lines; tribes' trucks taken as data) | The verge where it was lost, 6–10 m off the road | 2 season days; at the cure the epilogue takes the last 4 | *"A Bastion truck on its side in the verge, stencil just readable: MORTON. The crates are empty."* | ★ Mechanic: *"Shot through the radiator. It stopped here on purpose."* (robbed) or *"The doors are open. Nobody shot at anything."* (taken by a pack) |
| L6 | **Bastion fading** | A sandbag half ring and a *"CHECKPOINT. HALT."* sign on its side where a road post stood; new Bastion marks thin out | Bastion's road posts (the last 6 places, saved); Bastion presence under 0.25 stops new Bastion door writes outside its holdings | Where a road post stood, on the verge | 2 season days a relic; the thinning lasts the season | *"Sandbags on the verge and a HALT sign on its side. Nobody's stood here for days."* | ★ Soldier: *"They pulled the post. They took the kettle."* |
| L7 | **Red hands spreading** | More hands and tallies on doors round the crews | Each season day while Red Hand strength is 50+: one door in a town within 1.5 km of a Red Hand holding or road post. Under 35: hands get struck through (nobody's collecting) | Shops and garages first | As door marks | *"Red hands on three doors in this street. One of them has six strokes beside it."* | ★ Ex-Con: the tallies as debts |
| L8 | **Halcyon's beaks** | Beaks on doors round the lab and its field station, and at nests it's sampling | Halcyon's sample jobs and a nest under study | As door marks | As door marks | *"A plague doctor stencilled on the barn door, with a ring round it. Underneath: PLEASE DO NOT BURN."* | ★ Lab Technician: the code |

### Player traces

| # | Trace | You see; props | Trigger | Where | Lasts, ages | Text | Reads more |
|---|---|---|---|---|---|---|---|
| P1 | **A town you emptied** | Ticked bears and ring-and-dot chalk on the houses people came out of; the built season mark on the road in | The town log's `rescue` (people brought in alive) | The houses nearest the job's site | Door marks as the grammar; the season mark as built | *"A bear on the door with a tick through it, and an arrow: → L."* | ★ Elder: whose house, by a seeded name |
| P2 | **Ash where the bodies were burned** | A black patch, a burnt pallet, a count chalked on a post | Burn the Bodies, a nest burned, or Clear the Trail's bodies burned | Where the job burned them | 2 season days | *"A black patch in the grass and a count on the post: 9. D27."* | ★ Police Officer: *"Somebody counted before they lit it. Good."* |
| P3 | **A grave** | Two fence palings and a mound; a name or a description in biro | A notable death (a life of 2 hours or more, or a death the town log recorded: a mission, the stand, a pack) and someone to dig: a Haven house within 3 km, or The Digger stranger finished there | At the place of death, if 100 m from another grave and 150 m from a scene and a safehouse; else on the verge of the nearest road out, 150–300 m on; else text only | 2 season days; one per identity, 8 on the server; at the cure the epilogue's "Their dead" takes over | *"Two fence palings and a name in biro: VERA. MECHANIC. DAY 23."* (or *"someone who'd been a mechanic"*) | ★ Elder: what the house said at supper |
| P3b | **The memorial candles** | A candle jar on the step of every safehouse | A Haven Champion dies (Remembered) | Each safehouse's door | 1 season day, at least 4 hours played (the epilogue design's rule, built here) | *"Candles on the step. The house read a name at supper."* | — |
| P4 | **A candle near where you died** | A chalk candle with a description on the nearest door | As P3's notable death, when no grave is dug, in a town | The nearest marked or unmarked door within 80 m | As door marks (chalk) | *"A candle in chalk, and small underneath: 'someone who'd been a hunter. D31.'"* | ★ Elder |
| P5 | **The town log, made physical** | The built season mark, with a look by its kind: a sheet on two posts (rescue), a board (burned), a stencil board (cleared, taken), tape between posts (sealed), flowers on a post (emptied) | The town log (built: salience 3+) | As built: the verge of the road in | As built | The built line | — |

**Names** follow [your dead run](../docs/INFECTED.md#your-dead-run): the description by default, the name only
on **Named**, with the player's opt-in and the name filter; one entry per identity in a town; never in a
worst-slot trace; never a player who caused a death or a fire.

### Sound and light

| # | What | How | Where and when | Text | Reads more |
|---|---|---|---|---|---|
| S1 | **Lanterns where people live** | A lit lantern prefab by the door of a lived-in place; camp life's table lamp swapped for its lit twin | The 4 lived-in places nearest players, 20:30–05:00 (from 18:30 in fog or rain). Never in a town nobody holds | — | — |
| S2 | **Lit windows** (spike) | Client-side point lights 1.5 m inside the outside doors of lived-in buildings, from a replicated list | As S1. Only if the spike shows light through the windows and not through the walls ([feasibility](#sound-and-light-feasibility)) | — | — |
| S3 | **A radio in an empty house** | A looping hiss on a static radio prop; once, within 12 m, the broadcast's current line as a subtitle | One per vignette that has one (V1, F5), 2 on the server; the batteries die after 3 season days (Overrun for F5) | *"A radio somewhere in the house: '...Remain indoors. Aid is on its way.'"* | ★ Radio Operator: *"It's on the government frequency. Somebody left it on so the house wouldn't be quiet."* |
| S4 | **Dogs, then nothing** | A one-shot barking sound at a town edge; it stops dead | 5 minutes before a pack's hunt sets out, in a town within 1.5 km of the lair on the night's first trail, one town a pack a night | Players within 800 m. A ★ Hunter within 800 m gets *"The dogs in Levie have stopped."* | — |
| S5 | **A generator's hum** | A looping hum on camp life's Halcyon generator piece | At a field station and the lab grounds while camp life dresses them; quieter at night | — | — |
| S6 | **One stroke of the bell** | One bell stroke at `day4.church` | At 18:30 on an evening a Remembered name is read (P3b); 10 heat at the church, never at night | *"One stroke of a church bell, from the direction of {canon_town:day4.church.town}."* | — |
| S7 | **The town loudspeaker** | A chime and static from a loudspeaker pole piece at a Bastion stronghold or outpost; the line as a subtitle | 05:30 and 17:30, within 300 m | Presence 0.25+: *"Remain indoors. Relief is on its way."* Under 0.25: *"Remain indoors."* After the cure: silent | ★ Soldier: *"The man reading it has stopped believing it."* |
| S8 | **Jars on the church step** | The F4 candle jars lit | Dusk to 22:00 on evenings with S6, and the first two evenings of V3's chair | — | — |

The tribes keep their call sounds and the dusk call ([INFECTED.md](../docs/INFECTED.md#signs-and-sounds)); Traces
adds no infected sound.

### The Long Table: absence

The house is never described, and no ambient line hints at it ([WRITING.md](../docs/WRITING.md)). Traces
shows it only by what isn't there:
- **No marks within 2 km of the farm** after Day 3: no Haven chalk, no Red Hand paint, no Halcyon beaks, no
  scratches. Bastion's sweep never reached it (Days 2–5 stay 1.5 km clear, as territory does).
- **No scenes and no vignettes within 2 km** (the same ring errands' strangers keep).
- **No candles** for anyone the house took: takings never reach the town log, the graves or the candles.
- The built tells (the boots, the guestbook, the tracks turning at the old wall) belong to the Long Table and
  the tribes, unchanged.

A player who walks every village notices that one stretch of country has no writing on any door. Nothing says
why. ([Decision 5](#decisions-for-the-author) has the one option the author may add.)

### The line

| # | What | You see; props | Trigger | Where | Lasts | Text |
|---|---|---|---|---|---|---|
| N1 | **Crosses** | Fence-paling crosses in a row, up to 7 | Seeded: 2 at the north road (*"D4"*, *"D6"*). Live: +1 for each death in the kill zone within 1 km (the world memory's death signal), 7 at most a place | On the verge just outside the warning zone, 320–380 m from the edge, at the north road and at the other checkpoints where someone died | All season (never in the warning zone; nobody can go nearer) | *"Seven crosses of fence palings. No names on most. One says 'TRIED. D4.'"* |
| N2 | **The Day 1 sheets** | Sheets of paper on the verge and in hedges, rain-soaked | Always, at the north road and each sealed checkpoint | 350–700 m back from the barrier, on the verges ([decision 3](#decisions-for-the-author)) | All season | Reader: *"BY ORDER. THIS AREA IS UNDER QUARANTINE. REMAIN INDOORS. DO NOT APPROACH THE PERIMETER. D1."* The same words as the broadcast, Day 1 |
| N3 | **The loudspeaker at the line** | Already the Perimeter's voice. Traces adds nothing on the line itself: nothing is built along the edge but the checkpoints | — | — | — | — |
| N4 | **The epilogue's queue** | The epilogue's (people on the verge 150–250 m from the barrier) | The cure | In front of F1 | The week | The epilogue's |

## How things age

Every trace has a made day (season days, the season clock), so its age is worked out when it's shown: nothing
ticks, and a restart changes nothing.

| Stage | Door marks | Scenes | What changes |
|---|---|---|---|
| **Fresh** | Under 1 season day | First third of its life | Crisp paint, wet chalk; the Hunter says *"today"* |
| **Weathered** | 1–5 season days (chalk 1–3) | Middle third | Faded look (the decal's second step); the text adds *"rained on"*, *"faded"* |
| **Old** | Over 5 season days (chalk over 3) | Last third | Ghost look; scenes drop pieces (a suitcase gone, a chair fallen) |
| **Gone** | Chalk at 8 season days with no rewrite; a fourth layer pushes the oldest off | End of life | Removed from the data |

- **Rain** (0.3 or more, the environment's reading) ages chalk twice as fast while it lasts. Paint and stencils
  don't wash. Scratches don't fade, they grey.
- **Rewrites.** The Haven re-chalks a mark while the fact still holds (the drop at a working pump, a candle's
  new number), which resets its age.
- **First-days scenes** age by outbreak stage, not days: Early, Collapse, Overrun, Aftermath each drop or change
  a piece, so a long season and a short one look the same at the same point in the story.
- **After the cure** nothing new is written except what the epilogue asks for; everything carries on ageing.
- Props show age by variant (a fresh and a faded board) when the author sets both; with one prefab, only the
  text ages. Decals take the age as their alpha and their atlas step.

## Placement on any map

| Anchor | How it's found | Rules | Fallback |
|---|---|---|---|
| **A door** | `RZ_BuildingQuery.FindBuildings` in the town radius, `RZ_DoorGeometry.FindDoors` and `IsOuterDoor`; the mark at the door's wall, 0.6–1 m beside the leaf, 1.2–1.8 m up (`OutsidePoint`, `WallNormal`, `DoorwayCentre`) | Never on the door leaf or in its swing; never in a camp life ring; never inside a running mission's 25 m | A building with no doors found: the nearest one that has them; none in the town: text only |
| **The verge** | `RZ_Marks.Verge` (built: 6–10 m off the nearest road's centre line within 80 m) | 25 m from a junction, nothing else within 3 m | No road within 80 m: the anchor point itself, on dry land |
| **A town's edge** | The settlement radius, on the side toward a named thing (a pack's range, the nearest lived-in place) | Flat ground (camp life's slope test), off roads | The radius on the side of the town nearest the map's centre |
| **A landmark** | `RZ_LandmarkFinder` kinds: *school*, *church*, *pharmacy*, *fuel*, *well*, *monument*, *station* | Not a locked building's footprint; not a running mission's | The next kind on the scene's list, then the largest building |
| **A canon spot** | `CanonPos` keys (`day2.checkpoint`, `redhand.crossroads`, `origin.site`, the new keys above) | As the scene | As each key's rule |
| **Pieces round a centre** | Camp life's placer, extracted as `RZ_DressPlacer`: rings by role, flat ground, outside building footprints (2 m), apart from each other | 6 m from a road's centre line, 4 m from any quartermaster or medic | A piece with no good spot after ten tries is left out |

- **Never** on `origin.clinic`'s clinic, within 300 m of `origin.site` or the lower store, or on any building a
  questline stage uses while it runs: those are clues' ground.
- **Never** in the warning zone or the kill zone; never within 60 m of a dressed place (camp life) except a
  door mark on the safehouse itself; never within 600 m of a lair; never within 2 km of the Long Table's farm;
  never within 300 m of a running mission or 600 m of the evening's set piece (`IsQuietAt`) when it's placed;
  never within 1 km of an active questline clue of the same faction.
- **Island maps:** the column waited on the slipway (LORE). F1 and F2 follow the road to the canon spot,
  whatever it is; with no checkpoint, `day2.checkpoint` is the Haven's safehouse and F1, F2 and N1 aren't placed.
- **No school or church:** F3 and F4 fall back as their rules say; with no building at all, the scene is its
  approach line at the town's centre.

## Caps and performance budget

**No bodies.** Traces spawns no AI and asks nothing of the one body budget.

| What | Budget |
|---|---|
| Town history | 8 events a town, saved (about 10 KB at most) |
| Door marks, data | Seeded ones aren't saved. Live ones: 600 on the server, saved (about 24 KB). 6 doors a town (8 in a stronghold's town), 3 layers a door |
| Door marks, drawn | **Decal path** (recommended): the server keeps a replicated list of marks on doors within 80 m of any player, at most 48 entries (about 1.5 KB), rebuilt on the 5 s tick only when it changes; each client draws decals for doors within 70 m of itself. **Board path** (fallback): a `DOORS` pool in `RZ_Marks`, 24 props on the server, 8 a player cluster, spawned within 60 m, removed past 90 m |
| Scenes, data | First days: 8, seeded, not saved. Vignettes: 12 at once (3 dealt a season day). Live traces: 24. Player traces: 16. Saved: about 6 KB |
| Scene props | A `TRACES` pool in `RZ_Marks`: 40 props on the server, 12 a player cluster, 3 spawned per 5 s tick, within 150 m, removed past 200 m, up at least 10 minutes. A scene is 1–6 pieces; a piece is one prefab of at most 12 children. At most 3 scenes with props within 300 m |
| Lights | 12 on the server, 20:30–05:00, within 200 m of a player |
| Loops | 4 on the server (radio, generator), within 80 m, removed past 120 m |
| One-shot sounds | 4 at once (dogs, the bell, the loudspeaker), on top of the tribes' 6 |
| Lookups | A town's doors resolved once a run (one sphere query, at most 64 buildings and their door children), when a player first comes within 400 m; one town per tick. Reading checks only players inside a town's radius, against its marked doors |
| Ticks | Writes are event-driven. The 5 s marks tick spawns and removes. Vignettes are dealt on the 30 s errands tick. Age is worked out when shown |

`RZ_Marks` needs three small changes, shared with everyone who uses it: a yaw and position given by the caller
(for walls and verges facing the road), a per-cluster cap (the epilogue design wants it too), and the two new
pools. Measure in the runtime test: the status line reads
`Traces: doors N (drawn M), scenes S, props P/40, lights L, loops O`.

## Data model and owners

### What feeds it (all built)

| Built system | What Traces reads or hears |
|---|---|
| World facts (`RZ_WorldFacts`) | The season seed; canon (`day2.checkpoint`, `redhand.crossroads`, `origin.site`, `origin.clinic`, the new keys); flags (`haven.rule`, `table.exposed`); `TownOwner`, `IsLivedIn`, `IsNearLivedIn`, `Day()`, `IsDusk`, `IsNight`, `PerimeterSealed`, `IsCured`, `BastionPresence` |
| World memory | The town log (P1, P5, V2); signals (deaths for N1, P3, P4; fires; trucks); reasons (`house.hungry`, `house.bitten`, `town.infested`) |
| Town memory, fire | Burned buildings with time and place (L2); Barricaded (V2); Powered |
| Settlements | Towns, centres, radii, size |
| Territory | Holdings, founds, takes, abandons (L3, V7, L7); road posts' places (L6); strength |
| Safehouses | Where, people, tier, `m_iDied`, emptied (bears, candles) |
| Interactions | Locks (*"SEAL"*), wells and pumps (the drop, V8), building types (shop, clinic, garage, farm) |
| The Director, crawlers | Stage (first days' ageing), town infection (scratches, smears), nests |
| Tribes | Kill sites (L1), named young and where they came from (L4), ranges and lairs (exclusions), the night's hunt and first trail (S4), the column's pack (F1) |
| Missions | Finished and failed jobs by kind, through a small `m_eTrace` on `RZ_Mission` (or a table by mission class in the writers) |
| Errands | Found hooks (old marks), The Digger (P3) |
| Perimeter | Checkpoints and the edge (N1, N2), the warning distance |
| Supply lines | Where a truck was lost and how (L5) |
| The Long Table | The farm's position (the exclusion) |
| Backgrounds, reputation | The reader's background and access rank |

### New classes

| Class | File | Does |
|---|---|---|
| `RZ_Traces` | `World/Traces/RZ_Traces.c` | The owner: tick, save, settings, status line; reached as `gm.GetTraces()` |
| `RZ_TraceSettings` | same | **World > Traces** (attributes, no initialisers; made with `RZ_Util.NewWithDefaults`) |
| `RZ_TraceKit` | `RZ_TraceKit.c` | Every optional prefab, one attribute each; the epilogue's optional props read the same kit |
| `RZ_TownHistory` | `RZ_TownHistory.c` | Per town: up to 8 events (kind, faction, day, count, a point), saved |
| `RZ_TraceWriters` | `RZ_TraceWriters.c` | One-line hooks called by the owning systems, as `RZ_WorldWriters` |
| `RZ_FirstDays` | `RZ_FirstDays.c` | Seeds Days 1–10; the new canon rules |
| `RZ_DoorMark`, `RZ_EDoorSymbol`, `RZ_EDoorAuthor` | `RZ_DoorMarks.c` | One layer: town, slot, symbol, author, day, fields (three small ints), flags (slash, ring, struck, overpainted), made day |
| `RZ_DoorMarks` | same | The grammar: writes, layers, overpaint rules, pruning, reading text by level |
| `RZ_DoorSlots` | `RZ_DoorSlots.c` | Resolves a town's door slots (runtime cache) |
| `RZ_SceneDef`, `RZ_ScenePiece`, `RZ_Scene` | `RZ_Scenes.c` | The catalogue as data: anchor, pieces with roles and stage masks, lifespan, text keys |
| `RZ_Scenes` | same | Deals vignettes, places first days and traces, ages them, hands them to `RZ_Marks` |
| `RZ_DressPlacer` | `Factions/RZ_DressPlacer.c` | Camp life's placement, extracted and shared |
| `RZ_TraceText` | `RZ_TraceText.c` | Every line, by key and level, filled with `Fill` |
| `RZ_DoorMarkClient` | `UI/RZ_DoorMarkClient.c` | Client: reads the replicated list and draws decals (`World.CreateDecal` on the building), removes them out of range |
| `RZ_AmbientLoopComponent` | `World/Traces/RZ_TraceSounds.c` | On loop prefabs: plays a sound event on stream-in, stops it on delete (the pattern of `RZ_CallSoundComponent`) |
| `RZ_TraceLightsClient` | `UI/RZ_TraceLightsClient.c` | Only if S2's spike passes: client point lights from a replicated list |

Saved in `state.json` with the world facts: town history, live door marks, live scenes, the last road posts.
Not saved: seeded first days and their marks, props, the drawn list, door slots, lights, sounds.

## How it meets the other systems

### The epilogue

- **Handover at the commit.** Traces' in-season graves come down; the epilogue's "Their dead" places its own
  from the record. The last 4 lost-truck wrecks pass to the epilogue with their places. V7's fallen posts pass
  to the epilogue's outpost boards.
- **Inputs for beats.** Traces gives the snapshot a census per town (candles and their numbers, bears ringed or
  ticked, hands, beaks, scratches), so a town beat can say *"four candles on the doors in Brod, and one bear"*.
- **Writes on request.** The epilogue's door chalk (*"GONE HOME: {n}"* at a safehouse in processing, a
  wanderer's name *"chalked on the last door they rested at"*) is written through `RZ_DoorMarks.Write`, so it
  follows the grammar and the caps. The epilogue's own marks stay free-standing, as its design says.
- **The north road.** F1 and F2 stay all season and through the week; the epilogue's queue, the Names board and
  the paper's table stand nearer the barrier. Traces places nothing new there after the cure.
- **One kit.** The epilogue's optional props (sign board, sheet on posts, stencil board, candle, candle in a jar,
  grave marker, tape between posts, truck wreck) are the same prefabs as Traces' kit.

### Errands

- **The old mark.** The found hook's *"old mark"* asks `RZ_DoorMarks.OldMarkNear(pos)`: an existing mark within
  60 m if there is one, else a two-hour chalk or stencil written in the grammar for the faction that runs near
  there. Its text is the door line at the reader's level. The built random lines go (one of them calls an X the
  Haven's, which the grammar contradicts).
- **Anchors for cards.** Designed, unbuilt cards can anchor on traces: Boarded Up on V2, Is She Gone? near L1,
  His Effects on L5 or V7 (a driver's papers, a soldier's tags). Traces never deals an errand; errands may read
  traces.
- **Strangers.** The Digger finishing makes a grave (P3). A fresh event's spot may be a live trace's place.

### Tribes

Tribes own their signs. Traces reads them: kill sites (L1), named young and where they came from (L4), the
night's hunt (S4), lairs and fear places (no scenes within 600 m of a lair), the column's pack (F1's
scratches). Scratches written by Traces are in towns, on doors; the tribes' marks are at a range's road edge.

### Camp life

- Camp life owns everything in a lived-in place's rings. Traces adds only the patched bear (a door mark), the
  lantern at night (S1) and the loudspeaker sound (S7) to its pieces.
- Every Traces prop goes through `RZ_DressingGuard` and is marked as dressing, so the interaction scan, the loot
  spots and the landmark finder ignore it. The guard learns to allow Traces' look-only components
  (`RZ_AmbientLoopComponent`, a decal root).
- `RZ_DressPlacer` is extracted from camp life for both.

### Tone, missions, questlines and the majors

- Tone's ambient notice stands at holdings' and safehouses' gates; F6's layered notices only in towns nobody
  lives in. Neither reuses a clue's text.
- Traces never places within a running mission's 300 m; missions' props always win a spot.
- The majors' crossroads props are theirs; F7 is only the shutter. On a set piece's evening Traces places
  nothing new within 600 m.
- Flags may vary a line (at most two conditions a line, as Tone's rule), never a trace's existence.

## Abuse rules

| Rule | How |
|---|---|
| **Nothing lootable** | Every prop through the dressing guard (no storage, no pick-up item, no supplies, fuel, vehicle, spawn point, arsenal or action). Car wrecks are static meshes: no Search the boot, no stripping, never a fixable car. Sample cases are empty and closed. Readables are text in the reader, never items |
| **Nothing that blocks** | Props 6 m or more from a road's centre line; door marks beside the door, never on the leaf or in its swing; planks on windows, never across a doorway; no prop inside a locked building's blocker zone; small footprints with no navmesh obstacle where possible (a VERIFY); a scene doesn't spawn while a vehicle or player is within 8 m of its spot |
| **No griefing through marks** | Players never write a mark. Marks come only from server data. A player's deed can cause one trace a town a season day at most, and nothing a player does can make a trace about another player |
| **No names used against people** | The naming rules above: descriptions by default; names only on Named with the player's opt-in and the filter; never in a worst-slot trace; never naming whoever caused a death or a fire |
| **No positions given away** | Nothing marks where a player is or was in the last 2 played hours, except their own death (P3, P4), after the fact. Props stay up 10 minutes and appear at 150 m, so pop-in doesn't track anyone |
| **No loot hints** | No mark says what's inside a building except people and danger. *"SEAL"* repeats what the door already says |
| **Nothing pays** | Reading, finding or standing near a trace pays no Fame, Favor or items. The one tie is a pack becoming Heard of |
| **No farming** | Writes are capped per town and per server a season day; vignettes per town; a vignette's trigger can't be repeated by the same player in a season day |
| **True when lived-in** | Marks at a lived-in place are kept current; anything older is dated |

## Sound and light: feasibility

Engine APIs found in `/root/base_src`:

| API | Where | Use |
|---|---|---|
| `World.CreateDecal(entity, origin, project, near, far, angle, size, stretch, material, lifetime, color, atlasNum)`, `CreateDecal2(entity, matrix, ...)`, `BaseWorld.RemoveDecal(decal)` | `core/scripts/GameLib/generated/World/World.c`, `Core/generated/World/BaseWorld.c` | Door marks drawn on each client. Lifetime 0 makes a static decal and returns it for removal. The colour's alpha can carry age; `atlasNum` picks the symbol from one atlas |
| `DecalEntity`, `WorldDecal` | `core/scripts/GameLib/generated/Entities/DecalEntity.c`, `GameLib/entities/Decal.c` | A decal as an entity, for the server-spawned fallback |
| `SCR_EditableDecalComponent` | `data/scripts/Game/Editor/Components/EditableEntity/` | Game Master's pattern: a replicated root with a child `DecalEntity` prefab, re-created to move |
| `SCR_BaseInteractiveLightComponent` (`ELightState.LIT_ON_SPAWN`; `ToggleLight` is local; state in `RplSave`/`RplLoad`) | `data/scripts/Game/Lights/` | Lanterns and candle jars: a "lit" prefab variant lights on every client by itself; no toggling needed |
| `LightEntity.CreateLight(type, flags, radius, color, LV, position, direction)` | `core/scripts/GameLib/generated/Entities/LightEntity.c` | S2's client lights |
| `BaseSoundComponent.PlayStr`, `SoundComponent.SoundEvent`, `Terminate(handle)`, `IsPlaying` | `core/scripts/GameLib/generated/Components/BaseSoundComponent.c` | Loops and one-shots |
| `RadioBroadcastComponent.EnableRadio` | `data/scripts/Game/generated/Components/RadioBroadcastComponent.c` | Vanilla radio music: not used (S3 plays a hiss, [decision 12](#decisions-for-the-author)) |
| `RZ_CallSoundComponent` (built) | `Zombies/RZ_TribeProps.c` | One-shots: dogs, the bell, the loudspeaker |
| The replicated list pattern (built) | `gm.GetLockedBuildings()` | The drawn door-mark list and S2's list |

**VERIFY points** (each goes on the VERIFY list in `WORKBENCH_SETUP.md` when built):

| # | Check | Fallback |
|---|---|---|
| 1 | `World.CreateDecal` on a static building entity from a client script projects onto its wall at a door, and is removed with `RemoveDecal` | The board path ([below](#decals-or-boards)) |
| 2 | The client finds the same building from a replicated position (a small sphere query) | Send the door's wall point and normal instead of a building key |
| 3 | A decal atlas material with 16 cells and alpha from the colour works with `atlasNum` | One material per symbol and age |
| 4 | A runtime-spawned "lit" lantern prefab lights on clients that stream it in (initial state) | No lantern; S1 is a line at dusk |
| 5 | S2: a point light 1.5 m inside a door shows at windows and doesn't bleed through walls | Drop S2; keep S1 |
| 6 | A looping sound event started on stream-in plays until the entity is deleted, and stops with it | One-shots every 30 s |
| 7 | Static wreck, sandbag and plank props spawned at run time don't cut the navmesh in a way that traps AI on a verge | Smaller props; no wrecks in F1, text only |
| 8 | The dressing guard passes every Traces prefab with no `Dressing: removed` line | A plain mesh copy |
| 9 | The bell one-shot at a church carries to 1.2 km | Shorter reach; text to players within 1.2 km |

### Decals or boards

| Path | Looks | Cost | Risk |
|---|---|---|---|
| **Decals** (recommended, after the spike) | Real paint on real doors; ages by alpha | One material atlas and a client script; no entities, no replication beyond one short list | VERIFY 1–3 |
| **Boards** | A plank, a card or a sheet leaned or nailed beside the door, with the symbol painted on it as a decal on our own prop | One prop per door in the `DOORS` pool; replicated entities | Looks propped-on; more props |
| **Text only** | Nothing to see; the door line still comes when you stop there | None | Players won't think to stop at doors |

Ship text first (phase 1), spike decals in Workbench, then choose. Boards are the fallback, never both.

## What the author builds in Workbench

All optional: every slot empty means text only. Build in `Prefabs/ReforgerZ/Traces/`. Rules as camp life's
compositions: an empty root with an `RplComponent`, vanilla props as children, nothing with physics, no
storage, no ammo boxes, supply boxes, arsenals, fuel, wells, pumps or radio stations with actions
([WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#camp-life-the-compositions-and-kits)). Check the log for
`Dressing: removed` after placing each once.

**Door marks**

| Asset | What | Notes |
|---|---|---|
| `RZ_DoorMarks_atlas.edds` and `RZ_DoorMarks.emat` | A 4 × 4 atlas: X, slash, post code, C, ATT, CLEARED, bear, patched bear, candle, hatching, ring-and-dot, arrow, drop, red hand, tally, beak; a second atlas for the overlays (ring, tick, strike, PAID, scratches outside, scratches inside, smears, thumbprint, grey overpaint) | Drawn in the UI's art direction (hand-made survivor marks, [UI.md](../docs/UI.md)); fields stay text |
| `RZ_Trace_DoorBoard_*.et` (board path only) | A plywood offcut, a cardboard sheet, a sheet of paper (`Notice_A4_Vertical_V1.et`), each with the symbol | One per author: Bastion, Haven, Red Hand, Halcyon, Residents |

**Scenes** (vanilla props by type; pick static meshes)

| Composition | Props | Scene |
|---|---|---|
| `RZ_Trace_QueueCar_A`, `_B`, `_C` | A static civilian car wreck (hatchback, saloon, estate), doors open; a suitcase on the roof or the grass | F1 |
| `RZ_Trace_Suitcases` | Three suitcases, a holdall, a folded coat | F2 |
| `RZ_Trace_GymDoor` | Stacked camp beds, folded blankets, a painted board *"GYM OPEN"* | F3 |
| `RZ_Trace_Bus` | A static bus wreck | F3 |
| `RZ_Trace_ChurchDoor` | A painted board, a kitchen chair, a folded blanket | F4 |
| `RZ_Trace_CandleJars` | 3–5 jam jars with candles; a lit twin with the light component on `LIT_ON_SPAWN` | F4, S8, P3b |
| `RZ_Trace_Roadblock_D1` | `Sandbag_Barricade_USSR_04.et` or loose sandbags, a barrier pole on the ground, a camp chair, empty tins, a field radio mesh on `CrateWooden_01_plain.et` | F5 |
| `RZ_Trace_NoticeWall` | A board on two posts with 5 `Notice_A4_Vertical_V1.et` sheets overlapping | F6 |
| `RZ_Trace_Shutter` | A garage door or shutter mesh on two posts (only where no garage stands) | F7 |
| `RZ_Trace_OP` | A groundsheet, a ration tin, a fence post | F8 |
| `RZ_Trace_LastMeal` | `PicnicTable_01` or a garden table, 4 chairs, plates, a jug, a radio mesh | V1 |
| `RZ_Trace_HeldOut` | Planks (static, not the barricade item) for windows, one hanging off a frame, casings | V2 |
| `RZ_Trace_EdgeChair` | A kitchen chair, a blanket, a jam jar | V3 |
| `RZ_Trace_IOUs` | Sheets pinned to a board, a tin on `CrateWooden_01_plain.et` | V4 |
| `RZ_Trace_DroppedRun` | A closed sample case mesh, a gas mask with a long filter, a bicycle on its side | V5 |
| `RZ_Trace_WaitingRoom` | Six plastic chairs in a row, a sign on an easel | V6 |
| `RZ_Trace_FallenPost` | The roster board from `RZ_Dress_RosterBoard`, a cold `BarrelMetal_01_rust.et`, a few sandbags | V7 |
| `RZ_Trace_WaterQueue` | Buckets and plastic water bottles (meshes, never jerrycans) | V8 |
| `RZ_Trace_Scorch` | An ash pile; a scorch decal for the wall | L2 |
| `RZ_Trace_TruckWreck_Bastion`, `_RedHand`, `_Haven`, `_Halcyon` | A static burnt truck wreck, two empty crates | L5 |
| `RZ_Trace_RoadPostRelic` | Sandbags in a half ring, a *"CHECKPOINT. HALT."* sign on its side | L6 |
| `RZ_Trace_Pyre` | A black patch decal, a burnt pallet, a burnt tyre, a post | P2 |
| `RZ_Trace_Grave` | Two fence palings and a mound | P3 |
| `RZ_SeasonMark_Sheet`, `_Board`, `_Stencil`, `_Tape`, `_Flowers` | A sheet on two posts; a board; a stencilled board; tape between two posts; flowers on a post | P5 (the built season mark's look by kind) |
| `RZ_Trace_Crosses` | One fence-paling cross (placed up to 7 times) | N1 |
| `RZ_Trace_Sheets` | Two or three paper sheets on the ground (a decal or a flat mesh) | N2 |

**Sound and light**

| Prefab | What | Use |
|---|---|---|
| `RZ_Trace_Lantern_Lit` | `LampKerosene_01_green.et`'s lit twin (light on `LIT_ON_SPAWN`, no switch action) | S1 |
| `RZ_Sound_RadioHiss` | A static radio mesh with a `SoundComponent` and `RZ_AmbientLoopComponent` on a looped static event | S3 |
| `RZ_Sound_Dogs`, `RZ_Sound_Bell`, `RZ_Sound_Loudspeaker` | `RZ_CallSoundComponent` one-shots | S4, S6, S7 |
| The Halcyon generator piece | `RZ_AmbientLoopComponent` with a hum event added to `RZ_Dress_Generator` | S5 |
| `RZ_Dress_Loudspeaker` | A pole with a horn speaker, for Bastion's camp life kit | S7 |

About 40 prefabs and two atlases. Plan three evenings, after the decal spike (one evening).

## Settings, commands and log lines

**World > Traces** (`RZ_TraceSettings`): Enabled (on); Door marks (on); Doors a town (6); Layers a door (3);
Live writes a season day (20); First days (on); Vignettes (on), at once (12), a season day (3); Live traces
(on), at once (24); Player traces (on); Sound and light (on); Lit windows (off until the spike); Props on the
server (40), a cluster (12); Door drawing (Decals, Boards, Text); Debug (off). Names follow **Zombies > Tribes
> Your dead run**. Prefabs in the kit.

**Admin commands:** `/traces` (every trace and door within 300 m, with its source fact), `/traces town <name>`
(the town's history and doors), `/traces write <symbol>` (debug only, at the nearest door), `/traces clear
<town>`.

**Log lines:** `Traces: first days seeded (gym <town>, church <town>, roadblock <grid>, N towns swept)`,
`Traces: door <town>#<slot> <author> <symbol> D<day> (<cause>)`, `Traces: overpaint <town>#<slot> by
<author>`, `Traces: scene <key> at <grid> (<trigger>)`, `Traces: scene <key> gone (<why>)`, `Traces: no
anchor for <key> in <town>`, `Traces: prop budget full (<cluster>)`.

## Writing the traces

Every line follows [WRITING.md](../docs/WRITING.md). For traces in particular:
- **Channel:** "Notices and boards": the printed line, then whoever wrote on it. Approach lines are one plain
  sentence and one detail. Door lines at Read and Know level are the writer's voice, briefly.
- **Never:** "alpha", "tribe" or "crawler"; gore; a child hurt or a child's body; pronouns for seeded people;
  any hint at the Long Table; a joke at a victim. The Haven never writes "taken", and never puts "walked out"
  and "taken" in one line.
- **Numbers** appear only where something written shows them (a tally, a candle's number, a code).
- **Dates** always: *"D17"* on the mark, *"Day 17"* in prose.
- **Clue echo:** no trace repeats a questline clue's text (the lint's `clue-echo`). F5's log, F6's orders and
  Brod's clinic X stay inside what LORE says out loud.
- **Lint:** the new pools go into `tools/strings_inventory.py` as channel `trace` (and `door` for door lines by
  level); `tools/voice_lint.py` runs `length` (200), `bang` (none), `token`, `never`, `long-table`, `infected`,
  `pronoun`, `clue-echo`, `punchline`.

Samples:
- Glance: *"An orange X on the door, with writing in its corners. Day 3."*
- Read (Bastion): *"Bastion's search code. Day 3, 14:20, section 2B. Infected inside. One living sent to the
  gym, one dead."*
- Know (★ Police Officer): *"The ring round it means someone came back two days later to look again. You don't
  do that unless you're looking for someone."*
- Glance: *"A bear in white paint, with a chalk ring round it and an arrow: → M."* Read (Haven): *"Children were
  here. They went to Morton."*
- Glance: *"A candle in chalk, and a 3 under it."* Know (★ Elder): *"Under it, small: Hana, Josef, Vilma."*
- Glance: *"Hatching across the door frame, like a gate. D29."* Know (★ Nurse): *"Somebody here said it out loud
  at supper, and was walked out at dusk."*
- Read (Red Hand): *"A red hand and six strokes. Six weeks paid."* Know (★ Ex-Con): *"A thumbprint low on the
  frame. Someone the Haven put out sleeps here, and the crews know where."*
- Read (Halcyon): *"S-031, day 17, three samples. The lab's series."*
- ★ Hunter, scratches: *"Outside, at hand height. Three of them, one tall. A week old."*

## Phases and rough sizes

Each phase is playable, ends with `tools/compile_check.sh` and the runtime test, and ships with its docs
(`docs/TRACES.md`, DESIGN.md's link, WORKBENCH_SETUP.md's prefab list and VERIFY count, ROADMAP, README).

| Phase | Builds | Rough size |
|---|---|---|
| **0. Spike** (Workbench) | VERIFY 1–8: a decal at a door from a client, a lit lantern, a loop, a static wreck through the guard and the navmesh | 1–2 evenings, ~150 lines throwaway |
| **1. History and words** | `RZ_TownHistory` and the writers; `RZ_FirstDays` and the new canon; the grammar generator, layers and overpaint as data; reading by level and background; the town summary and door lines; the journal's Read on doors; errands' old mark through the grammar; P5's text by kind; the admin commands | ~1,800 lines |
| **2. Doors you can see** | The drawn list and `RZ_DoorMarkClient` (or the board path); ageing looks; the atlas | ~700 lines, the atlases |
| **3. The first days and the line** | `RZ_DressPlacer` extracted; `RZ_Marks`' yaw, cluster cap and pools; F1–F8, N1, N2 | ~1,100 lines, 14 compositions |
| **4. Vignettes and live traces** | The scene dealer; V1–V8; L1–L8 | ~1,300 lines, 16 compositions |
| **5. Player traces** | P1–P4, the memorial candles, the season mark's looks | ~500 lines, 7 prefabs |
| **6. Sound and light** | S1, S3–S8; S2 if its spike passed | ~600 lines, 2 components, 6 prefabs |
| **7. Epilogue handover** | The census for the snapshot, the graves and wrecks handed over, writes on request (with the epilogue's phase 2) | ~200 lines |

About 6,200 lines in all, and three to four evenings in Workbench.

## Risks

| Risk | Mitigation |
|---|---|
| Decals don't project or replicate the way the API suggests | Phase 0 spike before phase 2; the board path; text always works |
| Clutter: every town covered in symbols | 6 doors a town, most towns 0–3; writes capped a day; marks age out; sparse is a principle |
| Players never stop at doors | The town summary on entry; the journal; F-scenes on roads people use |
| Stale marks read as current | Every mark dated; lived-in places kept current; text says "faded" and "D17" |
| Pop-in | Spawn at 150 m (doors at 70 m are small), 10 minutes up; decals drawn client-side at 70 m |
| Server load from props | No bodies; pooled caps; decals cost the server one short string; measured in the runtime test |
| Map variety (no schools, churches, roads, checkpoints) | A fallback per anchor; text when nothing fits |
| Clashing with the epilogue, tribes, Tone, camp life | The ownership table; one kit; handovers at the commit |
| Long Table leaks | Absence only; takings never reach the log, graves or candles; the lint |
| Tone: children, walk-outs, real search codes | No child hurt or shown; bears mean children safe or moved; inside scratches never with a bear; the X has fields of its own; the walk-out is hatching, never words |
| Interaction scans read props as wrecks, wells or stoves | Every prop marked as dressing; the scans already skip dressing |
| Navmesh cut by runtime props | VERIFY 7; small props off the paths |
| Save growth | About 40 KB at most; seeded content isn't saved |
| Text volume | Pools by key and level, the lint, two flag conditions a line at most |

## Decisions for the author

| # | Decision | Options | Recommendation |
|---|---|---|---|
| 1 | How door marks look | Decals drawn on clients; boards beside the door; text only | **Text first, spike decals, then decals**; boards only if the spike fails |
| 2 | Where the design lives | A section in DESIGN.md; a new doc | **A new doc, `docs/TRACES.md`** (it owns a grammar and a catalogue, like INFECTED.md); the Doc map and README updated |
| 3 | Where the Day 1 sheets near the line came from | Dropped from the drones on Day 1 (the outside acts once); Bastion's cordon detail handing out Command's order | **Bastion's sheets with Command's words**: the outside stays silent but for the broadcast, as LORE says |
| 4 | Candles where a town was rescued | A candle means the dead, only; candles for rescues too | **One meaning per symbol: the candle is the dead.** A rescue shows as ticked bears, arrows and the built sheet |
| 5 | The Long Table | Absence only; absence plus the Haven's Day 3 chalk arrow toward the farm at the last village, never rewritten | **Absence only**, to keep WRITING's rule that no ambient line hints at it |
| 6 | Scratches at a ringed bear (the young come home, LORE's line) | Yes, with the rules; no | **Yes**: only outside, only at a ringed bear (the children are safe), never at a lived-in house, the Hunter's line never says who |
| 7 | Names on in-season graves and candles | Follow Your dead run; a setting of their own | **Follow Your dead run** (Described by default) |
| 8 | The bell at the Day 4 church | One stroke for a Remembered name; none | **One stroke, 10 heat, at 18:30, never at night** |
| 9 | Lit windows (S2) and the map's own lamps | Spike both; lanterns only | **Lanterns first; spike S2; leave the map's lamps alone** (toggling them needs a client script per lamp and isn't needed) |
| 10 | Scenes indoors (a table laid inside, cots in the gym) | Outdoor and doorway versions; indoor on loot's furniture spots | **Outdoor and doorway by default**; indoor later behind a setting, after a play-test |
| 11 | The column's cars | On the verge; on the road | **On the verge**, 6 m off the centre line: nothing blocks a truck |
| 12 | What the radio in an empty house plays | A hiss and the broadcast as a subtitle; vanilla radio music | **The hiss and the broadcast line**: music would be a different world |
| 13 | First-days places each season | Re-seeded each season (a new Zone); fixed per map | **Re-seeded**: each season is a new outbreak ([LORE.md](../docs/LORE.md#endings)) |
| 14 | Does reading pay | Nothing; a small Fame | **Nothing**; only a pack Heard of from L4 |
| 15 | Errands' old mark | Through the grammar; keep the random lines | **Through the grammar**, which also fixes the line that calls an X the Haven's |
| 16 | The Red Hand and the bears | The crews leave bears alone; they paint over everything | **They leave the bears**: half the crews have family in a safehouse, said sideways |
