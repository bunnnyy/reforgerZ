# The epilogue: the week the Zone remembers

**Status: designed, not built.** Approved by the author on 2026-11-11.

## What changed in review

Conflict numbers are the plan's, counted from 1, as in [PLAN.md](PLAN.md). Decisions are numbered as in the
plan's "Decisions for you". The epilogue's own options are taken as recommended; the alternatives are gone.

- **The epilogue lasts one week, 168 real hours** (decision 12, changed by the author). Every phase and timer is a
  share of the epilogue's length, so the week works at any length an admin sets ([The week](#the-week-at-a-glance)).
  The 10-day pacing column is gone (conflict 49: the pacing is built), and so is the 72-hour plan.
- **Finishing everything** (decision 12). The week holds closing work that lets every story reach an ending inside
  it: Last Rounds, Loose Ends, Bring Them Home, Quiet the Ridges, Take Back a Town, the Long Table's last week, the
  Walk Out, Settle Accounts, the Last Broadcast, Personal Goals and the Last Entry
  ([Finishing everything](#finishing-everything)). None of it pays anything that carries into next season except
  hall-of-fame lines.
- **Dynamic, never on fixed days** (decision 10, changed by the author). Every closing item that can time out does
  so from live signals (room left in the week, players who could take it, whether anyone is engaged, the time since
  its last beat), with a last call and a grace first, the way the majors' fallbacks work
  ([When something closes on its own](#when-something-closes-on-its-own)).
- **One dose-outlet table** (conflict 40), owned here, with a row for every finale term. The Red Hand's board sells
  at the boss's price (30 Favor, strangers ×1.5), not 60/45/30. The chair says *"Two tins a dose"*. The boss's grey
  choice exists only under the boss's price. Exiles are served under *by the bite book*, the Independent's *all of
  them*, or the chair's grey choice. Halcyon's table always cures; home batches only under *Published*. The finale's
  route lines and errand card 41 (What the Cure Costs Here) read the same table.
- **Bastion's words** (conflicts 3, 7 and 41). Stance 1 is The Call Stands. The Deserter talks about Bastion's
  ending and its roster, and whether anyone answers for him at the roll. "Attached" means "Civilians, attached";
  "struck off" is only Bastion's word for a deserter, and the book says "paid".
- **The last call is the majors'** ([MAJORS.md](MAJORS.md#last-rounds-the-codas-in-the-epilogue-week)). Bastion's
  20:30 call on the first evening is the last nightly call of the season. After it the operator keeps a listening
  watch at 20:30, and the CO's grey choice is whether to keep it.
- **Tribes replace Grounds** (conflict 42). The ground beats are tribe beats: still calling, brought home, taken
  alive for the lab, killed or burned out, walking with it, the great alpha. The director's grey choice is "with the
  index in isolation" (the finale's Act III specimen). The tribes' props are their own pool through `RZ_Marks`.
- **The finale's keys** (conflict 39). `finale.route` 0 is nobody (handed over), `finale.finisher` 0 is the lab
  alone, and every lock is read with `RouteOf(identity)`. Route marks are private (`finale.marks_public` 0): the
  finale decided, so the public branch is gone.
- **The epilogue's lane** in the season budget: morale ±5 per faction for the week, inside the majors' lane, for
  the grey choices and the closing work together (the plan's budget table).
- **The last errands are errands' epilogue deck** ([ERRANDS.md](ERRANDS.md#after-the-cure-the-epilogue-week)). They
  pay through errands' small-pay ledger, not "like the contract". The Tags and The Wreck's Name are His Effects'
  epilogue variants, Come In is errands' amends card, and Processing and What the Cure Costs Here join the deck.
- **The paper's table stands just outside the kill zone, about 110 m from the barrier**, not 75 m
  ([MAJORS.md](MAJORS.md#act-vi-the-paper-coda)).
- **The Walk Out is the one crossing**: in processing, at the north road only, people walk out one at a time, and the
  voice at the barrier asks *"Name?"*. Nothing comes back in, and nothing past the barrier is ever shown or said.
- **Dialogue is off by default** (decision 13): every topic, grey choice and hand-in here also works from the action
  menu. **Strangers only where one would make sense** (decision 14): the week's asks come from holdings' people, the
  queue and residents; a stranger speaks only at one of errands' spots.
- **Season Fame and the hall** (decisions 4 and 5). The hall's season block carries the finale's "Zone's Fame" line
  beside the ending. The Independent pays no chunk unless it chose *one side*.
- **The confirmed lore** (decisions 2 and 3): Private Marek Fiala (Twice) can answer *"Here"* at the last roll, and the
  woman bitten on Day 4, the eight and the radio ham appear where their owners put them.
- **Titles are reputation's names** ([REPUTATION_NAMES.md](REPUTATION_NAMES.md)): the example's "Good-Natured Rascal"
  is now Favourite Rascal.
- **Seeded names never use LORE's people** (conflict 30): no Halas at a Haven house.
- **No new doc** (decision 16 lists the four new docs, and this isn't one): the epilogue goes into DESIGN.md and
  MISSIONS.md ([Docs](#docs-in-the-same-commits)).

---

It runs on built systems: world facts (flags, canon, `Fill`, `Changed`), leaders, wanderers, safehouses and their
notice boards, radio and the radio station, headlines, readables, the Long Table's tells, the hall of fame, and
today's epilogue (the line stands down, research stops, the gates open with the wipe). It reads the designed systems
once they exist: the finale ([FINALE.md](FINALE.md)), the majors ([MAJORS.md](MAJORS.md)), tribes and crawlers
([INFECTED.md](INFECTED.md)), the reputation names ([REPUTATION_NAMES.md](REPUTATION_NAMES.md)), dialogue
([DIALOGUE.md](DIALOGUE.md)), errands ([ERRANDS.md](ERRANDS.md)), mission style's records
([MISSION_STYLE.md](MISSION_STYLE.md)) and disguise ([DISGUISE.md](DISGUISE.md)). Each has a fallback, so the
epilogue can ship on what's built and grow as the others land. [TONE.md](TONE.md) owns how every line here sounds.

## In short

New Vegas tells its ending in slides. ReforgerZ tells it in the world, for a week. Once the cure is out, the things
that already talk get something to say about the season, and small things turn up where events happened:
- a board by the lab gate with the names of the people who held it;
- candles on a safehouse step, with a number chalked beside them;
- a board at an outpost gate: a red hand over a Bastion stencil;
- a queue on the verge of the north road that doesn't move until the last day.

There are no cut-scenes, slides or new screens. A player who wants the ending goes and looks; one who doesn't still
hears it on the radio.
- Every route has a real way to be treated, priced the way that faction would price it.
- Every story still open at the cure gets a way to end inside the week, and the world gives you the work to end it.
- Everyone gets small last errands, chosen by where they are, who they are and what happened there.
- On the last day the queue finally moves, and your character can walk out for good.
- Nothing physical survives a player walking away, a restart or the wipe. Only lines in the hall of fame and
  `last_season.json` carry over.

## The week at a glance

**Epilogue hours** (L) is a Season setting: **168** by default (an attribute default, never an initialiser; today's
built default is 48). Every clock below is a share of L, so an admin who sets a shorter epilogue gets the same shape,
squeezed.

| Clock | Share of the epilogue | At 168 hours |
|---|---|---|
| **A. The news** | From the commit to the first game dusk with a player online: at least 1 hour, at most L ÷ 28 | 1–6 real hours |
| **B. The days after** | Between A and C | about 138–143 real hours |
| Evolving steps | At a third and at two thirds of phase B | about every 46 real hours |
| **C. Processing** | The last L ÷ 7, never longer than **Season > Warn hours** | the last 24 real hours: the built wipe warnings and processing coincide |
| Empty server ends early | Nobody online for 2L ÷ 7 in phase B starts phase C at the next login | 48 real hours |
| Minimum epilogue (a late cure) | L ÷ 3 | 56 real hours |
| Callbacks expire | The plan's epilogue rule | 24 real hours |
| Set pieces' second airing | At least 8 real hours after the first | — |
| Test scale | Multiplies every real-time clock here | 0.008 runs the week in about 81 minutes |

**A typical week** at the built time acceleration of 1:1 (the calendar adapts to faster servers):

| When | What usually happens |
|---|---|
| The cure (phase A) | The cure line and everyone's first line. The queue forms. Leaders' verdicts. The outlets open. Losing routes' marks come down at the first dusk |
| Evening 1 | **Last Rounds** begin: the Haven's Gate at 18:30, Bastion's last call at 20:30 |
| Dawn 2 | Bastion's walk and the last roll at 05:30 |
| Evening 2 | Halcyon's paper goes out, from the 17:30 change of shift |
| Evening 3 | The Red Hand's last line: the book is closed at dusk |
| Days 2–6 | Loose Ends, Bring Them Home, Quiet the Ridges, Take Back a Town, Settle Accounts, the Last Broadcast's tally, Personal Goals, the epilogue deck. The Haven's supper readings on free evenings. Evolving beats step twice |
| Day 7 (phase C) | Processing. The queue at its longest; anything still open closes. The Walk Out: the queue moves at 05:30 and 17:30. The Last Broadcast airs. The Long Table packs, if nobody found it. On the last dusk every pack's leader calls once and nothing answers |
| The wipe | The voice at the barrier reads the names put down for it. The gates open with the wipe, as built |

## Principles

1. **Show first, then tell.** A beat's home is a place: a mark or a scene. People, the radio and boards echo it and
   point there. Only the journal and the hall of fame summarise.
2. **One story for everyone.** Beats are picked once, at the cure, from the season's seed, and saved. Every player,
   early or late, finds the same board. Personal lines differ; the world doesn't.
3. **Never contradict the world.** Live numbers and names are filled in when a line is shown. Every beat checks its
   subjects again each time it is shown. If one has changed, its "since" version plays, or the beat is dropped.
4. **Data first, props when set, text always.** Every mark and scene works as text when its prefab slot is empty. A
   number appears only where something written shows it (chalk, a board), never as a count of props.
5. **Nothing to farm, nothing carried over.** Beats pay nothing. Closing work pays through the ledgers it's built on,
   once, and all of it is per-life standing that the wipe clears. Hall-of-fame records freeze at the cure; the week
   adds only lines to the season block, by fixed rules.
6. **Every story can end.** Anything open at the cure gets a way to end inside the week, live if someone plays it,
   as data if nobody can. A rushed or bitter ending is still an ending.
7. **The outside never changes because of anything players did.** The broadcast and the voice at the barrier say the
   same thing on every route. After the cure the outside says only its processing order, *"Leave it on the table. Step
   back. It will be processed with you."* at the paper's table, and *"Name?"* at the north road in processing.
8. **Grey, with a dark joke.** Every route has an ugly side and a joke about it, Independent included, and the joke
   comes from a rival. Jokes are aimed at institutions, never at a named player's death ([TONE.md](TONE.md)).
9. **Dense at the hubs, sparse in the wild.** Most beats and closing work sit where players go anyway: the lab, the
   queue, the strongholds and the safehouses. Most of the map stays abandoned and quiet.

## Timeline

### All season: the town log and season marks

**The town log** (part of `RZ_WorldMemory`) keeps two entries per settlement: **the best thing** that happened there,
and **the worst**.

| Slot | Kinds (each has a base salience, 1–5) |
|---|---|
| The best | a rescue (with its count), a horde held off, a town cleared, water tested clean, a pack's leader laid down by name, power restored, a puzzle solved, a questline stage that ended well |
| The worst | burned, shelled or a Fire Mission, a Quarantine Line, a toll racket, a mass grave, a truck lost, a leader killed, an outpost lost, a notable death, a wanderer's death |

- **Salience** is the kind's base, plus 1 if three or more unique identities took part (5 at most).
- **Replacing:** an entry is replaced only by a strictly higher salience, and ties keep the older one. A fire can't
  erase a rescue, because they sit in different slots.
- **Names:** up to 2 identities per entry, each with the name they played under at that moment (frozen). An identity
  is named in at most one entry per town and 6 in all. After that, its entries are written unnamed (*"someone"*).
- **Visits:** each town also keeps up to 16 identities who *did* something there this season: searched, fought,
  handed something in, finished a mission step, or talked to someone. Standing about doesn't count.
- **The Long Table's takings never enter the log** (no "never arrived" entries) unless `table.exposed` is set.
- **During the season** the log feeds callbacks. A leader says *"You're the one who got six out of Brod."* A rumour
  names the right town. The log also feeds errands: a resident asks their rescuer to fetch a photo from home; a burned
  town's survivor asks who lit it.
- **Written by:** mission ends (an optional `LogEntry()` on `RZ_Mission`), world events, supply lines (where a truck
  was lost), territory, leaders, wanderers, town memory (burned), notable deaths, the tribes, questline `Resolve` and
  story arc endings. About 25 KB at most.

**Season marks.** An entry with salience 3 or more puts one mark at its place for 2 season days, and for at least 8
hours played. For example: a sheet tied between two posts on the road into Brod, painted *"6 OUT · THANK YOU · MARA"*.
- **At most 40 at once.** When the limit is reached, a new mark replaces the oldest, lowest-salience mark that isn't
  protected. Memorials are protected, and so are best-slot marks for their first 8 hours played. If nothing can go, the
  new mark is text only (its approach line).
- **Memorials:**
  - a Haven Champion's death (Remembered) lights a candle at every safehouse for one season day (at least 4 hours
    played);
  - a notable death's grave stays until the wipe (see [Their dead](#their-dead)).

Players learn this vocabulary as things happen. When candles stand on a step after the cure, they know what candles
mean.

### From 90%: route marks

The finale locks each player into a route ([FINALE.md](FINALE.md#4-the-lock)). The walls show it.
- **Where:** on the verge of holdings' roads and the lab road. One mark per route, at most 2 per place and 12 in all,
  refreshed on the territory tick.
- **Who counts:** only **active** locks: identities with 2 or more hours played this season who have been online in
  the last 2 real days. Alts and players who locked in and left don't count.
- **How many places each route gets:** the same share for every route with an active lock (`finale.marks_public` 0),
  so the walls never hand out numbers in a fight between players.

| Route | Mark |
|---|---|
| The Haven | An amber ribbon on a fence post; chalk on a board: *"SHARE IT"* |
| Bastion | A stencil board: *"ALL TREATMENT TO BE SECURED. REPORT IT."* |
| The Red Hand | A painted board: *"PRE-ORDERS TAKEN. ASK FOR THE BOOK."* |
| Halcyon | A printed notice on a post, with the plague doctor in one corner: *"THE METHOD WILL BE PUBLISHED. DO NOT HOARD."* |
| Independent | Chalk on a board: *"IT'S OURS. ALL OF OURS."* |

Leaders open with the player's lock, face to face only ([TONE.md](TONE.md#34-the-race-and-the-point-of-no-return-90)):
*"You've thrown in with Bastion. Fine. Wipe your feet."* None of this has mechanics. At the first dusk of the
epilogue the losing routes' marks come down, taken down by their own people, and the winner's become final.

### At the cure: the hand-over

In this order, starting on the cure's tick:
1. **The finale reports** `finale.route`, `finale.finisher`, `finale.how`, `finale.terms`, the condition and partner,
   up to 4 deciders, the stand's holders and the routes' states ([FINALE.md](FINALE.md#what-the-finale-hands-the-epilogue)).
   All by identity, with names frozen. The snapshot waits for the release (at most 3 minutes, plus the hold).
2. **The hall of fame freezes**, both this season's records and the all-time ones. Lives running at the cure count up
   to the cure. Nothing in the week adds to a record; it adds only season-block lines (below).
3. **The contest ends** (the finale's job). Route locks end. Every grudge and hunter squad the contest started stands
   down once. Parley reopens with everyone: *"It's over. Talk."* Fame and Infamy stay, because they're per life.
4. **The record's snapshot** is copied from every owning system in that one tick, and a save is requested.
5. **Over the next 10 ticks** the beats are chosen from the snapshot, then committed in one step (`story.built`). If
   the server restarts before the commit, the build runs again from the saved snapshot.
6. **The closing work is listed** from the same snapshot: every open questline, every named dead without a home, every
   pack still calling, every open account (the [closure set](#finishing-everything)).
7. **Within a minute of the commit, everyone online** gets the cure line and their own first line as a notice,
   including the players still standing at the lab. Players who are offline get theirs at their next spawn.

### The three phases

| Phase | Runs | What changes |
|---|---|---|
| **A. The news** | From the commit to the first game dusk with a player online (1 to L ÷ 28 real hours) | The cure is announced. The route marks become final. The queue forms (3 people). Leaders give their verdicts. The candles are lit. The dose outlets open. The closing work opens |
| **B. The days after** | Until phase C | Last Rounds on the first free evenings. The rest of the closing work. Evolving beats step at a third and at two thirds of phase B. Set pieces follow the evening calendar |
| **C. Processing** | The last L ÷ 7 (24 real hours) | The queue at its longest. Residents have bags. Boards ask for last notes. The radio station adds a "Going home" preset. The built wipe warnings carry the processing line. Anything still open closes. The Walk Out opens, and the queue moves for the first time |

**Evolving beats** have up to four versions: phase A (or the start of B), a third of the way through B, two thirds of the
way, and phase C. They work the same at any epilogue length.
- The Red Hand's price board, under the boss's price: *"2 TINS"*, *"2 TINS. NO CREDIT."*, *"2 TINS. LAST WEEK."*,
  then in phase C *"FREE. WE'RE NOT CARRYING IT OUT."* The board is the shop, so the price is real (see the outlets).
- The candles: *"two still lit"*, then *"all out, the wax scraped up for the next ones"*.
- Halcyon's notice under *Published*: *"PAGE 1 OF 6"*, then *"COPIES LEFT: 40"*, then *"COPIES LEFT: 0. COPY SOMEONE
  ELSE'S."*
- Bastion's roster board: *"NO REPLY"* gains a stroke after every 20:30 watch.
- The queue's sign: none, then *"QUEUE STARTS HERE"*, then *"QUEUE STARTS HERE. STILL."*, then in phase C an arrow.

### The evening calendar

The set pieces run on the game clock: supper at 18:30, Bastion's 20:30, the walk at 05:30, the change of shift at 17:30.
The server's time acceleration decides when that falls in real time. So:
- **One set piece an evening**, server-wide, on the one set-piece calendar the majors own. An evening is a game dusk
  (18:30–20:30) with a player online.
- **At the commit**, the epilogue works out how many evenings phase B holds, from the server's time acceleration
  (VERIFY: reading it; if it can't be read, assume 1:1). **Last Rounds go first** (the majors' codas, in their order),
  then the epilogue's own pieces. Over a week at 1:1 there are about six evenings, so everything fits; on a faster
  server there are more.
- **The Haven's supper reading** (once the Gate has run) takes the evening only if nothing else has it. On other
  evenings the names go out on the Haven channel, with no scene.
- **Bastion's 20:30.** The first evening's 20:30 is the majors' last call. Every 20:30 after it is the operator's
  listening watch: radio only, so it runs every evening without taking the slot.
- **Every piece airs twice:** live on its evening, and again, words only, on the next free evening at least 8 real
  hours later.
- **Every piece leaves a standing version** from its first airing until the wipe. So a player who only ever plays at
  lunchtime still gets it:
  - the supper reading: the names pinned on the house board, and a clerk who reads them out on request (*"Ask about
    last night's reading"*);
  - the last call: its transcript on the roster board, and a radio operator who plays it back;
  - the majors' codas: the standing versions [MAJORS.md](MAJORS.md#last-rounds-the-codas-in-the-epilogue-week) lists.

### The world settles

This is on by default (**Epilogue > The world settles**), so outpost and leader beats stay true.
- A new world fact, `HoldingsFrozen()`, is true in the epilogue. It's read by:
  - the territory tick: no towns taken or dropped, only its floor kept;
  - contracts that take or raid an outpost: not offered;
  - The Last Convoy arc: not started;
  - questline, major and closing-stage endings that would hand over an outpost: words only, and logged;
  - leaders' off-screen deaths: a quarter of the usual rate.
- A mission already running whose end would move a holding still pays. The holding stays, and the result line says so:
  *"Morton holds, for now."*
- Fights, trucks, missions, questlines, the tribes and the crawlers carry on, on their epilogue rules.
- With the setting off, everything runs as usual and the since versions carry the changes.

### When something closes on its own

The author's rule (decision 10) holds in the week too: nothing closes on a fixed day. Each closing item that can time
out is read on the existing questline tick against live signals, as the majors' [dynamic
fallbacks](MAJORS.md#dynamic-fallbacks) are:

| Signal | What it is |
|---|---|
| **Room left** | Real hours until processing begins, × the share of real hours with someone online over the last 7 real days. On a server that's busy half the time, 60 real hours left is 30 hours of room |
| **Need** | Roughly how much play the item takes: a closing stage 1 session, a delivery half a session, a tally 1 evening |
| **Qualifies** | A player who could take it now: the standing, background and flags it asks for, not hunted by its giver |
| **Engaged** | A stage of it is running, or someone touched it (took, carried, talked about it) in the last session of play |
| **Beat gap** | Hours played since the item's giver last had something happen |

- **The squeeze.** When an item's room left falls below its need plus the needs of the other open items its giver
  holds, it puts out a **last call** in its giver's voice (*"{head} says the stores get counted tomorrow at supper,
  with or without you."*), then gives a **grace** of one session of play with someone who qualifies. Then it closes as
  data, with its **default** ending.
- **Guards.** Never while it's engaged (at most one session more). Never on an empty server: a default decided while
  nobody is on is told at the next login, in order, one headline a real day at most. Never twice for one giver in one
  session. Never on the evening of a set piece within 600 m.
- **The hard stop is processing.** Anything still open when phase C begins closes as data at its next hour, and its
  standing version goes up. The Walk Out, the Last Entry, Settle Accounts and Bring Them Home deliveries are the
  exceptions: they're open until the wipe.
- So a busy server's players finish their stories live and on time, and a quiet server's stories still end, after a
  fair warning, with nobody waiting on players who aren't coming.

## The cure in hand: dose outlets

Today, after the cure, the lab makes no more doses, so the bitten have fewer options than before. That contradicts a
week about handing the cure out. So each route gets a real way to be treated, and its terms decide the price.

**Rules for every outlet:**
- **Given on the spot.** It cures the infection the player has now. It is never an item, so it can't be stockpiled,
  traded or carried off.
- **Only for the bitten.** It's offered only while the server says the bite has taken (the antidote's own check).
- **One outlet dose per life, from all outlets together.** After that, the built treatments apply as usual.
- **A small stock per outlet**, refilled in real time: 1 every 30 minutes, 3 at most, unless the table says otherwise.
  When it's empty, the outlet says when the next one comes: *"Come back at six."*
- **Standing, never clothes.** Outlets read standing. A uniform does nothing: *"Nice jacket. Whose was it?"*
- **Favor is the faction's own** unless the row says otherwise. A tin is 15 Favor.
- The lab's built stock and the Red Hand's stolen doses run down as built.

**The one outlet table** (conflict 40). The finale's terms pick the row. The finale's route lines, the leaders' lines
and errand card 41 read the same rows.

| Route | Terms | Where | Price | Stock | Refused to |
|---|---|---|---|---|---|
| **The Haven** | Her house, and her boy | Every safehouse of Community size or more: *"Ask for the dose"* | Free at her house; elsewhere 5 Haven Favor, *"for the pot"* | 2 per house, 1 an hour | Exiles (Hunted by the Haven), unless the chair chose *All of them* |
| | The far house first | The same | Free at the far house; elsewhere 5 Haven Favor | 2 per house, 1 an hour | As above |
| | By the bite book | The same | 5 Haven Favor, or nothing if you have none: *"the sick first, whoever they are"* | 2 per house, 1 an hour | Nobody. An exile comes unarmed to the door, as at the knock |
| | The crossed-out first | The same | 5 Haven Favor | 2 per house, 1 every 90 minutes (the crossed-out residents go first) | Exiles, unless the chair chose *All of them* |
| **Bastion** | The roster | The corporal's table at the queue, and each Bastion outpost gate: *"Sign for treatment"* | Sign on as *"Civilian, attached"* and hand over one firearm, or pay 15 Bastion Favor. Anyone below Known is searched first, as at a checkpoint (contraband taken) | 3, 1 every 30 minutes | Hunted or Kill on Sight by Bastion |
| | The column | The same | As the roster. Anyone with a Day 2 slip is served first and free | 3, 1 every 45 minutes (the column goes first) | As the roster |
| | By need | The same | Free to anyone bitten. Come unarmed: weapons on the table, handed back after. No search, no name | 3, 1 every 30 minutes | Nobody |
| | The census | The same | Your name and background into the census, and one firearm. Everyone is searched, Trusted included | 3, 1 every 30 minutes | Hunted or Kill on Sight by Bastion |
| **The Red Hand** | The boss's price | The price board at Rudi's crossroads; the board is the shop | 2 tins (30 Red Hand Favor); strangers ×1.5. Free in phase C. The boss's grey choice can drop it to 1 tin at once | 4, 1 every 20 minutes | Hunted by the Red Hand |
| | Pay when you can | The same board | 1 tin now, and 1 tin on your line in the book | 4, 1 every 20 minutes | As above |
| | The first pan to the houses | The cook's sister's Haven house, *"Ask for the dose"*; and the board | Free at her sister's house; the board at the boss's price | The house 2, 1 an hour; the board 4, 1 every 20 minutes | At the house, as the Haven's door; at the board, Hunted by the Red Hand |
| | The book first | The board | Free, and 30 Favor goes on your line in the book (a line is opened if you have none) | 4, 1 every 20 minutes | Hunted by the Red Hand |
| **Halcyon** | The authors first | The lab's trestle table: *"Ask for a dose"* | 10 Halcyon Favor; free and first for anyone with initials in the log (Trusted with Halcyon) | 3, 1 every 30 minutes | Hunted by Halcyon |
| | The volunteers first | The same | 10 Halcyon Favor; free and first for anyone recorded as a volunteer this season (a trial dose, or the volunteers' book signed) | 3, 1 every 30 minutes | As above |
| | By the numbers | The same | Free if the bite took in the last 20 minutes; else 10 Halcyon Favor | 3, 1 every 30 minutes | As above |
| | Published | The lab's table, and every safehouse with a medic: *"Ask for a home batch"* | At the lab, 10 Halcyon Favor. A home batch: 10 Favor of that place's faction; it cures 80% of the time, otherwise it halves the time left (the trial dose's odds: the method in amateur hands) | The lab 3, 1 every 30 minutes; each house 2, 1 an hour | At the lab, Hunted by Halcyon; at a house, whoever that place's faction hunts |
| **Independent** | Open | A crate stencilled *ONE EACH* at the queue | Free, once per identity for the whole epilogue | 3, 1 an hour | Nobody: first come, first served |
| | One side, with a condition | That faction's outlet, under the terms its stronghold leader's traits pick (as when nobody chooses), changed by the condition | That faction's price, except: *all of them, exiles too* (the Haven serves exiles); *no names taken* (Bastion: no search, no roster, no census); *one tin, not two* (the Red Hand: 15 Favor, strangers too); *a home batch at every house* (Halcyon: Published's home batches, free) | That faction's | As that faction, changed by the condition |
| | Sold | The same crate | 30 Favor of any one faction, the buyer's choice. The payment goes nowhere | 3, 1 an hour | Nobody |
| | Kept back | None; only the built stock | — | — | — |
| **Nobody** | Handed over, or the slow cure (`finale.route` 0) | The outlet of the faction with the most season Fame, under the terms its stronghold leader's traits pick | That row | That row | That row |

The grey side is in the table:
- The Haven's dose is for its own, unless the bite book or the chair says otherwise.
- Bastion disarms the Zone a pistol at a time, and under the census it writes every name down.
- The Red Hand gives it away only once nobody can carry it out; under *the book first* it's free, and you owe for
  ever.
- Halcyon's table always works; its open method works four times out of five.
- The open crate goes to whoever reaches the north road first, and Brod is 9 km away.

## How it was won

The finale reports `finale.how` ([FINALE.md](FINALE.md#how-it-was-won-finalehow)). Each value has a clause in the cure
line and its own version of the stand board.

| How | The cure line says | The stand board |
|---|---|---|
| **1 held**: nobody else came for it | *"...held the lab"* | *"HELD THE LAB, DAY {cure_day}:"* and the names |
| **2 force**: the controller took the room, or held off a real challenge | *"...fought for it at the lab"* | The same, bullet-pocked, and underneath *"CAME FOR IT: {n}"*: a count of routes, never names |
| **3 deal**: two routes held it together | *"...agreed it between them"* | Two factions' marks side by side, *"BY AGREEMENT."*, and two deciders' names |
| **4 betrayal**: a pact partner turned at the stand | *"...was turned at the end"* | Two names scratched out at the top and written in again at the bottom, on the other side of a line |
| **5 theft**: Take the Lab or the Coat Run, and kept | *"...was taken from under them"* | *"HELD THE LAB, DAY {cure_day}"*, then in a different pen *"FOR 14 MINUTES."* and *"TOOK IT: {route_name}"*. After the Coat Run, the batch sheet is pinned under it: *"Released to: —"* |
| **6 handed over**: nobody held the room | *"...went to {faction}"* | The plain board |

A value the epilogue doesn't know uses *held*. With the slow cure (`cure.slow`), the board adds *"THE LAB'S OWN WORK."*

## The season's story: what it reads

`RZ_SeasonStory` is built from one snapshot at the cure, or at the start of the last day if there's no cure (see
[Edge cases](#edge-cases)). It is saved, and wiped with the season.

| Part | Fields | From |
|---|---|---|
| The cure | `finale.route` (0 nobody, 1 Haven, 2 Bastion, 3 Red Hand, 4 Halcyon, 5 Independent), `finale.finisher` (0 the lab alone), how, terms, condition, partner, up to 4 deciders, the stand's holders with their seconds at the lab and in the room, stands lost (days), batches taken (days, and by whom), the cure's day, `cure.slow` | the finale (fallback: most season Fame, how = handed over) |
| Majors | per faction: ending key, branch, deciders, epilogue line, real or default | the majors' `SeasonStory` |
| Questlines | `<key>.ending`, the deciding approach and who took it; every run still open at the cure | questline runs |
| Factions | season Fame, morale, strength, outposts at the start and now, towns won and lost, trucks delivered and lost, leaders lost (a count and the last 3 names) | territory, supply lines, faction life, leaders |
| Safehouses | tier at the start and now; people; died, rescued in and left hungry this season; shortages; council-head generations; the house's dead by name (6 at most: Remembered players, heads and named wanderers; the rest as a count) | safehouses |
| Outposts | owners this season (3 at most, in order), commander generations, the commander now | territory, leaders |
| Towns | the town log, visits, Burned, infection at the cure, crawlers at the cure | town log, town memory, crawlers |
| Tribes | each pack's leader: name, the place, fate (still calling, brought home, taken alive, killed, burned out), who did it; up to 3 named young walking with it; the great alpha if one lived | tribes |
| The Long Table | its `table.*` flags | the Long Table |
| Wanderers | the living (name, role); the dead (name, role, how, where, day); the Kid and the Preacher. Takings are not deaths, and are left out | wanderers |
| The named dead | every named dead person with a keepsake somewhere in the world (a body, a tag on a turned one, a grave, a notebook), and where home is | His Effects, tribes, journals of the dead, world records |
| Words | remembered moments (8), headlines (10), journals of the dead (the best one per identity, with where they died) | faction life, headlines, town memory |
| Deeds | per identity, the season scope of `RZ_Deeds`: what they did, for whom, against whom, and what the giver hid | the deeds store (fallback: questline participant records) |
| Players | per identity with a deed or at least an hour played: the season picture, a title per faction from their best life with it, `RouteAtLock`, deeds (6 short phrases with a place, names frozen), frozen records, Champion of, Wanted, crew name, Bastion rank, deaths this season, open accounts, notable score | reputation's season record, titles, questlines, majors, hall of fame |

- **New counters this needs:**
  - per safehouse: died, rescued in and left hungry this season, and shortages (today's `m_iDied` only counts
    "lately");
  - per outpost: the owner history;
  - per faction: trucks, and towns won and lost;
  - a wanderer death list;
  - the best journal per identity;
  - each identity's best life per faction (reputation's season record).
- **Each counter saves the day it started tracking.** A beat that reads a counter started after the season's first day
  uses its version without numbers. So a season where this ships halfway through never says a house lost nobody.
- **Size:** about 12 KB, plus 150 B per player entry. Drive-by joins get no entry.

## Beats: how the ending is chosen

A **beat** is one fact about the season, told by the world. It has a subject, a place, a voice, up to four phase
versions, its lines for each carrier, and the **subjects** it depends on.

**Slots** (New Vegas's slides, mapped onto the Zone):

| NV slide | Slot | At most |
|---|---|---|
| The Mojave | **The cure**: route, how, terms | 1, always |
| The factions | **One per faction**: its major's ending, else its season | 4 |
| The places | **Hubs:** the lab 3, the queue 3, each stronghold 2, each safehouse 3 | about 20 |
| | Outposts 1 each; towns 8 (one per 6 settlements on small maps); roads 4; tribes 4; the Long Table 1; stations 2 | about 25 |
| The characters | wanderers, the Kid, the Preacher, leaders who died | 6 |
| Side stories | questline and story arc endings that no major tells | 6 |
| — | **The truth**: hidden things the deeds store knows | 6 |
| — | **Since the cure**: new entries during the week (a town taken back, a name brought home, a pack's leader laid down, a closing stage done, a house that ran dry) | 20 |
| The Courier | each player's own lines, and up to 12 notables named in the world | 3 per player |

That's about 85 beats on Everon and 55 on Arland, counting the since-the-cure ones.

**Choosing** happens once, at the cure, with the season seed:
1. Every registered beat's `Applies(story)` runs. Each one that applies is a candidate for its place.
2. **Weight** = base (1–5, by beat type) × people × change × choice × known:
   - **people:** 1 + 0.25 per unique identity with a deed in it, ×2 at most;
   - **change:** ×1.5 if the place ended up different from how it started (a Shelter that became a Town, an outpost
     that changed hands, a town that burned);
   - **choice:** ×1.25 if it comes from a choice someone made (a questline approach, or a deed with a beneficiary). So
     one player's grey choice can beat a crowd's routine job;
   - **known:** ×1.25 if 3 or more identities did something there this season (from the visits).
3. Slots fill in the order of the table. Each place takes its best candidate; ties go by the seed.
4. Each picked beat is frozen and saved: names (as frozen at the deed), place and versions. Live numbers stay as tokens.
   Picking a beat sets `epilogue.beat.<key>`, which the epilogue deck requires.
5. **Since the cure:** new town log entries and closing work during the week can add up to 20 beats.

**Checked every time it's shown.** Each beat lists its subjects. They're looked up again each time a carrier shows the
beat, and at each refresh (each dusk, each real day):

| Subject | Still true while | If not |
|---|---|---|
| A leader seat | the same generation sits in it | since: *"Morton's commander? Bitten, Tuesday. Ask the new one."* |
| A wanderer, the Kid, the Preacher | alive and on the roads | since: their name chalked on the last door they rested at; else dropped |
| A pack's leader | its fate is unchanged | since: the faction mark gets a line through it |
| A safehouse | people in it, the same tier band | since: beds stacked in the yard |
| A town or an outpost | the same owner | since: the new owner's line |
| A town's crawlers | the count at the cure | since: the town taken back, with its candle |
| A number written on a mark | the same | re-chalked: *"The 11 is rubbed out. It says 12."* |
| A wanted poster | that life is alive and still Kill on Sight | since: *"PAID. DAY {d}."*, *"SETTLED. DAY {d}."*, *"NOT PAID. THE INFECTED GOT THERE FIRST."* or *"WALKED OUT. NOT PAID."* |
| The Long Table | `table.exposed` unchanged | the farm's beat moves to its new state |
| An outlet | stock left | since: *"Come back at six."* |

The check is lookups only, with no scans.

Beats are code, written the way canon rules are: small `RZ_EpilogueBeat` subclasses in `Director/Epilogue/`
(`RZ_Beats_Cure.c`, `_Places.c`, `_Haven.c`, `_Bastion.c`, `_RedHand.c`, `_Halcyon.c`, `_World.c`, `_Closing.c`),
registered in one list. Each says which facts it reads, so a beat for an unbuilt system never applies.

## Carriers: how the world tells a beat

Every beat has one **home** (a mark or a scene at its place) and up to three **echoes** that point to it.

| Carrier | What it is | Budget |
|---|---|---|
| **Mark** | One free-standing thing: a board, a sheet on posts, a stencil board, chalk on a board, candles, a grave, a poster, a trestle table, a wreck. "Read" or "Look at" shows its text. With no prop, a player who comes within 40 m gets the line instead, once a session | see [Performance](#performance) |
| **Scene** | People doing something: the queue, supper, a group at the lab gate | 3 with bodies at once |
| **Talk** | Leaders (epilogue topics), named people, safehouse residents (*"Ask how they are"*), wanderers' road news | existing bodies |
| **Radio** | Open channels, secure channels, the radio station's reel, the broadcast | the chatter's existing slot |
| **Board** | Pinned notes on safehouse notice boards | 3 per board |
| **Readable** | Things found in searches, dated in the epilogue | 30% of finds |
| **News** | The commit notice, and "Word on the roads" at a first spawn | 4 lines |
| **Summary** | The journal, the hall of fame, `last_season.json` | — |

Mission style's world records with an epilogue line use these carriers (their Chalk falls back to Board, since marks
are free-standing). **The echo rule:** an echo names the place and one detail, never the whole story. *"There's a board
by the lab gate with names on it. Somebody's added one in biro."*

## Places, and how they change

### The lab

The lab is Halcyon's stronghold: it never falls, and the director stays. What changes is who stands at the gate.

| Route | Scene at the gate | Mark (free-standing) | The director |
|---|---|---|---|
| The Haven | Two Haven medics with a cold box; people holding their names on paper | A sheet on two posts by the fence: *"DOSES WENT TO THE HOUSES. ASK AT ANY HAVEN DOOR."* | *"The Haven has the batch in its kitchens. They're writing down names. We asked for numbers."* |
| Bastion | A Bastion group at the gate; stencilled crates (props with no storage) | A stencil board: *"TREATMENT ISSUED AGAINST NAME AND SIGNATURE. NORTH ROAD. 0530."* | *"The method is in a crate, and the crate is signed for. I signed for it. Twice: the first one smudged."* |
| The Red Hand | Nobody at the gate; the lab's stickers stuck to the step | A board with a red hand, leaned against the lab's sign post | *"They took the batch and left the paperwork. They said we could keep the paperwork."* |
| Halcyon | The doors open; a trestle table, and a beak behind it | A notice by terms. *Published:* *"THE METHOD. 6 PAGES. READ IT. COPY IT. PASS IT ON."* Otherwise: *"TREATMENT HERE. ORDER OF ISSUE ON THE DOOR."* | *Published:* *"It went out on the open channel at six this morning, every reading. If anyone outside is writing our numbers down, they can stop."* Otherwise: *"We're doing it in order. The order is on the door. The order is correct."* |
| Independent, open | The clean room door wedged open | A board: *"TAKEN OUT FOR EVERYONE. {route_by}."* | *"{route_by} took the batch out of my hands and gave it to everyone. I would have done the same. Eventually. Probably."* |
| Independent, one side | The door wedged open; that faction's people carrying crates out | A board: *"HANDED TO {faction}. ON TERMS. {route_by}."* | *"The conditions are written on the back of my own form. They're better than the form."* |
| Independent, sold | A tin nailed to the step, empty | Chalk on a board: *"IT'S OURS. ALL OF OURS. 30 FAVOR."* | *"They're selling it at the crossroads price, minus the crossroads. I'm told that's progress."* |
| Independent, kept back | The clean room padlocked, with a padlock that isn't ours | Chalk on a board at the door: *"BACK SOON. {route_by}."* | *"It's somewhere safe. Nobody will tell me where. That was my line for three weeks."* |
| Nobody, or the slow cure | A duty officer at the door with a clipboard | A notice: *"BATCH COMPLETE. ISSUED TO {faction}. THE LAB'S OWN WORK."* | *"Nobody came for it in the end. We finished it anyway. It's what we're for."* |

**The stand board** stands by the gate, written in marker pen:
- *"HELD THE LAB, DAY {cure_day}:"* and up to 8 names: the controller's holders (at the lab for half the stand, or in
  the clean room for 60 s), longest first, then *"and 6 more"*. Turning up at the end doesn't put a name at the top.
- The version for how it was won (above).
- A lost stand adds a line: *"Didn't hold it, Day 47. Held it, Day 52."*
- A batch taken adds: *"One batch, Day 49. We know who."*
- The deck's **Take a Name to the Board** adds a dead person's name in biro underneath.

### The north road: the queue, the bend and the barrier

Already built: the line stands down, the loudspeaker reads the new order, and anyone who walks into the kill zone (100
m) or up to a barrier is turned back. This design adds the following at the canon spot `day2.checkpoint` (the majors'),
where the Day 2 column was turned back.

| Distance from the barrier | What stands there |
|---|---|
| 150–250 m | **The queue**, on the verge, and the route's table beside it |
| 150 m, at the bend | Where the chair reads the List (the Gate) and Bastion's walk halts. On the far verge from the queue: **the Names board** (the wall of names, and the Last Entry) |
| about 110 m | The paper's table, when Proof's coda puts it there |
| up to the barrier | Nothing, and nobody, until processing. Then the Walk Out |

All of it is in the warning zone, where no drone comes after the cure, and outside the kill zone and the barrier's turn
back, so nobody is ever turned back on the way to it.

**The queue** is a lasting scene.
- Phase A: 3 people with bags. Phase B: 6. Phase C: 6 bodies, the sign *"QUEUE STARTS HERE"*, and the line written on
  it: *"{n} WAITING"*.
- Each will talk: *"Been here since the order. Nobody's been processed. I'm keeping a place for my sister. She's in
  Brod. She won't come, but it's a good place."*
- Pinned locals walked here by errands' **Processing** card sit in it.
- In processing it moves (the [Walk Out](#the-walk-out)).

**The route's table** stands by the queue:

| Route | At the queue |
|---|---|
| The Haven | A pot and cups. *"Soup's for the queue. The queue's for everyone."* (words only; the dose is at the houses) |
| Bastion | A corporal at a folding table with the roster; this is the outlet. *"Weapon. Thank you. Name. Sign. No, here. Arm."* |
| The Red Hand | A lad on a kitchen chair with a tin. *"The queue's free. A place near the front isn't."* (the dose is at the crossroads; the place near the front is real in processing) |
| Halcyon | A beak with a clipboard, reading out the order of issue (the dose is at the lab) |
| Independent | Open: the crate stencilled *ONE EACH*, nearly empty, with a name chalked on the lid. Sold: the same crate with a price chalked on it. One side: a folding table and a note saying where the batch went. Kept back: nothing, and a longer queue |
| Nobody | A folding table with nobody behind it, and a sign: *"ENQUIRIES: {faction}"* |

- **Last Rounds** run here on their evenings: the List at the first dusk, the tags walked at 05:30, the paper left on the
  table.
- **Other sealed checkpoints** get one route mark each and no queue: there's only one queue in the Zone. On a map with
  no checkpoint, the queue and the Names board stand at the gate of the Haven's stronghold safehouse (the majors'
  fallback).
- **Road posts.** Under Bastion's or the Red Hand's route, the road posts that still go up prefer roads that lead to the
  queue, inside the existing caps.

### Strongholds

Strongholds never fall, and their leaders never change. Each gets two beats: its leader's verdict, and its place's beat.

| Stronghold | The place's beat | Without the major |
|---|---|---|
| The Haven | After the Gate, supper at 18:30 on free evenings: the chair reads this season's dead (the house's names, Remembered players and named wanderers), then *"and the ones we don't say"*. Haven-Known players hear it within 300 m and on the Haven channel. Standing version: the names on the board, read by the clerk on request | Read on the calendar's free evenings, from the houses' dead |
| Bastion | The roster board: *"EFFECTIVE STRENGTH {count}. STRUCK OFF {n}. CIVILIANS, ATTACHED: {n}. NO REPLY: {days}."* The last call on the first evening (the majors'), then the 20:30 watch: five minutes on the net, *"Bastion listening. Nothing heard. Bastion out."*, and a stroke on the board. Standing version: the last call's transcript on the board | The board, from Bastion's losses; the watch from the first evening |
| The Red Hand | Rudi's crossroads: the lorry, with the book nailed open at the last page once the last line has run (the majors' optional props). The price board stands here | The price board alone |
| Halcyon | The lab (above) | — |

### Outposts

Each outpost gets one beat. Its board is a single free-standing prop in the current owner's style; older coats of paint
are described in the text, not built as props.

| What happened | What it shows |
|---|---|
| Held all season | The owner's board at the gate, dated Day 11 and repainted. The commander: *"Same post since the start. Same kettle."* |
| Changed hands | The new owner's board, with the text: *"MORTON POST. A red hand over a Bastion stencil. The stencil still shows through."* The new commander has a line about the old owner |
| Changed hands twice | The text names both coats: *"Under the red hand there's a Bastion stencil, and under that, in chalk, the Haven's KEEP OUT."* |
| Lost or abandoned | The last notice still on the gate: *"BACK AT 17:30."* If a military pack grew from the lost post, Bastion's contact board carries its number with the post's name |
| Its commander died | A grave on the verge outside, in the faction's own way: for Bastion a tag on a nail, for the Red Hand boots on a post, for Halcyon a notebook number painted on a stone, for the Haven a candle |
| Always | The commander's line about the route. A Rogue commander under Bastion: *"Signed for. Some of it."* |

### Haven safehouses

Each safehouse gets up to three beats:

| Data | What it shows |
|---|---|
| The season's dead | Candles on the step (one prop per death, 5 props at most) and a number chalked beside them: *"Candles on the step, and 11 chalked on the stone. Two are still lit."* The council head reads the names at supper |
| Rescued in | Residents say who brought them: *"The nurse brought six of us in from Brod. Mara. Didn't stay for soup."* |
| Grew a tier | A new rota on the door: *"ROTA. 33 NAMES. NO EXCUSES."* Bunting (an optional prop) |
| Shrank, or went hungry | Beds stacked in the yard. By the pot: *"Half portions until the gates open. Then we'll see."* |
| The house rule (`haven.rule`) | Shared pot: *"SHARED POT. NOBODY'S A MOUTH."* Workers first: *"WORKERS EAT FIRST."* Counted: *"COUNTED. CURFEW 20:30."* The arrangement: a tin on a nail, empty |
| Exiles, under the shared pot, Every Name or *All of them* | A board note: *"If we put you out, come and eat. — {head}"* |
| The route | A notice on the door: how the dose reaches this house (from the outlet table) |
| In processing | *"GONE HOME: {n}"* chalked by the door, as the Walk Out takes the house's people |

**The board** pins up to 3 notes: the head's line, the names, and a thank-you to a notable player. Pinned notes don't
count toward the board's 8 notes, and they stay until the wipe. Names brought home to the house are read at supper and
listed under the names.

### Towns

Most of the map stays abandoned. **At most 8 towns carry an epilogue mark of any kind**, beats and candles together.
Each town beat comes from its log:

| Log entry | Mark |
|---|---|
| A rescue | A sheet on two posts on the road in: *"6 OUT · THANK YOU · MARA"* |
| Burned | A board on the road in: *"BURNED DAY 33. NOBODY INSIDE (WE CHECKED)"* |
| Shelled, a Fire Mission, cleared | A Bastion stencil board: *"C-7 · CLEARED · 0400 · D27"* |
| Quarantine Line | Tape between two posts, and a stencil: *"CONTAINED D19. LIFTED D52."* |
| A toll racket | A red-hand board with a tally of tins, the last stroke crossed out |
| A mass grave | A cross of fence palings, and a number |
| A notable player died there | A grave ([Their dead](#their-dead)) |
| A wanderer died there | Their name chalked on the last door they rested at |
| A puzzle solved | The riddle's note on the step, worked out in pencil |
| A questline stage | That stage's own object. After The Well Story: a bucket on the well, and *"TESTED. CLEAN."* |
| Taken back after the cure | The candle ([Take Back a Town](#take-back-a-town)) |

If a town has both slots filled, the echo may name the other one as its detail: *"There's a sheet on the road into Brod
with a name on it. The town behind it burned."*

### Roads

- **The last 4 lost trucks:** a wreck on the verge where each was lost (an optional prop, else a line). *"A burnt-out
  Bastion truck, its stencil just readable: MORTON."* Its driver's papers are His Effects' epilogue variant.
- **The Last Convoy's end,** if that arc ran.
- **Road posts** keep their route lines. A Bastion checkpoint asks if you've been treated, *"for the record"*. A Red Hand
  roadblock's toll is *"one tin, or one dose, whichever's cheaper this week."*

### Tribes

After the cure nobody new turns, and the packs run down on the epilogue's own clock ([INFECTED.md](INFECTED.md#after-the-cure-the-last-week)).
Each pack can have one beat, and its home is the faction mark where a road enters its range (from the tribes' props
pool). Lines use each faction's own word, always with the place, and no pronouns, because the leaders' names are seeded.
Player text never says "alpha" or "tribe".

| Beat | What it shows |
|---|---|
| **Still calling** | *"Josef still calls from the quarry every evening. Fewer answer now."* (the Haven's name for him). The mark: *"QUARRY LOT. NOT AFTER 7."* The since version plays when that changes |
| **Brought home** | The Haven's chalk gets a line through it and becomes *"JOSEF. HOME D47. LEVIE."*, with the name of whoever carried his card home underneath |
| **Taken alive for the lab** | *"The index is in the lab's isolation room. It still calls, very quietly. The night shift wears earplugs."* |
| **Killed** or **burned out** | Bastion's stencil: *"P-2 · QUARRY · CLOSED · D49"*; the Red Hand's: *"POSTMEN"* with a line through it |
| **Walking with it** | *"Pavel's with the quarry lot now."* Up to 3 named young, players' characters among them (by the names setting) |
| **The great alpha** | *"Something still walks round the hills with all the others behind it. The lab's paper has a page for it with nothing written on it."* |

### The Long Table

It's never described. Its beat lives in ordinary things, and only where people knew. Its last week is part of the
closing work ([The Long Table's last week](#the-long-tables-last-week)).

| State | What it shows |
|---|---|
| Never found (`table.exists`, not exposed) | The house goes on, with smoke most evenings. Grace: *"For what we are about to receive. For everyone going home."* The host's road line: *"Long walk to the checkpoint. Longer queue. Come and eat first."* No beat anywhere else, because nobody knew |
| Burnt | The town is Burned (already built). A Bastion stencil board at the lane: *"INFECTED CLUSTER. CLEARED. D34."* |
| They left, as C1 ends it (`table.walked_out`) | An amber ribbon on the gate. The long table laid for one |
| The mother at the lab | On Halcyon's secure channel: *"Appendix C stays out of the paper."* |
| Moved | The old farm empty, with one place still set. The new farm shows "never found" |
| Ended by hand | The chairs turned to the wall |
| The boy got out | The Kid wanderer, or one more child at the nearest house: *"He eats what he's given."* |
| Gone to the queue (the last week) | The farm empty, the long table laid, the chairs pushed in. No smoke. Nothing else, anywhere |

**Nothing leaks from the epilogue's own carriers until `table.exposed` is set:**
- Takings never enter the town log or the record's dead.
- The Preacher reads only the recorded dead.
- Readables never name the farm's town.
- No beat, reel entry, board note or leader topic mentions the never-arrived.

The built tells (the quiet "never arrived" headline, the boots, the satchels, the rumours from suspicion 30) keep
working in the week, as built, so the house can still be found out. **The host's road meeting is unchanged.** It still
needs a player within 1.5 km of the farm, nobody else within 150 m, and happens once a session, never with a character
who crossed the house. Only his line changes, and only for a player on a road toward the queue.

### Stations

- **The power station:** who restored it last, and which towns have lights. *"Three towns have lights. Nobody's sure who
  paid for the diesel."*
- **The radio station:** the reel ([Radio](#radio)), and the Last Broadcast's tally.

## What people say

### Leaders: the epilogue topics

Every leader gets epilogue topics in the dialogue tree. Dialogue is off by default (decision 13), so until it's on,
*"Ask about the season"* on `RZ_LeaderAction` answers the first three topics as a notice, "Where to look" is a second
action, and each grey choice is two actions.

| Topic | Shows when | Says |
|---|---|---|
| **The cure** | always | the route in the faction's voice, with how it was won and the terms' outlet, coloured by traits: a Greedy leader prices it, a Merciful one asks if you've eaten |
| **This place** | the place has a beat | the beat, in the first person |
| **Me** | always | the player's own line for this faction; else their title with it, and the route they stood with |
| **The dead** | the place has dead in the record | up to 6 names, then *"and the ones we don't say"* |
| **What you didn't tell me** | the deeds store holds a hidden truth that touches this faction or this leader | the persuade check (your access rank, mixed tiers one lower): pass, and you get the truth; fail, and you get a deflection. A Rogue leader lies again |
| **Where to look** | always | two places with beats or open closing work the player hasn't met yet, with no markers |
| **A grey choice** | stronghold leaders, until someone decides | two answers, decided with the decide-scene hold; the others hear what was decided |

**The four grey choices.** Each is decided once a season, by the players who take part, through missions' 5-second hold
with one objection each. Its effects come out of the epilogue's lane (morale ±5 per faction for the week, inside the
majors' lane). The chooser gets at most ±5 Fame with that faction. Nobody gets an item.

| Leader | The question | One answer | The other |
|---|---|---|---|
| The chair | *"Whose names do we read tonight?"* | **Ours**, as usual | **All of them**, the exiles too (and the farm's guests, if `table.exposed`). Haven morale −3. Every board pins *"If we put you out, come and eat."* Exiles can ask for the dose under any Haven terms |
| The CO (once the last call has run) | *"The 20:30 watch. Do we keep listening?"* | **Keep the watch** | **Close the net.** The operator's last words: *"Bastion closing the net. No reply expected. Bastion out."* The board's NO REPLY gains *"NET CLOSED D{day}"*. Bastion morale −4. The Red Hand's kettle joke stops: *"Nobody left to laugh at."* |
| The boss (only under the boss's price) | *"The price. Leave it to the market?"* | **The market** | **Drop it now** (needs Red Hand Trusted, or ★ Raider). The board goes to 1 tin at once. Red Hand morale −3 (*"bad for the book"*), Haven +2 |
| The director (only with the index in isolation) | *"Do we keep it for the paper?"* | **Keep it** | **Let it go quiet.** The isolation beat moves to its since version: *"The isolation room is quiet. The night shift took their earplugs out."* Halcyon morale −2 |

Under the Red Hand's other terms the boss has no choice to offer: *"Already decided. Not by me."* With no index in
isolation, the director has none: *"Nothing in the isolation room. For once."*

**The stronghold leaders by route** (written without pronouns, because names are seeded; the Red Hand's row is under
the boss's price, and the other terms swap the price clause from the outlet table):

| Route | The chair | The CO | The boss |
|---|---|---|---|
| The Haven | *"Everyone gets a dose, then everyone gets a bowl. In that order, for once."* | *"Cure is with the civilians. Requesting instructions. Over."* A long hiss. *"No reply."* | *"They're giving it away. You can't sell water by a river. We'll sell cups."* |
| Bastion | *"They want our names before they'll give us a dose. We've got a list of names. We've had one for weeks."* | *"Cure secured. Distribution by roster. All signed for."* | *"Bastion's handing it out in a queue. We're selling places in the queue."* |
| The Red Hand | *"Two tins a dose at the crossroads. The houses are pooling. We're always pooling."* | *"The Red Hand holds the cure. Bastion is buying it. Entered in the log as 'contribution'."* | *"Market price. Everybody pays. That's what fair means."* |
| Halcyon | *"The lab put the recipe on our board. Half of us can't read it. The nurse can. Mostly."* (Published) | *"Published without clearance. Noted. Bastion out."* | *"They gave it away. Who does that? We've gone into printing."* |
| Independent | by terms (below) | *"Unauthorised distribution by civilians. Logged. Not stopped. Bastion out."* | by terms (below) |
| Nobody | *"It came to {faction}. Nobody fought for it. We'll take it."* | *"Cure issued. Source: the lab. Logged."* | *"Nobody won. Everybody's buying. Best week we've had."* |

The director's lines are in the lab's table.

| Independent terms | The chair | The boss |
|---|---|---|
| Open | *"{route_by} walked in with a crate and didn't ask for anything. We've counted it twice."* | *"Free. Fine. Then nobody owes anybody. First time for everything."* |
| One side | *"It went to {faction_short}, with conditions. We read the conditions at supper. Twice."* | *"Somebody walked in with the only thing worth selling and gave it away on terms. Amateurs with terms."* |
| Sold | *"It's for sale at the north road. We're pooling."* | *"Somebody's undercutting us with our own business model. I'm nearly proud."* |
| Kept back | *"{route_by} has it somewhere. We've sent a note. We've sent a pot."* | *"Holding it till the price is right. Now that I respect."* |

Outpost commanders and council heads say shorter versions, coloured by their loyalty. Each seat has at least 8 plain
lines per ending for the week ([TONE.md](TONE.md#312-repeats)).

### Named people and residents

The cook, the sentry, the toll man and a safehouse's residents (*"Ask how they are"*) each give one line in the first
person, from their place's beat or its season: *"I've been on the gate since Day {day_start}. {nights_words} nights.
Tonight I'm sitting down."* Residents with a seeded name never use a name from LORE. In processing they talk about going:
*"I've packed twice. I keep taking the kettle out."*

### Wanderers

Their road news becomes epilogue news, by role. They prefer beats from places they've passed, and closing work near
them, so they point players there.

| Role | Talks about |
|---|---|
| Trader | What a dose costs where, by the outlet table. *"Tins are worth nothing and everything now."* |
| Doctor | Who got dosed and who didn't; the bitten; where the nearest outlet is |
| Scavenger | Places worth seeing: *"There's a board outside the lab with names on it."* Towns gone quiet: *"Nothing moving in Brod. Somebody's lit a candle."* |
| Deserter | Bastion's ending and its roster, and whether anyone answers for him at the roll. He can hand his tag to the sergeant before the last roll ([MAJORS.md](MAJORS.md)) |
| Preacher | The dead. Where he rests, he reads the names of that town's recorded dead and the players' graves. He rings his bells once at dusk (heat as built) |
| Courier | Last letters (the deck) |
| Kid | Very little. Where he's going |
| Mechanic | The roads and the wrecks; a car *"for the queue"* |

A wanderer who remembers a player says their line about that player first. The never-arrived are never mentioned unless
`table.exposed`.

### Radio

- **The broadcast** reads the same new order on every route. Its phase C wording is part of the built processing
  warnings (*"Residents should bring identification."*), the same on every route. The Haven's channel answers: *"Mine's in
  a car on the north road."*
- **Open channels:** route chatter and beats about the faction's places, in the chatter's existing slot. *"Morton's open.
  The pot's on. Bring a cup."*
- **Secure channels,** for Radio Operators, carry the grey truth under the open line:
  - Bastion: *"All stations: issue to the roster first. Civilians after. Log it as priority."*
  - The Red Hand: *"Double at the checkpoint road. Half price for the Haven. Don't tell the Haven it's half."*
  - Halcyon: *"How many copies of page one are left? Don't hand them out at the barrier."*
- **The reel.** The radio station keeps one broadcast per identity (their latest before the cure), 12 at most, chosen to
  spread over the season's days (one per day first).
  - In the week it plays one every 20 minutes on the open channels.
  - Each entry plays at most once per player per session, and a player hears at most 3 a session.
  - Names pass the note filter, and anonymous players are skipped.
  - Broadcasts made during the week go out live, as built.
  - In phase C there's a "Going home" preset (*"This is Mara, at the north road. Going home."*). It replaces that
    identity's reel entry.
  - In phase C the Last Broadcast takes the reel's slot on the hour.

### Readables

30% of the readables found in the week are dated in it: *"A note on a kitchen table in Brod, Day 53: 'Gone to the queue
at the north road. Took the dog. Petr, the key's where it was.'"* That note offers an errand. Readables never name the
farm's town unless the house is exposed.

## And you

### Your three lines

Each player gets up to three lines about themselves. They're frozen when the story commits, and refreshed if the player
does something in the week. In priority order:
1. their route, and whether they decided it;
2. a major's decision;
3. a truth the deeds store holds about them;
4. a questline ending they decided;
5. holding the final stand;
6. a frozen hall-of-fame record;
7. Champion, or Wanted;
8. their rescues;
9. their dead characters.

**Who tells them:**
- **Line 1** comes in the commit notice, to everyone online: *"Word is you were on the lab road for the Red Hand."*
  Players who were offline get it in their next spawn's news.
- **Line 2** comes from the first leader whose faction it touched.
- **Line 3** comes from a wanderer who remembers them, a grave or a poster.

### Titles: who the Zone thinks you are

- **Reputation owns the names** ([REPUTATION_NAMES.md](REPUTATION_NAMES.md)): the 16 cells, such as *Favourite Rascal* or
  *Merciful Thug*, and the faction's word for you.
- The epilogue asks for `TitleFor(identity, faction)`. The answer comes from the identity's **best life this season with
  that faction**, from reputation's season record. It never depends on whichever life happens to be current.
- **Who says which:** a speaker calls you by your title with its own faction. The queue, residents and wanderers use your
  Haven title.
- **"From <town>":** the town of your highest-salience town log entry; else the town where most of your deeds happened;
  else it's left out.
- Until the names are built, the tier name of that best life is used (*"the Haven's Champion"*).
- **A disguise never changes your title** or what people say about you.
- People speak about the **name**, so a player who dies in the week isn't greeted as a stranger: *"Don't know the face.
  Know the name."*

### After the point of no return

A player locked into a losing route hears their route's people carry on with dignity: *"We stood on the lab road. We
lost. The book says we're square."* The winners' leaders open with *"You stood with the Red Hand on the lab road."*
There's no new hostility: standing is what it was, and the contest's grudges ended at the cure.

### Their dead

Each player's most notable dead character this season can get a grave, made from their journal of the dead: *"Two fence
palings and a name in biro: VERA. MECHANIC. FIXED THE BUS. DAY 23."*
- **Only notable deaths:** a life of 2 or more hours played, or a death the town log recorded (in a mission, at the
  stand, to a pack). Never a death by the player's own hand, and never a fall.
- **One per identity, 12 on the server.** The least notable goes first.
- **Never stacked:** a grave stands at least 100 m from another grave, and at least 150 m from a home mark, a scene or a
  safehouse. If the place of death breaks that rule, the grave goes on the verge of the nearest road out, 150–300 m away;
  else it's text only. Nobody can choose to be buried at the lab gate.
- A death in the week makes a grave only if it would count anyway, and only replaces a less notable one.
- A character who walks out leaves no grave and no journal of the dead.

### Wanted

Players who are Wanted at the cure stay on posters at Bastion posts and Red Hand camps: *"WANTED: KESSLER. BY BASTION AND
THE HAVEN. 50 FAVOR. THE CURE DOESN'T COVER IT."*
- **Wanted** means Kill on Sight for 4 or more hours played in that life before the cure, with the bounty still unpaid.
  Going Kill on Sight the night before the cure doesn't count.
- The bounty is paid as built, per life.
- When it ends, the poster changes to its since version: *"PAID. DAY 54."* for a player kill; *"SETTLED. DAY 54."* when
  the life left Kill on Sight through Settle Accounts; *"NOT PAID. THE INFECTED GOT THERE FIRST."*; or *"WALKED OUT. NOT
  PAID."*

### New in the epilogue

A player who first arrives during the week spawns from an epilogue-only scenario, **The empty kitchen**: a note on the
table, *"Gone to the queue at the north road. Took the dog. Petr, the key's where it was."* It points to the queue and
offers The Key Where It Was. The first leader they meet tells them the season in three lines: *"New face. You missed it.
Sit down."* Late joiners can take any closing work they qualify for; nothing in it needs the season behind you.

### The truth comes out

The deeds store's season scope keeps, per identity, what they did, for whom, against whom, and what the giver lied
about or hid. The week is where that truth lands.
- **How many:** up to 6 truth beats in the world, picked by harm (for example, a rival faction hurt by something the
  player didn't know they did). Each player gets at most one as a personal line.
- **Where:** in a leader's *"What you didn't tell me"* topic, or in a readable. *"The crates you carried for Novak on Day
  19 were Halcyon's. Novak sold them back to the lab. Twice."*
- **The check:** the persuade check gets the truth. Failing it gets a deflection, and a Rogue giver lies again.
- Unresolved lie records from errands come here on the last day ([ERRANDS.md](ERRANDS.md#after-the-cure-the-epilogue-week)).
- Until the store exists, the only source is questline participant records, and truth beats never apply.

### Notables, and keeping names safe

Up to 12 players are named in the world (**Epilogue > Notables**). On a server with fewer than 24 identities with deeds,
at most half of those identities are named.

| Deed | Score |
|---|---|
| Decided the route | +5 |
| Decided a major | +4 each, 8 at most |
| Held the final stand (as the controller's holder) | +3 |
| Champion this season (best life) | +3 per faction, 6 at most |
| Wanted at the cure | +2 |
| A frozen hall-of-fame record in the top 3 | +2 |
| Remembered | +2 |
| Decided a questline | +1 each, 3 at most |
| A crew name or a Bastion rank | +1 |

- **Epithets:** each notable gets an epithet from their biggest deed (*"who held the lab"*, *"who sold the pages"*). The
  majors' "At the barrier" and "On the paper" lists become epithets here.
- **Caps:** each name appears in at most 3 beats (2 on a server with fewer than 6 identities with deeds), and at least half
  of all beats name nobody. The stand board and the Names board are lists, so they're exempt.

**The names policy** (`RZ_Names`, the plan's one policy):
- **Frozen at the deed.** The text is the name they played under when they did it, so renaming later changes nothing.
- **Filtered.** Every name passes the note filter. A name the filter changes is replaced by the player's title, or "a
  stranger".
- **Opt out.** `/anonymous` keeps a player out of the world's text, the reel and the hall of fame (they're shown as *"a
  survivor"*). It's stored by identity in `hall_of_fame.json`, so it lasts across seasons; `/anonymous off` undoes it.
- **Admins.** `/epilogue strike <name or beat>` replaces a name with its title everywhere, or drops a beat. Both are
  logged.
- **Never on air** except a player's own broadcasts and honour lines; never a joke's butt.
- **Epilogue > Names:** *Full* (the default), *Titles only* or *Off*.

## Finishing everything

The author's rule (decision 12): the week holds quests and missions that let you actually finish everything. This is
the **closure set**: eleven pieces of closing work, so that every story the season opened reaches an ending inside the
week, played live if anyone plays it, closed as data if nobody can.

### Rules for every item

1. **Finishable within the week.** Each item has a need (how much play it takes) and closes on its own, after a last
   call and a grace, when the week runs out of room for it ([When something closes on its own](#when-something-closes-on-its-own)).
   Nothing waits on a fixed day.
2. **Nothing carries into next season except hall-of-fame lines.** Pay inside the week is ordinary per-life standing
   and Favor through the ledger the work is built on (errands' small-pay ledger, a contract's or a coda's own pay), and
   the wipe clears it. Next season's rules are fixed at the cure; nothing in the week changes them. Crew names, Legacy
   and unlocks start over as built.
3. **Hall lines by fixed rules.** Records froze at the cure. The week adds only the lines listed in [What the week adds to
   the hall of fame](#what-the-week-adds-to-the-hall-of-fame), each under the names policy.
4. **No free loot.** Closing work never spawns a crate, a dose item or anything from a Trusted or Champion shelf. In-kind
   pay is the giver's real stock, as errands pay.
5. **Works without dialogue.** Dialogue is off by default (decision 13), so every topic, hand-in and choice here is also
   an action-menu action or a puzzle object.
6. **Asked by the people it's about.** Givers are holdings' people, the queue, residents, leaders' "who to see" and the
   wanderers who pass. A stranger asks only at one of errands' spots ([ERRANDS.md](ERRANDS.md#where-a-stranger-can-find-you)),
   never on a timer and never in the empty middle of nowhere. Unasked offers stay inside the interruption budget.
7. **The world settles.** No closing work moves a holding; endings that would, say so in words.
8. **One lane.** World effects come out of the epilogue's lane: morale ±5 per faction for the whole week, inside the
   majors' lane, shared with the grey choices. Anything clipped becomes words.
9. **Bodies under one budget.** Scenes, escorts and props go through the one scene-people spawner, the one body budget
   and `RZ_Marks`, with a text fallback for each.
10. **Open to anyone the giver will talk to.** Late joiners included. The finale's locks are gone at the cure.

### The eleven at a glance

| Item | Who gives it | What you do | How it always finishes |
|---|---|---|---|
| [Last Rounds](#last-rounds) | The four strongholds (the majors' codas) | Escort, hold the road, read a name, carry a case, stand at the lorry | Live on its first free evening; as data at processing |
| [Loose Ends](#loose-ends) | Each open questline's own person | One short closing stage, several ways | Its default ending, after a last call |
| [Bring Them Home](#bring-them-home) | Posts, houses, the lab, the book | Carry tags, ID cards and keepsakes home; the wall of names fills | What nobody carries is listed "not brought home" at processing |
| [Quiet the Ridges](#quiet-the-ridges) | Bastion, the Haven, Halcyon, the Red Hand | Hunt the last pack leaders, bring them home by name, or sit their last call | The packs starve; on the last dusk every leader calls once and nothing answers |
| [Take Back a Town](#take-back-a-town) | Residents, Bastion, the crews | Clear a town of its crawlers; it stays cleared | Each town is its own small finish |
| [The Long Table's last week](#the-long-tables-last-week) | Nobody: somebody has to notice | Find it out, or watch them pack the jars and leave | Found, or gone to the queue by processing |
| [The Walk Out](#the-walk-out) | The queue, and the voice at the barrier | Escort the queue up; walk out for good | Opens in processing; names put down are read at the wipe |
| [Settle Accounts](#settle-accounts) | Every faction's parley, the boss's table, quartermasters | Clear bounties, pay the book, give back kit, make amends | Open to the wipe; "square" is checked at the end |
| [The Last Broadcast](#the-last-broadcast) | The radio station | Decide what the Zone tells the outside | A tally, closed before processing; it airs in processing |
| [Personal Goals](#personal-goals) | Your background | One last goal, errand-sized | Open to processing |
| [The Last Entry](#the-last-entry) | The Names board at the bend | Write your own line | Open to the wipe |

### Last Rounds

Each faction's closing arc: the majors' codas, made playable. [MAJORS.md](MAJORS.md#last-rounds-the-codas-in-the-epilogue-week)
owns the scenes and their pay; this design owns the calendar, the standing versions' carriers and the beats they leave.

| Coda | Who gives it | What you do (the ways) | What it changes in the world and the beats |
|---|---|---|---|
| **The Gate** (the Haven reads the List at the north road) | The chair; open to anyone the Haven hasn't exiled | Escort the houses' people to the bend (up to 3 groups); hold the road while the names are read; read a name of one of your own dead characters; read the crossed names yourself (with `list.truth` 1, and Trusted or ★ Elder) | The List is pinned on every house board. The bend gets the Names board, first filled with the List's names. Under Counted a Bastion officer reads the Haven's number; under Paid at the Door a Red Hand lorry waits on the verge; if the finale found the woman, her name is read last, uncrossed |
| **The last call** (Bastion) | The CO, at the radio, 20:30 on the first evening | Read the last call at the radio (Talk) | The roster board's transcript. From the next evening, the 20:30 watch and the CO's grey choice |
| **The walk and the last roll** (Bastion) | Lieutenant Sedlak, at the next 05:30 | Escort the box of tags from the nail to the bend; side with the Haven and bring a house's people to walk first; bring a man home for the roll (a Deserter's tag, or Twice, if Bastion's route brought him home) | The last roll is read at the halt: the living answer *"Here"*, the dead are answered for by everyone, and nobody answers for the struck off. Tags brought home before 05:30 ([Bring Them Home](#bring-them-home)) are in the box. "AT THE BARRIER" goes on the roster board |
| **The Paper** (Halcyon) | The director's carrier, a beak, from the next free 17:30 | Escort the case to the table about 110 m short; fight off a Red Hand crew if the boss was cheated; persuade the director at the sealing; put your kept pages in the case; take the case by way of a safehouse | The voice: *"Leave it on the table. Step back. It will be processed with you."* The case stays on the Zone's side until the wipe. The paper's ending, its line and its "On the paper" names can change; next season's rule can't |
| **The last line** (the Red Hand closes the book, paid or not) | The boss, at Rudi's crossroads, at dusk on the next free evening after the paper | Stand at the lorry for your own line; hold the crossroads while the horns go for the season's dead; ask the boss about the first line (Trusted, or with a crew name) | Every open line is read out with its name, *"Paid"* written beside the ones that are. The book is nailed open at the last page for the rest of the week. Lines paid in [Settle Accounts](#settle-accounts) before the evening are read as paid |

**The finale's routes get their closing lines too.** Whoever controlled the cure, each route's subject gets its line,
carried here ([FINALE.md](FINALE.md#the-losing-routes)): the Haven's channel says her name every evening if she was found;
Bastion's last call reads the four R numbers; the cook stands at the lorry with `cook.line_paid`; the readout keeps saying
*"Reference stock: wanted"*; and *"WE TRIED."* is chalked on the lab gate if the Independent never finished.

- **Pay:** the majors' (ordinary hold pay, the coda's Fame and Favor), all per life.
- **Caps:** each coda runs live once; its second airing is words only.
- **When nobody comes:** each coda runs live on its first free evening with a player who could take part. One still not
  run when processing begins runs as data at its next hour, and its standing version goes up.
- **Hall lines:** "At the barrier" (Bastion's walk) and "On the paper" (the paper's authors), as epithets in the season
  block.

### Loose Ends

Every questline still unfinished at the cure gets a **short closing stage**, so every story reaches an ending, even a
rushed or bitter one.

**Rules:**
- **Which ones.** Every questline run that started and hasn't resolved at the cure. A questline that never started gets
  no stage: there's no story to end (the Long Table's two have their own item). The majors never need one: every
  climax is settled by the gate, and A Week On still open at the cure closes as data with its ending's line.
- **One stage, one place, one session.** The stage is offered by the questline's own person, at its own place, on the
  existing questline engine (`RZ_Questline`, a new `ClosingStage()` per questline). Its ways come from the questline's own
  approaches, so it takes no new systems.
- **It ends on a built ending.** Each way picks one of the questline's existing endings. Done live, the ending pays half
  its built pay (inside the questline's own limits) and its world effect comes out of the epilogue's lane at half.
- **The rushed and bitter endings.** If nobody does it, the last call comes in the person's own voice, then the grace,
  then the questline resolves as data on its **default** ending at half effect, with no pay. Its epilogue line says so,
  plainly.
- **Errands are never loose ends:** a held errand finishes normally or lapses quietly. Route stages were called off at
  the cure with no pay and no penalty. Lock cards end with the locks.

**The closing stages**

| Questline | The closing stage | The ways | Default (nobody did it) |
|---|---|---|---|
| **H2 The Price of the Door** | **The Last Envelope.** Before the crews pack up, the crew boss's man waits at the house's approach for the last envelope | Carry it (the built "sold" ending); bring the exile home first, and the head pays the crew in tins instead (the house −2 food); tell the chair, and the arrangement ends with the house's line in the book | The head sends a resident with the envelope. *"Nobody asked who carried it."* |
| **H3 The Edge of Town** | **The Last Walk.** A resident declares a bite, and now there are outlets | Get them to an outlet; let the walker walk them, as before; sit with them and let the nurse decide | The walker walks one more and reads twelve names at supper. *"The outlet was four kilometres away."* |
| **Short Weight** | **Count the Stores.** Before the house packs for the queue, the storekeeper's books are opened | Count it honestly (Full Weight); cover for him (Short Weight); let the head find it at supper | The head finds it. The storekeeper eats last |
| **B2 The Census** | **The Last Count.** The CO wants the count before the last call (or, after it, for the roster board) | Bring the real count (Counted); forge it (Forged); burn the cards, the Haven's way | The count stays *"as last reported"*. A clerk files it unsigned |
| **B3 The Last Pilot** | **The Last Flight.** The line is down; he wants to fly once round the Zone at dawn, inside the warning zone, never into the kill zone | Fly with him and bring him back (his name is read at the last roll); talk him into walking with the column at 05:30 instead; the Red Hand's way (`pilot.redhand`): the aircraft sold for parts | He doesn't fly. The aircraft sits on the pad with its doors open until the wipe |
| **R2 Cargo** | **The Last Load.** The driver's last run: the crews want the jar money collected for the last market | Drive the van to the queue instead, with whoever's in the back (`cargo.freed`); hand the van to the crew; follow it (`cargo.buyer_known`, which counts as the Long Table's proof stage) | The van goes, and comes back empty. The driver walks to the queue alone |
| **R3 The Well Story** | **The Last Toll.** The village is packing for the queue; the crew boss wants his last payment | Pay it yourself (one tin); stand with the village (hold the lane, one wave); tell the story one last time (the Fear check) | The village pays in its last tins and walks out lighter |
| **HC2 The Other Two** | **His Coat.** The director asks that someone from the Haven come for the volunteer's coat | Carry the coat to his sister (his name goes on the paper if the paper isn't sealed yet); take his notebook number to her house's board; leave it on the hook at the lab | The coat stays on the hook. *"Nobody came for it. We've labelled it."* |
| **C1 and C2** (the Long Table) | See [the Long Table's last week](#the-long-tables-last-week) | | |
| **A story arc running at the cure** | It finishes as built (its truck on the road, its mission). The Last Convoy doesn't start in the week | — | As built |

- **Who gives it:** the questline's own person (the council head, the walker, the storekeeper, the CO, the pilot, the
  driver, the crew boss, the director). A leader only says who to see.
- **What it pays:** half the built ending's pay, once, to the players who took part. Nothing carries over.
- **Caps:** one closing stage per questline run; at most 2 offered to a player at once, and never on a set piece's
  evening within 600 m.
- **Beats:** a closing stage done live adds a since-the-cure beat at its place (*"A bucket on the well, and 'TESTED.
  CLEAN.'"*); a default adds its bitter line to that place's beat instead.
- **Hall lines:** the season block lists each questline's ending in one line, as today.

### Bring Them Home

The tags, ID cards and keepsakes of the named dead (players' dead who ran with the packs among them) are carried back to
their houses and posts, and a wall of names fills.

**What can be brought home.** Every named dead person in the record with a keepsake somewhere in the world:
- a Bastion soldier's tag and effects, from His Effects' bodies ([ERRANDS.md](ERRANDS.md)), or off a turned one from a
  military pack ([DISGUISE.md](DISGUISE.md#the-last-week));
- a crewman's lighter, or a debtor's last tin;
- a resident's, walk-out's or runner's ID card, off a named young laid down by name ([INFECTED.md](INFECTED.md));
- a researcher's or courier's notebook (the deck's Into the Record);
- a driver's papers from a wreck (His Effects' epilogue variant, The Wreck's Name);
- a player's dead character: their ID card, if the character turned and ran with a pack, or the name off their grave.

**Where home is, and who takes it in:**

| Whose | Taken in at | What happens there |
|---|---|---|
| Bastion's dead | The nail behind the radio, at the stronghold or any post | The sergeant hangs the tag and reads the name on the next watch. Before the 05:30 walk, it goes into Sedlak's box and is read at the last roll |
| A deserter's tag (struck off) | The same nail | His KIA is rubbed out, and if the last roll hasn't run, he's answered for. A living Deserter wanderer who hands in his own tag answers *"Here"* |
| The Red Hand's dead | The book, at Rudi's crossroads | The boss writes *"sent off"* against the line. What he owed stays in the book, against whoever he left |
| Halcyon's dead | The lab | The notebook is read into the record; if the paper isn't sealed, the name goes into its author list |
| The Haven's dead, residents, walk-outs | Their first safehouse (else any) | The head reads the name at supper; it goes under the house's names on the board |
| Wanderers | The last door they rested at | Their name is chalked there |
| A player's character | The safehouse where that character first came in (else any) | Read at supper, by the name setting |

**Ways:** find the body or the pack (by day a named young may be out with the day party; a pack's leader lies up at the
lair), lay it down by name, and carry the keepsake (a mission item: it can't be used or traded). Or take a name off a
grave to a board (the deck's Take a Name to the Board). Or buy a keepsake back from the fence: the grey way, Red Hand
Favor and a line in the book if anyone hears.

**The wall of names.** Every name brought home also goes on the **Names board** at the bend on the north road, with the
place it went home to: *"PAVEL NOVAK. LEVIE."* Phase A starts it with the List's names, if the Gate read them. It's one
to four free-standing boards (props), and its full text is one readable list. At processing, every named dead person
nobody carried home is added in a second column: *"NOT BROUGHT HOME"*. The Long Table's takings never appear, unless the
house was exposed.

- **What it changes:** the tag on the nail, the line in the book, the name at supper, the chalk crossed through at a
  pack's range; Sedlak's box at the last roll; a since-the-cure beat for each place (*"Eleven tags on the nail. One new
  this morning."*).
- **Pay:** errands' small pay for the delivery (Fame 1 with the house's faction, Favor 5), at most 3 paid deliveries a
  player a real day. More deliveries still count on the wall, unpaid.
- **Caps:** once per dead name. Kit off a man you killed is refused (*"You'd know. You were there."*, +3 Infamy with his
  faction). Your own dead character, or a group member's, can be brought home, for no pay: *"You bring your own name
  home. The clerk writes it in without asking."*
- **Hall lines:** *"Brought home: {n} names."* and the first name on the wall.

### Quiet the Ridges

Nobody new turns after the cure ([INFECTED.md](INFECTED.md#after-the-cure-the-last-week)). The packs run down on the
epilogue's clock: hunger rises every 12 real hours, the starving lose a fifth every 12 real hours, and the calls answer
thinner each evening. Hunt the last leaders, or let them die out.

**Who gives it, in their own words:**

| Faction | Its job | The way it wants |
|---|---|---|
| Bastion | **Cull** (*"Not our sector"*): close a contact near a post. Trusted (Champion for a Prime leader) | Kill the heavy, or three of its guard. The contact board: *"P-2 · QUARRY · CLOSED"* |
| The Haven | **Bring Them Home** for a leader the Haven knew by name | Lay it down by name and carry its card home. The chalk: *"JOSEF. HOME D47."* |
| Halcyon | **The last reading**: be within 1 km of a pack's lair at dusk and count the answers, 30 s at two points (Contact Report's rules, no research) | Watch it die out; the readings go into the paper's appendix if it isn't sealed |
| The Red Hand | A book on the ridges: *"Evens on the Quarry. Nobody's betting on the Choir."* | A side bet in Favor on which pack calls last, paid at the last dusk. It never names a player |

**Ways:**
- **Hunt** (Cull, Track Them Home to find the lair at dawn, Burn the Lair). An alpha killed after the cure ends its pack:
  the vigil until dawn, then the rest scatter and die out within a day. Nobody takes up the call.
- **Lay it down by name** (the Haven's Bring Them Home), when somebody knew who it was.
- **Let it die out.** Do nothing, or sit its last call: on the last dusk, be within 1 km of a living leader's lair when it
  calls once and nothing answers.
- **Keep the moat** (the Red Hand). The crew that kept a pack fat can feed it one last time, through the built moat rule's
  feeding (a tin of Favor). It lasts a little longer and calls last.

- **What it changes:** each pack's beat moves to its since version (killed, brought home, burned out); the faction marks
  get a line through them; the evening calls thin out on the radio (*"The Quarry still calls. There's hardly anyone left
  to come."*); a pack laid down adds a since-the-cure beat. On the last dusk, every leader still living calls once, and
  nothing answers. At dawn it lies down at its lair.
- **Pay:** the job's built pay (Cull's Fame; Bring Them Home's), with no samples, once per pack. Sitting the last call
  pays nothing. The Red Hand's bet pays in Favor that the wipe clears.
- **Caps:** one Cull per pack; kills drop no samples; bets at most 15 Favor a player.
- **Hall lines:** *"The ridges went quiet on Day {d}. The last to call was {the Haven's name for it}, at {place}."* and up
  to 3 names of those who sat the last call or laid down the last leader.

### Take Back a Town

After the cure, crawlers no longer refill: the kill ledger stops halving and a town's infection never rises
([INFECTED.md](INFECTED.md#after-the-cure-the-last-week)). So a town cleared stays cleared.

**What counts.** A settlement that had at least 6 crawlers at the cure and has none left. Who took it back: the identities
with kills there in the week (up to 2 names, under the town log's naming rules).

**Who gives it:**
- **Residents** of a Haven house rescued from that town: *"I'd like to see the kitchen once more before the queue."*
  (the deck's Pack for the Queue, which sends you there anyway);
- **Retake the Town**, the built contract, now only for towns with crawlers;
- **Bastion:** a stencil job, *"C-7 · CLEARED"*;
- **the crews:** loot rights: *"The furniture's moved out. So can we."*

**Ways:** clear it by hand; burn the nest (Burn the Nest); clear it quietly at night under stealth's rules; or take a
resident home for an hour first and clear it with them waiting at the edge (an escort that ends at the house).

**The candle.** Each town taken back gets a candle. The approach line says it burns in the window of the first house on
the road in; the optional prop is a candle in a jar on that house's doorstep, because windows differ by building set and
every mark here is free-standing. At most 8 towns carry an epilogue mark: a town that already has a beat gets its candle
there; a new town gets a candle only while fewer than 8 towns carry marks, and otherwise its candle is text only (the
approach line, the house board's count and the hall line).

- **What it changes:** the town stays cleared to the wipe; the town log's best slot gets "taken back" (salience 3), which
  can add a since-the-cure beat; its houses get errands' asks again; the Haven house its people came from says so
  (*"Somebody lit a candle in Brod. Mara's mother asked who."*); the house board keeps a count: *"CANDLES: 5"*.
- **Pay:** Retake the Town's built pay, or the deck card's, once per town in the week. The candle pays nothing.
- **Caps:** one paid clearing per town; a town counts once.
- **Hall lines:** *"Towns taken back after the cure: {n}."* and up to 3 town names.

### The Long Table's last week

If the house was never exposed (`table.exists`, and not exposed, broken or burnt), the family has a last week too. Find
it out, or watch them pack the jars and leave. It's never described, and no ambient line hints at it.

**The week, as data:**
- **The house goes on.** Smoke most evenings; grace at the long table: *"For what we are about to receive. For everyone
  going home."* The host's road meeting is unchanged, with his queue line.
- **The tells keep working, as built.** Takings go on at the built cap (one a real day at most, only while hunger is over
  50), now usually someone walking alone to the queue from a hungry house. Each leaves its built tells: the quiet *"never
  arrived"* headline (Haven-Known players are told), a boot on the verge, a satchel at a fence. So C1 can still start,
  from suspicion 30 and a never-arrived, as built.
- **Loose Ends for C1 and C2.** A running C1 gets its closing stage at its proof: anyone who has its earlier stages can
  bring the proof to a faction at once (no wait for a real day), and C1's last stage (end it, let them leave, tell
  everyone) is offered at the farm. A running C2 offers its last meal; the host's last offer is the queue: *"We're going
  to the queue. We'll keep you a place."*
- **They pack.** When processing begins, the chimney stops. A player within 300 m of the farm sees the family loading a
  handcart: the host, the quiet son, the boy, and the mother last, each with a covered basket. **Look at the basket:**
  *"Jars, packed in straw. Each lid has a date on it, in a neat hand."* That's a tell (+15 suspicion, as the guest rule).
- **They leave.** At the next 05:30 (as data if nobody is near), they walk to the queue and stand at the back, with
  their baskets. The mother says grace over a bowl she doesn't share. Three of them are bodies, inside the queue's 6; the
  rest are text. At the Walk Out they go up with the others, and they're processed with everyone. The farm is left with
  the long table laid and the chairs pushed in.

**Find it out.** Exposure in the week (C1's proof reaching a faction, the basket with C1's proof, suspicion 100, or the
built crossings) works as built: the headline, each faction's reaction, no more takings, and the epilogue's beat moves to
its exposed state. Then:
- **At the farm:** C1's last stage, and the built burn by Bastion within a real day while Bastion's stronghold stands. A
  farm is never a holding, so the world settling doesn't stop it.
- **At the queue:** the queue moves away from them. The ways are words and the built rules: tell the corporal (under
  Bastion's route, their names are taken, *"detained"*); tell the chair (the Haven's reaction); end it by hand (the Long
  Table's built shooting rules, and the queue's safe-ground cost until the house is exposed); or let them walk. If they
  reach the barrier, the voice asks *"Name?"*, like everyone. The outside doesn't care.
- **The boy:** if anyone talks the boy out of the queue (C1's built way with the boy), he becomes the Kid, or one more
  child at the nearest house: *"He eats what he's given."*

- **What it pays:** C1's and C2's built pay. Nothing new.
- **Hall lines:** none if it was never found: the absence is the horror. If it was exposed in the week, C1's ending line
  goes in the season block, as built.

### The Walk Out

The Perimeter has stood down since the cure. In processing, the queue finally moves: escort people to the north road
barrier, and your character can walk out for good, retired with a hall-of-fame line.

**The one crossing.** It's the gates opening, a day early, one person at a time:
- only in phase C (processing);
- only at the north road (`day2.checkpoint`); every other checkpoint turns people back as built;
- only out. Nothing comes back in, nobody comes back, and nothing past the barrier is ever shown or said;
- the outside says one word: *"Name?"*.

**A change to built behaviour** (`World/RZ_Perimeter.c`): in phase C, at `day2.checkpoint` only, an unarmed person on the
road who walks up to the barrier hears *"Name?"* from a post speaker instead of being turned back. Anyone armed, or off
the road, is turned back as built. Everywhere else, the built turn-back holds to the wipe.

**Who gives it:**
- **the queue**, and the route's table, which sets the order (below);
- pinned locals walked there by errands' **Processing** card, who go with the Walk Out;
- **the voice at the barrier** (dialogue's `post` speaker; the action menu's "Give your name" without dialogue).

**Ways:**
1. **Walk the queue up.** At each change of shift in phase C (05:30 and 17:30), the queue walks: up to 6 scene people and
   any pinned locals, from the queue to the barrier, with escorts (one escort group, never at the hour of a coda at the
   barrier). On the road: what's left of the crawlers, and on a Red Hand road, the deck's One Last Toll. At the barrier
   each one gives a name, one at a time, and goes through. The queue shuffles up, and new people join at the back from
   the houses.
2. **The order is the route's terms.** Bastion: the roster first, civilians attached after (the census: names taken
   again at the barrier's halt). The Haven: the houses' order (her house, the far house, the bite book or the crossed-out
   first). The Red Hand: the lad sells a place near the front, *"The queue's free. A place near the front isn't."*, for 1
   tin (15 Favor), which walks you up with the next change of shift instead of the one after. Halcyon: the authors, the
   volunteers or the newly bitten first. Independent: first come, first served. Nobody: as it stands.
3. **Walk out yourself.** Any time in phase C, walk up the road unarmed: anyone armed is turned back as built, so hand
   your weapons to a friend first, or drop them (the built loose-item cleanup takes them).
   At the barrier: *"Name?"* Give it (the name this character played under, through the note filter; *"a survivor"* if you're anonymous). The barrier arm lifts a hand's width, and you go
   through. The character is **retired**: no death, no grave, no journal of the dead; the inventory is gone with it. You
   go back to the deploy menu and can start a new character for what's left of the week, or stop there.
4. **Put your name down.** In phase B, at the queue's table, put this character's name down for the walk. If you're
   online in phase C, you walk it yourself. If not, at the wipe the voice reads the names put down, and someone in the
   queue answers for you: your line says *"Walked out, Day {d}. Somebody answered for you."* A name put down comes off
   if that character dies.

**Who may walk out:** a character alive with 2 or more hours played in this life, not in a fight, once per identity a
season. A Wanted player can walk out; their poster changes to *"WALKED OUT. NOT PAID."* The barrier asks nothing about
bites: the outside asks only for a name, and nothing ever says what it did with anyone.

- **What it changes:** the queue shrinks and refills from the houses; each house chalks *"GONE HOME: {n}"*; residents
  pack (*"I've packed twice. I keep taking the kettle out."*); Bastion walks out last of all: the CO stays on the net to
  the wipe, *"Somebody has to be on the net."*; the boss doesn't queue, *"Somebody has to stay and count."*; the reel's
  "Going home" preset. The queue's beat moves to its since version each walk: *"The queue moved this morning. It hasn't
  since."*
- **Pay:** nothing. The walk is its own end. Escorts pay errands' small pay through the Processing card at most.
- **Caps:** once per identity a season; one escort group per walk; at most 6 walkers' bodies; the safe-ground rule covers
  the road from the queue to the barrier in phase C.
- **Hall lines:** *"Walked out"*: up to 12 names with their epithet or title and the day (*"Mara, Favourite Rascal, Day
  52"*), then *"and {n} more"*.
- **Fallbacks:** with no checkpoint, the walk goes to the edge point nearest the Haven's stronghold safehouse along the
  nearest road (one of the line's built mission points), where the voice asks the same. With no land edge at all (an
  island map), the Walk Out is off, and everyone is processed at the wipe.

### Settle Accounts

Clear bounties, pay what you owe the book, give back borrowed kit, make amends.
[REPUTATION_NAMES.md](REPUTATION_NAMES.md#14-the-last-week-settling-accounts) owns the hooks and the budget; nothing here
is free.

| Account | Who settles it | Ways | What it changes |
|---|---|---|---|
| Hunted by a faction | Its parley, open with everyone from the cure (*"It's over. Talk."*), once per 2 real hours per faction | Talk (the parley's amends offer), pay, or Come In: walk unarmed to its outlet or gate and wait (for Bastion, the surrender rule) | Infamy worked off through `Forgive()`; the leader's opener changes |
| A Kill on Sight bounty | The same | Leave Kill on Sight through the parley's amends or settling up | The bounty comes down as built; the poster's since version: *"SETTLED. DAY {d}."* |
| Infamy with a faction you can visit | Its people | Amends errands, paying, settling up, the knock, Come In | All through `Forgive()`, inside the 30 a life. The budget doesn't reset for the week |
| A line in the book | The boss's table, at the crossroads | Pay it in Red Hand Favor; pay someone else's (a debtor's mother's, a dead crewman's); or the deck's The Toll Man's Last Debt | The book marks it paid. At the last line, it's read aloud as *"Paid"* |
| Borrowed kit (signed out to someone else) | Its buyer's quartermaster | Hand it back; clean kit from the Red Hand's shelf sells back to the fence at half | No catch, no Infamy. The buyer hears it came home: *"Your jacket came home. Clean, mostly."* |
| A leader at −2 | That leader | Make it right | Through `Forgive()`, once a season per leader |
| A promise (errands' Hand to Hand, a callback) | The person promised | Keep it, or go and say so | A closing line: kept, or *"You meant it. That counts for something."* |
| Borrowed food (the deck's Borrowed) | The lending house | Carry it back, or lie for the borrowers | ±2 food at a house |

- **Square.** At the start of phase C, and again at the wipe, a player is **square** with a faction if their Infamy with
  it is under 15 and they have no unpaid line in the book. It's kept in the season record.
- **Pay:** nothing. Settling costs Favor or work, and pays only the line.
- **Caps:** the forgiveness budget, the parley's 2 real hours, `Forgive()`'s floors. Nothing settles a Hunted standing in
  one go.
- **Hall lines:** *"Left the Zone square with all four"*, under the names policy. It changes nothing next season.

### The Last Broadcast

Decide what the radio station tells the outside about the season. The outside never answers.

**Who gives it:** the radio station (built, `World/RZ_RadioStation.c`), and every faction lobbies for its version, face to
face:
- the Haven's chair wants the names read;
- Bastion's operator wants the roster read, effective strength and all, and *"No reply"* logged;
- the Red Hand wants nothing said: *"Nobody rats. Not even on air."* (and the lad sells airtime, a tin a minute, which
  buys nothing: the tally decides);
- the director wants the paper read, Day 1 and not before.

**What it can say** (each needs something in the open):

| Version | Needs | It reads |
|---|---|---|
| **The names** | Always | The wall of names, the houses' dead, Bastion's dead by name, the struck off as "KIA", the season's named dead. Never the never-arrived unless the house was exposed |
| **The three weeks** | The three weeks in the open: Proof's The Whole of It, or HC1's pages read out (`weeks.*`) | What Halcyon did before Day 1, from its own pages |
| **The truth** | At least two stronghold pages in the open (`majors.pages_open` ≥ 2) | Every page in the open: the List, the Day 3 orders, the book's back page, the site log |
| **The plain line** (the default) | — | *"This is the Zone. We're still here."* |

**Ways to take part:** bring something to the station this week: a name from the wall, a page, a reel recording, the
List's copy. Each player who brought something has one say. The decision is a tally in `RZ_QuestVote` at the station,
closed by missions' decide-scene hold (5 seconds, one objection each), any time in phase B. A Radio Operator's say counts
twice.

- **When it closes:** the tally has a need of one evening; it's squeezed and given its last call (*"The station goes out
  at processing, with or without your names."*) like any closing item, and closes before processing begins. Nobody
  decided: the plain line.
- **What it changes:** in phase C it airs on every open channel on the hour, in the reel's slot, and in full on the last
  dusk. The radio ham writes it down if the finale found him: *"Got it."* Then a long hiss. Nothing. Morale from the
  epilogue's lane: the names, Haven +2; the three weeks, Halcyon −2 and Haven +1; the truth, every faction −1 and the Red
  Hand −2; the plain line, nothing.
- **Pay:** nothing.
- **Caps:** once a season; no player's name on air except the names honoured on the wall (honour lines, under the names
  policy).
- **Hall lines:** *"The last broadcast: the names."* (or the version), in the season block.

### Personal Goals

Each background's goal gets a finisher. In the week, a life's personal goal is its background's **last goal**:
errand-sized, finishable in a session or two, and never "deliver samples", which the cure ended.

| Background | Last goal | What you do |
|---|---|---|
| Nurse | The last round | Treat three people, or sit with a bitten resident until they're dosed or it's over |
| Mechanic | One for the queue | Get a car running and leave it at the queue, *"for the queue"* |
| Hunter | The cabin | Reach your cabin (the built goal's town) and bring one thing back to a house |
| Police Officer | Close the file | Take one truth from the deeds store (a "What you didn't tell me") to the person it's about |
| Ex-Con | The stash | Go back for the old stash (the built goal's town) and leave it at the queue for whoever needs it |
| Lab Technician | Into the record | Bring a dead researcher's notebook home (Bring Them Home), or carry one name to the paper |
| Radio Operator | Sign off | Make a last broadcast with the "Going home" preset, or bring something to the Last Broadcast |
| Unemployed | Something to do | Finish two pieces of closing work of any kind |
| Soldier | Answer the roll | Be at the last roll, or bring a tag home to the nail |
| Doctor | The last patient | Walk a bitten local to an outlet that will serve them (What the Cure Costs Here) |
| Raider | Square the book | Pay one line in the book: yours, or someone else's |
| Elder | Read a name | Read a name at the Gate, or at a house's supper (Read Them Out) |
| Drifter | One more look | See every stronghold once more, or walk out |

- **Who gives it:** your background, at your first spawn in the week, and the guidance's goal line.
- **What it changes:** a since line from whoever it touched; nothing else.
- **Pay:** as built goals pay (a Legacy point and 10 Haven Fame), both wiped with the season. Your "And you" lines gain
  *"Finished what you came for."*
- **Caps:** one last goal per life, and once per identity per background in the week.
- **Hall lines:** none per player; the season block counts them: *"{n} finished what they came for."*

### The Last Entry

Write your own line on the memorial board.

- **Where:** the **Names board** at the bend on the north road (on a map with no checkpoint, at the Haven's stronghold
  safehouse gate). The same boards hold the wall of names; the last entries are a column of their own.
- **What you do:** "Write a line" on the board: up to 80 characters, through the note filter, signed with the name you
  play under (or unsigned, if you're anonymous). One line per identity a season; you can rewrite it until processing
  ends. A character who walks out can write theirs on the way.
- **What it changes:** the board's text (reading it shows the newest 12, then *"and {n} more"*); the Haven's clerk reads a
  few at supper in phase C.
- **Pay:** nothing.
- **Caps:** one line per identity; admins can strike a line (`/epilogue strike`); a struck line is gone, not starred out.
- **Hall lines:** only if **Epilogue > Archive last entries** is on (off by default): then an admin picks up to 5 with
  `/lastnotes`, one per identity, for the season block.

### What the week adds to the hall of fame

Records froze at the cure. The week adds only these lines to the season's block, each under the names policy, and nothing
else carries over:

| Line | From | At most |
|---|---|---|
| At the barrier | Bastion's walk | its names, as epithets |
| On the paper | The paper's authors | its names, as epithets |
| Each questline's ending | Loose Ends | one line each |
| Brought home | Bring Them Home | a count, and the first name |
| The ridges went quiet | Quiet the Ridges | one line, 3 names |
| Towns taken back | Take Back a Town | a count, 3 town names |
| Walked out | The Walk Out | 12 names, then a count |
| Left the Zone square with all four | Settle Accounts | names (reputation's line) |
| The last broadcast | The Last Broadcast | one line |
| Finished what they came for | Personal Goals | a count |
| Last entries | The Last Entry, if archived | 5 |

### A week, three ways

- **A busy server** (12 regulars, 1:1 time). The Gate and the last call on the first evening, the walk at dawn, the paper
  and the last line by the third evening, all live. Loose Ends are taken in the first days; two packs are culled by Bastion
  players and one is brought home by name. Nine towns are cleared: eight candles stand, and the ninth is text. The tally
  at the station goes to the names on day 5. In processing the queue walks at 05:30 and 17:30, and seven players walk out.
  Nothing closes as data except the Short Weight stores, which nobody wanted to count.
- **A quiet server** (3 regulars, evenings only). The Gate runs live; the paper waits for an evening with a Halcyon-Known
  player and gets one on day 4. B3's pilot gets his last call on day 5 (*"He says he's flying at dawn, with or without
  anyone."*), nobody comes, and he doesn't fly. Two players clear one town and bring four names home. Nobody decides the
  broadcast, so the plain line goes out. One player puts her name down in phase B and isn't online at the wipe: the voice
  reads it, and someone answers.
- **A lunchtime player** (12:00–14:00, never a dusk). She misses every set piece live, and reads them on the boards and
  from the clerk and the operator. She does Bring Them Home and Settle Accounts at midday, writes her line on the Names
  board, and walks out at 13:10 on the last day, between the two walks.

## The epilogue deck: last errands

The last errands are errands' epilogue deck, dealt by errands' dealer ([ERRANDS.md](ERRANDS.md#after-the-cure-the-epilogue-week)).
This design owns their content. They're small, close by, and always about what happened: the beat near you, your
standing, the phase. Each is built on existing blocks, sets `m_bAfterCure`, and is offered only when its beat was picked
(`NeedFlag("epilogue.beat.<key>")`) or its condition holds.

**Rules:**
- **Each card once per identity per beat.** It pays through errands' small-pay ledger (Fame 0–2, never above 39 with a
  faction; Favor 3–10, at most 40 an hour), in the giver's real stock. No item comes from anywhere but the giver's shelf at
  the player's tier.
- **Not limited by the season's two cards a day**, so a player can finish the closing work in a week of normal play.
- **Most pay in glory:** your name on the mark, a line from the giver, a resident who tells others who helped.
- **Nothing counts for the hall of fame's records,** which froze at the cure.
- **Picked per player:** up to 2 offered at once, from beats and closing work within 3 km, the player's standing (who will
  even talk to them) and the phase. A stranger gets cards from the queue and the houses. A player Trusted by the Red Hand
  gets the toll man's. A Hunted player gets Come In. A stranger only asks at one of errands' spots.

| Card | Giver | Built on | Offered when | Feeds | The grey way |
|---|---|---|---|---|---|
| **Carry the Dose** | the route's people | deliver a sealed cold box (a mission item, not a dose) to a named holdout in an abandoned town | the route has outlets, and the town log or a board note names a holdout | — | open it: one dose for you (only if bitten; it counts as your outlet dose), none for the holdout, 10 Infamy with the route's faction |
| **Pack for the Queue** | a resident (Haven Known) | visit points, then deliver: two things from their old house, in the town they were rescued from | the house has people who were rescued in | Take Back a Town | bring one and say you couldn't find the other; they never find out |
| **Bring a Cup** | anyone in the queue | hand over one food or water item (pays as a food hand-in) | the queue exists | — | — |
| **The Key Where It Was** | the kitchen note | find the key in the named house (the built key system) and give it to Petr in the queue | the note was read | — | use it yourself: the room behind the door searches as a locked room does. Petr keeps your place in the queue anyway |
| **Borrowed** | a fed house's council head | deliver 3 food back from the house that "borrowed" it | the season had borrowing (the shared pot's transfers) | Settle Accounts | the borrowers' head asks you to say you couldn't find them: Haven Fame with them, and Infamy with the lenders if they hear about it later |
| **Take a Name to the Board** | a resident or the Preacher | talk, then visit: take a dead person's name from the record or a grave to the stand board, a house board or the Names board | a board exists and the record has dead nobody has written up | Bring Them Home | — (no pay: the name goes on in biro, with yours under it) |
| **The Toll Man's Last Debt** | a Red Hand toll man | deliver 3 food to the Haven house the crew took from | the Red Hand's route, or the book balanced | Settle Accounts | pocket it: 5 Haven Infamy. *"Course you did."* |
| **Sign Here** | Bastion's corporal | talk to three people in the queue and bring back their names | Bastion's roster or census terms; Bastion Known | The Walk Out (the order) | forge them: the roster gets a seeded joke name. Unless you're Bastion Trusted, it's found out at the next phase: 5 Bastion Infamy |
| **Copies for the Far House** | Halcyon | an unarmed delivery of the method to the farthest Haven house | Halcyon's Published terms | — | sell the copies to the Red Hand fence: Red Hand Favor, 10 Halcyon Infamy |
| **His Effects, epilogue variants** (The Tags; The Wreck's Name) | the faction that lost the man or the truck | visit the body or the wreck, take the tags or the driver's papers to the faction's nearest post | a lost man or a lost-truck beat | Bring Them Home | errands' His Effects rules (the fence buys the photo frame) |
| **Clear the Way** | the route's faction, or the Haven | kill tagged: loose ones on the road to the queue | crawlers or a pack within 1 km of the queue road | The Walk Out | — |
| **The Last Watch** | an outpost commander (Known) | hold the gate for 10 minutes at the evening change of watch, against the outpost's usual infected | the outpost held all season, or lost its commander | — | leave early: the commander mentions it next session |
| **Read Them Out** | a council head | talk, at supper (or its standing version) | the house has dead | Personal Goals (Elder) | — |
| **Last Letters** | the Courier | an unarmed delivery from the houses to the queue | the queue exists | — | — |
| **One Last Toll** | a crew boss | take a toll on the queue road (Haven Infamy), or talk the crew out of it | the Red Hand's route, or the book balanced | The Walk Out | the toll is the grey way |
| **Into the Record** | Halcyon | deliver a dead researcher's notebook to the paper's table, or the lab after the sealing, so their name goes on the record | a researcher or courier died | Bring Them Home | — |
| **Somebody Should Tell Them** | a resident | talk, then a pack: their relative runs with it. Ends in Bring Them Home, or a grave | packs, with names walking with them | Bring Them Home, Quiet the Ridges | — |
| **The Boy** | a council head | escort the Kid to the queue or a house. He's an escort, not a companion: he stays there | the boy got out | — | — |
| **Processing** (errands) | a pinned local | walk them to the queue, 150–250 m short of the barrier; never to the barrier itself | the cure is out, and a checkpoint is within 1.5 km | The Walk Out | talk them into supper instead |
| **What the Cure Costs Here** (errands, card 41) | a sick local | the outlet table decides whether the terms reach them and at what price; the card offers only the way the table allows | the terms keep this local from a dose | Personal Goals (Doctor) | errands' card |
| **Come In** (errands' amends family) | a faction that hunts you | an amends contract: walk unarmed to its outlet or gate and wait (for Bastion, the surrender rule) | Watched or Hunted, before processing | Settle Accounts | — |

## Scenes: who stands there, and what if they're shot

- **Slots:** the queue and the lab gate are **lasting scenes**; supper or a coda is **the evening's set piece**. At most 3
  scenes have bodies at once: 2 lasting and 1 evening. The evening slot is shared with the majors' set piece, which keeps
  its own body rules. If players are at more places than that, the evening piece goes first, then the queue, then the lab
  gate. A scene that can't spawn shows its home mark and its approach line instead.
- **Escorts** (the codas' walks, the Walk Out's walks, Loose Ends' escorts): at most 3 escort groups server-wide, bodies
  only within 400 m of a player, never into the kill zone except the Walk Out's last stretch at the north road in
  processing.
- **Bodies:** at most 6 people per scene, from the one scene-people spawner (Puzzles > Civilian, or a faction's guard
  prefab under its usual rules). They spawn within 300 m of a player and are removed past 400 m, inside the one body
  budget.
- **Clearance:** before spawning, the spot must be free of vehicles and players within 8 m (VERIFY: the query). If it
  isn't, the scene slides along the verge up to 30 m; failing that, it waits.
- **Safe ground:** the queue, the road from the queue to the barrier in phase C, and the lab gate follow the safe-zone rule
  (the lab already does). Within 80 m, firing with no infected within 40 m costs 2 Infamy with the scene's faction and
  draws its guards' fire for a few minutes. At the queue that's the Haven, unless the route's faction has people there.
  Bounty hunting can still happen, at that price.
- **Shooting scene people:** they count as people. Shooting at one costs 4 Haven Infamy; killing one costs 15 (the
  wanderer rule) plus 5 with the route's faction, and the guards fire back as the shooting-back rule says. The scene stays
  one person short until the next phase, with a since line: *"A gap in the queue, and a bag on the verge nobody's
  touched."* It's never refilled early.

## The journal, the news, the hall of fame, next season

- **The commit notice and the first-spawn news** give four lines: the cure (the route and how it was won), the player's
  own line, one beat near them, and *"where to look"* (two places named, with no markers).
- **The journal.** The majors' "The season" tab gets two sections: **As the Zone tells it** (every beat the player has met,
  listed by place with its line, plus **And you**) and **Unfinished** (the closing work open to this player: what, where,
  and its last call if it has one). The cure line is always there. **In phase C the whole list opens**, so nobody's ending
  stays unseen. Without the layout, `/season` gives the same as a notice.
- **The hall of fame** freezes at the cure. Each season gets one block, **The season**: the ending (the route line, how it
  was won, up to 4 deciders), the Zone's Fame line from season Fame (*"The cure: the Haven, by force. The Zone's Fame:
  Bastion, 1,640."*), the season's stories (the majors' lines), up to 8 names with epithets, and the week's lines
  ([above](#what-the-week-adds-to-the-hall-of-fame)). It replaces separate "Named in the Zone", "At the barrier" and "On
  the paper" lists. It's shown in `/hall` and the hall menu.
- **`last_season.json`:** the route, how it was won, the terms, the deciders, the majors' endings, the Last Broadcast's
  version, and the 10 strongest beats as plain lines.
- **Next season** belongs to Seasons, the finale's next-season rules and the majors' story rules (their first-days rumour
  and first board note), which read those lines. All of them are fixed at the cure. The epilogue adds nothing of its own.

## Edge cases

### No cure: the Unsigned last day

When an After-weeks wipe is due within **Warn hours** and there's no cure, a new world fact, `IsLastDay()`, becomes true.
`IsEpilogue()` stays false, so research, samples and Halcyon's work carry on, and the cure can still come.
- The record is built, without a route beat, and its beats show.
- The broadcast uses the built long-stall line: *"The perimeter will be maintained indefinitely."* The line stays up.
- Chalk tallies appear on boards on the verge, 150 m up from each barrier, counting to the day. They're in the warning
  zone, so a drone comes down over you while you read.
- The majors resolve at half effect (their `ResolveAtSeasonEnd`) and run their no-cure codas.
- There's no week, so there's no closure set: no Walk Out, no outlets.
- The hall of fame's ending: *"Nobody won."*

### A late cure

A cure that comes less than **Season > Minimum epilogue hours** (56, a third of the week) before an After-weeks wipe
pushes the wipe back to the cure plus that minimum, and says so. This also applies in *whichever comes first* mode. The
record is rebuilt at the cure. Every clock is a share of the epilogue's real length, so the phases shrink with it and the
closing work is squeezed sooner. For admins with fixed restart days, **Season > No stand in the last hours** (off by
default) holds the final stand back instead.

### A shorter epilogue

An admin can set **Epilogue hours** lower. Everything scales: at 72 hours, phase C is about 10 hours and the closing work
is squeezed from the start. Under 24 hours the closure set is off (only beats show), because nothing can be finished in
it, and the Walk Out with it.

### Wipe Off, and an empty server

- **Wipe Off** (by hand): phase B never ends by itself; `/epilogue phase C` starts processing.
- **An empty server:** if nobody is online for 2L ÷ 7 real hours (48) during phase B, phase C starts at the next login
  (**Epilogue > Empty server ends early**). The wipe can come earlier, never later.

### A restart mid-week

The record, the beats, the marks (as data), the closing work's states (decided, open, last call given), the wall of names,
the last entries, the names put down for the Walk Out, the deck's once-per-beat flags and the phase are all saved. Props,
scene bodies and escorts are not; they come back when a player is near. Every beat checks itself again the next time it's
shown.

### Shipping mid-season

Counters that started late use their versions without numbers. The town log only knows what happened after it shipped, so
towns with earlier events get no beat. The closure set reads what exists at the cure.

## Performance

| What | Budget |
|---|---|
| The record | One snapshot in one tick at the cure, processed over 10 ticks, committed in one step. About 12 KB, plus 150 B per player entry |
| The town log | 2 entries for each town where something happened, plus up to 16 visit identities per town. About 30 KB at most |
| Beats | About 85, frozen. About 28 KB. The validity check is lookups only |
| The closing work | Open items and their states, about 4 KB; the wall of names, 200 names at most, about 10 KB; last entries, 80 characters per identity; names put down for the Walk Out, about 2 KB |
| Marks (data) | 40 during the season, 100 in the week |
| Mark props | Spawned within 150–200 m of a player (the distance varies by mark), removed past 220 m and never before they've been up 10 minutes, so pop-in doesn't give away where a player is. 8 per player cluster (players within 600 m of each other share), 30 on the server, 2 spawned per 5 s tick, and at most 3 epilogue props in any 300 m (mission style's rule). Priority: the cure's, the lab's and the queue's home marks first in their cluster, then the Names board, then the Long Table's tells, then by salience. The tribes' props keep their own pool (30) through the same spawner. Never saved |
| Scenes | 3 with bodies (2 lasting, 1 evening), 6 people each, spawned within 300 m, removed past 400 m. Escorts: 3 groups |
| Text | An approach line once per mark per player per session; 4 lines of news; the chatter's existing slot; at most 3 reel entries per session; the notice lanes' limits |
| Ticks | Marks and scenes on the 5 s tick, by player cluster. Beats only at the commit, at each dusk and each real day. The closing work's clocks on the questline tick |

The save grows by about 100 KB at most, and that goes with the wipe. Measure the two prop pools together (up to 60 props).

## Any map: where things stand

Every place is something the map already provides: settlements, holdings, safehouses, the lab, the stations and the canon
spots (`day2.checkpoint`, `redhand.crossroads`, `origin.site`).
- **On the verge, never on the road.** A mark by a road stands 6–10 m from the road's centre line (VERIFY: road width from
  the road network; else 8 m), at least 25 m from a junction, with nothing else within 3 m. Supply trucks, players' cars
  and anyone driving fast never meet a board in the road.
- **Where marks stand:**
  - at a holding's gate (on its road, else on a ring round its centre);
  - on a town's road in, where a road crosses its radius; with no road network, on the side facing the nearest lived-in
    place;
  - or by a building's door (`RZ_BuildingQuery`).
- **Always** on dry land (`LandPointNear`). Never in the warning zone, except the north road's queue, Names board and
  tables, and the checkpoint marks, which stand outside the kill zone and the barrier's turn-back.
- **Free-standing props only:** boards, posts, sheets on posts, tables, candles and crates. No wall decals, no windows,
  and nothing painted over another prop. Layers and coats of paint are text on one board, so the design works with any
  building set.
- **Fallbacks:** with no checkpoint, the queue and the Names board go to the gate of the Haven's stronghold safehouse, and
  the Walk Out to the nearest land edge (above). With no farm, no tribes or no majors, their beats and closing work simply
  never apply.

## Multiplayer and small servers

- The world's story is one story, shared by everyone. Personal lines are per identity, on the owner-only payload.
- Offline players are named as recorded, never by where they are now.
- Two players at the same mark see one prop; reading it is per player.
- A grey choice or a tally is decided with the decide-scene hold by the players who take part, and everyone hears what
  was decided (not who, if they're anonymous).
- Nothing in the week changes standing on its own. Closing work pays as its ledger does.
- A player who walks out is gone for that character only; their group carries on.
- **Small servers:** with 2–4 players the same names would fill everything. So notables are at most half the identities
  with deeds, each name appears in at most 2 beats, and at least half the beats name nobody. Personal lines still go to
  every player. Every closing item has a solo way.

## Settings, prefabs and admin tools

**Epilogue** (on the game mode; no initialisers, created with `NewWithDefaults`):

| Setting | Default |
|---|---|
| Enabled | on |
| Names | Full (or Titles only, or Off) |
| Season marks | on |
| Season mark days | 2 (season days, and at least 8 hours played) |
| Mark props per player cluster | 8 |
| Mark props on the server | 30 |
| Scenes with bodies | 3 (2 lasting, 1 evening) |
| People per scene | 6 |
| Notables | 12 |
| Towns | 8 |
| Since-the-cure beats | 20 |
| Reel | on |
| The world settles | on |
| Closing work | on |
| Walk Out | on |
| Empty server ends early | on |
| Archive last entries | off |
| Test scale | 1 (0.008 runs a 168-hour week in about 81 minutes) |

**Season settings:** Epilogue hours (168; the built attribute default changes from 48), Minimum epilogue hours (56), No
stand in the last hours (0, which is off).

**Optional prefabs.** Each one left empty means text only: sign board, sheet on posts, stencil board, price board, notice
on a post, candle, candle in a jar, grave marker, wanted poster, trestle table, crate (no storage), bunting, ribbon, truck
wreck, tape between posts, handcart, names board. Scenes use Puzzles > Civilian and the faction guard prefabs.

**Commands:**
- **Admins:**
  - `/epilogue preview`: builds a record from the live world without a cure, and lists the beats and the closing work it
    would pick, in chat and in the log;
  - `/epilogue phase A|B|C`;
  - `/epilogue strike <name, beat or entry>`;
  - `/epilogue rebuild`: rebuilds from the saved snapshot;
  - `/epilogue close <item>`: closes one closing item as data now (for testing);
  - `/lastnotes`.
- **Players:** `/season`, `/anonymous`.

**Log lines:**
- `Epilogue: story built (N beats, M players, K open items)`
- `Epilogue: beat <key> at <place> (weight W)`
- `Epilogue: beat <key> dropped (<why>)`
- `Epilogue: beat <key> since (<why>)`
- `Epilogue: phase B`
- `Epilogue: last call <item>`
- `Epilogue: closed <item> (live|default)`
- `Epilogue: walked out (<n> today)`
- `Epilogue: prop budget full (<cluster>)`

**Tests:** the runtime test harness gets a last step. It finishes the cure, sets Test scale to 0.008, and walks the fake
player past the lab, the queue and a safehouse through phases A to C. It uses one outlet, hands in one keepsake, closes
one open questline live and lets one close as data, writes a last entry, and walks out. It reports the beats picked,
dropped and moved to since versions, and every closing item's end.

## Build in phases (each one playable)

1. **Words.** The record (snapshot and commit), beats with their checks, and every carrier that talks: leaders (with the
   action-menu fallback), residents, named people, wanderers, radio and the reel, boards, the news, the journal, and the
   hall of fame (the freeze and The season block). The dose outlets. **Epilogue hours** 168. Until the finale exists, the
   route is the most-Fame faction, *handed over*.
2. **Marks.** Through `RZ_Marks`, with verge placement, shared with the Long Table's tells (the tribes' props get their own
   pool on the same spawner). The town log, season marks, graves, posters and the Names board.
3. **Scenes and the deck.** The queue, supper, the lab gate, scene safety, and the epilogue deck on errands' dealer.
4. **The closing work.** The closing clock (room left, last call, grace, default). Loose Ends' closing stages on the built
   questlines. Bring Them Home and the wall. Settle Accounts on reputation's hooks. The Last Broadcast. Personal Goals.
   The Last Entry. The Walk Out (with its Perimeter change). The Long Table's week.
5. **Hooks, as the other designs land.** The finale's keys, the outlet table's terms and the locks; Last Rounds (the
   majors' codas and their standing versions); Quiet the Ridges and Take Back a Town (with tribes and crawlers); titles;
   dialogue topics and grey choices; the deeds store and truth beats.

Every phase ends with `tools/compile_check.sh` and the runtime test, and ships with its docs.

## Docs in the same commits

The epilogue gets no doc of its own (decision 16 lists the four new docs).
- **DESIGN.md:** a new "The epilogue" section after Cure research: the week and its phases, the outlets, the calendar, the
  world settling, the closing work's clock and its eleven items; the town log and marks under Town memory; Seasons:
  Epilogue hours 168, the minimum epilogue, the last day, the hall of fame freeze and The season block; the Perimeter: the
  Walk Out at the north road in processing.
- **LORE.md:** Endings rewritten for routes and the week in the world, and for the Walk Out: the barriers stay closed
  until processing, and then only the north road opens, outward, one name at a time.
- **REPUTATION.md:** route control; titles from the best life; Wanted and its since versions; outlets read standing, never
  clothes; Settle Accounts.
- **MISSIONS.md:** the epilogue deck and the closing work, with ✅ only when built. **QUESTLINES.md:** each questline's
  closing stage.
- **UI.md and UI/LAYOUTS.md:** "As the Zone tells it", "Unfinished" and "And you" in the journal; any widget names.
- **WORKBENCH_SETUP.md:** prefabs, settings, commands, the VERIFY count, log lines, the play-test checklist.
- **ROADMAP.md and README.md:** the week and its closing work, built or planned.
- Admins with fixed restart days need the 48 → 168 change and the minimum epilogue in the changelog.

## Writing the epilogue

[TONE.md](TONE.md) owns how it sounds (its 3.5 is the epilogue's phrasing). The rules this design adds to the writing
guide:
- One object, one name, one day, one mistake. A mark never sums up the season.
- Every route has its grey side and its dark joke, and the joke comes from a rival. Jokes are aimed at institutions,
  never at a named player's death. The losing side carries on with dignity.
- A player is named for something they did, once per beat.
- **The outside never answers, and never changes its words because of what players did.** After the cure it says only its
  processing order, the line at the paper's table, and *"Name?"* at the north road in processing. Nothing comes after it.
- A closing stage's rushed or bitter ending is said plainly. A walk-out is a goodbye, never a joke.
- No pronouns for seeded names: leaders, packs' leaders, wanderers.
- The Long Table is never described. The Haven's dark side is still said sideways. Bastion still signs for things. The
  Red Hand still prices things. Halcyon still counts.

**The writing load, honestly:**

| Part | Lines |
|---|---|
| Route-specific: route marks, the lab, the queue, the four stronghold leaders, secure channels, outpost commanders by loyalty, door notices, route chatter, readable clauses, outlet offers by terms, refusals and empties, Independent terms, how clauses, evolving steps | about 180 |
| Route-neutral beats: about 75 beat types, each with a home and up to 3 echoes | about 270 |
| Leader topics, grey choices and residents | about 50 |
| The epilogue deck: 21 cards, each with a brief, success, failure and the grey way | about 85 |
| The closing work: Loose Ends' 8 closing stages (about 10 each), Bring Them Home's take-ins, Quiet the Ridges, Take Back a Town, the Long Table's week, the Walk Out, Settle Accounts, the Last Broadcast's four versions and the lobbying, Personal Goals (13), the Last Entry | about 300 |
| **Total** | **about 885**, about five built questlines' worth |

Lines are built from parts where they can be: a route clause, a place line, a "you" line. Tier 1 ships first, about 150
lines: the cure, the lab, the queue, the four stronghold leaders, the outlets, the news, and the Walk Out. The leaders'
plain lines for the week are counted in TONE.md.

## Guardrails

- **Sealed Zone.** Nothing and nobody comes in, ever. Until processing nobody goes out; the queue stands 150–250 m from
  the barrier and the paper's table about 110 m, both outside the kill zone and the turn-back. In processing, the north
  road alone lets people out, one name at a time, and nothing past the barrier is shown or said. The outside never changes
  its words because of the route.
- **No sleep, no time skip.** Phases run on real time. Set pieces on the game clock wait for the next evening, air twice,
  and leave a standing version. Nothing in the week is skipped.
- **Nothing overpowered.**
  - Beats pay nothing.
  - Doses are given on the spot, only to the bitten, one per life, from a capped stock, priced as each faction's terms
    would price them.
  - The deck pays through errands' small-pay ledger, once per identity per beat; closing stages pay half their built
    ending, once.
  - Grey choices and closing work share the epilogue's lane.
  - Records freeze at the cure; the week adds lines, by fixed rules.
  - Nothing carries into next season except hall-of-fame lines.
  - Strongholds and their leaders are untouched; the world settles.
  - Clothes never open anything.
- **Performance first.** Everything is data. Bodies and props appear only near players, under caps, and nothing physical
  is saved.
- **Any map.** Places come from the map and canon, marks stand on the verge, and every feature has a fallback.
- **Multiplayer.** One story for everyone and lines per identity, delivered to everyone online at the commit. Late joiners
  get their own way in, and can still finish things.
- **Most of the map stays abandoned.** At most 8 towns carry an epilogue mark.

## Risks

- **Writing load:** about 885 lines, about five built questlines' worth. Mitigation: build lines from parts, ship tier 1
  first (about 150 lines), and keep each closing stage to about 10 lines.
- **A week is long.** If players run out of things to do by day 4, the week drags. The closing work is the answer; measure
  in play-tests how much of it gets done, and when. A server that's finished everything by day 5 has a quiet two days,
  which is an ending too.
- **Unbuilt designs.** The finale, the majors, tribes, titles, dialogue and the deeds store each have a fallback, and their
  beats and closing work never apply until they exist, but the best moments need them. The finale must provide its keys
  in the shape named here.
- **The Walk Out** changes the built Perimeter and the lore's "the gates open with the wipe". Keep it to the north road,
  in processing, outward, one name at a time. VERIFY: removing a character without a death and returning the player to
  the deploy menu.
- **Balance of the outlets at the very end:** given on the spot, only to the bitten, one per life, with small stocks. Even
  so, check bite rates after the cure's −70%, and make sure no outlet trivialises the last days, especially the free crate,
  *by need*, *the book first* and the Red Hand's free phase C.
- **Contradiction:** a frozen beat overtaken by the live world. The check on every showing, the since versions and the
  world settling cover it. Test with a restart mid-week, a leader killed by a player, an outpost taken while settling is
  off, and a Long Table exposed during the week.
- **Defaults that feel like a timer.** The squeeze could close something a player meant to do tomorrow. The last call
  comes first, in the giver's voice and in the journal's "Unfinished"; the grace is a session of play with someone who
  qualifies; engaged items wait.
- **Props:** two pools (marks 30, the tribes' props 30) through one spawner. Measure frame time at 60 props. A busy evening
  near the north road may show fewer marks than designed; the reserved home marks and the text fallback keep it readable.
- **Time acceleration:** if it can't be read (VERIFY), the calendar assumes 1:1, and players with fixed hours rely on the
  standing versions.
- **Verge placement on unknown road widths:** a board too close to a narrow road, or a queue where players park. The
  clearance checks and the slide-along rule help; play-test on Everon and Arland with a supply truck driving past the
  queue.
- **Names in public places:** frozen text, the note filter, `/anonymous` and `/epilogue strike` limit it, but a name that
  passes a plain word filter can still be offensive. The Last Entry is the most exposed. Admins need to know the strike
  command exists.
- **Grey choices and tallies decided by a few** can feel like griefing (someone closes Bastion's net for everyone).
  Mitigation: the decide-scene hold with objections, the lane's cap, and an announcement naming what was decided.
- **Bounty hunting and ambushes at the hubs** the news sends everyone to, and on the Walk Out's road. The safe-ground cost
  may not deter determined players. Watch it in play-tests; the news names places, never times.
- **The Long Table leaking** through the town log, the Preacher, readables, wanderers or the Names board. Every one of
  those excludes takings unless `table.exposed`. Test a season where the house is never found and check that no
  epilogue line mentions it.
- **Changing the default epilogue length** (48 to 168 hours) moves the wipe later, and the minimum epilogue can push an
  After-weeks wipe back. Admins with fixed restart days need both in the changelog.
- **Save size:** about 100 KB more during the season and the week. Check it against today's `state.json`.
- **The one-tick snapshot** at the cure copies from about a dozen systems. Measure it; if it hitches, copy the largest
  parts (the town log, profiles) on the tick before the cure is announced.
- **Small servers:** on 2–4 players a few names would fill everything. The caps need a play-test with 3 players.

## Examples

- **A Bastion season** (built pacing). On Everon the Bastion route held the lab on Day 46 (how = held), under the roster
  terms. The majors ended Every Name, The Call Stands, The Book Balances and Under Seal. Ana is online at the Haven house in
  Levie when the story commits. Within a minute her notice says: *"The cure went to Bastion, who held the lab. They're
  issuing it at the north road against name and signature."* Then: *"Word is you carried six people home from Brod."* On
  the step: five candle props, and 11 chalked on the stone. At the north road, on the verge 200 m up from the barrier, a
  corporal at a folding table: *"Weapon. Thank you. Name. Sign. No, here. Arm."* Ana was bitten on the way, so she hands
  over her pistol, signs on as *"Civilian, attached"*, and is treated on the spot. A second bite this life will need the
  built treatments. That evening the chair reads the List at the bend, and at 20:30 the last call goes out: *"Command,
  Bastion. Holding at the barrier. Requesting relief. Over."* A long hiss. *"No reply."* At 05:30 she carries a tag she found on the Morton road to the nail, in time for Sedlak's
  box.
- **Independent, sold** (a grey route). Mara and Kessler took the batch on the Independent route and chose to sell it. The
  board at the lab gate reads, in chalk: *"IT'S OURS. ALL OF OURS. 30 FAVOR."* The crate at the queue has the same price
  chalked on its lid, and the Favor paid goes nowhere. The director: *"They're selling it at the crossroads price, minus
  the crossroads. I'm told that's progress."* The boss: *"Somebody's undercutting us with our own business model. I'm
  nearly proud."* Kessler had been Kill on Sight with Bastion for six hours of that life, so his poster hangs at the Morton
  post. On day 3 he goes to Bastion's parley and works his way out of Kill on Sight; the next player past the post reads
  *"SETTLED. DAY 49."* stamped across his face.
- **One safehouse, from its inputs** (the Haven's route, *her house, and her boy*). The house in Levie went from Shelter to
  Settlement (change ×1.5). It has 33 people, 11 dead this season and 9 rescued in. Its rule was the shared pot, and its
  council head is the third this season. Its three beats are the candles, a resident's line about who brought them in, and
  the board's three pinned notes. On day 2 a Remembered player dies on the road, and the next visitor reads: *"The 11 is
  rubbed out. It says 12."* Two thirds of the way through phase B the candles change to *"all out, the wax scraped up for
  the next ones"*. The medic offers "Ask for the dose" for 5 Haven Favor. One is left, and the next comes at six. On day 6
  a player brings Pavel's card home from the quarry, and the head reads it at supper.
- **An outpost that changed hands twice.** Morton went from Bastion to the Red Hand on Day 19 and back to Bastion on Day 33.
  Its beat is one Bastion board at the gate. Its text reads: *"A fresh stencil over a red hand, and the hand still shows
  through. Under both, in chalk, the Haven's KEEP OUT."* The commander, the second Bastion commander here: *"The Red Hand
  had it a fortnight. We're still finding tins in the ceiling."* On the verge 600 m down the road stands the wreck the Red
  Hand left on Day 18: *"A burnt-out Bastion truck, its stencil just readable: MORTON."* The Wreck's Name is offered there
  to Bastion-Known players; the driver's papers go on the nail.
- **The Long Table, never found.** Its suspicion stayed at 26 all season. Two wanderers never arrived. The town log never
  recorded them, so no beat, reading or readable mentions them. On day 3 a woman walking alone to the queue never arrives:
  the quiet headline, a boot on the verge. A player who finds the boot starts C1. On day 5 she has its proof and takes it to
  the chair; the house is exposed, and Bastion burns it the next day. If she hadn't, the chimney would have stopped when
  processing began, and at 05:30 four people with covered baskets would have stood at the back of the queue.
- **One player's own lines.** Vera is a Mechanic locked into the Haven route who swung a council head to the pot at the
  second sitting. Her best Haven life this season was a Favourite Rascal. She's still at the lab when the story commits, so
  line 1 arrives at once: *"Word is you spoke for the pot at the Haven's second sitting."* Line 2 comes from the chair:
  *"You're the one who put the pot before the List. The houses ate on it all month."* Line 3 is a grave on the verge of the
  road out of Brod: *"Two fence palings and a name in biro: VERA. MECHANIC. FIXED THE BUS. DAY 23."* Her current life is a
  two-hour Stranger, but people still place her by her best life. Her last goal is One for the queue: she gets the bus
  running and leaves it on the verge by the queue.
- **A late joiner in processing.** The Red Hand route won on Day 44, under the boss's price. Lukas first joins on the
  week's last day. He spawns in The empty kitchen: *"Gone to the queue at the north road. Took the dog. Petr, the key's
  where it was."* The first leader he meets: *"New face. You missed it. Sit down. The Red Hand has the cure, two tins a
  dose. We pooled tins. The queue's at the north road, and today it moves."* He's offered Bring a Cup and The Key Where It
  Was. At the crossroads the price board reads *"FREE. WE'RE NOT CARRYING IT OUT."* Because it's phase C, his journal
  already shows the whole "As the Zone tells it" list. He has under 2 hours in his life, so he can't walk out; he walks the
  17:30 group up as an escort instead.
- **As it happens: a rescue in Brod, then a fire.** On Day 17 Mara got six people out of Brod. The town log puts it in the
  best slot (salience 4), and a sheet on two posts stands on the road in for two season days. During the season the Levie
  council head opens with *"You're the one who got six out of Brod."* On Day 33 the Red Hand burns Brod, which goes in the
  worst slot; the rescue stays. At the cure the rescue outweighs the fire for Brod's one beat, and the sheet comes back for
  the whole week. On day 4 two players clear Brod's last crawlers, and a candle stands on the doorstep of the first house
  on the road in.
- **A pack's leader.** Josef was a farmer from Levie; the Haven named him, and his pack is the Quarry lot. He was still
  calling at the cure: *"Josef still calls from the quarry every evening. Fewer answer now."* On day 3 a Haven-Known player
  finds him lying up at the lair by day, lays him down by name and carries his card to Levie. The chalk at the range's edge
  becomes *"JOSEF. HOME D47. LEVIE."*, with her name underneath, and the head reads it at supper. The rest of the quarry lot
  scatter and die out within a day.
- **The contest at 90%.** Eleven players are locked, nine of them active: 5 Bastion, 2 Haven, 1 Red Hand, 1 Independent.
  Two other Haven players locked in and haven't been online for four days, so they don't count. Route marks are private,
  so each of the four routes gets 3 of the 12 places. A Red Hand crew boss to a Bastion-locked player: *"You've thrown in
  with the army. They'll make you sign for your own arm."*
- **No cure, then a cure on the last day.** An After-weeks season reaches its last 24 hours at 97% research. `IsLastDay`
  is true, but `IsEpilogue` isn't, so samples still count. Chalk tallies appear 150 m up from each barrier, and the
  broadcast says *"The perimeter will be maintained indefinitely."* At hour 20 the stand is won. The minimum epilogue pushes
  the wipe back to 56 hours after the cure, announced to everyone. The record is rebuilt with a route, the tallies give way
  to the route's marks, and the week runs squeezed: phase C is the last 8 hours.
- **The truth comes out.** On Day 19 Jonas carried three crates for Novak, a Rogue Red Hand crew boss, who said they were
  tins for a Haven house. The deeds store knows they were Halcyon's lab crates. In the week Halcyon's director has a new
  topic for Jonas: "What you didn't tell me". Jonas is Trusted with Halcyon, so the check passes: *"The crates you carried
  for Novak on Day 19 were ours. Novak sold them back to us. Twice."* Asked the same thing, Novak lies again: *"Tins. I told
  you. Tins."* A Police Officer takes that truth to the house it was meant for: Close the file.
- **Abuse attempts that fail.** A notable renames to an advert after the cure: the stand board still shows the name frozen at
  the stand. A stranger shoots into the queue: 15 Haven Infamy per person killed plus 5 with the route's faction, the guards
  fire on him, and a gap and a bag stay on the verge until the next phase. A player kills his own long-lived character at
  the lab gate to get a grave there: a death by his own hand makes no grave. A player spawns a fresh character to walk out
  for a hall line: it needs 2 hours played in the life, and once per identity. Two friends kill each other's turned
  characters to farm Bring Them Home: no pay for a group member's name. A player writes an advert on the Names board: the
  note filter, and the admin strike.
