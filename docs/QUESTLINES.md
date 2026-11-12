# Questlines

Long stories that run over many sessions, New Vegas style: one season-long major per faction
([The majors](#the-majors)), the smaller stories beside them (three per faction at first, and two for
the Long Table). Each stage can be done several ways. Choices set flags that later stages, other
questlines and the ending read. Every ending pays the player and moves the world a little,
always through systems that already exist.

**Status: all fourteen are built, and the four majors** (each section notes how the code differs from the design; the example Short Weight is in [MISSIONS.md](MISSIONS.md#questlines)).
While the majors run, five of the fourteen live inside them: H1 is Every Name's act III, B1 is Await
Relief's acts I–III, R1 is The Book Has to Balance's acts 2–3, and HC1 and HC3 are Proof's inner
lines. Their sections below stay as their own stages' reference.
How a questline runs (stages, waits, conditions, flags, who
takes part, saving) is in [DESIGN.md](DESIGN.md), "Questlines". Who the people are is in
[LORE.md](LORE.md); standing, tiers and leaders are in [REPUTATION.md](REPUTATION.md); the
missions the stages reuse are in [MISSIONS.md](MISSIONS.md).

## The questlines

| # | Questline | Key | Giver | Needs | Spans | Crosses |
|---|---|---|---|---|---|---|
| H1 | [The List](#h1-the-list) | `list` | Haven | Known; Trusted by the end | 6 stages, 3–5 sessions | B2, R2, C1 |
| H2 | [The Price of the Door](#h2-the-price-of-the-door) | `door` | Haven or Red Hand | Known with either | 5 stages, 3–4 sessions | R1, R2, R3, B2, C1 |
| H3 | [The Edge of Town](#h3-the-edge-of-town) | `walk` | Haven | Known | 5 stages, 3–5 sessions | HC1, HC2, R1 |
| B1 | [Await Relief](#b1-await-relief) | `orders` | Bastion | Known; Trusted for the stove | 5 stages, 3–5 sessions | R1, B2, H2 |
| B2 | [The Census](#b2-the-census) | `census` | Bastion | Known | 6 stages, 4–6 sessions | H1, H2, R3 |
| B3 | [The Last Pilot](#b3-the-last-pilot) | `pilot` | Bastion | Known; Trusted to talk him down | 5 stages, 3–5 sessions | HC3, R1 |
| R1 | [The Book](#r1-the-book) | `book` | Red Hand | Known; Trusted to return it | 5 stages, 3–5 sessions | H2, H3, B1 |
| R2 | [Cargo](#r2-cargo) | `cargo` | Red Hand (or Haven) | Known with either | 5 stages, 3–5 sessions | H1, H2, C1 |
| R3 | [The Well Story](#r3-the-well-story) | `well` | Red Hand | Known; Trusted at the end | 5 stages, 3–4 sessions | H2, B2 |
| HC1 | [The Three Weeks](#hc1-the-three-weeks) ✅ | `weeks` | Halcyon | Known | 5 stages, 3–5 sessions | H3, HC3 |
| HC2 | [The Other Two](#hc2-the-other-two) ✅ | `other_two` | Halcyon or Haven | Known with either | 5 stages, 3–4 sessions | H3 |
| HC3 | [Authors](#hc3-authors) ✅ | `authors` | Halcyon | Trusted, cure 50%+ | 6 stages, to the final stand | B3, HC1, H3 |
| C1 | [The House That Went Quiet](#c1-the-house-that-went-quiet) ✅ | `table` | Nobody (the world) | Any | 5 stages, 2–4 sessions | H1, H2, R2 |
| C2 | [A Place at the Table](#c2-a-place-at-the-table) ✅ | `place` | The host | Ate with them twice | 4 stages, 2–3 sessions | C1 |

A session is a few hours of play. A questline usually takes 2–5 real days, because of its
waits and because some stages only open at dusk, at night or at a cure percentage.

## The majors

✅ Built (phase 5, simplified as each major says). One season-long story per faction, its climax
on a cure milestone. Each stronghold leader has a page they'd rather burn, and every page put in the
open (`majors.pages_open`) makes the next leader's "read it" one step easier, once. How the lane,
the acts, the votes and the dynamic fallbacks work: [DESIGN.md](DESIGN.md#the-majors). Their stage
missions: [MISSIONS.md](MISSIONS.md#the-majors-acts). Each act's remaining approaches are more
people and choices on its scene (`Director/RZ_MajorRest.c`), listed under each major.

| Major | Faction | Key | Grows out of | Climax | The page |
|---|---|---|---|---|---|
| [The Book Has to Balance](#the-book-has-to-balance) | The Red Hand | `book` | R1 The Book (acts 2–3) | Settling day, 50% | The book's back page |
| [Await Relief](#await-relief) | Bastion | `orders` | B1 Await Relief (acts I–III) | The muster, 60% | The Day 3 orders |
| [Every Name](#every-name) | The Haven | `every_name` | H1 The List (act III) | The second sitting, Aftermath or 70%, after the muster | The List |
| [Proof](#proof) | Halcyon | `proof` | HC1 (`weeks`) and HC3 (`authors`) as inner lines | The reading, 75% | The three site-log pages |

The climaxes come in that order. After the cure each major has a coda (its last act) in the
epilogue: the Haven's Gate, Bastion's last call and last roll, the Paper, then the Red Hand's last
line, in that order, at the north road (`day2.checkpoint`) and Rudi's crossroads. Each coda leaves a
**standing version** until the wipe and airs again as words on the next free evening
([EPILOGUE.md](EPILOGUE.md#the-evening-calendar)). **With no cure**, the codas end as radio on the
last day: the Haven's Gate at the chair's house at the first wipe warning, Bastion's last call in
the last 24 real hours, the Red Hand's ending on its current branch (Proof Unsigned as built).

**Rules every major keeps.** One per faction, outside Max active. Every act has a default, which
pays nobody and reads the stronghold leader's traits or the rule in force. Stronghold leaders are
never replaced; their traits are only read. Pay goes through the majors' caps (25 Fame, 40 Infamy,
60 Favor per player per faction a major; 40, 40, 80 across them), and world effects through the
season budget's majors lane. A major's ending writes the season's story (`last_season.json`).
Persuade is the built fixed score. Every sway, ask and persuade is an action on a person in the
act's scene.

### The Book Has to Balance

The Red Hand ([LORE.md](LORE.md#the-red-hand)). The boss holds the book, and the cure is coming:
when the line stands down, a toll is a lorry across a road, and the Red Hand is only what it's owed.
The back page (who outside owes it) goes missing in front of every crew. Code:
`Missions/Questlines/RedHand/RZ_Major_Book.c` (a subclass of R1's class).

| Act | Opens | Anchor, need, patience | What players do | Default | Status |
|---|---|---|---|---|---|
| 1. Send Rudi off | Collapse's second evening; a set piece 16:00–21:00 at Rudi's crossroads | 50%; 1; 2 offered evenings | Ask a crew boss to come (persuade 1, ★ Ex-Con), pay a greedy one (15 Favor), hold the crossroads 4 min | The crews send Rudi off without players; the page goes missing the same way | ✅ |
| 2. The torn page | After act 1 (R1 stages 1–2, as built) | 50%; 1 | As R1 | The thief burns the page and runs (`book.burnt`, `book.thief` 5) | ✅ |
| 3. Who gets the page | After act 2 (R1 stages 3–6, as built) | 50%; 3 | As R1; its endings are the act's results (the page in the book, kept, Bastion's, burnt) | The page stays out; the thief goes in the pit | ✅ |
| 4. Settling day | 50% (or a lead), a session after act 3; a set piece | the muster; 1; 2 offered evenings | Hold the crossroads while it's read; laugh him off (Known); name him (persuade 2, only with the page in the book); speak for him (persuade 2); stand with him (+15 Infamy); tell Bastion where the crews will be (Bastion: Known); offer his crew a Haven door (Haven: Known) | Page in the book: laughed off; else purged half the time | ✅ |
| 5. The last page | 80% (or a lead), a session after settling day, after the reading; at the boss's seat | the gate; 1; 2 offered evenings | Stand for nothing (persuade 2); sell it by the page (whole book); send it off loud (persuade 2, 1 with a Paranoid or Pragmatist boss, a living thief or a broken book); keep the first page out (Trusted); lift the book for Bastion (unseen); hand the bill to the lab (Halcyon: Known; refused under stance 1 or 3: stand for nothing instead); warn the director | The boss's traits: Paranoid or Pragmatist send it off loud; Greedy sells a whole book; Ruthless, Greedy or a broken book: the boss takes what's owed; else the bill | ✅ |
| 6. At the lab road | The final stand | past the gate | Sent off loud: hold the fire at the crossroads 3 min first. At the cure, `book.stand` from the last page and the room (1 held the road, 2 took the room: the Red Hand route controls the cure, 3 beaten off, 4 nothing; no cure: the wipe's default) | Read as data | ✅ simplified: the stand itself is the finale's, and the last page decides whether Take the Lab is offered to the Red Hand route ([FINALE.md](FINALE.md#take-the-lab-and-the-coat-run)) |
| 7. The last line | The epilogue, after the Paper | the wipe | Stand at the lorry for your line; ask about the first line (Trusted); hold the crossroads for the dead | The boss closes the book alone | ✅ |

| Ending (`book.last`) | The world | Epilogue line | Status |
|---|---|---|---|
| **The book balances** (1 the bill, 2 stood for nothing) | Red Hand morale +7, strength +3; the deciders +15 Red Hand Fame; Debt Collection ×1.2; stood for nothing: a Halcyon–Red Hand truce | *"When the gates opened, the boss of the Red Hand walked out with a book under one arm, and everyone in it watched."* | ✅ |
| **Sold by the page** (3) | Red Hand morale +3, strength +3; the deciders +15 Red Hand Fame | *"...everyone in the Zone had bought their own page back from the Red Hand."* | ✅ |
| **Sent off loud** (4) | Red Hand morale −5, then +5 after the fire; `book.debts_dead`; a page in the open; a Haven–Red Hand truce; Protection Racket and A Quiet Arrangement ×0.8; holders of the fire +15 Red Hand and +10 Haven Fame | *"...the Red Hand walked out with nothing written down. Some of them went home."* | ✅ |
| **Bastion's file** (5) | Red Hand morale −7, strength −2; Bastion morale +5; a Bastion–Red Hand feud; Bust Out ×1.2 | *"...Bastion handed over a ledger with half the Zone in it."* | ✅ |
| **The book doesn't balance** (6) | Red Hand morale −5; The Rat ×1.2 | *"...the Red Hand's book still didn't balance. Somebody had the page."* | ✅ |

**More approaches**: at the send-off, *Sit in for a crew boss* (Known) and *Sound the horns for Rudi*;
at settling day, *Slip off with the real page* (unseen, `book.slipped`); at the last page, *Carry the
bill to the lab* (`book.bill_carried`: the deciders +10 Halcyon Fame at the ending) and *Lift the book
at supper* (unseen, `book.lifted`: +25 Red Hand Infamy at the ending). The **Red Hand lorry** stands at
the crossroads for the send-off and the last line, the **bonfire** when it's sent off loud (optional
props, near players only). The book balancing keeps its deciders' crew names for next season's
greeting.

*Simplified:* the forgery, the sale of the page to the rival, the chair's bid for the ash and the
crew name at the pit aren't built; the sale by the page and the bill are one choice each (no courier
run, no doses moved); the crews at the crossroads are words; the noise of the horns doesn't draw the
infected. R1's own endings still pay as built, inside the caps.

### Await Relief

Bastion ([LORE.md](LORE.md#task-force-bastion)). The burst after the call is Bastion's own Day 3
signal; the runner's page says "expended". The CO has known since Day 3. Before the gates open, the
CO says in front of every post what Bastion is. Command never answers. Code:
`Missions/Questlines/Bastion/RZ_Major_Orders.c` (a subclass of B1's class).

| Act | Opens | Anchor, need, patience | What players do | Default | Status |
|---|---|---|---|---|---|
| I. Reply received | Collapse, after a nightly call (B1 stages 1–2); any commander gives it if no Independent one exists | the muster; 2 | As B1 | A patrol walks the runner's road; the page goes in the stove | ✅ |
| II. Who reads it | B1 stage 3, as built | the muster; 1 | As B1 | The stove | ✅ |
| III. The reckoning | B1 stages 4–5; its endings are the act's results (`orders.reckoning` 1–4) | the muster; 2 | As B1 | The branch settles itself at half effect | ✅ |
| IV. Effective Strength | Overrun, a session after act III; at the radio by the CO's seat | the muster; 3 | The confession, then the true count from the three posts nearest the stronghold: ask (Known; Independent: persuade 2, ★ Police or Soldier), read the roster at night (unseen), or buy the quartermaster's copy (15 Favor); hand the CO the count | The count stays padded (`orders.padded` 3) | ✅ |
| V. The muster | 60% (or a lead), a session after act IV, after settling day; a set piece at the nightly call (19:00–22:30) | the gate; 1; 2 offered evenings | Keep the call (Known); read it to them (persuade 3, easier with the true count, proof, a Merciful or Idealist CO, `weeks.public`, a page in the open; harder with a Paranoid CO) and hold the stronghold's gate 3 min; answer it yourself (persuade 2, Known enough with the Haven counted, proof the lab knew, or a Ruthless or Paranoid CO); hold the relay for the Red Hand 3 min (Trusted, or Known with `orders.rh_knows`, `.mocked` or `pilot.redhand`; always Trusted after `book.settled` 5); take the relay back for Bastion first (Bastion: Known; +10 Bastion Fame, the crew can't hold it); pay the quartermaster for rations before the call (30 Bastion Favor: All Stations' morale hit halved) | The CO decides alone: a Merciful or Idealist CO after "The men were told": All Stations; a Ruthless or Paranoid CO: Bastion, Command; else the call stands | ✅ |
| VI. The last call and the last roll | The epilogue, after the Gate | the wipe | Read the last call at the radio; stand at the halt on the north road for Sedlak's roll (the roll's line counts Struck Off's men: walked home, they answer "Here"; still out, nobody does) | The CO reads it alone; Sedlak walks whoever's left | ✅ |

| Ending (`orders.stance`) | The world | Status |
|---|---|---|
| **The Call Stands** (1) | Bastion morale +6, strength +3; the deciders +20 Bastion Fame | ✅ |
| **All Stations** (2) | Bastion morale −8; Red Hand and Halcyon morale −3; a Haven–Bastion truce; a page in the open; Requisition ×0.8, Hearts and Minds ×1.2; the deciders +20 Bastion and +10 Haven Fame | ✅ |
| **Bastion, Command** (3) | Bastion morale +8, strength +5; Haven morale −8; Requisition and Checkpoint Duty ×1.2; the deciders +25 Bastion Fame | ✅ |
| **Expended** (4) | Bastion morale −10, strength −5; Red Hand morale +5; a Bastion–Red Hand feud; Struck Off and Requisition ×1.2 | ✅ |

**More approaches**: at Effective Strength the duty officer's desk (*Report a padded post*, *Cover for
the commander*, *Take a dead man's name off the nail* as a trick that counts a post, *Buy the fence's
rifle count* for 20 Red Hand Favor); at the muster *Buy the clerk's copy* (30 Favor, `orders.copies`)
and *Walk the chair's clerk to the radio*; at the last roll *Answer Here for a struck-off man* (the
count +1 at the ending). The last call reads the captain's R numbers when the cure is Bastion's,
airs at the next 20:30 as radio once the coda has run, and the walk at the 05:30 after it; Sedlak's
box at the roll counts the tags brought home in the week. A per-life roster flag and a rank text are
kept ([DESIGN.md](DESIGN.md#the-majors)).

*Simplified:* the padding isn't covered post by post (one desk for the three posts), the curfew walk
isn't offered, and the presence floor and the curfew aren't set.

### Every Name

The Haven ([LORE.md](LORE.md#the-haven)). When there isn't enough, who is the Haven for? The council
writes the second rule down, then keeps or changes it once someone has found out what the first
council did, and whose pencil it was. Code: `Missions/Questlines/Haven/RZ_Major_EveryName.c` (a
subclass of H1's class, under a new key).

| Act | Opens | Anchor, need, patience | What players do | Default | Status |
|---|---|---|---|---|---|
| I. The ones who can't work | The season's first evening; a set piece 17:00–21:00 at the house nearest the stronghold | Collapse; 1; 3 offered evenings | Talk the head round (persuade 2, 1 with a Merciful head; ★ Elder, Nurse, Doctor), pay the stores (20 Favor), walk two to a Bastion post (Bastion: Known), walk two back to the road | A Merciful or Idealist head takes all four; any other turns two away | ✅ |
| II. The first sitting | Collapse, a house short (or the Haven's need is food; Overrun anyway); a session later, a set piece at supper | Overrun; 1; the first offered evening | Speak for a rule to each head (Known); table Bastion's cards (Bastion: Known) or the crossroads' tins (15 Red Hand Favor); read the best-fed house's stores (unseen: one head to the pot). The vote (`RZ_QuestVote`) is taken at the end of supper; the chair breaks ties | The council votes as data | ✅ |
| III. The List | After the first sitting, once a house has lost people or The List contract is done (H1 stages 1–4, as built) | 70%; 4 | As H1; sets `list.truth` (1 the farm and whose hand, 2 only "a farm out that way") | Knowledge only: the open stages close | ✅ |
| IV. The hungry week | Overrun, or Haven morale under 45; at the family's house, else the hungriest | 70%; 2; 3 sessions | Pay for the food (30 Favor), hold the house, walk two to a Bastion post, walk three to the far house. The heads lean the way it went | The rule's own answer | ✅ |
| V. The second sitting | Aftermath or 70% (or a lead), never before the muster; a set piece at supper | the gate; 1; the first offered evening | The vote again, from the first's leans; read the List out (persuade 2, easier with proof and a page in the open, harder with half the truth) and hold the gate 3 min; give the copies to the chair; carry them to Bastion or a crew boss (unseen) | Votes as data; the List is kept unless the chair knows the truth and is Merciful or Idealist | ✅ |
| VI. A week on | After the sitting: a house goes short, or a 2-session gap | past the gate; 1; 2 sessions | By the rule: split it between the houses (the pot), feed the far house (20 Favor), pay the collection (25 Red Hand Favor) | The numbers stand | ✅ simplified: one scene, a choice per rule |
| VII. The Gate | The epilogue's first evening | the wipe | Stand with the chair at the bend; read the crossed names yourself (persuade 2, ★ Elder, with the truth, when the ending isn't Every Name); hold the road | The chair reads it with the houses' people | ✅ |

| Ending | The rule (`haven.rule`) | The world | Status |
|---|---|---|---|
| **Every Name** | 1, the pot, and the List read | H1's "The names read" (Haven morale −8, `list.read`, a head steps down); a page in the open; the deciders +15 Haven Fame | ✅ |
| **The Closed Book** | 1, the pot, the List kept | Haven morale +3; `list.buried`; the deciders +10 Haven Fame | ✅ |
| **The Far House** | 2, workers first | Haven morale +5 (−8 if the List was read), strength +3 | ✅ |
| **Counted** | 3, Bastion's cards (`haven.counted`) | Haven morale −5; Bastion morale +5, strength +3; Requisition ×0.8 | ✅ |
| **Paid at the Door** | 4, the arrangement (`haven.paying`) | Haven morale −5; Red Hand morale +5, strength +3; A Quiet Arrangement ×1.2 | ✅ |

**More approaches**: at the family's door *Call it a chest cold* (Nurse or Doctor: all four in); at the
first sitting *Hear who they'd call a mouth* (Elder) and *Hear what each offer is for* (Radio
Operator); in the hungry week *Carry four food over* (the house fed), *Move two mouths at night*
(unseen) and *Hand the stranger to the crew* (`door.exile_sold`); before the second sitting *Gather
the others' pages first*. The chair's backstory is canon: *"Forced the doors of the school gym on Day
2, for a busload the line had turned back."* ([LORE.md](LORE.md#the-long-table)).

*Simplified:* the raid on the first sitting, the winter store, the bus driver and the boy, the
cards' curfew and the rule's readers in upkeep, trucks and Requisition aren't built; the family are
words.

### Proof

Halcyon ([LORE.md](LORE.md#halcyon)). The paper is the only thing from this Zone that will ever go
out. Its first page says Day 1; the clinic's file says three weeks earlier, and the director's site
log says what the lab did with them. Code: `Missions/Questlines/Halcyon/RZ_Major_Proof.c`.

| Act | Opens | Anchor, need, patience | What players do | Default | Status |
|---|---|---|---|---|---|
| I. Day One | Early Outbreak | 25%; 2 | At the clinic: copy the margin first (unseen). Then hand the file in sealed, ask the director to read the date (persuade 2, ★ Doctor, Lab Tech), give it to Bastion's adjutant or the chair's clerk. Sets `proof.day_one` (1–4) and `weeks.records` | The field lead fetches it: Day 1 stays the lab's | ✅ simplified: a scene, not Patient Zero's Ledger |
| II. The Three Weeks | 25%, a session after act I, or at once after the copy: HC1 starts (as built) | 50%; 4 | As HC1; `proof.weeks` and `proof.pages` from its ending | Not started when squeezed: nobody asked (`proof.weeks` 0) | ✅ |
| III. Authors | 50%: HC3 starts (as built); done at its stage 5 | 75%; 3 | As HC3; it publishes `authors.terms` | Goes on | ✅ |
| IV. The reading | 75% (or a lead), after the second sitting; a set piece 17:30–20:30 at the lab, never in a stand | the gate; 1; 2 offered evenings | Ask for the whole of it (persuade 3, easier when the Zone knows, the pages were read, the volunteers named, a page in the open, an Idealist or Merciful director; harder with a Paranoid or Ruthless one or nobody asked); stand with the director (Trusted); put the pages on the table; take up the chair's question (Haven: Known, when the clerk sits); have Bastion countersign (Bastion: Known, with terms, the file or the notebook, or last season's `legacy.terms`; never under Expended) | The director's draft stands | ✅ |
| Before the stand | After the reading, until the final stand begins; at the boss's seat and the lab's records room | past the gate | Sell the director's pages to the boss's man (Red Hand: Known, with the pages; +10 Red Hand Fame, +15 Halcyon Infamy). Once they're sold, clear the line: pay it (40 Red Hand Favor), put the debt in your own name (Red Hand: Trusted; `proof.debt_yours`), or lift them from his satchel unseen. Swap the first page of the draft in the records room unseen (with the pages, `weeks.public` or the three weeks asked about) | Settled as the stand begins: a line nobody cleared (the book not sent off loud, the paper not already the whole of it) is read on the Red Hand's channel: **Two Tins a Copy**. A swap stands under an Idealist or Merciful director (the whole of it, `proof.first_page` 3), else the draft goes back | ✅ simplified: the swap is any hour, with no disguise of its own |
| V. The final stand | The cure | past the gate | The finale's ([FINALE.md](FINALE.md)); HC3 reads it as built | — | ✅ |
| VI. The Paper | The epilogue, after Bastion's walk | the wipe | Walk the case to the table on the north road (110 m, outside the kill zone) and set it down | The duty officer walks it down alone | ✅ |

| Ending (`proof.first_page`) | The world | Status |
|---|---|---|
| **The Whole of It** (2) | Halcyon morale −8 (−3 if the three weeks were public), Haven morale +5; a Haven–Halcyon truce; a page in the open; `proof.haven_gate`; the director's two minutes on the open channel | ✅ |
| **Under Seal** (4) | Bastion strength +3; Halcyon morale −5, Haven −3 | ✅ |
| **Day 1** (1) | The Zone didn't know: Halcyon morale +5, strength +2. It knew: "Origin: under investigation", Haven morale −3, a rumour | ✅ |
| **Two Tins a Copy** (checked first, as the stand begins) | The sellers +5 Red Hand Fame; Halcyon morale −5, Red Hand +5; "Halcyon made this" as a rumour for 48 hours; the boss's man reads page one on the Red Hand's channel; epilogue line *"There's a copy of the lab's first page in every Red Hand camp."* | ✅ |
| **Unsigned** | No cure by the wipe: the paper stays in the lab's safe | ✅ |

**More approaches**: at Day One *Have it read on the crews' channel* (30 Red Hand Favor,
`proof.rh_read`); at the reading *Hand the director the microphone* (`proof.read_air`: under The Whole
of It the director's two minutes go out as a reading on Halcyon's channel); at the Paper *Read the
author list at supper* (Haven: Known) and *Read the paper's first page to the radio ham* (*"Got
it."*).

*Simplified:* HC1's new choice (keeping the director's pages) isn't built, so the pages-on-the-table
choice and the sale need `proof.pages` 2 from another source; the Red Hand's last page doesn't read
the lab's line in the book (`proof.pages` 5). With last season's `legacy.records`, Day One's default
is the director reading the date (`proof.day_one` 2).

### Next season: the story rules

✅ Built (`Director/RZ_StoryRules.c`). A fresh season after a cure takes one small rule from each
major that ended live last season (not by its defaults alone), read from its ending as it stood at
the cure: nothing the codas change carries over (**Season > Story rules**, on; it needs Winner
rules). The rules go on top of the controller's rules and the finale's half rule set, through the
same combine and caps ([DESIGN.md](DESIGN.md#next-season-rules-from-the-last-winner)), and move a
faction's strength at most 2 in all. Players hear them as one line, "Last season's stories". A
season that ended without a cure leaves none.

| Major | Ending | Next season |
|---|---|---|
| The Book | The book balances | Red Hand strength +2 |
| | Sold by the page | Trader Caravan offered ×1.1 |
| | Sent off loud | Nothing |
| | Bastion's file | Bastion +2, Red Hand −2 |
| | The book doesn't balance | The first boss is Paranoid; The Rat ×1.1 |
| Await Relief | The Call Stands | Bastion's share of road posts 0.6 (meets the winner's in the middle) |
| | All Stations | Requisition ×0.9 |
| | Bastion, Command | Bastion strength +2 |
| | Expended | Bastion strength −2 |
| Every Name | Every Name | The Long Table's chance ×0.5; the chair's line on every house board on the first real day |
| | The Closed Book | One rumour for two days |
| | The Far House | The Long Table's chance ×1.5 (never over 1) |
| Proof | The Whole of It | Starting infection ×0.95; `legacy.records` (act I's default reads the date) |
| | Under Seal | Bastion strength +2; `legacy.terms` (the countersignature has its terms) |
| | Day 1 | Specials ×1.05; one rumour for two days |
| | Two Tins a Copy | Red Hand strength +2; the black market starts with one stolen dose |

And three that act through the world: the book balancing **greets a returning player** once by last
season's crew name (words only); **Counted** puts Bastion's first road post of the season on a road
into a safehouse town (a player within 1.5 km of a house); **Paid at the Door** has a 30% chance on the
season's first day that a house puts one person out at the door (one fewer there, a rumour).

### The stand's inputs

✅ Built. The final stand belongs to the finale ([FINALE.md](FINALE.md#the-stand-a-fight-for-the-clean-room));
the majors decide which faction groups a major sends to it (`RZ_Majors.SendsStandGroup`), inside
the finale's **Groups at the lab** cap, the finisher's faction first:

| Faction | Sends a group when | Where |
|---|---|---|
| The Haven | Proof ended The Whole of It (`proof.haven_gate`) and **Territory > Haven guard** is on | At the gate |
| Bastion | The stance is 1 or 2; under 3 its checkpoint; HC3's terms (`authors.terms` 1–2) or its road post (`authors.road_post`); never under 4 | 1–2 and HC3's terms at the gate; 3 and HC3's road post on the lab's road (150–250 m out) |
| The Red Hand | The last page balanced or stood for nothing (`book.last` 1–2), unless Bastion holds the road (stance 3) | On the lab's road |

Take the Lab has one decider: the finale offers it to the Red Hand route unless the last page closed
it (`book.last` 1–4). Questlines hear the stand through `OnFinalStandStart` and `OnFinalStandEnd`
([DESIGN.md](DESIGN.md#the-majors)), and when the clean room changes holder through `OnStandHolder`.

### The season's lines and the season tab

✅ Built.
- **Epilogue lines** by who controls the cure: each ending has a line for when its own faction
  controls it (*"Command, Bastion. Cure secured. Requesting relief. Over." A long hiss. "Bastion
  out."*), the one it wrote for another controller or the Independent, and one for no cure at all
  (*"The gates never opened. The book balanced anyway."*). The own lines never say how the cure is
  handed out: the finale's terms line says that ([FINALE.md](FINALE.md#endings-and-next-season)).
- **The hall's lines** in the season block, under the epilogue's names policy
  ([EPILOGUE.md](EPILOGUE.md#names)): "The season's stories", one line a major with its ending and up
  to four who decided it, at the epilogue's commit (with no cure, at the wipe); "At the barrier",
  everyone at Bastion's last roll; "On the paper", whoever set the case down first, then everyone who
  walked it.
- **The season tab** of the notebook ([UI.md](UI.md#the-notebook)): every major as it stands (its
  act, the rule or stance, the leans, any last call) and each ending told; in the epilogue, *As the
  Zone tells it* and *And you* with what's unfinished.

---


## How to read a questline

- **Premise:** who asks, and the hard edge it's about ([LORE.md](LORE.md)). People are named by
  role, because leaders, wanderers and named people are generated each season.
- **Needs:** tier with the giver, other standing, ★ backgrounds that get an easier or better
  approach, 🔒 locks.
- **Stages:** each one offers one mission, or waits for time or a world condition. Waits count
  real time with players online. Nothing is ever skipped or slept through.
- **Approaches** in each stage:

  | Approach | What it means |
  |---|---|
  | **Talk** | Speak to a leader, a wanderer or a named person. Some lines need a standing check (a tier with someone) or a background ★ |
  | **Sneak** | Do it unseen: lookouts, a heat limit, the night shift |
  | **Fight** | Kill what the stage spawned. Usually costs Infamy with someone |
  | **Bribe** | Pay in a faction's Favor |
  | **Betray** | Take the job, then hand the result to someone else |
  | **Side with** | Bring another faction in: Fame with it, Infamy with the giver |

- **Sets** a flag in `code style`. Flags are the server's for the season. Where a choice is the
  player's own (they'll be remembered for it), it says *(you)*: that belongs to the character
  and ends with the life, like reputation.
- **Uses** names the existing mission or building block a stage reuses. A stage that needs an
  optional prefab offers that approach only when the prefab is set, as missions do.
- **Endings:** what the player gets, and what the world gets.

### Limits every questline keeps

These are the design's targets; where the engine sets its own caps, its caps win
([DESIGN.md](DESIGN.md), "Questlines").

| Consequence | At most, per ending |
|---|---|
| Fame / Infamy / Favor | 25 Fame, 40 Infamy, 60 Favor with any one faction |
| Items | a reward crate, or one item from the giver's shelf at your tier. No new items, no new perks |
| Faction morale | ±10 per faction, recovering as usual |
| Faction strength | ±5 per faction, drifting back as usual |
| Leaders | one outpost or safehouse leader replaced, or one loyalty swing. Stronghold leaders never change |
| Territory | one outpost changes hands, by the usual strength rules. Never a stronghold or the lab |
| Towns | ±15 infection in one town; one town flag (Barricaded, Burned) |
| Safehouses | ±4 people and ±4 food at one safehouse; never below the upkeep floor of 2 people |
| Wanderers | up to two added (an existing role) or one gone |
| Truces and feuds | one started, under the usual rules |
| Research | +3 over the whole questline (scaled like any mission's research) |
| Contract weights | ×0.8–1.2 for one contract, for the season |
| Words | headlines, rumours, open-channel lines, leaders' lines: as many as fit |

And always:

- **The Zone stays sealed.** Command never answers. Nothing and nobody crosses the line, either
  way. The gates open only with the cure. A questline can find out what the outside did *before*
  the gates closed; it never makes the outside act.
- **No faction wins through a questline except the cure finale ([FINALE.md](FINALE.md)), and none
  falls.** A questline's Fame counts toward season Fame like any other Fame. Strongholds never fall.
  The finale is the one place past these limits: its own caps (50 Fame per faction per player over
  all finale content, the route's capped chunk of season Fame, one `AddFinaleResearch` call at the
  seal) replace them.
- **People are data until a player is near.** Everyone a stage needs is spawned only within a
  few hundred metres of a player; nothing a questline leaves is a physical trace.
- **Getting there is forgiving.** A stage step done by reaching a place (a chain's step, a watch or
  patrol point, a house to walk up to) uses the one arriving rule: 15 m of a spot, 25 m of a
  building's or town's edge, in a vehicle too ([arriving](DESIGN.md#mission-building-blocks)). A
  stage's talk, use or read is still the interaction it was; holding a place keeps its own radius.

### Crossing lines

Siding against a faction matters in that faction's own questlines:

- **Personal:** every ending records *(you)* which faction you worked against, if any. That
  faction's questline Talk checks need one tier more from you this life, and its leaders open
  with it ("You're the one who sold the List.").
- **The world:** the flags in [Crossings](#crossings) open, close or bend stages in other lines.

The cure finale's route stages never write this mark: the lock replaces it, and it ends at the cure
([FINALE.md](FINALE.md#the-lock)).

### Contracts that wait for a questline

A standalone contract that tells part of a questline's story isn't offered while that questline
runs, so a player never meets the short and the long version of one story at once. The gate is
the questline's key (`NotWhileQuest`, [DESIGN.md](DESIGN.md#world-facts)), and it's set only when
the contract is on its own: a questline that runs the same mission as a stage isn't stopped by
it. ✅ all.

| Key | Contract kept out | Why |
|---|---|---|
| `list` | The List (Haven #21) | The same crossed-out names; it opens the questline when it's done |
| `door` | A Quiet Arrangement | Stage 1 is its walk |
| `walk` | Walk Him Out | Stages 1 and 5 are this contract |
| `orders` | The Real Orders (also once the questline has ended) | The Day 3 runner's orders are its story |
| `pilot` | Rescue the Pilot | The questline has the pilot |
| `cargo` | Tied Down | Cargo is its long version, with the same van |
| `well` | Protection Racket | A place that doesn't pay |
| `weeks` | Patient Zero | The same clinic records; it opens the questline when it's done |

Last Helicopter has no flights while The Last Pilot has the helicopter, and No Reply never runs at
the post of a running Await Relief (both their own checks).

---

## The Haven

### H1. The List

While the majors run, H1's stages 1–4 are [Every Name](#every-name)'s act III, and its endings are
reached through the second sitting; H1 isn't offered on its own.

Every Haven house keeps a list of names, and some are quietly crossed out. A woman at a safehouse
found her husband's name crossed out while he was still out on a supply run. He never came
back. Her council head wants to know who crosses names out, and why, without the chair of the
council finding out who's asking. The answer is the Haven's oldest shame. In the first hungry
week the council sent its "mouths" to a farmhouse past the last village that said it could feed
them. The farmhouse went quiet, and the council crossed the whole house off.

**Built** ([MISSIONS.md](MISSIONS.md#the-list-the-haven)). The contract The List is one crossed-out
name ([MISSIONS.md](MISSIONS.md#the-haven), #21); this questline asks whose pencil it was. Changes
from the design: it also opens once that contract has been done this season, and "lost people to
upkeep" is read as "ran short, or lost people lately"; the council head's ask is heard on the
radio, in their voice, with who to ask, and the job is offered in person; the asking house's own list is the first copy (fewer copies on a map without the houses);
the reading is held at the gate on the road in. In a season with the Long Table, stage 4's farm is
its house (unless it moved before H1 began), and getting there is the finding: lived in, the house
is found (`list.found_table`); gone, the table is left laid, or burnt. There's nothing to look round
there, and the meal is the house's own. The names read raises the Long Table's suspicion through
`list.read`; Sold doesn't change the exile-leak chance (`list.to_redhand` is published instead).
*(you)* choices are season-wide flags and the record of who took part.

**Needs:** Known with the Haven; Trusted (or ★ Elder) to win the last stage by talk. Not while
exiled. ★ Radio Op, ★ Police, ★ Elder.
**Starts:** an Independent or Rogue council head, once the contract The List has been done this
season or any safehouse has run short or lost people lately.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Find the husband** | **Follow his trail**: a note, his pack, a third clue. Half the time he's alive: he walked off when he saw his name crossed out, and comes home (+1 person, `list.husband_alive`). **Ask the wanderers**: a Scavenger sells his last sighting for 1 food. ★ Radio Op: hears his last call on the Haven channel, grid included | Trail of Clues |
| 2 | **Get three copies of the List** | **Talk** to council heads: a Merciful one hands it over; a Loyal one asks the chair first and refuses unless you're Trusted; a Paranoid one won't. **Sneak**: read it in the stores at night past two lookouts (seen: +8 Haven Infamy, `list.stolen` *(you)*). **Bribe**: deliver 3 food to that house. **Side with the Red Hand**: their list of strays counts as one copy, for 15 Red Hand Favor | A Quiet Arrangement's lookouts; delivery |
| 3 | **Read the pattern** | **Work it out** at a safehouse: the names crossed out in the same week on all three copies, against the ration notes, point to one farmhouse. Wrong: locked 5 min; twice: you only learn "a farm out that way" (`list.half_truth`). ★ Elder or Police: the method spelled out. **Ask the chair** (Trusted): she tells you half and asks you to stop (`list.chair_asked`) | Patient Zero's Ledger (a deduction) |
| 4 | **Go and look** | If the Long Table lives there this season, it's their house: the host meets you at the gate and sets a place (C1 starts at stage 2, `list.found_table`). If not, an empty farm: nine graves, and one too few. **Look round** (three points, 10 s each). **Sit down and eat** (the guest rule applies, [REPUTATION.md](REPUTATION.md#the-long-table)). **Leave and report** | Visit points |
| 5 | **Wait** | One session (2 real hours with players online). Gossip in the news: *"Someone's been asking about the List."* Then the chair of the council asks to see you | Wait |
| 6 | **The chair's decision** | **Talk: read it out**, every house, crossed names too (Trusted, ★ Elder, or Known with `list.husband_alive` as proof): then hold the reading at the stronghold safehouse while the bells bring the infected (3 min, 2 waves). **Keep it quiet**: give the copies to the chair (`list.buried`). **Side with Bastion**: deliver them to a Bastion seat for its census (`list.to_bastion`). **Side with the Red Hand**: sell them to a crew boss (`list.to_redhand`). Either delivery must pass two Haven lookouts unseen | Hold; delivery |

**Branches:** stage 1 decides whether you have proof at stage 6. Stage 2's Red Hand copy makes
`list.to_redhand` cheaper to reach (the crew boss already trusts you). Stage 4 starts C1 when the
Long Table is there.

| Ending | You get | The world |
|---|---|---|
| **The names read** | Haven Fame +20, 30 Favor; the council head who asked remembers you as having *saved* them | Haven morale −8, recovering; the safehouse leader who crossed out the most names is replaced; the Long Table's suspicion +30. Headline: *"Every Haven house read the List at supper last night, crossed names and all."* |
| **Buried** | Haven Fame +10, 20 Favor; the chair remembers you helped | Nothing changes. If the Long Table is found later, the Haven deals with it quietly (C1 leans to walking them out) |
| **The census** | Bastion Fame +15, 30 Bastion Favor; Haven Infamy +15 | Haven morale −10; players the Haven exiled take +5 Bastion Infamy (as Names for Bastion); B2 starts at stage 3 |
| **Sold** | Red Hand Fame +10, 40 Red Hand Favor; 20% it's found out: Haven Infamy +20 | A hungry safehouse sells an exile's location 35% of the time instead of 25%, for the season; R2 reads `list.to_redhand` |

### H2. The Price of the Door

A hungry safehouse pays the Red Hand to be left alone. This month the crew boss doesn't want
tins. He wants a stranger: a man the house put out at night for stealing from the stores, and
where he sleeps now. The council head would rather not know what she's selling. Someone has to
carry the envelope.

**Built** ([MISSIONS.md](MISSIONS.md#the-price-of-the-door-the-haven-or-the-red-hand)). Stage 1 is
A Quiet Arrangement's walk, with its lookouts. Changes from the design: "short" means hungry, or it
has lost people lately, and never the chair's house or one at the Preacher's prayers; stage 4 is a
wait, then the vote; Bastion's checkpoint for the night is in words, and Under Bastion doesn't make
road posts likelier (`door.bastion` is published); Paid in tins is −4 food once and `door.tins`,
not a cost at every second upkeep; the cook takes the chair in words (the leaders' usual successor
fills the seat). `cargo.exile` is now `door.exile_sold`, set whenever he's walked to the crew; in a
season with a Long Table he's one of the people who never arrive.

**Needs:** Known with the Haven or with the Red Hand (each side gives its own version). ★ Ex-Con,
★ Elder.
**Starts:** an Independent or Rogue council head of a short safehouse; or the crew boss at the
nearest Red Hand outpost.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Carry the envelope** | **Deliver it** to the crew's drop past two lookouts (`door.paid`). ★ Ex-Con: lookouts spot you only at 25 m. **Open it**: a grid and a name (`door.knows` *(you)*). **Refuse**: the house finds someone else; go to stage 3 (`door.refused`) | A Quiet Arrangement |
| 2 | **The man at the grid** | He's a Haven exile, sleeping in a barn. **Talk: warn him**: he moves on as a wanderer elsewhere (`exile.warned`). **Bring him back to plead** (escort to the safehouse; he needs someone Trusted with the Haven, or ★ Elder, to speak for him at stage 4) (`exile.home`). **Betray: hand him over**: walk him to the crew's drop (+15 Haven Infamy if a Haven wanderer or lookout sees, `exile.sold` *(you)*). **Leave it**: the crew finds him in a day | Escort |
| 3 | **The crew comes to collect** | Only if the price wasn't paid. **Fight**: hold the safehouse gate against a Red Hand raid, 3 min (`door.held`). **Bribe**: 40 Haven Favor or 25 Red Hand Favor to the crew boss (`door.bought`). **Talk him down** (Trusted with the Red Hand, or ★ Ex-Con): a debt is a debt, but it can be yours (`door.debt_yours` *(you)*). **Side with Bastion**: bread to the nearest Bastion post for a patrol, and a Bastion checkpoint goes up on the road for the night (`door.bastion`) | Trader Caravan's raid; Over the Fence |
| 4 | **Wait, then the vote** | One session. The house votes on its council head. **Speak for her** (Known). **Speak against her** (Trusted, or ★ Elder): the cook takes the chair. **Speak for the exile** (with `exile.home`; Trusted or ★ Elder): he's let back in (+1 person) | Wait; talk |
| 5 | **The house stands, or pays** | `door.held` or `door.bastion`: put the barricades up (Fortify the Safehouse). `door.paid` or `exile.sold`: carry next month's envelope, which is tins again. `door.debt_yours`: pay the first instalment, in the crew's pit (House Rules) or 20 Red Hand Favor | Fortify the Safehouse; House Rules |

| Ending | You get | The world |
|---|---|---|
| **Stands alone** | Haven Fame +20; Red Hand Infamy +10 | The town counts as Barricaded (until the next restart, as Fortify); the council head is replaced if the vote went that way; the crew boss remembers you crossed him; `door.shut` |
| **Under Bastion** | Bastion Fame +12, Haven Fame +8 | Bastion checkpoints are twice as likely on the roads near that town for the season (within the road-post caps); the house is on Bastion's census (`door.bastion`) |
| **Paid in tins** | Red Hand Fame +10, 30 Red Hand Favor; Haven Fame +5 | The house loses 1 food at every second upkeep for the season and is never raided by that crew. With `exile.sold` and the Long Table active: the crew sells him on (`door.exile_sold`), and *"never arrived"* is in the news |
| **The debt is yours** | Red Hand Fame +15, Haven Fame +15 | Nobody was sold; R1's page has your name on it |

### H3. The Edge of Town

At one house, someone who declares a bite is walked to the edge of town at dusk, and the walker
comes back alone. He has walked eleven people out. A nurse at another house says eight in ten of
them could have walked out of Halcyon's lab instead. The walker says the lab writes down the
other two, and at least at the edge of town somebody holds your hand. Both of them are right.
The houses want someone to settle it.

**Built** ([MISSIONS.md](MISSIONS.md#the-edge-of-town-the-haven)). Stages 1 and 5 ("the walks
stay") are the contract Walk Him Out at the walker's house; the lab road is the contract Cold
Chain. Changes from the design: the nurse's house has a medic when one does (else any other
house); Cold Chain here has no cure threshold; the cut dose's "a session later" is a sixth stage (a
wait); the walker isn't a named person the world keeps (a headline and `walk.walks_stay`). Brokering
the lab road needs Known with Halcyon and Trusted with the Haven: a saved man makes it Known,
`weeks.public` makes it Trusted again.

**Needs:** Known with the Haven. Talking to Halcyon at stage 3 needs Trusted with Halcyon.
★ Nurse, ★ Doctor, ★ Elder, ★ Lab Tech.
**Starts:** the nurse (a named person at a safehouse with a medic), after any player has
finished Walk Him Out.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **One more walk** | A bitten member waits at the edge of town at dusk. **End it** or **let him walk on**, as in the contract (`walk.walked`). **Take him to the lab** for the trial dose before he turns (escort; 8 in 10 he walks out and joins a safehouse, `walk.lab_saved`; else `walk.lab_lost`) | Walk Him Out; escort |
| 2 | **The walker's tally** | The graves at the edge of town (three points) and his notebook. **Talk** (Known): he says every name. **Sneak**: read the notebook while he's on watch. ★ Elder: he asks you to walk the next one with him | Visit points |
| 3 | **The lab's numbers** | What happens to "the other two". **Talk** to a field lead (Trusted with Halcyon, or ★ Lab Tech/Doctor). **Sneak** into the records on the night shift when the lights go dim (a heat limit; seen: +8 Halcyon Infamy). **Bribe** a courier: 20 Halcyon Favor. Sets `walk.knows_other_two` *(you)*: HC2 opens at stage 2 | Letters' quiet way; talk |
| 4 | **Wait; two offers** | One session. The research director offers, on the secure channel, to take every bitten Haven member and sell doses to Haven clinics at Known prices (Radio Operators hear it; others hear it from the nurse). The Red Hand offers the cut dose, cheap, for every house. **Broker the lab road** (Known with Halcyon). **Refuse both**. **Side with the Red Hand** (Trusted with the Red Hand) | Wait; talk |
| 5 | **Make it real** | Lab road: an antidote batch to a Haven clinic before it spoils. The walks stay: the walker's last walk, with you. The cut dose: a crate from a Red Hand drop to a safehouse, unseen by Halcyon's courier | Cold Chain; Walk Him Out; delivery |

**Branches:** a saved man at stage 1 makes the lab road easier (Known is enough at stage 4). If
HC1 ended with the three weeks public (`weeks.public`), the houses don't trust the lab: brokering
it needs Trusted with the Haven.

**The packs** ✅ ([INFECTED.md](INFECTED.md#people)): each "walk on" feeds the nearest pack within
2 km of the edge (a named young, a walk-out) instead of raising the town's infection. An ending that
stops the walks (the lab road, the cut dose) leaves that pack hungry: +20 hunger, kept hungry for 2
season days, so it tests the house's wire.

| Ending | You get | The world |
|---|---|---|
| **The lab road** | Haven Fame +15, Halcyon Fame +15 | Research +2; Haven morale −4 when the first of the other two are written up (a headline names them, or numbers them if HC2 ended that way); a Haven–Halcyon truce starts if the rules allow; `walk.lab_road` |
| **The walks stay** | Haven Fame +15; the walker and the nurse both remember you helped | The walker becomes a named person at his house; Halcyon morale −3 |
| **The cut dose** | Red Hand Fame +12, 30 Red Hand Favor; a session later, Haven Infamy +10 | One safehouse loses 2 people; R1 counts the Haven's debt. Headline: *"Half of them walked out. The Red Hand calls that a good rate."* |

---

## Task Force Bastion

### B1. Await Relief

While the majors run, B1 is [Await Relief](#await-relief)'s acts I–III, and its endings are act III's
results. ✅ Built ([MISSIONS.md](MISSIONS.md#await-relief-task-force-bastion)); the changes from this design
are listed under its endings.

Every night the operator reads the numbers to Command and writes "No reply". Some posts chase a
voice on the hour (the No Reply contract). At one outpost the operator heard something else after
the call: a long burst, too broken for voice. He wrote "Reply received", then crossed it out, four
nights running. His commander wants it dealt with before the stronghold hears about it. The truth
is smaller and worse. The relay on the hill still rebroadcasts the Day 3 signal on a loop, because
nobody ever acknowledged it: the signal a runner set off with, decoded by hand, and never
delivered. The runner is still out there, and so is the decoded page in his pocket.

Command never answers. The "reply" is Bastion's own Day 3 signal, coming back.

**Needs:** Known with Bastion; Trusted (or ★ Soldier) to take the page to the commanding officer.
★ Radio Op, ★ Soldier.
**Starts:** an Independent outpost commander, in person, after the nightly call (20:30). Or the
No Reply contract, when its source is the tape: the reel carries the burst after the voice, and
the line starts at that post.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **The operator's tape** | Take it to the relay on high ground and keep the relay running 3 min while it's compared (a horde after the first minute). ★ Radio Op: hears it's a loop at once (half the hold). **Sneak**: copy the tape rather than take it (`orders.copy` *(you)*) | Radio Relay |
| 2 | **The runner** | The relay's log gives the hour. Follow his trail across three towns (his pack, his fire, his body), past turned soldiers at the last. His page is water-stained, legible in parts: *"...hold the line... no relief will... authorised to... expended."* | Step chain (Struck Off's trail); Missing Patrol |
| 3 | **Who reads it** | **To the commanding officer** (Trusted, or ★ Soldier): it goes in the stove (`orders.stove`). **To the commander** who sent you: he reads it to his men at the shift change (`orders.read`). **Side with the Red Hand**: a crew boss reads it on the open channel, for 40 Red Hand Favor (`orders.mocked`). **Side with the Haven**: the council chair keeps it, as proof of what Bastion was told (`orders.haven`) | Delivery |
| 4 | **Wait** | One session. Read: two men walk off the post. Mocked: Bastion's open channel is silent for a night while the Red Hand laughs every hour. Stove: the operator is moved to the stronghold. The Haven: the chair says nothing, yet | Wait |
| 5 | **The reckoning** | *Read:* the commanding officer sends a squad to arrest the commander. **Fight** for him (+20 Bastion Infamy; he goes Rogue and keeps his post, and Rein In follows), **arrest him yourself** (escort to the stronghold), or **talk the CO down** (Champion with Bastion, or Trusted and ★ Soldier: loyalty +25). *Mocked:* find the operator who leaked it, and bring him in or walk him to a safehouse. *Stove:* a copy, if you made one, can still go to anyone in stage 3, once. *The Haven:* Bastion rations to the chair's house, as her price for keeping it | Escort; Struck Off; Desertion Route; Hearts and Minds |

| Ending | You get | The world |
|---|---|---|
| **Into the stove** | Bastion Fame +20, 30 Favor; the CO remembers you helped | Bastion morale +5; the outpost commander is replaced by a Loyal one; the log still says "No reply" |
| **The men were told** | Bastion Fame +8; Haven Fame +8 if the deserters reached a safehouse | Bastion morale −8; that commander goes Rogue or is replaced; up to two Deserter wanderers; the safehouse +2 people |
| **On the open channel** | Red Hand Fame +15, 40 Red Hand Favor; Bastion Infamy +20 | Bastion morale −10; a Bastion–Red Hand feud starts; `orders.mocked` |
| **The Haven's leverage** | Haven Fame +15, Bastion Fame +5 | Bastion's Requisition never picks the chair's safehouse this season; Haven morale +5 |

**Built, with these changes** (key `orders`; the numbers are in [MISSIONS.md](MISSIONS.md#await-relief-task-force-bastion)):

- Renamed from "No Reply", the contract that now leads into it. The burst isn't the voice that
  contract chases; when its source is the tape, the line starts at that post.
- With R1's `book.bastion` (the Bastion commander's leader id + 1), that commander is the one,
  if he's Independent and still in the chair.
- The copy is a record of who sneaked it, by identity rather than by life. Copying needs a radio
  on you and nobody noticing you at the post.
- The people are standing stand-ins: the commanding officer's duty officer, the commander himself
  (his own body is held for the stage), a crew boss's radio man, the chair's clerk. Only the
  squad and the people you walk somewhere are AI. The "turned soldiers" are the stage's infected.
- Into the stove: the commander is replaced like any replacement, so his successor isn't always Loyal.
- The men were told: the two who walked off join the nearest safehouse instead of becoming
  Deserter wanderers.
- On the open channel: Bastion's channel isn't silenced. It's a headline and a rumour.
- Standing is split between the choice and the ending, within the engine's caps.
- While it runs, The Real Orders (the same lost orders, as a leader job) isn't offered, and No
  Reply picks another post.

### B2. The Census

✅ Built ([MISSIONS.md](MISSIONS.md#the-census-task-force-bastion)); the changes from this design
are listed under its endings.

The nightly call reads "civilians sheltered" to Command, and everything Bastion looks after is
signed for, people too. The commanding officer wants a real count: every safehouse, every name, for
ration cards, a curfew and a "contribution". The Haven won't give names. It remembers what Bastion's
lists were for on Day 2. A civilian who can walk into both is useful to everyone.

**Needs:** Known with Bastion; never after Await Relief ended Expended (`orders.stance` 4). 🔒 not Elder for stage 4's collection (as Requisition). ★ Police,
★ Radio Op.
**Starts:** the commanding officer's radio contract, while Bastion's need is Supplies or Security.
With `list.to_bastion` it starts at stage 3: the count is done.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Count a safehouse** | **Talk** to its council head (Known with the Haven): an honest count. **Sneak**: count from outside, unnoticed. ★ Police: the count is exact either way. With `door.bastion` (H2), that house counts itself | Quiet Count |
| 2 | **Ration cards** | Rations to two houses, on the books. **Skim** a crate: 15 Bastion Favor for you, 25% the quartermaster notices (+10 Bastion Infamy). **Give extra** from your own pack: Haven Fame | Hearts and Minds |
| 3 | **The curfew** | Patrol a Haven town at night. **Enforce it**: Bastion Fame, +3 Haven Infamy. **Patrol gently** (★ Police, or Known with the Haven): no Infamy, half the Fame. **Betray**: let a Red Hand toll in (Known with the Red Hand): a roadblock at the town for the night, Red Hand Fame | Curfew Patrol |
| 4 | **The contribution** | Collect 4 food from a counted house. **Bribe**: pay it yourself, 15 Bastion Favor. **Give it back** to another house. **Falsify the ledger** (Trusted, or ★ Police): it shows collected; 25% found out at stage 6 | Requisition |
| 5 | **Wait; the Red Hand hears** | One session. A crew hears the cards mean full stores and raids a counted house. **Fight**: defend it. **Side with the Red Hand**: hold its stores 90 s while the crew loads up. **Let it happen** | Trader Caravan's raid; Pharmacy Heist |
| 6 | **The nightly call** | Stand by the radio at the stronghold at 20:30 and give the count: **true**, **forged high** (more rations for the houses) or **forged low** (fewer collections). Forged: 25% found out. Command doesn't answer | Visit point |

| Ending | You get | The world |
|---|---|---|
| **Counted** | Bastion Fame +20, 30 Favor; Haven Infamy +5 | Counted houses get +2 food once from Bastion's next truck; Haven morale −5. Headline: *"Bastion read two hundred and eleven names to Command last night. No reply."* |
| **Forged** | Haven Fame +15, Bastion Fame +10. Found out: +15 Bastion Infamy instead of the Bastion Fame | High: two counted houses +2 food once. Low: Requisition never picks them this season |
| **Refused** | Haven Fame +15; Bastion Infamy +10 | Requisition picks those houses more often (×1.2); `census.refused` |
| **The Red Hand's count** | Red Hand Fame +15, 30 Red Hand Favor; Bastion Infamy +15, Haven Infamy +10 | The raided house −3 food; Red Hand strength +3 |

**Built, with these changes** (key `census`; the numbers are in [MISSIONS.md](MISSIONS.md#the-census-task-force-bastion)):

- Two houses carry the whole line: the one counted at stage 1 and the nearest other house (its
  ration cards put it on the books). The contribution and the raid come from those two.
- With `door.bastion` (H2), the house `door.house` names is the first house, and it counts itself.
- With `list.to_bastion` (H1) the line starts at the curfew: stages 1 and 2 are skipped.
- Counting from outside is 45 s at a watch point 70–110 m out, then writing it down. Without
  ★ Police it's a person or two out.
- The Red Hand toll holds the town's edge until dawn (a real crew only with the Red Hand group
  prefab), and the curfew stage ends then.
- The raid needs the Red Hand group prefab; without it the Red Hand hears, but nobody comes.
- "Refused" is reached by tearing up the count at the nightly call.
- Counted: the houses get their +2 food at once, not off a truck. The headline's number is the
  people in the two counted houses.
- A falsified ledger found out at the call: the house pays its 4 food after all, and whoever
  falsified it takes +10 Bastion Infamy if they're online.
- Requisition reads the line's flags: never a spared house (`census.spared`, and the chair's after
  B1's `orders.haven`), refused houses first and ×1.2 as often (`census.pressed`).

### B3. The Last Pilot

✅ Built ([MISSIONS.md](MISSIONS.md#the-last-pilot-task-force-bastion)); the changes from this
design are listed under its endings.

Of the helicopters Bastion came in with, only one still flies, and only one pilot is left who can
fly it. Everyone wants a flight: Halcyon for a sample run, the Haven for its wounded, the Red Hand
for the helicopter itself. On Day 3 he was wingman to the pilot who flew the wounded for the line
and was shot down short of it. Lately he talks about the line at the shift change, and asks
people what it looks like from the ground.

**Needs:** Known with Bastion; Trusted (or ★ Soldier) to talk him down. ★ Mechanic, ★ Radio Op,
★ Soldier. No storm for the flights.
**Starts:** the pilot, after any player finishes Last Helicopter.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Fuel** | The helicopter is dry. **Bring 2 jerrycans** to the LZ. **Sneak**: take fuel from a Red Hand depot (+8 Red Hand Infamy if seen). **Bribe**: buy it from a crew, 25 Red Hand Favor (the boss hears Bastion is buying) | Delivery; Arson's depot |
| 2 | **Whose flight** | Keep the LZ clear until it lifts off, and choose the load: **Halcyon's samples** (Halcyon Fame +6, research +1), **the Haven's wounded** (Haven Fame +6, +2 people at a safehouse) or **Bastion's ammunition** (Bastion Fame +6, strength +2) (`pilot.flew_for`) | Last Helicopter; choice |
| 3 | **{friend}'s wreck** | Find the Day 3 wreck near the line, at the edge of the warning zone (never in the kill zone), and bring him the black box. ★ Mechanic: twice as fast. ★ Radio Op: you can play it there, on a radio | Crash Site |
| 4 | **The pilot decides** | He wants to fly for the line. **Talk him down** (Trusted, or ★ Soldier; Known is enough if he's heard the black box: the line shooting his friend) (`pilot.stays`). **Report him** to the CO (`pilot.grounded`). **Help him**: fuel to a far LZ (`pilot.line`). **Side with the Red Hand** (Trusted with them): sell the plan, so a crew can take the helicopter on the ground (`pilot.redhand`) | Talk; delivery |
| 5 | **The last of it** | Stays: one more run, with the LZ kept clear. Grounded: walk him to the stronghold; he goes quietly. The line: hold the far LZ while he lifts off. The Red Hand: hold the LZ for the crew 90 s. They can't fly it, so they burn it | Last Helicopter; escort; hold |

| Ending | You get | The world |
|---|---|---|
| **He stays** | Bastion Fame +20, 30 Favor | Last Helicopter runs twice as often for 2 hours; Bastion morale +5 |
| **Grounded** | Bastion Fame +10 | No Last Helicopter for 2 real hours, until a new pilot (a named person) is found |
| **He flew for the line** | Nothing | No Last Helicopter for 2 real days, until Bastion's mechanics patch up a grounded airframe; Bastion morale −10; Bastion's Crash Site starts at his wreck at the edge of the warning zone (the line's truth). Headline: *"Bastion's last pilot flew for the line at dawn. The line shot him down short of it, like the one before."* |
| **Burnt for the Red Hand** | Red Hand Fame +15, 40 Red Hand Favor; Bastion Infamy +25 | No Last Helicopter for 2 real days; Bastion morale −10, Red Hand morale +5; a Bastion–Red Hand feud |

**Built, with these changes** (key `pilot`; the numbers are in [MISSIONS.md](MISSIONS.md#the-last-pilot-task-force-bastion)):

- It starts as soon as someone finishes Last Helicopter, if Bastion's
  questline slot is free (later otherwise). It needs the Perimeter (the wreck is at the line), and
  never starts if its pilot already died in Rescue the Pilot.
- There's no helicopter prop, as in Last Helicopter: the pad is by Bastion's stronghold, the pilot
  is a person and the fuel points are puzzle objects. The Red Hand's depot sits by its nearest
  seat (the fuel depot prefab is only its look).
- The siphoned fuel is carried back to the pad. ★ Mechanic: one jerrycan is enough.
- Nobody chose a load: ammunition, as ordered. Fuel bought from the Red Hand brings a crew to look
  at the flight (with the Red Hand group prefab).
- A ★ Radio Operator at the wreck plays the box when it comes out; then he hears it at the hand-over.
- "Help him" needs a far LZ on the map: past the warning zone, at least 1.2 km from the pad.
  Nobody deciding, twice: he's grounded anyway. Nobody holding the far LZ: he's grounded.
- Keeping watch for the Red Hand is a Red Hand contract. A dead crew can't burn anything.
- The groundings are world flags in real time (times **Wait scale**): no Last Helicopter or
  Rescue the Pilot until then, and the first Last Helicopter after says who flies
  it now. Last Helicopter also waits while the story has the helicopter (stages 1, 2 and 5), and
  Rescue the Pilot while the line runs. Losing Rescue the Pilot's pilot grounds it for 2 real hours.
- "Twice as often" isn't saved across a restart.

---

## The Red Hand

### R1. The Book

While the majors run, R1 is [The Book Has to Balance](#the-book-has-to-balance)'s acts 2–3, and its
endings are act 3's results.

Everything owed is in the book, and the book has to balance. Last week somebody tore a page out:
the one nobody is meant to read, with who *outside* the Red Hand owes it and for what. A Bastion
outpost commander for rifles that fell off the inventory, a Halcyon field lead for live ones, a
Haven council head for her quiet arrangement. Whoever holds that page can ruin someone in every
faction. The boss wants it back, and the man who tore it out put in the pit.

**Needs:** Known with the Red Hand; Trusted to return the page (Known with `orders.mocked`: the
boss owes you). ★ Ex-Con, ★ Police.
**Starts:** the boss's radio contract, from the second real day of the season. The page names
the faction's Rogue leaders where there are any, else Independent ones.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Who tore it** | Three crew members, one of them lying. ★ Police or Ex-Con: more to go on. Wrong: the wrong man goes in the pit and you're told so (+5 Red Hand Infamy, `book.wrong_man` *(you)*) | The Rat |
| 2 | **Find {thief_short}** | His trail through two towns to a barn. **Talk** him out (unarmed; ★ Ex-Con, he knows you're one of his). **Fight** (two friends with him). **Bribe**: 20 Red Hand Favor and he runs (`book.thief_ran`). He tells you why: his sister's house is on the page | Step chain; Struck Off |
| 3 | **Read the page** | Three names. For each: **collect** the debt (talk 60%; threaten, +3 Infamy with their faction; or pay it, 15 Red Hand Favor), **leave it**, or **betray: turn them in** to their own stronghold (at most one name: that leader is replaced, their faction's morale −3, Fame +10 with that faction and +15 Red Hand Infamy, because nobody rats). With `door.debt_yours` (H2), your name is on it too: a fourth debt, yours | Debt Collection |
| 4 | **The page** | **Return it** to the boss (`book.returned`). **Keep it** (`book.kept` *(you)*). **Burn it** (`book.burnt`). **Side with Bastion**: the whole page to the commanding officer (`book.bastion`) | Delivery |
| 5 | **The thief** | The boss wants him in the pit. **Let it happen**. **Fight**: take his place (4 min of waves). **Bribe**: pay his debt, 30 Red Hand Favor. **Get him out**: walk him to a safehouse (+10 Red Hand Infamy if seen) | Fight Pit; Desertion Route |

| Ending | You get | The world |
|---|---|---|
| **The book balances** | Red Hand Fame +20, 40 Favor; the crew gives you a name, used in headlines and by Red Hand leaders this life | Red Hand morale +5; every leader you collected from remembers you crossed them |
| **Held over them** | 20 Favor, once, from each faction whose leader you squeezed; when the boss hears (a session later), Red Hand Infamy +15 | The leader you squeezed hardest swings toward Rogue |
| **Bastion's file** | Bastion Fame +20; Red Hand Infamy +25 | The Bastion commander on the page is pulled into line (loyalty +25); Red Hand strength −3; Bastion keeps Red Hand prisoners longer (Bust Out a little more often, ×1.2) |
| **Burnt** | Haven Fame +5 if the thief lived; Red Hand Infamy +25 (a debt is a debt) | The debts die with the page; Red Hand morale −5; the three leaders remember you saved them |

**Built** ✅ (stages, numbers and simplifications: [MISSIONS.md](MISSIONS.md#the-book-the-red-hand)).
Changes from the design: the thief goes in the boss's own pit; turning a name in means carrying its
line to that faction's stronghold; a name whose leader leaves the chair drops off the page (the debt
was theirs); the fourth line from `door.debt_yours` is text only (the flag doesn't say whose debt it
is), and `orders.mocked` lets anyone Known give the page back; the crew name is in the ending's
headline and rumour only.

### R2. Cargo

Some trucks carry people, not tins, and where to, nobody says. The driver of one of them wants
out. He has carried strays the Haven turned out, debtors from the book, a deserter or two. Most
went to the pit, or to the line, to learn the drones. Lately one crew boss sends a load somewhere
else: a farm past the last village, where a woman pays in jars. The driver has stopped eating meat.

**Needs:** Known with the Red Hand; or Known with the Haven, for the Haven's version (a Haven
wanderer saw the van). ★ Ex-Con, ★ Mechanic, ★ Police.
**Starts:** the driver (a Red Hand named person) at an outpost, at dusk. Without the Long Table
this season, the van goes to the pit, and stage 3 ends there.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Meet the driver** | At a crossroads at dusk. **Talk** (★ Ex-Con: he trusts you sooner). **Threaten** him: he talks, then tells his crew (stage 2's camp is on guard). He shows you the van, padlocked | Talk |
| 2 | **The load** | Three people in the back of the van at a crew camp. **Let them out**: cut the lock (hold **Cut the padlock** at the van's door, 12 s) and walk them to a safehouse (+3 people; +10 Red Hand Infamy, `cargo.freed`). **Drive it** to where it's going (`cargo.delivered` *(you)*). **Side with Bastion**: drive it to a Bastion seat: three "civilians sheltered", the driver arrested (`cargo.bastion`). ★ Mechanic: the van starts first time | Bust Out; vehicle zone |
| 3 | **Who buys** | **Sneak**: watch the drop at night from 150 m, 2 min unseen: the host comes out with jars. **Talk** to the fence (Trusted): he says "pork" and laughs. **Bribe** the driver, 10 Red Hand Favor: he names the crew boss. Sets `cargo.buyer_known`; C1's proof stage counts as done | Visit point; talk |
| 4 | **Tell someone** | **The boss** (Trusted with the Red Hand): the code has no rule about this, and the boss says it doesn't need one (`cargo.purge`). **Keep quiet** for a cut (`cargo.cut` *(you)*). **Side with Bastion**: the crew's camp to Bastion, and the farm's grid with it (`cargo.bastion_raid`). **Side with the Haven**: the farm's grid to the chair (C1 starts at stage 4) | Talk; Sell Out a Rival Crew |
| 5 | **The driver** | **Get him out**: walk him to a safehouse; he joins it. **Leave him driving**. Purged: the boss wants the driver in the pit too: take his place or let it be. Bastion's raid: assault the crew's camp | Desertion Route; Fight Pit; Raid the Red Hand |

| Ending | You get | The world |
|---|---|---|
| **Purged** | Red Hand Fame +20, 30 Favor | The crew boss is replaced; the Long Table loses its buyer and takes people half as often (`table.supplier_cut`); Red Hand morale −3 |
| **The cut** | 20 Red Hand Favor a session for three sessions; if the Long Table is exposed while you're on it, Haven Infamy +20 | The vans keep running; one more *"never arrived"* in the news |
| **Bastion's raid** | Bastion Fame +20; Red Hand Infamy +25 | The crew's outpost goes to Bastion if Bastion is strong enough by the usual margin, else the Red Hand drops it; Bastion knows the farm (`table.bastion_knows`) |
| **Freed and told** | Haven Fame +15; Red Hand Infamy +10 | The safehouse +3 people, and the driver; C1 starts with the Haven |

**Built** ✅ ([MISSIONS.md](MISSIONS.md#cargo-the-red-hand-or-the-haven)). It's the long version of
the Haven's contract Tied Down ([MISSIONS.md](MISSIONS.md#the-red-hands-hard-edges)) and shares its
van and padlock: the Haven's version starts only once Tied Down has reached its shed this season
(`cargo.truck_seen`), and Tied Down isn't offered while Cargo runs. Changes from the design: the van
is the Red Hand supply truck, so No Vehicles takes Cargo out; the farm is wherever the Long Table
says it is (`table.town`), and without it the loads go to the crew boss's own pit (stage 3 ends
there: no proof for C1); stage 3 is skipped when you drove the van to the drop yourself; Bastion's
raid is the crew at its camp, with no Bastion soldiers on the ground, and a Bastion too weak to take
the outpost costs the Red Hand strength instead; a session's cut is paid only if its taker is on;
"Freed and told" is the Haven's ending whether or not the load was freed.

### R3. The Well Story

Nobody in the Red Hand says what happens to a place that doesn't pay. They tell the story about
the well, and let you finish it yourself. A crew boss with a village that has stopped paying
wants the story told, and if that doesn't work, he wants it to stop being a story. The boss says
the story works best untold: you don't kill the cow you milk. Both of them think you're on their
side.

**Needs:** Known with the Red Hand; Trusted when the boss sends for you. 🔒 not Elder (as
Protection Racket). ★ Ex-Con, ★ Lab Tech, ★ Nurse.
**Starts:** a crew boss at a Red Hand outpost (Ruthless or Greedy ones more often). The village
is the nearest Haven safehouse town, or a quiet village under 30 infection. With `door.shut`
(H2) or a house counted by Bastion (B2), it's that house: it has someone else now.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Collect** | The keeper won't pay | Protection Racket |
| 2 | **Why {village} stopped** | **Talk** to the council head (Known with the Haven): they're short, or they pay Bastion now. **Bribe**: pay their dues yourself, 20 Red Hand Favor. The crew boss doesn't care who paid: the story ends at "The story worked" | Talk |
| 3 | **Tell the story** | At the well, at dusk, to whoever's there. **Talk** (★ Ex-Con: they believe you): 60% they pay (`well.paid`). **Warn them instead**: they put two people on the well (`well.warned`, Haven Fame +5) | Talk at a point |
| 4 | **The crew boss's order** | If they didn't pay: put something in the well. **Do it**, at night, unseen (`well.done` *(you)*). **Fake it**: something dead in the bucket; ★ Lab Tech or Nurse knows what's harmless (`well.faked`). **Side with Halcyon**: get the water tested and the well sealed (`well.tested`). **Fight**: the crew comes to do it themselves; hold the well | Visit point; The Poisoned Well; Trader Caravan's raid |
| 5 | **The boss hears** | A session later the boss sends for you. **Own it**. **Blame the crew boss**: he goes in the pit. **Say nothing** | Wait; talk |

| Ending | You get | The world |
|---|---|---|
| **The story worked** | Red Hand Fame +20, 30 Favor; the boss remembers you helped | The village −2 food once; nobody hurt; the well becomes a rumour |
| **The well was done** | Red Hand Fame +5; the boss is angry: Red Hand Infamy +10; if anyone saw, Haven Infamy +30 | Town infection +15 (Fever Town may start there); the safehouse −3 people; the crew boss is replaced for a bad deal. Headline: *"Nobody in [the village] is drinking from the well."* |
| **Faked** | Red Hand Fame +10; Haven Fame +8 if you told the council after; 25% the crew boss finds out: Red Hand Infamy +10 | The village −1 food once; the story spreads anyway |
| **Stood up** | Haven Fame +15; Halcyon Fame +8 if tested; Red Hand Infamy +10 | Infection −10 there if tested; the crew boss's loyalty swings toward Rogue; `well.stood` |

**Built** ✅ ([MISSIONS.md](MISSIONS.md#the-well-story-the-red-hand)). Changes from the design: H2's
house is the one `door.house` names (set with `door.shut` or `door.bastion`), else B2's
`census.counted`; a warned well can't be
done unseen; if nobody stands in the way, the crew does it themselves; telling the council after a
fake is talking to their council head during the session's wait; paying the dues yourself isn't
refunded; the poisoned well doesn't start Fever Town.

---

## Halcyon

### HC1. The Three Weeks

While the majors run, HC1 is [Proof](#proof)'s act II and starts only from it.

The first case was three weeks before the gates closed, and the Institute was told. Someone
wrote "notify the Institute" in the margin and underlined it twice. A young field lead has found
the clinic's copy and wants to know what the lab did with those three weeks. The research
director wants the lab to stay the one way out, which means nobody asks. The only person who
knows is the driver who came back alone from the site on Day 3, and he won't say what he saw.

This line is about the past. It never makes the outside act: the Institute's last call on Day 2
is still the last.

**Built** ✅: the stages as they are in code are in [MISSIONS.md](MISSIONS.md#the-three-weeks-halcyon).
It replaces the old Three Weeks contract, whose remote site and keypad store are stage 2. Changes
from the design: the driver waits at the edge of the town nearest the site, and a Radio Operator
can also talk him round (they heard his call on Day 3). The notebook is carried as data; stage 4's
people are stand-ins at each seat, and reaching the director needs Trusted, or Known as a Lab
Tech or Doctor. Stage 5 is a wait that also waits out a final stand, then the raid or the walk-out as
missions.
The personal rewards ("Fame +15 with whoever you gave it to") are paid at the choice, within the
engine's caps. **The packs** ✅: the site stages (2 and 3) hold the pack whose ground takes in the
site (its claim, so it doesn't move or split while they run), and its groups keep 150 m from the store
(their data spots are pushed out, so bodies appear outside). It opens on `weeks.records`, which Patient Zero and Patient Zero's
Ledger set when they're solved.

**Needs:** Known with Halcyon. ★ Lab Tech, ★ Doctor, ★ Radio Op.
**Starts:** an Independent or Rogue field lead at the field station, after any player has
finished Patient Zero or Patient Zero's Ledger.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **The driver** | A Scavenger wanderer who won't talk to anyone in a lab coat. **Talk** (Known with the Haven, and not Trusted with Halcyon). **Share food** twice. **Threaten** him (+3 Haven Infamy). He names the site, and says *"she stayed because she wanted to."* | Wanderer talk |
| 2 | **The site** | The closed site on the highest ground away from towns. A keypad; the code is on three bodies from the team that went up with her. ★ Lab Tech: the first digit. Three wrong: locked 2 min, loud | Locked Bunker |
| 3 | **The team lead** | Her notebook is inside, and so is she: she turned a long time ago. **Mercy**: end it, take the notebook. **Live capture**: bring her to the lab (`weeks.captured`, research +2). **Sneak**: take the notebook and leave her | Live Capture |
| 4 | **Who reads it** | The notebook doesn't say what the site was for. It says the Institute was told on the first day of the three weeks, and wrote back one word: *"Continue."* **To the director** (Trusted): it goes in the record; the pages about the director go in the stove (`weeks.director`). **To the field lead**: she wants it out (`weeks.public`). **Side with Bastion**: the commanding officer was right about Halcyon (`weeks.bastion`). **Side with the Haven**: the chair reads it at supper (`weeks.public`). **Side with the Red Hand**: 40 Red Hand Favor, and they read it on the open channel (`weeks.public`) | Delivery |
| 5 | **The lab answers** | One session. Public: the Red Hand thinks the lab has more to hide and raids it (hold the lab). In the record: the field lead walks out; walk her to a safehouse, or bring her back to the lab. Bastion: the CO's demand goes to the director (HC3 opens with it) | Wait; Lab Defence; escort |

| Ending | You get | The world |
|---|---|---|
| **In the record** | Halcyon Fame +20, 30 Favor; research +1 | Halcyon morale +3; the field lead is replaced (she leaves: a life event) |
| **Out on the open channel** | Fame +15 with whoever you gave it to; Halcyon Infamy +10 | Halcyon morale −10; *"Halcyon made this"* is a common rumour for the season; H3's lab road gets harder |
| **Bastion's file** | Bastion Fame +20; Halcyon Infamy +15 | Bastion strength +3; HC3 starts with Bastion's terms already on the table |
| *(with any ending)* **She's at the lab** | Research +2 at stage 3 | The director speaks for two minutes on the open channel, about her |

### HC2. The Other Two

Eight in ten walk out of the trial dose, and the lab writes down what happens to the other two.
A Haven woman's brother volunteered and didn't come back, and the lab won't tell her more than a
number. The answer is at the field station. The other two leave with half the time they came in
with, and some of them are kept, in a back room, so that someone with a clipboard can be there at
the moment of turning.

**Needs:** Known with the Haven or with Halcyon. ★ Doctor, ★ Nurse, ★ Lab Tech.
**Starts:** the sister (a Haven named person) at a safehouse. With `walk.knows_other_two` (H3),
at stage 2.

**Built** ✅: in [MISSIONS.md](MISSIONS.md#the-other-two-halcyon). It replaces The Author List
contract: stage 5's names come from its deduction (the trial list by the door and the man who
walked them up; ★ Elder reads the council's pencil). Changes from the design: the stages are the
sister's (Haven) contracts, offered to anyone Known with either; her house is picked within
3.5 km of the field station when it can be, and the brother has long enough to be walked home
rather than a few minutes. Stage 2's courier is an escort. Sold ends the line at stage 4; the
names to the chair is Brought home; leaving the numbers, or nobody naming them, is The work goes
on. Sold's Haven Infamy is the engine's 15. It doesn't start after the cure. It reads H3's flags
as `walk.knows_other_two` and `walk.lab_road`.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **{brother}'s number** | **Talk** at the lab (Known with Halcyon): a number and a date. **Sneak**: the volunteers' ledger on the night shift (a heat limit; seen: +8 Halcyon Infamy). **Bribe** a courier: 20 Halcyon Favor. ★ Lab Tech reads the ledger's codes | Letters' quiet way; talk |
| 2 | **The back room** | **Get in as a courier**: walk a cold case there with Halcyon's courier. **Talk**: the field lead's leave (Trusted with Halcyon). **Sneak**: at night, unseen. Inside: three people in the Final stage, and a chair with a clipboard by each | Escort; sneak |
| 3 | **{brother} in the back room** | He has a few minutes left. **Sit with him** (`brother.sat`). **Take him to his sister** before he turns (`brother.home`). **Take the sample** at the moment of turning (research +1, `brother.sample` *(you)*). **End it** | Patients Not Samples; escort |
| 4 | **The field lead** | **Talk** (Trusted with Halcyon, or ★ Doctor): she closes the room (`ward.closed`). **Report her** to the director: she's replaced and the room moves to the lab (`ward.lab`). **Keep quiet**: 25 Halcyon Favor (`ward.kept`). **Side with the Red Hand** (Trusted with them): one of the other two goes to a crew as a "live one", for 40 Red Hand Favor (`ward.sold` *(you)*) | Talk |
| 5 | **Names** | The paper lists the volunteers by number. **Bring the names** to the director (Trusted, or Known with the sister beside you after `brother.home`). **Side with the Haven**: the chair reads them at supper instead. **Leave the numbers** | Delivery; talk |

| Ending | You get | The world |
|---|---|---|
| **Named** | Haven Fame +15, Halcyon Fame +15 | A Haven–Halcyon truce if the rules allow; Haven morale +5. Headline: *"The lab put fourteen names on its paper last night. The Haven read them at supper."* |
| **Brought home** | Haven Fame +20 | Halcyon morale −3; the room closes or stays, by stage 4 |
| **The work goes on** | Halcyon Fame +15, 25 Favor; research +2 | `other_two.numbers` (H3's headline uses numbers); Haven morale −3 |
| **Sold** | Red Hand Fame +15, 40 Red Hand Favor; 35% found out: Haven Infamy +25 | A headline about something new in the Red Hand's pit |

### HC3. Authors

While the majors run, HC3 is [Proof](#proof)'s act III and the stand, and starts only from it; it
publishes `authors.terms`.

At half research, the cure stops being a hope and becomes something a person can own. The
research director would trade what the lab knows for Bastion's guns at the final stand. A field
lead would rather give the method to every Haven clinic for nothing. The Red Hand would buy the
batch outright. Whose names go on the paper decides who's remembered for the cure. Who controls the
cure is the finale's ([FINALE.md](FINALE.md)); this line is about who is standing at the lab when it's
made.

**Needs:** Trusted with Halcyon; cure 50% or more; the final stand not yet won. ★ Lab Tech,
★ Doctor, ★ Soldier.
**Starts:** the research director's radio contract when research reaches 50%. With
`weeks.bastion` (HC1), stage 1 is done: Bastion already has terms.

**Built** ✅: in [MISSIONS.md](MISSIONS.md#authors-halcyon). It adds no research and never changes
the stand; the endings are paid after it, so after the winner is chosen when it's won. Changes
from the design: it starts only below 90% research (later it would only fizzle), and every stage
waits out a stand and never runs after the cure. Stage 3 is a wait, then a mission at Bastion's
adjutant; stage 5 waits for 75% research; stage 6 waits for the stand, then lasts as long as it.
The Bastion checkpoint on the lab's road is a Bastion group there during the stand. While the cure
finale runs, stage 6 places no groups and pays nothing: its Bastion terms go to the finale's groups
inside the lab's cap ([The stand's inputs](#the-stands-inputs)), never under Expended, and the
pilot's extra Halcyon guard isn't sent (Halcyon never takes sides at the stand). With the boss
paid, Take the Lab is offered to anyone Known with the Red Hand, not only Champions; while the cure
finale runs, the controller offers it to the Red Hand route instead ([FINALE.md](FINALE.md#take-the-lab-and-the-coat-run)). The endings
are taken in the order the Red Hand's batch, Bastion's guard, open doors, the lab alone. The money
is the engine's 40 Favor, and the endings' Fame and Infamy are within its caps. It reads B3's flag
as `pilot.flew_for`, the faction the pilot flew for (Halcyon: the samples).

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **The director's letter** | Carry her terms to the commanding officer. **Read it first** (`authors.read` *(you)*): the method for Bastion alone, in return for guards. **Sneak** a copy to the field lead | Delivery |
| 2 | **The field lead's plan** | She wants the method at a Haven clinic first. **Carry it**, cold, before it's missed (`authors.haven_copy`). **Report her** to the director (`authors.reported`). **Side with the Red Hand**: sell it (Sell the Formula) | Cold Chain; Sell the Formula |
| 3 | **The CO's answer** | One session. The CO wants a locked crate, a key and a guard on the clean room. **Agree** for the director. **Bargain** (Trusted with both): two guards at the gate, no key. **Refuse** for the lab (+5 Bastion Infamy) | Wait; talk |
| 4 | **The Red Hand's offer** | The boss offers to buy the batch, and to leave the lab alone during the stand. **Take it to the director**: she refuses. **Betray**: take the money (60 Red Hand Favor); Take the Lab is offered during the stand. **Side with Bastion**: tell the CO, and a Bastion checkpoint goes up on the lab's road | Talk |
| 5 | **Ready the lab** | One of Power the Lab, Burn the Bodies at the lab or Lab Defence, picked by the director from your flags | Power the Lab; Burn the Bodies; Lab Defence |
| 6 | **The final stand** | The stand itself. With `pilot.flew_for` samples (B3), Halcyon's people remember who flew for them, and one Halcyon guard more stands at the gate | Final stand |

| Ending | You get | The world |
|---|---|---|
| **Under Bastion's guard** | Bastion Fame +15, Halcyon Fame +10 | One Bastion guard group at the lab gate through the final stand (it fights the waves; the stand's rules don't change); Bastion strength +3; Haven morale −3 |
| **Open doors** | Halcyon Fame +15, Haven Fame +10 | Infection −3 in every safehouse town (as Leak the Data); the field lead's name goes on the paper first |
| **The Red Hand's batch** | Red Hand Fame +25, 60 Red Hand Favor; Halcyon Infamy +30 | Take the Lab runs during the stand, with its usual result; Halcyon morale −10 |
| **The lab alone** | Halcyon Fame +20 | Nothing more: the lab holds or doesn't, on its own |

---

## The Long Table

Who they are is in [LORE.md](LORE.md#the-long-table); how they work and how players deal with them
is in [DESIGN.md](DESIGN.md#the-long-table) and [REPUTATION.md](REPUTATION.md#the-long-table).
Both lines exist only in a season that has a Long Table, and never run together (one slot).
Both are built; their stages, numbers and what each approach does are in
[MISSIONS.md](MISSIONS.md#questlines).

### C1. The House That Went Quiet

Past the last village there's a farmhouse with smoke in the chimney, in a valley where nothing
else burns. Wanderers who take the road past it don't always arrive. A Haven courier's satchel
turned up at a Red Hand fence. The man you meet on that road is the friendliest person in the
Zone. Nobody gives this job. Somebody just has to notice.

**Needs:** Any. ★ Hunter, ★ Police, ★ Nurse, ★ Elder.
**Starts:** when the house's suspicion reaches 30 and a player hears a *"never arrived"*
rumour; or from H1 (at stage 2), R2 (at stage 4) or C2 (at stage 4).

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **Never arrived** | Follow a missing wanderer's last road: a boot on the verge, a satchel at a fence, the gate of the farm. ★ Hunter or Police: grid references | Trail of Clues |
| 2 | **Guest** | The host meets you on the road. **Talk: accept** and eat: in daylight, in company or bitten, it's only a meal (`table.guest` *(you)*). **Sneak: decline** and watch from outside, unseen. **Declare your bite** if you have one: they bless you and send you on | Talk; visit points |
| 3 | **Proof** | Three of: the guestbook and its small crosses, the boots by the door, a dead courier's letters in a drawer, the cellar (locked: the mother's key, or a crowbar, which is loud). **Sneak** in during supper (18:30). **Talk** to the boy, away from the house, unarmed (★ Nurse or Elder): he says one thing. **Fight** your way in (before the cellar is open, shooting them costs Haven Infamy, [REPUTATION.md](REPUTATION.md#the-long-table)). With `cargo.buyer_known` (R2) or `place.undercover` (C2): done | Door locks; clues |
| 4 | **Who to tell** | **The Haven**: horror and shame; the chair wants them brought home and walked out (`table.haven`). **Bastion**: containment (`table.bastion`). **Halcyon**: the director wants one of them alive, and the mother's kitchen notebook: why they never eat the bitten (`table.halcyon`). **The Red Hand**: the boss pays the house to go away (`table.redhand`). **Nobody**: deal with it yourself | Talk; delivery |
| 5 | **The end of the house** | *Haven:* walk the family to the edge of town at dusk, unarmed. They go quietly, except the quiet son. *Bastion:* the guns, after five minutes' warning; get the boy out first (talk him down, nothing within 25 m), or don't. *Halcyon:* walk the mother to the lab as a "volunteer". *Red Hand:* hold the farm 90 s while a crew moves them on. *Yourself:* **fight** (three or four of them), or **talk** the last ones out once the mother is gone | Walk Him Out; Bastion Shelling; escort; hold |

| Ending | You get | The world |
|---|---|---|
| **Walked out** | Haven Fame +20, 30 Favor | Haven morale −8, then +5 when it's done; the boy joins a safehouse (+1) or becomes the Kid wanderer. Headline: *"Every Haven house lit a candle last night. Nobody said who for."* |
| **Burnt** | Bastion Fame +20, 30 Favor | The farm's town is Burned (town memory); Bastion morale +3. Headline: *"Bastion reports an infected cluster cleared past the last village."* If the boy wasn't got out, nobody reads his name |
| **Studied** | Halcyon Fame +20; research +2 | The mother is at the lab; the rest scatter. The host becomes a Scavenger wanderer who never asks anyone to dinner again |
| **Paid to go** | Red Hand Fame +15, 40 Red Hand Favor | The house moves 2–5 km and is quiet for a real day, then takes people half as often; C1 can start again, once |
**By your own hand** is no ending of its own: the house is gone for the season, the boy lives if you
got him out, and the proof, taken to a faction after, pays half that faction's Fame.

If nobody ends it and it's never found out, the house goes on until the season ends, and next
season there may be a long table somewhere else. Found out and not ended within a real day,
Bastion burns it ([DESIGN.md](DESIGN.md#the-long-table)).

**Built** ✅ (`Missions/Questlines/LongTable/RZ_Quest_HouseWentQuiet.c`). Changes from the design:
- A first stage routes it: `list.found_table` to stage 2; `cargo.buyer_known`, `place.undercover`
  or `place.proof`, or the house already found out, to stage 4.
- *Guest:* accepting walks you up to the farm, and the meal is at the table. Declining points you
  to a rise behind the farm: a minute there unseen is the watch; seen, they're wary at stage 3.
- *Proof:* the boy's one thing is also where the key hangs (without it, the cellar needs a
  crowbar). Outside supper, any of them within 20 m sees you (30 m once they're wary). Caught, they
  set on you (a fight) or, without the fight prefab, shut the house for the day. A fight that
  finishes the house ends the line by your own hand.
- *Who to tell* is any leader of that faction, talked to by someone who took part (by anyone, when
  the proof came from another line); Halcyon needs the lab, Bastion its stronghold. The house is
  found out then, and the teller gets the faction's Fame then (+10, the Red Hand +4).
- *The end of the house* the factions' way is that faction's contract, offered in person, so an ending's
  Fame is the telling's plus the contract's (10 Fame and 30 Favor; Halcyon 10 and 20; the Red Hand
  11 and 40). The Haven's walk is between 16:00 and 21:30, and the family waits for you at the edge
  of town rather than walking with you; the quiet son is talked round (persuade) or let go.
  Halcyon's volunteer is the mother, or one of them alive without her. By your own hand: fight them
  (the fight prefab), or tell the mother you know (persuade with the Haven, difficulty 2; ★ Elder
  or Nurse) and she walks out, then tell the rest to go; a failed word and they set on you. "Taken
  to a faction after" is a last wait of two hours of play or 12 real hours.
- Its endings also publish what happened (`table.walked_out` and the rest, in [Crossings](#crossings)).

### C2. A Place at the Table

The host doesn't only invite people to dinner. To someone who has eaten with him twice and come
back, he makes a small offer. The dead don't need what they have. Tell the family where the dead
are, before anyone else finds them, and the family will feed you and tell you what the roads are
doing. It starts with the dead. Their story started that way too.

This is the darkest line in the mod. Most players take it to get proof for C1. A player is never
the one taken, and nothing here is described: it's all said sideways ([LORE.md](LORE.md#the-long-table)).

**Needs:** ate with the Long Table twice this life; not Trusted with the Haven (the host doesn't
ask the Haven's friends). ★ Police: anything you learn counts as proof.
**Starts:** the host, on your third visit.

| # | Stage | Approaches | Uses |
|---|---|---|---|
| 1 | **The dead** | Stand where a wanderer or a truck crew died to the infected in the last hour, then tell the host. A bowl and true road news (where the next truck is going) (`place.dead` *(you)*) | Visit point; talk |
| 2 | **The dying** | A bitten wanderer *"who won't make it."* **Tell him where** (the wanderer never arrives, `place.dying` *(you)*). **Lie**: send him the wrong way, and he's cooler with you. **Save the wanderer**: give them your antidote | Talk; wanderer |
| 3 | **A stray** | He wants a living one: an exile the Red Hand keeps track of. **Betray: bring him** (escort to the farm, `place.stray` *(you)*). **Pretend**: walk him to a safehouse or a Bastion seat instead (`place.undercover`). **Refuse**: you're not a guest any more, and the guest rule holds for you in daylight too | Escort |
| 4 | **Grace** | The mother asks you to sit at the long table and say a name. **Sit and say it** (`place.family` *(you)*). **Get up and go**, with the proof (C1, stage 4). **Fight**, at the table (C1, stage 5, by your own hand) | Talk |

| Ending | You get | The world |
|---|---|---|
| **Family** | A bowl when you visit (one food item a real day) and true road news; no Fame anywhere | The guest rule never applies to you. When the house is exposed while you're family, the Haven and Bastion hold it against you ([REPUTATION.md](REPUTATION.md#the-long-table)), and your name is in the headline |
| **Undercover** | C1's proof; +5 Fame with whoever you take it to, on top of C1's | The exile lives, with the Haven or Bastion |
| **Walked away** | Nothing | The host stops meeting you on the road |

**Built** ✅ (`Missions/Questlines/LongTable/RZ_Quest_PlaceAtTable.c`). Changes from the design:
- It's personal: only the character the offer was made to does its stages, and only in that life.
  The host makes the offer on the third meal of a life.
- Stages 2 to 4 wait 20 minutes of play or 6 real hours first; Grace is in the evening (17:00 to
  22:00).
- *The dead:* a wanderer or a truck crew who died to the infected in the last hour, within 4 km;
  if there isn't one, a traveller in a ditch, laid out by the stage, with two infected about.
  Telling the host also feeds the house a little (fewer taken).
- *The dying* is a wanderer from the roster, held by the stage (a stranger if nobody is free). Sent
  the wrong way, he's the usual bitten wanderer; your antidote saves him (+8 Haven Fame).
- *A stray:* refusing is said to the host at the farm gate. A safehouse takes the exile in (+1
  person); Bastion logs him as sheltered.
- Walking away from the host at stage 1, and Grace's "get up and go", end it Walked away; Grace's,
  or a ★ Police guest's walk, carries the proof (`place.proof`): C1 starts at stage 4.
- The fight at the table finishes the house there: the line ends By your own hand (C1's outcome,
  reached here).

---

## Crossings

The flags other lines read. They're the season flags of the world facts
([DESIGN.md](DESIGN.md#world-facts)), so contracts, arcs and voice can read them too. They use
the short names below (named for what they're about, not for the questline's key), and these
names are reserved for them. A flag only its own questline reads is `<key>.<flag>`. Rows marked "(facts)" are facts an ending publishes for ordinary contracts,
events and rumours. A flag is 1 when set, unless its row gives other values.

| Flag | Set in | Read in | What it does there |
|---|---|---|---|
| `list.to_bastion` | H1 | B2 | B2 starts at stage 3: the count is done |
| `list.to_redhand` | H1 | R2 | The driver's strays came off the List; exiles are sold more often |
| `list.found_table` | H1 | C1 | The Haven found the farm lived in: C1 starts at stage 2 |
| `door.debt_yours` | H2 | R1 | Your name is on the page |
| `door.shut`, `door.bastion` | H2 | R3, B2 | That house is the one that won't pay; it's on Bastion's census |
| `door.house` | H2 | R3, B2 | Which house that is: its safehouse index + 1 (set with `door.shut` / `door.bastion`) |
| `door.exile_sold` | H2 | R2, the Long Table | The exile was walked to the crew: he is one of the people who never arrived |
| `walk.knows_other_two` | H3 | HC2 | HC2 starts at stage 2 |
| `walk.lab_road` | H3 | HC2 | The back room holds Haven people: stage 1's talk needs only Known with the Haven |
| `walk.cut_dose` | H3 | R1 | The Haven owes the Red Hand for the cut dose |
| `list.contract`, `walk.contract` | The contracts The List and Walk Him Out, when done | H1, H3 | They can start (`list.contract` holds the safehouse, + 1) |
| `list.read` | H1 | the Long Table | The List was read at every house: suspicion +30 |
| `list.buried`, `list.sold_known`, `list.husband_home`, `list.head_replaced` | H1 | (none yet) | What happened, for contracts, events and rumours: the List was put back in the drawer; who sold it is known; the husband came home; a council head stepped down |
| `door.paid`, `door.knows`, `door.tins`, `door.exile_home`, `door.exile_warned`, `door.head_replaced` | H2 | (none yet) | The envelope was carried, or opened; the house pays in tins; the exile came home, or took to the roads; the house voted its council head out |
| `walk.lab_saved`, `walk.walks_stay`, `walk.truce` | H3 | (none yet) | A bitten man walked out of the lab; the walks stay; the lab road began with a truce |
| `other_two.numbers` | HC2 | H3 | The lab road's headline gives numbers, not names |
| `other_two.numbers` | HC2 | H3 | The lab road's headline gives numbers, not names |
| `orders.mocked` | B1 | R1 | The boss owes you: returning the page needs only Known |
| `orders.haven` | B1 | B2; Requisition | The chair's house is never asked for a contribution, and Requisition never comes to it |
| `orders.stove`, `orders.read` | B1 | (facts) | Where the Day 3 page went: into the stove, or read to a post's men |
| `orders.tape`, `orders.tape_gen` | No Reply | B1 | The post (leader id, generation) whose tape leads into B1 there |
| `census.counted` | B2 | R3 | The first house Bastion counted, as its safehouse index + 1 |
| `census.refused` | B2 | H1 | The chair trusts you: the last stage's talk needs only Known |
| `census.spared`, `census.pressed` | B2 | Requisition | Safehouses (one bit per safehouse index) Requisition never visits, or visits first and ×1.2 as often |
| `census.forged` (1 high, 2 low), `census.found_out`, `census.raided` (house + 1) | B2 | (facts) | How the count went to Command, and which house the Red Hand raided |
| `pilot.flew_for` (the faction the flight was for: Halcyon's samples, the Haven's wounded, Bastion's ammunition) | B3 | HC3 | Halcyon: one more Halcyon guard at the final stand |
| `pilot.lz_kept` | Last Helicopter | B3 | Someone kept an LZ: B3 can start |
| `pilot.down_until` (Unix time), `pilot.down_reason` (1 new pilot, 2 airframe), `pilot.new_pilot`, `pilot.back_told` | B3; Rescue the Pilot | Last Helicopter, Rescue the Pilot | The helicopter is grounded or lost until then; the first run after says why (`new_pilot` seeds the new pilot's name, `back_told` marks it said) |
| `pilot.stays`, `pilot.grounded`, `pilot.line`, `pilot.redhand`, `pilot.heli_lost`, `pilot.dead` | B3; Rescue the Pilot (`dead`) | B3 (`dead`); (facts) | How the last pilot's story ended; the helicopter lost; its pilot dead (B3 never starts) |
| `majors.pages_open` | the majors (the book read whole, Sent off loud, All Stations, the List read, the whole of the first page) | every major's "read it" check; Crash Site's stove (paid 80%) | One step easier, once per major |
| `book.sendoff`, `book.rival` | The Book Has to Balance, act 1 | settling day | How many crew bosses came and stayed (0–3); the rival's seat id + 1 (0: the boss's second) |
| `book.settled` | settling day | the muster | 1 laughed off, 2 purged, 3 pulled into line, 4 walked to the Haven, 5 Bastion raided the camp (the Expended betray needs Trusted), 6 stood with |
| `book.last`, `book.stand`, `book.debts_dead`, `book.first_page`, `book.coda` | the last page, the stand, the last line | the last line; the epilogue | The last page's answer (1–6); how the Red Hand turned up at the stand (1–4); every debt died with the book; the boss keeps the first page; the coda ran |
| `orders.reckoning`, `orders.count`, `orders.padded` | Await Relief, acts III–IV | the muster, the last call | B1's ending + 1; the true count; posts still padded |
| `orders.stance` | the muster | the Haven's sittings (no cards after 4), the reading (no countersignature under 4), the last page (no bill under 1 or 3), the voice bank, the last call, Crash Site's pay (75% under 4) | 1 the call stands, 2 all stations, 3 Bastion, Command, 4 expended |
| `orders.coda`, `orders.last_line` | the last roll | Proof's paper | The last call and roll ran |
| `every_name.family`, `every_name.week`, `every_name.ending`, `every_name.held`, `every_name.far_fed`, `every_name.crossed_read`, `every_name.coda` | Every Name | its own acts; Bastion's last call (after the Gate) | The family (1 all in, 2 two to Bastion, 3 two turned away); the hungry week's answer; the ending (0 Every Name … 4 Paid at the Door); A Week On held; the far house fed; the crossed names read at the Gate; the Gate ran |
| `haven.rule`, `haven.counted`, `haven.paying` | the sittings | the muster ("Answer it yourself" at Known under 3), the voice bank | 1 the pot, 2 workers first, 3 Bastion's cards, 4 the arrangement |
| `list.truth` | Every Name, act III | the second sitting | 1 the farm and whose hand, 2 only "a farm out that way" |
| `proof.day_one`, `proof.copy`, `proof.weeks`, `proof.pages`, `proof.first_page`, `proof.haven_gate`, `proof.coda` | Proof | the muster (`day_one` 3), the reading, the last page | Who knows the date (1–4); a copy of the margin; HC1's ending (0 nobody asked, 1 record, 2 public, 3 Bastion); where the site-log pages went (1 stove, 3 read, 4 Bastion's file); the first page (1, 2, 4); a Haven group at the gate; the paper sealed |
| `authors.terms`, `authors.road_post` | HC3 stages 3–4 | the reading, the finale's groups at the stand | 1 agreed, 2 bargained, 3 refused; the CO was told of the Red Hand's offer |
| `book.bastion` | R1 | B1, Bust Out | Bastion knows which of its commanders sold rifles: B1's outpost commander is that one, if they're still in the chair (value: that commander's leader seat id + 1; 0 if nobody on the page is still in the chair). Bust Out ×1.2 |
| `book.returned`, `book.kept`, `book.burnt` | R1 | — | What became of the page (or `book.bastion`) |
| `book.thief` | R1 | — | The thief: 1 went in the pit, 2 someone took his place, 3 his debt was paid, 4 walked to the Haven, 5 ran |
| `book.wrong_man`, `book.turned_in` | R1 | — | The wrong man went in the pit; a name on the page was turned in to its own people (that faction + 1) |
| `cargo.buyer_known`, `table.supplier_cut` | R2 | C1, the Long Table | C1's proof is done (C1 starts at stage 4); the house takes people half as often (both only when the loads went to the farm) |
| `cargo.told_haven`, `table.bastion_knows` | R2 | the Long Table | The Haven's chair knows where the strays go: with the farm as the buyer (`cargo.buyer` 1), the house is found out, to the Haven, and C1 starts at stage 4. Bastion knows the farm: found out, to Bastion |
| `cargo.truck_seen` | Tied Down | R2 | The Haven has followed a truck to its shed: R2's Haven version can start |
| `cargo.freed`, `cargo.delivered`, `cargo.bastion` | R2 | — | What happened to the load at stage 2 |
| `cargo.buyer`, `cargo.purge`, `cargo.cut`, `cargo.bastion_raid` | R2 | the Long Table (`cargo.buyer`, with `cargo.told_haven`) | Who buys (1 the farm, 2 the crew boss's pit); who heard it at stage 4 |
| `cargo.driver` | R2 | — | The driver: 1 at the Haven, 2 still driving, 3 in the pit, 4 in a Bastion cell |
| `well.stood` | R3 | H2 | The house stands: H2's "Stands alone" needs no barricades (which house: `well.house`) |
| `well.town`, `well.house` | R3 | — | R3's village (settlement index + 1) and its safehouse (index + 1; 0 if it has none) |
| `well.paid`, `well.warned`, `well.done`, `well.faked`, `well.tested` | R3 | — | What happened at the well |
| `weeks.public` | HC1 | H3 | Brokering the lab road needs Trusted with the Haven |
| `weeks.bastion` | HC1 | HC3 | HC3 starts at stage 2 |
| `weeks.records` | Patient Zero, Patient Zero's Ledger | HC1 | HC1 can start: the clinic records are out |
| `weeks.record`, `.captured`, `.lead_left` | HC1 | (facts) | The notebook is in the lab's record; Dr Wolff is in the lab's isolation room; the field lead left her station |
| `other_two.named`, `.haven_names`, `.ward`, `.sold` | HC2 | (facts) | The lab named its volunteers; the Haven did instead; the back room (1 closed, 2 at the lab, 3 kept, 4 sold from, 5 gone); one of the other two is in the Red Hand's pit |
| `authors.bastion_key`, `.guard`, `.open`, `.lead_first`, `.batch_sold`, `.formula_sold` | HC3 | (facts) | Bastion holds a key to the clean room; a Bastion squad held the lab gate; the method went to the Haven's clinics; the field lead's name is first on the paper; the boss paid for the batch; the method was sold to the Red Hand |
| `place.undercover` | C2 | C1 | C1's proof is done: it starts at stage 4; the undercover guest gets +5 Fame on top when they tell |
| `place.proof` | C2 | C1 | Got up from grace (or a ★ Police guest walked away) with the proof: C1 starts at stage 4 |
| `place.family`, `place.by_hand` | C2 | any | Someone sat at the long table and said a name; the house was finished at its own table |
| `packs.moat`, `packs.index`, `packs.contact`, `packs.home` | the packs, once a season day ([INFECTED.md](INFECTED.md#questline-hooks)) | questlines not written yet (seeds) | The Red Hand feeds a pack now (1/0); leaders Halcyon has tagged (a count); a pack was founded at a lost Bastion post (1); named ones brought home (a count) |
| `packs.book_told` | the packs | — | R1's rumour of "the crew in the woods" was told for the run that started at that time (unix) |
| `table.exists`, `table.town` | the Long Table, from the season's start | R2, any | There's a house this season (0 once it's broken up); the settlement it's past, as its index + 1 (0: no house, or none near). R2's loads go there, else to a crew boss's pit |
| `table.exposed`, `table.exposed_to` | the Long Table, once the house is found out | R2, any | Found out (to the faction told; -1 everyone), for the rest of the season. R2: the cut stops, and the Haven holds it against whoever took it or delivered a load |
| `table.broken`, `table.burnt`, `table.moved`; `table.suspicion` | the Long Table (the first three only once true) | any | Gone for the season; burnt by Bastion; moved once; how suspected (0–100) |
| `table.suspicion_add` | any line | the Long Table | Added to its suspicion, then cleared |
| `table.haven`, `table.bastion`, `table.halcyon`, `table.redhand` | C1 | any | Who was told |
| `table.walked_out`, `table.studied`, `table.host_wanders`, `table.paid`, `table.by_hand`, `table.boy_lost` | C1 (`table.by_hand` also C2) | any | How the house ended: walked out; the mother at the lab; the host on the roads as a Scavenger; paid to move; by your own hand; the boy not got out |

The Long Table also accepts the setter's own spelling for flags the table names under `table.`
(`cargo.supplier_cut`, `cargo.bastion_knows`).

**The cure finale's flags** ([FINALE.md](FINALE.md#endings-and-next-season)):

| Flag | Set in | Read in | What it does there |
|---|---|---|---|
| `finale.open`, `finale.finisher`, `finale.route`, `finale.how`, `finale.terms`, `finale.partner`, `finale.came` | the controller | the epilogue's record, outlets and stand board ([EPILOGUE.md](EPILOGUE.md)) | The routes are open; who finished (route + 1, 0 the lab alone); who controls (0 nobody: handed over); how it was won (1 held, 2 force, 3 deal, 4 betrayal, 5 theft, 6 handed over); the terms (1–4); the pact partner (route + 1); how many routes came for the room |
| `cure.slow`, `finale.taken`, `finale.coat_run` | the controller | (facts) | The lab alone made the batch; the season day Take the Lab and the Coat Run passed |
| `held.rh_knows`, `held.named`, `held.processed` | Held for Collection | (facts) | The Red Hand knows the cold store; the four ledgers named the tubes; Twice was brought home |
| `cook.heard`, `cook.still_known`, `cook.line_paid`; `cook.lines`, `cook.paid` | The Cook; every cut dose taken | The Cook's terms | Someone heard the cook's terms; found the still; her line marked paid. The cut doses written in her book, and those that held |
| `finale.plan_sold` | Sell the Plan (`Questlines/Finale/RZ_FinaleRoute.c`) | Take the Lab | The bay hold is 150 s |

## Loose Ends

✅ Built with the epilogue week ([EPILOGUE.md](EPILOGUE.md#the-closing-work)). Every questline run
still open at the cure gets one **closing stage**, so every story reaches an ending inside the week,
even a rushed or bitter one. It's offered by the questline's person at its place (else its faction's
stronghold seat): a person standing there with two or three ways, each ending on one of the
questline's built endings (`<key>.ending` set, its `OnEnding` world effect as built, and the world
settles). Whoever chooses is paid half a stage: Fame 3 and Favor 5 with the questline's faction, once.
If nobody does it, the last call comes in the person's voice when the week runs short of room, then
a session's grace, then the default ending as data, with no pay, said plainly. Processing is the hard
stop. The majors never need one (their codas are Last Rounds); the Long Table's two have their own
week; a questline with no row below ends as data at processing (*"... ended with the season,
unfinished."*). Each ending's line goes into the season block. *Simplified:* the ways are choices
with no checks or holds (but the Well Story's tin), the pay is fixed, and the world effect isn't
halved.

| Questline | The closing stage | The ways (ending) | Default (nobody did it) |
|---|---|---|---|
| H2 The Price of the Door | The Last Envelope | Carry the last envelope (Paid in tins); bring the exile home first (The debt is yours); tell the chair (Stands alone) | Paid in tins: *"The head sent a resident with the last envelope. Nobody asked who carried it."* |
| H3 The Edge of Town | The Last Walk | Get them to a dose (The lab road); let the walker walk them (The walks stay); sit with them, let the nurse decide (The lab road) | The walks stay: *"The walker walked one more and read twelve names at supper. The nearest dose was four kilometres away."* |
| Short Weight | Count the Stores | Count it honestly (Full Weight); cover for him (Short Weight); let the head find it at supper (Full Weight) | Full Weight: *"The head found the short weight at supper. The storekeeper eats last."* |
| B2 The Census | The Last Count | Bring the real count (Counted); forge it (Forged); burn the cards (Refused) | Counted: *"The count stays as last reported. A clerk files it unsigned."* |
| B3 The Last Pilot | The Last Flight | Fly with him, bring him back (He stays); talk him into walking instead (Grounded); sell the aircraft for parts (Burnt for the Red Hand) | Grounded: *"He didn't fly. The aircraft sits on the pad with its doors open."* |
| R2 Cargo | The Last Load | Drive the van to the queue (Told); hand the van to the crew (The cut); follow it (Told) | The cut: *"The van went, and came back empty. The driver walked to the queue alone."* |
| R3 The Well Story | The Last Toll | Pay it yourself, 15 Red Hand Favor (The story worked); stand with the village (Stood up); tell the story one last time (Faked) | The story worked: *"The village paid in its last tins and walked out lighter."* |
| HC2 The Other Two | His Coat | Carry the coat to his sister (Brought home); take his number to her board (Named); leave it on the hook (The work goes on) | The work goes on: *"The coat stays on the hook. Nobody came for it. The lab labelled it."* |
