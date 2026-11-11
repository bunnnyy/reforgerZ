# Environmental storytelling: the world's traces

**Status: designed; phases 1, 2 and 5 built.** Approved by the author on 2026-11-11 with all 16 of its
recommended decisions ([below](#decisions-as-decided)). Built and described in
[docs/TRACES.md](../docs/TRACES.md): door marks (the grammar, the first days, live writes, reading, the
journal, errands' old mark), doors you can see (decals or boards) and the player traces (P1 to P5). This
file holds what's left: the first days' scenes and the line (phase 3), vignettes and live traces (phase
4), sound and light (phase 6), the epilogue's handover (phase 7), and the few door-mark rules that wait
on them. Code goes in `scripts/Game/ReforgerZ/World/Traces/`.

## Contents

- [In short](#in-short)
- [Principles](#principles)
- [One owner per thing](#one-owner-per-thing)
- [Door marks: what's left](#door-marks-whats-left)
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
- [Phases and rough sizes](#phases-and-rough-sizes)
- [Risks](#risks)
- [Decisions as decided](#decisions-as-decided)

---

## In short

The Zone already remembers a lot as data: who held a town, who died in a house, where a truck burned, where a
pack took someone. Traces makes some of that visible, near players only, and never leaves anything behind.

What players will see:
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
- **Sound and light.** Lanterns only where people live, a radio hissing in an empty house, dogs barking and
  then going quiet before a hunt, a generator's hum at a field station, one stroke of the church bell, the
  town loudspeaker at shift change.
- **The line.** Crosses of fence palings outside the warning zone, Day 1 sheets blown about near the north
  road, and later the epilogue's queue in front of the column's cars.
- **The Long Table: nothing.** Its only trace is an absence.

Doors that talk and your own traces are built ([docs/TRACES.md](../docs/TRACES.md)). No cut-scenes, no
markers, no new screen. Nothing pays. A player who stops at a door learns something; a
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

## Door marks: what's left

The grammar is built ([docs/TRACES.md](../docs/TRACES.md#door-marks)). These rules wait on systems not built
yet:

| Rule | Waits on |
|---|---|
| **The thumbprint** (the Red Hand: a small red print low on the frame where someone the Haven put out sleeps; only an Ex-Con sees what it is). Written in a town nobody holds where an exile sleeps | A record of the wanderers the Haven turned out (none is kept today) |
| **The young come home**: fresh scratches outside a ringed bear, never at a lived-in house; the Hunter's line never says who; reading them makes the pack Heard of (decision 6) | L4 below |
| **Bastion fading**: under presence 0.25, no new Bastion door writes outside its holdings | L6 below |
| **Writes on request**: the epilogue's door chalk (*"GONE HOME: {n}"*, a wanderer's name on the last door they rested at) through the grammar's writer and caps; the census per town (candles and their numbers, bears ringed or ticked, hands, beaks, scratches) for the epilogue's town beats | Phase 7 |
| **The Digger** finishing makes a grave (P3) | The Digger stranger |

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
- **No marks within 2 km of the farm** after Day 3 (built: [docs/TRACES.md](../docs/TRACES.md#the-long-table-absence)).
- **No scenes and no vignettes within 2 km** (the same ring errands' strangers keep).
- **No candles or graves** for anyone the house took (built for the player traces).
- The built tells (the boots, the guestbook, the tracks turning at the old wall) belong to the Long Table and
  the tribes, unchanged.

A player who walks every village notices that one stretch of country has no writing on any door. Nothing says
why ([decision 5](#decisions-as-decided): absence only).

### The line

| # | What | You see; props | Trigger | Where | Lasts | Text |
|---|---|---|---|---|---|---|
| N1 | **Crosses** | Fence-paling crosses in a row, up to 7 | Seeded: 2 at the north road (*"D4"*, *"D6"*). Live: +1 for each death in the kill zone within 1 km (the world memory's death signal), 7 at most a place | On the verge just outside the warning zone, 320–380 m from the edge, at the north road and at the other checkpoints where someone died | All season (never in the warning zone; nobody can go nearer) | *"Seven crosses of fence palings. No names on most. One says 'TRIED. D4.'"* |
| N2 | **The Day 1 sheets** | Sheets of paper on the verge and in hedges, rain-soaked | Always, at the north road and each sealed checkpoint | 350–700 m back from the barrier, on the verges ([decision 3](#decisions-as-decided)) | All season | Reader: *"BY ORDER. THIS AREA IS UNDER QUARANTINE. REMAIN INDOORS. DO NOT APPROACH THE PERIMETER. D1."* The same words as the broadcast, Day 1 |
| N3 | **The loudspeaker at the line** | Already the Perimeter's voice. Traces adds nothing on the line itself: nothing is built along the edge but the checkpoints | — | — | — | — |
| N4 | **The epilogue's queue** | The epilogue's (people on the verge 150–250 m from the barrier) | The cure | In front of F1 | The week | The epilogue's |

## How things age

Door marks age as built ([docs/TRACES.md](../docs/TRACES.md#how-marks-age)). Scenes: every scene has a made
day (season days), so its age is worked out when it's shown: nothing ticks, and a restart changes nothing.

| Stage | Scenes | What changes |
|---|---|---|
| **Fresh** | First third of its life | As placed |
| **Weathered** | Middle third | The text adds *"rained on"*, *"faded"* |
| **Old** | Last third | Scenes drop pieces (a suitcase gone, a chair fallen) |
| **Gone** | End of life | Removed from the data |

- **First-days scenes** age by outbreak stage, not days: Early, Collapse, Overrun, Aftermath each drop or change
  a piece, so a long season and a short one look the same at the same point in the story.
- **After the cure** nothing new is placed except what the epilogue asks for; everything carries on ageing.
- Props show age by variant (a fresh and a faded board) when the author sets both; with one prefab, only the
  text ages.

## Placement on any map

| Anchor | How it's found | Rules | Fallback |
|---|---|---|---|
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
| Scenes, data | First days: 8, seeded, not saved. Vignettes: 12 at once (3 dealt a season day). Live traces: 24. Saved: about 6 KB |
| Scene props | The built `TRACES` pool in `RZ_Marks` (40 on the server, 12 a cluster, shared with the player traces), 3 spawned per 5 s tick. A scene is 1–6 pieces; a piece is one prefab of at most 12 children. At most 3 scenes with props within 300 m |
| Lights | 12 on the server, 20:30–05:00, within 200 m of a player |
| Loops | 4 on the server (radio, generator), within 80 m, removed past 120 m |
| One-shot sounds | 4 at once (dogs, the bell, the loudspeaker), on top of the tribes' 6 |
| Ticks | Vignettes are dealt on the 30 s errands tick. Age is worked out when shown |

`RZ_Marks`' facing given by the caller, its per-cluster cap and the two new pools are built
([docs/DESIGN.md](../docs/DESIGN.md#marks)). Measure in the runtime test with the built status line
(`Traces: doors N (drawn M), scenes S, props P/40, lights L, loops O`); its lights and loops are 0 until
phase 6.

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

Built: `RZ_Traces` and its settings and kit, `RZ_TownHistory`, `RZ_TraceWriters` and `RZ_TraceWatch`,
`RZ_FirstDays`, the door marks, `RZ_DoorSlots`, `RZ_TraceText`, `RZ_TraceDraw`, `RZ_PlayerTraces` and
`RZ_DoorMarkClient` ([docs/TRACES.md](../docs/TRACES.md)). Still to add:

| Class | File | Does |
|---|---|---|
| `RZ_SceneDef`, `RZ_ScenePiece`, `RZ_Scene` | `RZ_Scenes.c` | The catalogue as data: anchor, pieces with roles and stage masks, lifespan, text keys |
| `RZ_Scenes` | same | Deals vignettes, places first days and traces, ages them, hands them to `RZ_Marks` |
| `RZ_DressPlacer` | `Factions/RZ_DressPlacer.c` | Camp life's placement, extracted and shared |
| `RZ_AmbientLoopComponent` | `World/Traces/RZ_TraceSounds.c` | On loop prefabs: plays a sound event on stream-in, stops it on delete (the pattern of `RZ_CallSoundComponent`) |
| `RZ_TraceLightsClient` | `UI/RZ_TraceLightsClient.c` | Only if S2's spike passes: client point lights from a replicated list |

Saved in `state.json` with Traces' built save: live scenes, the last road posts. Not saved: the first days'
scenes, props, lights, sounds.

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

- **The old mark** is built through the grammar ([docs/TRACES.md](../docs/TRACES.md#errands-old-mark)).
- **Anchors for cards.** Designed, unbuilt cards can anchor on traces: Boarded Up on V2, Is She Gone? near L1,
  His Effects on L5 or V7 (a driver's papers, a soldier's tags). Traces never deals an errand; errands may read
  traces.
- **Strangers.** The Digger finishing makes a grave (P3). A fresh event's spot may be a live trace's place.

### Tribes

Tribes own their signs. Traces reads them: kill sites (L1), named young and where they came from (L4), the
night's hunt (S4), lairs and fear places (no scenes within 600 m of a lair), the column's pack (F1's
scratches). Scratches written by Traces are in towns, on doors; the tribes' marks are at a range's road edge.

### Camp life

- Camp life owns everything in a lived-in place's rings. Traces adds the lantern at night (S1) and the
  loudspeaker sound (S7) to its pieces (the patched bear on a safehouse's door is built).
- Every Traces prop goes through `RZ_DressingGuard` and is marked as dressing (built for the two pools). The guard
  still has to learn to allow Traces' look-only components (`RZ_AmbientLoopComponent`).
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
| `SCR_BaseInteractiveLightComponent` (`ELightState.LIT_ON_SPAWN`; `ToggleLight` is local; state in `RplSave`/`RplLoad`) | `data/scripts/Game/Lights/` | Lanterns and candle jars: a "lit" prefab variant lights on every client by itself; no toggling needed |
| `LightEntity.CreateLight(type, flags, radius, color, LV, position, direction)` | `core/scripts/GameLib/generated/Entities/LightEntity.c` | S2's client lights |
| `BaseSoundComponent.PlayStr`, `SoundComponent.SoundEvent`, `Terminate(handle)`, `IsPlaying` | `core/scripts/GameLib/generated/Components/BaseSoundComponent.c` | Loops and one-shots |
| `RadioBroadcastComponent.EnableRadio` | `data/scripts/Game/generated/Components/RadioBroadcastComponent.c` | Vanilla radio music: not used (S3 plays a hiss, [decision 12](#decisions-as-decided)) |
| `RZ_CallSoundComponent` (built) | `Zombies/RZ_TribeProps.c` | One-shots: dogs, the bell, the loudspeaker |
| The replicated list pattern (built) | `gm.GetDoorMarks()` | S2's list |

**VERIFY points** (each goes on the VERIFY list in `WORKBENCH_SETUP.md` when built; 1–3, the decals, are there
and in [the spike](../docs/WORKBENCH_SETUP.md#traces-the-workbench-spike-phase-0)):

| # | Check | Fallback |
|---|---|---|
| 4 | A runtime-spawned "lit" lantern prefab lights on clients that stream it in (initial state) | No lantern; S1 is a line at dusk |
| 5 | S2: a point light 1.5 m inside a door shows at windows and doesn't bleed through walls | Drop S2; keep S1 |
| 6 | A looping sound event started on stream-in plays until the entity is deleted, and stops with it | One-shots every 30 s |
| 7 | Static wreck, sandbag and plank props spawned at run time don't cut the navmesh in a way that traps AI on a verge | Smaller props; no wrecks in F1, text only |
| 8 | The dressing guard passes every Traces prefab with no `Dressing: removed` line | A plain mesh copy |
| 9 | The bell one-shot at a church carries to 1.2 km | Shorter reach; text to players within 1.2 km |

## What the author builds in Workbench

All optional: every slot empty means text only. The door marks' atlases and boards and the player traces'
props are already kit slots ([WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#4-the-prefab-manifest), *Traces*).
Build in `Prefabs/ReforgerZ/Traces/`. Rules as camp life's
compositions: an empty root with an `RplComponent`, vanilla props as children, nothing with physics, no
storage, no ammo boxes, supply boxes, arsenals, fuel, wells, pumps or radio stations with actions
([WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#camp-life-the-compositions-and-kits)). Check the log for
`Dressing: removed` after placing each once.

**Scenes** (vanilla props by type; pick static meshes)

| Composition | Props | Scene |
|---|---|---|
| `RZ_Trace_QueueCar_A`, `_B`, `_C` | A static civilian car wreck (hatchback, saloon, estate), doors open; a suitcase on the roof or the grass | F1 |
| `RZ_Trace_Suitcases` | Three suitcases, a holdall, a folded coat | F2 |
| `RZ_Trace_GymDoor` | Stacked camp beds, folded blankets, a painted board *"GYM OPEN"* | F3 |
| `RZ_Trace_Bus` | A static bus wreck | F3 |
| `RZ_Trace_ChurchDoor` | A painted board, a kitchen chair, a folded blanket | F4 |
| `RZ_Trace_CandleJars` | Built into the kit already (**Kit > Candle jars**, P3b); F4 and S8 use the same | F4, S8 |
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

About 30 prefabs. Plan two or three evenings, after the decal spike (one evening).

## Settings, commands and log lines

**World > Traces** (built, [docs/TRACES.md](../docs/TRACES.md)) gains: Vignettes (on), at once (12), a season day
(3); Live traces (on), at once (24); Sound and light (on); Lit windows (off until the spike); the scenes'
prefabs in the kit.

**Admin commands:** `/traces` and its [Admin] reply are built; they gain the scenes within 300 m.

**Log lines** to add: `Traces: scene <key> at <grid> (<trigger>)` and `Traces: scene <key> gone (<why>)` for
every scene (the player traces already write them), `Traces: no anchor for <key> in <town>`, `Traces: prop
budget full (<cluster>)`.

## Writing the scenes

Every line follows [WRITING.md](../docs/WRITING.md) (its row for Traces) and goes through the text lint as
channel `trace` (built). Approach lines are one plain sentence and one detail. No trace repeats a questline
clue's text: F5's log, F6's orders and the first patient's town stay inside what LORE says out loud.

## Phases and rough sizes

Each phase is playable, ends with `tools/compile_check.sh` and the runtime test, and ships with its docs
(`docs/TRACES.md`, DESIGN.md's link, WORKBENCH_SETUP.md's prefab list and VERIFY count, ROADMAP, README).

| Phase | Builds | Rough size |
|---|---|---|
| **0. Spike** (Workbench, the author's) | VERIFY 1–8: a decal at a door from a client, a lit lantern, a loop, a static wreck through the guard and the navmesh. The steps: [WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#traces-the-workbench-spike-phase-0) | 1–2 evenings |
| **1. History and words** | **Built** ([docs/TRACES.md](../docs/TRACES.md)) | — |
| **2. Doors you can see** | **Built**: the drawn list, `RZ_DoorMarkClient` and the board path; the atlases are the author's | The atlases |
| **3. The first days and the line** | `RZ_DressPlacer` extracted; F1–F8, N1, N2 (the canon spots they stand on are built) | ~1,000 lines, 14 compositions |
| **4. Vignettes and live traces** | The scene dealer; V1–V8; L1–L8 | ~1,300 lines, 16 compositions |
| **5. Player traces** | **Built** ([docs/TRACES.md](../docs/TRACES.md#player-traces)); the 7 prefabs are kit slots | The prefabs |
| **6. Sound and light** | S1, S3–S8; S2 if its spike passed | ~600 lines, 2 components, 6 prefabs |
| **7. Epilogue handover** | The census for the snapshot, the graves and wrecks handed over, writes on request (with the epilogue's phase 2) | ~200 lines |

About 3,100 lines left, and three evenings in Workbench.

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

## Decisions as decided

The author approved all 16 as recommended (2026-11-11). Built parts follow them already.

| # | Decision | Options | Decided |
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
