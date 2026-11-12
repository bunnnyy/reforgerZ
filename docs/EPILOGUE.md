# The epilogue week

After the cure the world stays open for a week (**Season > Epilogue hours**, 168 real hours; the
author's decision 12) and tells the season: on boards at the places it's about, in the queue at the
north road, in what leaders, residents, wanderers and the radio say. No slides and no new screens.
Every story still open at the cure can end inside the week, live if someone plays it, as data if
nobody can. Nothing in the week pays anything that carries into next season except hall-of-fame
lines. The cure itself (the finale, the release, its terms) is [FINALE.md](FINALE.md).

Code: `Director/Epilogue/` (the controller `RZ_Epilogue.c`, saved with the season; the beats, the
outlets, the places and the closing work in their own files; the fuller record and the beats in
full in `RZ_EpilogueRecord.c`; the standing versions, the moving scenes, the newcomer, the leaders'
topics and the closing work's rest in `RZ_EpilogueRest.c`). Settings: **Director > Epilogue**
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#5-settings-objects)).

## Contents

- [The week at a glance](#the-week-at-a-glance)
- [At the cure](#at-the-cure)
- [The world settles](#the-world-settles)
- [Beats](#beats)
- [Carriers](#carriers)
- [The dose outlets](#the-dose-outlets)
- [Places and scenes](#places-and-scenes)
- [The grey choices](#the-grey-choices)
- [The closing work](#the-closing-work)
- [The hall of fame and the season block](#the-hall-of-fame-and-the-season-block)
- [Names](#names)
- [Edge cases](#edge-cases)
- [Admin tools and the test](#admin-tools-and-the-test)
- [Simplified, and not built yet](#simplified-and-not-built-yet)

## The week at a glance

Every clock is a share of the epilogue's length L (Epilogue hours × **Epilogue > Test scale**), so
an admin who sets a shorter week gets the same shape, squeezed.

| Phase | Runs | What changes |
|---|---|---|
| **A. The news** | From the commit to the first game dusk with a player online: at least an hour, at most L ÷ 28 | The cure line and everyone's first line; the queue forms (3 people); the outlets open; the closing work opens |
| **B. The days after** | Until processing | The evolving step moves at a third and two thirds of B (the marks' versions); the closing work's last calls; the queue at 6 |
| **C. Processing** | The last L ÷ 7, never longer than **Season > Warn hours**; earlier after an empty stretch | The built wipe warnings; everything still open closes as data; the queue walks at 05:30 and 17:30; the Walk Out; the Last Broadcast airs on the hour |

**Empty server ends early:** nobody online for 2L ÷ 7 real hours in phase B starts processing at the
next login (**Epilogue > Empty server ends early**). **Wipe Off:** processing only by hand
(`/epilogue phase C`). Under 24 epilogue hours (before Test scale) the closing work is off and only
the beats show.

## At the cure

In this order, from the cure's tick ([RZ_GameModeComponent.OnCureCompleted](DESIGN.md#cure-research-and-the-seasons-end)):

1. The finale reports its controller, how it was won, the terms and the deciders
   ([FINALE.md](FINALE.md#endings-and-next-season)).
2. **The snapshot** (the next tick): the route, how, terms and condition, the Fame leader, the
   deciders and the stand's names (frozen), the majors' story lines, each safehouse's people and
   tier, each holding's owner, every town with 6 or more crawlers, every pack still calling, every
   profile's name and up to three lines, and the wanted (Kill on Sight at the cure). **The hall of
   fame freezes** the same tick.
3. **The beats** are chosen (the tick after).
4. **The closing work is listed**, the story **commits** (`story.built`), a save is made, and
   everyone online gets *"The cure went to Bastion, who held the lab. Bastion issues it at the north
   road and its posts, against name and signature. And one firearm."*, their own first line, one
   beat near them and where to look. Players offline get theirs at their next spawn; later spawns
   get it once a session as *Word on the roads*. New faces: *"New face. You missed it. The queue's at
   the north road."*

A restart before the commit builds it again from the live world. **Your three lines**: your route
(and whether you chose its terms), a major you decided, and Champion or Wanted with a faction. Line 1
comes at the commit, line 2 from the first leader you ask about yourself (*Me*), line 3 from a
wanderer who remembers you (*"I know you. ..."*). **Wanted** means Kill on Sight for 4 or more hours
played in that life before the cure (`RZ_EpiRecord` counts it all season). **Titles**: a leader's
*Me* starts with the identity's best life this season with that faction (`RZ_RepSeason.TitleFor`)
and the town whose log holds it highest: *"Don't know the face. Know the name. The Good Hand from
Levie."* *Simplified:* the three lines don't yet read the deeds store's truths, a questline ending you
decided, a frozen hall record, your rescues or your dead characters.

**The new face's kitchen** (the newcomer's scenario): an identity with no record at the commit gets,
at its first spawn in the week before processing, Petr's note in the empty kitchen (*"Gone to the
queue at the north road. If you're new, go there too. Petr, the key's where it was. Bring it, would
you?"*), which deals the epilogue deck's The Key Where It Was. It spawns where the spawn screen puts
it, not at a kitchen table.

**The fuller record** (`RZ_EpiRecord`, all season, saved): per faction the trucks delivered and lost,
the outposts taken and lost, the leaders lost (a count and the last three names); per outpost its
owners this season (three, in order); the wanderers who died; each identity's longest life. Each
counter keeps the day it started: a counter that started after the season's first day is told without
its numbers. *The season* tab's *As the Zone tells it* lists the record after the cure line.

## The world settles

**Epilogue > The world settles** (on): `RZ_WorldFacts.HoldingsFrozen()` is true in the week. The
territory tick keeps only its floor (no town taken or dropped), a questline ending that would hand
over an outpost is words only and logged, and The Last Convoy doesn't start. Fights, trucks,
missions, questlines, the tribes and the crawlers carry on. Leaders' off-screen deaths (bites and
raids) come at a quarter of their rate.

## Beats

A beat is one fact about the season, told at its place: a mark (an **Epilogue > Board prefab** on
the verge, else an approach line to whoever passes within 40 m), echoed by people and the radio.
Chosen once at the commit, frozen with their names, and **checked again every time they're shown**:
a changed subject (a new leader in the seat, a holding's new owner, a pack gone quiet, the farm's
state) plays the beat's since version, or drops it. Log: `Epilogue: beat <key> at <place> (weight
W)`, `... dropped (<why>)`, `... since (<why>)`.

| Slot | Beats |
|---|---|
| The cure | The lab gate's mark (by route and terms) and the stand board: *"HELD THE LAB, DAY 46."* and up to 8 names, or its how version |
| Hubs | The queue (its sign by step: *"QUEUE STARTS HERE. STILL."*, the route's table, the names put down, a gap after a killing); the Names board; Bastion's roster board (*"NO REPLY"*, *"NET CLOSED"*); Rudi's crossroads (the price board under the boss's price, by step, else the lorry and the book); the Haven's supper board; one beat per safehouse (candles and the season's dead, the rota or beds in the yard, *GONE HOME* in processing, how the dose reaches the house) |
| Places | One per outpost (its owner's board); towns from the town log, best slot first, **Epilogue > Towns** at most; packs still calling (4); the Long Table only once it's exposed |
| People | The named dead (wanderers: a name chalked on the last door they rested at, 6); wanted posters (*"SETTLED. DAY 49."*, *"PAID"*, *"NOT PAID. THE INFECTED GOT THERE FIRST."*, *"WALKED OUT. NOT PAID."*); graves of notable players' dead characters (a life of 2 hours or more, not by their own hand; 12, 100 m apart, off safehouses) |
| Side stories | World records with an epilogue line (6), by importance, at their subject's town or house |
| Since the cure | A loose end closed live, a name brought home, a town taken back (a candle): **Epilogue > Since-the-cure beats** (20) |

**Traces' part** ([TRACES.md](TRACES.md#the-cure-and-the-epilogue)): a town beat ends with Traces' census
of its doors (*"Four candles on the doors in Brod, and one bear."*); a named wanderer's name is also
chalked as a candle on that safehouse's door, and a house's *GONE HOME* by its door, through the door
marks; at the cure Traces takes its own graves down (these beats are the graves now) and keeps the
last four lost trucks' wrecks for the week.

**The record's beats** (after the rest are chosen): **roads**, a wreck for each of the last four lost
trucks; **stations**, the transformer's notice naming the towns with lights (Town memory's
Powered); **characters**, the Kid's chalk and the Preacher's bell where they are, and a grave for the
leaders a faction lost (in its way: a jam jar of flowers, a rifle upside down, a page nailed to a post,
a numbered marker); **one beat a faction** at its stronghold (its major's line, else its season on a
board). **The places in full** add to their beat's text: an outpost's coats of paint, oldest
underneath (*"On the gate, a red hand, and under that, a Bastion stencil"*); a house's rescued-in
count, the house rule's door notice (`haven.rule`), bunting where the house grew; a town's mark by its
log's kind (a sheet on two posts for a rescue, Bastion's stencil for a shelling, tape for a quarantine,
a cross of palings for a burning or a mass grave, the riddle's note in a jar); the farm's states once
exposed (the chain, the boy's boot, the cleared table).

Weight: base × people (1 + 0.25 a named player, ×2 at most) × known (3 or more visitors, ×1.25) ×
**change** (×1.5: an outpost that changed hands, a house that grew, a town burned or shelled) ×
**choice** (×1.25: a beat someone chose, a side story). **Hubs** (the lab, the queue, each safehouse)
keep at most 3 beats within 60 m and the strongholds' boards 2; the weakest drop. **Notables**: up to
**Epilogue > Notables** (12) players are named in the world, scored (decided the route +5, a major +4
each to 8, held the stand +3, Champion +3 a faction to 6, Wanted +2, a crew name +1); each name stands
in at most 3 beats (2 on a server of fewer than 6), at least half the beats name nobody, and with
fewer than 24 identities with deeds at most half of them are named. The stand board and wanted posters
keep their names. **Placing**: the cure's, the lab's and the queue's marks first, then by weight, at
most 3 in any 300 m; the marks pool keeps its server cap (**World > Epilogue marks**) and **Epilogue >
Marks per cluster** (8 within 150 m). Each kind has its own optional prop (sheet, stencil, price board,
notice, grave, poster, trestle, crate, bunting, ribbon, wreck, tape, handcart, Names board), else the
board, else text. **Route marks** are the finale's ([FINALE.md](FINALE.md#the-lock)): the losing
routes' come down at the week's first dusk.

## Carriers

| Carrier | What it says |
|---|---|
| **Marks** | The beat's text, on a board or as an approach line ("By the road") |
| **Leaders** | On the talk screen: **About the season** (the stronghold leader's verdict on the route, in the faction's voice; elsewhere where the dose is), the place's beat, your own line, the house's dead; **Where should I look?** (two places you haven't met yet, no markers); a stronghold's grey choice. Without the screen, the same on `RZ_LeaderAction` (*Ask about the season*, *Ask where to look*, *Answer: the usual way / the other way*) |
| **Residents** | "Ask how things are": one line in the first person about the season, packing in processing |
| **Wanderers** | Road news by role (the trader's prices, the doctor's nearest dose, the deserter on the roll...) |
| **Open channels** | Half the chatter in the week is about the season, by faction and route |
| **The reel** | Each player's latest radio-station broadcast from before the cure (12, one a day first), one every 20 minutes to players with a radio, 3 a session; the anonymous skipped. In processing the station's fourth preset is *"This is Mara, at the north road. Going home."* |
| **Boards** | Pinned notes that don't count toward the 8: names brought home, *"If we put you out, come and eat."*, last notes in processing |
| **Readables** | 3 in 10 found in the week are dated in it (*"Gone to the queue at the north road. Took the dog."*), never naming the farm's town |
| **The journal** | The notebook's **The season** tab ([UI.md](UI.md#the-notebook)), `/season` or the notebook item's action: *As the Zone tells it* (the cure line, every beat you've met, all of them in processing), **And you**, **Unfinished** (open closing work, last calls, what you carry, your last goal); the tab also shows the four majors |

**Leaders' topics one by one** (after *Where should I look?*): *The cure*, *This place* (an outpost
commander or council head adds their own first-person line: the cook's, the sentry's, the toll man's
*"One tin, or one dose, whichever's cheaper this week."*), *Me* (your title and line), *The dead*,
and at a stronghold *What you didn't tell me*: the oldest truth from the truth deck nobody found
(*"Since it doesn't matter now: the job about the well wasn't what we said. It was bite. We knew."*),
once a player, found by "epilogue"; a Rogue leader lies again. Each is coloured by the leader: a Greedy
one prices it, a Merciful one asks if you've eaten, a Rogue commander says *"Don't tell the
stronghold I said this."*, and a losing route's people carry on with dignity (*"We stood on the lab
road. We lost. The book says we're square."*). **Residents**: half their lines in the week come from
the route that controls the cure (eight a route). **The radio**: the Haven channel answers the Last
Broadcast the evening after it airs (*"Mine's in a car on the north road."*). *Simplified:* the
topics are leader choices, not a dialogue tree; no secure-channel lines for Radio Operators; the reel
isn't spread one a season day. With **Dialogue > Trees** on, every leader also has "Ask about the cure."
and "Ask what they'll do now.", and the occupants a greet for the week ([DIALOGUE.md](DIALOGUE.md#the-last-week)).

### The evening calendar

After Last Rounds the week keeps three evening habits, as radio only, so none takes the set-piece
calendar's evening (`RZ_EpilogueCalendar.c`):

| Habit | When | What |
|---|---|---|
| **The supper reading** | 18:30 each game day, once the Gate has run | The chair reads six of the season's dead on the Haven's open channel, round the list (*"Supper, and the names. ... And the ones we never had names for."*), the ones the Haven put out too if the chair chose *All of them*; the house boards keep the names |
| **Bastion's 20:30 watch** | 20:30 each game day, once the last call has run, unless the CO closed the net | *"Bastion listening. Nothing heard. Bastion out."* on Bastion's open channel, and a stroke on the roster board (its beat counts them) |
| **Second airings** | The first evening (18:00–22:00) at least 8 real hours (× Test scale) after a coda ran, one a game day | The coda's epilogue line again, words only, on its faction's open channel |

**The supper reading as a scene**: from 18:00 to 20:00 the chair stands at the long table outside
the chair's house (*Listen to the names*: tonight's names, and in processing three Last Entries
read by the clerk); the radio line reads the same entries. **Standing versions**, from each coda's
evening to the wipe, so a lunchtime player still gets them: the List pinned on the chair's house board
with a clerk under it (*Ask the clerk to read it*); the last call's transcript on the roster board with
the tags back on the nail and *AT THE BARRIER* chalked over them (*Ask the operator to play it back*);
the case on a trestle by the lab gate (*Read the notice*, by `proof.first_page`); the lorry at the
crossroads with the book nailed open and a lad on a kitchen chair (*Ask the lad to read your line*).
**Last Rounds' route lines**, once each after its coda ran, on that coda's faction channel: the
Haven's *"If you can hear this: her name is ..."*, Bastion's four R numbers read for the record, the
readout's *"Reference stock: wanted."* (*logged* under Halcyon), the cook's *"Soup. It keeps."*
(at the lorry with her line paid). **How many evenings**: at the commit the week works out how many
game evenings phase B holds (its played hours over a game day's length); with fewer than four, the
second airings may share an evening.

## The dose outlets

Each route gets a real way to be treated, priced the way that faction would price it. **Given on the
spot** (never an item), **only to the bitten** (the antidote's own check), **one outlet dose per
life** from all outlets together, from a small stock refilled in real time (x Test scale): *"None
left. Come back in about 20 minutes."* Outlets read standing, never clothes.

| Route | Terms | Where | Price | Stock | Refused |
|---|---|---|---|---|---|
| The Haven | her house, and her boy | every safehouse of Community size or more: *Ask for the dose* | free at the chair's house; else 5 Haven Favor | 2, 1 an hour | exiles, unless the chair chose *All of them* |
| | the far house first | the same | free at the house farthest from the chair's | 2, 1 an hour | as above |
| | by the bite book | the same | 5 Haven Favor, or free with less | 2, 1 an hour | nobody |
| | the crossed-out first | the same | 5 Haven Favor | 2, 1 every 90 min | as her house |
| Bastion | the roster / the census | the queue's table and each Bastion holding: *Sign for treatment* | one firearm from your weapon slots, or 15 Bastion Favor | 3, 1 every 30 min | Hunted by Bastion |
| | the column | the same | as the roster | 3, 1 every 45 min | as above |
| | by need | the same | free | 3, 1 every 30 min | nobody |
| The Red Hand | the boss's price | the price board at Rudi's crossroads | 30 Red Hand Favor (45 below Known); 15 once the boss drops it; free in processing | 4, 1 every 20 min | Hunted by the Red Hand |
| | pay when you can | the board | 15 now, and a line in the book | 4 | as above |
| | the first pan to the houses | the nearest Haven house to the crossroads, and the board | free at the house; the board at the boss's price | 2 / 4 | as above |
| | the book first | the board | free, and a line in the book | 4 | as above |
| Halcyon | the authors / volunteers / numbers first | the lab gate's table: *Ask for a dose* | 10 Halcyon Favor; free for the Trusted (authors), a trial dose this life (volunteers), a bite under 20 minutes old (numbers) | 3, 1 every 30 min | Hunted by Halcyon |
| | published | the lab, and every house with a medic: *Ask for a home batch* | 10 Favor; a home batch cures 4 times in 5, else halves the time left | 3 / 2 | as above |
| Independent | open | a crate at the queue: *Take one* | free, once per identity for the week | 3, 1 an hour | nobody |
| | one side | that faction's outlet under its stronghold leader's terms, changed by the condition | the exiles served; Bastion 15 Favor, no firearm; the Red Hand one tin for all; Halcyon's home batches free at every house | | |
| | sold | the crate | 30 Favor of the faction you have most with; it goes nowhere | 3, 1 an hour | nobody |
| | kept back | none | | | |
| Nobody | handed over, the slow cure | the Fame leader's faction's row, under its stronghold leader's terms (the chair: the bite book if Merciful or Idealist) | | | |

**Her house** is free too (the woman's house, any size, under her terms); Bastion's **census** reads
your name and background back with the dose (*"Next."*); at Bastion's and the crews' tables a player
below Known is **searched** (armed: *"Weapons on the table first."*); the **first pan** is at the cook's
sister's house while she lives; the **open crate** shows the names of whoever reached it first,
chalked on the lid; errand card 41, What the Cure Costs Here, offers the way this table allows
([ERRANDS.md](ERRANDS.md#the-epilogue-deck)). *Simplified:* the column has no Day 2 slips served first.

## Places and scenes

Each place is data; when a player is within 150 m (a scene 300 m) a person (Puzzles > Civilian, in
the body budget's epilogue share) or a table (**Epilogue > Table prefab**, else Puzzles > Object)
stands there, tagged so its choices come to the epilogue; it's removed past 220 m (400 m), and never
saved. At most 30 up at once, 2 a tick.

| Place | Where | Its choices |
|---|---|---|
| **The queue** | 200 m up the north road from the barrier (`day2.checkpoint`); with no checkpoint, at the Haven's stronghold house | *Bring a cup* (one food item, once a real day, +1 Haven Fame), *Put my name down for the walk* (phase B), *Leave a car for the queue* (a vehicle within 25 m), *Leave what you found for the queue* (an Ex-Con's last goal) |
| **The route's table** | beside the queue | the queue's outlet; under the Red Hand in processing, *Buy a place near the front* (15 Favor) |
| **The Names board** | the bend, 150 m up | read it (the wall of names, the newest 12 last entries); *Write your line* |
| **The barrier** | 30 m from the north road's barrier, processing only | *Give your name* (the Walk Out) |
| **The lab gate** | 20-30 m from the lab | the director's line and the stand board; the lab's outlet; *Hand in the notebook* |
| **Rudi's crossroads** | canon `redhand.crossroads` | the price board's outlet; *Pay into the book*; *Hand in a dead crewman's lighter* |
| **Each safehouse** | by the house | its outlet; *Bring a name home*; *Read the names at supper* |
| **Bastion's nail** | each Bastion holding | its outlet; *Hang the tags on the nail* |
| **The closing work's places** | keepsakes lying out, Loose Ends' people, the radio station, the farm's handcart | [below](#the-closing-work) |

**Safe ground** (the queue, its table, the barrier, the lab gate): within 80 m, firing with no infected
within 40 m costs 2 Infamy with the scene's faction (the route's at its table, Halcyon at the lab
gate, else the Haven) and a grudge, once every 20 s; firing within 40 m of its people costs 4 Haven
Infamy; killing one costs 15 Haven Infamy and 5 with the route's faction, and the queue stays one
short, with *"A gap in the queue, and a bag on the verge nobody's touched."*, until the next step.

**Scenes that move**: before processing one more person joins the queue from the houses every two real
hours (6 at most); in processing the queue picks up its bags and shuffles forward at each change of
shift (06:00, 14:00, 22:00), told to everyone within 300 m, one shorter each time; each person in it
has a line of their own. The lab gate's group stands by route (two Haven medics with a cold box, a
Bastion group by stencilled crates, two of the crew, a researcher behind a table). **Clearance**: a
car within 8 m of a scene slides it 30 m along the verge (VERIFY: the car query finds parked
vehicles). **Scenes with bodies**: at most **Epilogue > Scenes that last** (2) and **Evening scenes**
(1, the supper reading) at once; the rest stand as boards. **The Long Table in the queue** (gone to the
queue): three more at the back with a covered basket and the mother's grace over a bowl she doesn't
share; *Talk the boy out of the queue* (the Kid on the roads, else a child at the nearest house);
*Tell the corporal* or *Tell the chair* (`table.told` 1 or 2). **Holdouts**: Carry the Dose's holdout
stands in the doorway while the box is on its way. **More places**: the ridges below each pack still
calling (four), a resident at each town taken back who'd like to see the kitchen once more, the
houses' *Sit with someone who's bitten*, the crossroads' *Buy a keepsake back (10 Favor)*, and with no
north-road checkpoint a barrier at the land edge nearest the chair's house (none on an island map:
every bearing meets water; VERIFY: past the terrain's edge the ground reads as water). *Simplified:*
nobody walks along the road or escorts the queue; the queue and the gate change by count and text.

## The grey choices

Each stronghold leader asks one question once a season; whoever answers starts a 5-second hold,
and any other player within 30 m can object once (picking the other answer), which takes it off the
table. Effects come out of the epilogue's lane (morale ±5 per faction for the week, inside the
majors' lane); the chooser gets 2 Fame with that faction; everyone hears what was decided.

| Leader | The question | The other answer |
|---|---|---|
| The chair | *"Whose names do we read tonight?"* | **All of them**, the exiles too: Haven morale −3, every board pins *"If we put you out, come and eat."*, exiles served at Haven outlets |
| The CO (once the last call has run) | *"The 20:30 watch. Do we keep listening?"* | **Close the net**: Bastion −4, the roster board's *NET CLOSED* |
| The boss (only under the boss's price) | *"The price. Leave it to the market?"* | **Drop it now** (Red Hand Trusted): the board to one tin at once; Red Hand −3, Haven +2 |
| The director (only with something taken alive in isolation) | *"Do we keep it for the paper?"* | **Let it go quiet**: Halcyon −2 |

## The closing work

The **closure set** (decision 12). Items that can close on their own are read on the questline tick
against live signals: the **room left** (real hours to processing × the share of hours with someone
online, the majors' reading) against the item's **need** plus the needs of its giver's other open
items. Short of room: a **last call** in the giver's voice (a headline), a **grace** of one session of
play (an hour × Test scale), then the **default**, as data. An item someone touched in the last two
hours waits. **Processing is the hard stop.** Log: `Epilogue: last call <item>`, `Epilogue: closed
<item> (live|default)`.

| Item | Built as | How it always finishes |
|---|---|---|
| **Last Rounds** | The majors' codas ([QUESTLINES.md](QUESTLINES.md#the-majors)) on the set-piece calendar; listed as items, closed live when a coda runs | The majors' own hard stop runs any coda left at processing as data ([DESIGN.md](DESIGN.md#the-majors)) |
| **Loose Ends** | Every questline run open at the cure: a closing stage at its place, its person with two or three ways, each ending on one of its built endings ([QUESTLINES.md](QUESTLINES.md#loose-ends)); half a stage's pay (Fame 3, Favor 5) to whoever chose | Its default ending as data after the last call, no pay, said plainly; questlines with no row end as data at processing |
| **Bring Them Home** | Named dead people (wanderers; players' characters with a life of 2 hours or more) leave a keepsake where they died: *Take* it (carried as data, dropped where you fall), hand it in at its home: the nail (Bastion), the book (the Red Hand), the lab gate (Halcyon), a house (everyone else). Fame 1 and Favor 5 with the house's faction, 3 paid a real day; your own or your group's dead for nothing; the dead you killed refused (+3 Infamy). The Names board lists each with where it went home | Open to the wipe; at the wipe what nobody carried is listed *"NOT BROUGHT HOME"* |
| **Quiet the Ridges** | The tribes' last week ([INFECTED.md](INFECTED.md#the-last-week)); the item watches the packs calling at the cure | Closes when the last of them is gone; the hall line names the last to call |
| **Take Back a Town** | A town with 6 or more crawlers at the cure and none now (crawlers don't refill in the week): a candle beat, the town log's best slot (*taken back*, salience 3), word from the houses; who was there is kept | Each town is its own small finish |
| **The Long Table's last week** | Only if it was never exposed. Found out as built closes it live. At processing the family packs: a handcart at the farm with covered baskets (*Look at the basket*: +15 suspicion, once each); at the next 05:30 (or 6 hours as data) they go to the queue, *"At the back, people with covered baskets."* | Found, or gone to the queue |
| **The Walk Out** | Processing only, at the north road only, outward, one name at a time: an unarmed person who walks up hears *"Name?"* instead of being turned back (the Perimeter's change). *Give your name* (2 hours into this life, once per identity a season, nothing infected within 40 m) retires the character: no death, no grave, no journal, the inventory gone; back to the spawn screen (VERIFY). A wanted poster becomes *WALKED OUT. NOT PAID.* The queue walks at 05:30 and 17:30; each house chalks *GONE HOME* | Names put down in phase B are read at the wipe, and someone in the queue answers for them |
| **Settle Accounts** | The parley, reopened at the cure ([REPUTATION.md](REPUTATION.md)); *Pay into the book* at the crossroads (15 Red Hand Favor: your line marked paid, 5 Infamy worked off through `Forgive()`); wanted posters change when a life leaves Kill on Sight | **Square** with all four (Infamy under 15 with each, no unpaid line) is checked at processing and at the wipe |
| **The Last Broadcast** | At the radio station: everyone who did any closing work has one say (*the names*, *the three weeks* with `weeks.public`, *the truth* with two pages in the open, *keep it plain*); a Radio Operator's counts twice in the tally shown | The tally closes at processing (ties and nobody: the plain line); it airs on every open channel on the hour and in full on the last dusk; morale from the lane (the names: Haven +2; the three weeks: Halcyon −2, Haven +1; the truth: everyone −1, the Red Hand −2) |
| **Personal Goals** | A life's goal in the week is its background's **last goal**: Nurse (be there for someone's dose, or two cups), Mechanic (a car at the queue), Hunter (a name home, or a cup), Police Officer (a name home), Ex-Con (leave something at the queue), Lab Technician (a notebook into the record, or a say), Radio Operator (a say), Unemployed (two pieces of closing work) | As built goals pay: a Legacy point and 10 Haven Fame, once per identity and background |
| **The Last Entry** | At the Names board, *Write your line*: one per identity, rewritable to the wipe; a pad writes *"Day 52. Still here."* (each press the next line), a keyboard `/lastentry <text>` at the board (80 characters, filtered); signed with your name, *a survivor* if anonymous. In processing the Haven's clerk reads three at supper | Open to the wipe; with **Archive last entries** on, an admin picks up to 5 for the season block |

**The closing work's rest** (`RZ_EpilogueRest.c`):
- **Bring Them Home's other keepsakes**, added once when the closing work opens: the ID card of every
  named young a pack still runs with (never a player's character), below its ground; a driver's papers
  from each of the last four lost trucks a faction lost. The fence at the crossroads sells a keepsake
  nobody brought home back to you (10 Red Hand Favor): it's yours to carry home. The epilogue deck's
  The Tags and The Wreck's Name bring more ([ERRANDS.md](ERRANDS.md#the-epilogue-deck)).
- **Quiet the Ridges, the factions' jobs**, below each pack still calling (four): *Cull them, for
  Bastion* (Known: the contact logged for the post, 5 Bastion Favor), *Take the last reading, for
  Halcyon* (at dusk, 18:00–21:30: the paper's appendix, 5 Halcyon Favor), *Bet on which calls last*
  (15 Red Hand Favor, in the book), *Sit and listen to the last call*; once an identity a pack a job;
  up to three names in the hall line (*"Sat the ridges out to the last call: ..."*).
- **Take Back a Town's giver**: at each town taken back, a resident who'd like to see the kitchen once
  more (*Walk them to their kitchen*, once an identity a town); up to three names in the hall line;
  a candle only while fewer than 8 towns carry one; the houses' boards show *CANDLES* with the count.
- **The Walk Out's order** on processing's first real day (× Test scale), by the route's terms: Bastion's
  roster (Known with Bastion), the houses' order (Known with the Haven), the crew's people near the
  front with the next shift (Known with the Red Hand, or a line paid), the authors (Known with Halcyon);
  nobody's cure has no order; from then on, anyone.
- **Settle Accounts at a leader**: *Settle accounts*, once a faction in the week: hand back their kit
  if you're passing in it (5 Infamy off), else own up to what's owed (up to 10 Infamy off through
  `Forgive()`), else a goodbye.
- **Personal Goals**: the last five backgrounds' last goals: Soldier (sit out a pack's last call, or a
  name home), Doctor (two doses witnessed, or sit with someone bitten), Raider (pay into the book, or
  buy a keepsake back), Elder (hear the names at supper), Drifter (put your name down, or walk someone
  to their kitchen); the Nurse's goal takes *Sit with someone who's bitten* too.

*Simplified:* Loose Ends pay the fixed half stage and keep their endings' built world effects, with no
600 m quiet for a set piece; the Long Table's basket isn't part of C1's proof and C1 and C2 have no last
stages at the farm and the queue; the Walk Out has no escorts up the road, no One Last Toll on it and
nobody stays behind by name; Settle Accounts doesn't carry borrowed food back or read promises; the
Last Broadcast has no lobbying, airtime for sale or price for a say; the Hunter's, Ex-Con's and Police
Officer's goals don't read their goal's town or truth.

## The hall of fame and the season block

**The hall freezes** at the cure: no record counts after it, lives running stop counting. The week
adds only lines to the season block, at the event or at the wipe, each under the names policy:
each Loose End's ending, *"Brought home: 9 names. The first on the wall: ..."*, *"The ridges went
quiet on Day 52. The last to call was the lot at ..."*, *"Towns taken back after the cure: 3 (...)"*,
*"Walked out: Mara, Day 52, ... and 4 more."*, *"Left the Zone square with all four: ..."*, *"The last
broadcast: the names."*, *"5 finished what they came for."*, archived last entries, the majors'
"The season's stories", "At the barrier" and "On the paper"
([QUESTLINES.md](QUESTLINES.md#the-seasons-lines-and-the-season-tab)), the ridges' and the kitchens'
names, and on the Unsigned last day *"The cure: nobody. Nobody won."* The names in *Walked out* and
*Left the Zone square* carry their coda's epithet (*"Novak, at the barrier"*, *"... on the paper"*).
They're shown with the season in the hall's notice and menu, and archived with it at the wipe.

## Names

**Frozen at the deed** (the name played under then), **filtered** (Haven > Note filter: a name the
filter would change shows as the player's title), **opt-out**: `/anonymous` or the notebook's *My name
in the world* keeps a player out of the world's text, the reel and the hall (*"a survivor"*), stored by
identity in `hall_of_fame.json` across seasons. One helper (`RZ_Names.Public`) gives every broadcast
and every name a system keeps to show later (sample deliveries, a claimed bounty, *"has turned"*, town
memory, boards' notes, the radio station's signature, headlines, records, traces): *"a survivor"* for
a player who chose `/anonymous` or `/mydead hidden`, or whose name the **Name filter** catches. A
live bounty's notice and the bounty contract name the hunted on purpose, as does the hungry house that
sells an exile's name. **Epilogue > Names**: Full, Titles only (*"the Haven's
Champion"*), Off (*"someone"*). `/epilogue strike <name or beat>` shows that name as a title, drops a
beat, or removes a last entry; logged.

## Edge cases

| Case | What happens |
|---|---|
| **No cure: the Unsigned last day** | When an After-weeks wipe is under Warn hours away with no cure, `IsLastDay()` is true (`IsEpilogue()` isn't: the cure can still come). The record is built without a route, its beats show, and chalk tallies stand 150 m up from each barrier. No closing work, no outlets, no Walk Out. The hall: *"Nobody won."* A cure that day rebuilds the record with a route |
| **No stand in the last hours** | **Season > No stand in the last hours** (off), for fixed restart days: with a timed wipe closer than **Minimum epilogue hours** and no cure yet, the final stand doesn't start (*"The director won't start the synthesis this late."*), so the season ends on the Unsigned last day instead of the wipe being pushed back |
| **A late cure** | A cure under **Season > Minimum epilogue hours** (56) before an After-weeks wipe pushes the wipe back to the cure plus the minimum, and says so (also in *whichever comes first*) |
| **A restart mid-week** | The record, beats, items, names, entries, outlet stocks, choices and the phase are saved; marks, people and tables come back near players |
| **Shipping mid-season** | The season counters (a house's dead and rescued this season, the fuller record) start when this ships; a record counter that started late is told without numbers. Graves come from the named dead recorded since then |
| **A death in the week** | A notable life (2 hours or more, not by their own hand) leaves a fresh grave by the road (*"... THIS WEEK."*), a since-the-cure beat; with 6 graves standing, the least notable one goes |
| **The Unsigned last day in full** | Every two real hours (× Test scale) the broadcast's long-stall line, *"Day 52. The perimeter will be maintained. This message will repeat."*, alongside the wipe warnings; the majors' no-cure codas ([QUESTLINES.md](QUESTLINES.md#the-majors)) |

## Admin tools and the test

`/epilogue` (admins): `status`, `phase A|B|C`, `strike <name or beat>`, `rebuild` (or, before the
cure, `preview`: the beats it would pick, in the log), `close <item>` (as data now), `lastnotes
<name>`. Players: `/season`, `/anonymous [off]`, `/lastentry <text>`; the notebook's two actions do the
first two without typing (and the notebook's The season tab). At the wipe, `last_season.json` also keeps
the Last Broadcast's version (`broadcast`) and the week's 10 strongest beats as plain lines (`beats`).
Log lines: [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check). The runtime
test's last phase runs the week at Test scale 0.02
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#runtime-test-without-workbench)).

## Simplified, and not built yet

*Simplified* (marked above): the three lines' sources, the moving scenes by count and text (no
escorts, nobody walks the road), the column's slips, leaders' topics as choices, Loose Ends' half pay
as a fixed reward and their endings' world effects as built (not at half), Personal Goals on epilogue
deeds, the Long Table's family in the queue as words and a head count, the closing work's rest as
listed in its paragraph. Marks keep 25 m from junctions and off the warning zone through the marks
pool's own verge search, not the road network.

**The epilogue deck**, the last errands on errands' dealer, is
[ERRANDS.md](ERRANDS.md#the-epilogue-deck)'s: 20 cards built.

**Writing** follows [WRITING.md](WRITING.md); the week's lines are in `RZ_EpilogueText.c`,
`RZ_EpilogueRest.c` and `RZ_EpilogueRecord.c`. **Risks to watch in play-tests** are the roadmap's
([ROADMAP.md](ROADMAP.md#play-test-priorities)).
