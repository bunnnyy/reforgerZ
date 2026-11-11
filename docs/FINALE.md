# The cure finale

The last tenth of the cure is a fight over who controls it. At 90% the lab stops; five routes race
to finish the batch; players lock to one route for the season; and the final stand decides who holds
the clean room when the cure goes out. Strongholds and the lab never fall: the clean room changes
hands, the lab doesn't.

Code: `Cure/RZ_CureFinale.c` (the controller, saved), `Missions/Questlines/Finale/` (the five routes,
the desks, the tissue hunts, the items), `World/RZ_FinalePeople.c` (the people). Settings:
**Director > Finale** ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#5-settings-objects)). What's left to
build is in [designs/FINALE.md](../designs/FINALE.md).

## Contents

- [In one page](#in-one-page)
- [The gate](#the-gate)
- [The lock](#the-lock)
- [How a route runs](#how-a-route-runs)
- [The five routes](#the-five-routes)
- [Live tissue](#live-tissue)
- [What routes do to each other](#what-routes-do-to-each-other)
- [The finish, the batch and the seal](#the-finish-the-batch-and-the-seal)
- [The stand: a fight for the clean room](#the-stand-a-fight-for-the-clean-room)
- [The release: terms and deciders](#the-release-terms-and-deciders)
- [Fame](#fame)
- [Fallbacks](#fallbacks)
- [Endings and next season](#endings-and-next-season)
- [Items and people](#items-and-people)
- [Simplified, and not built yet](#simplified-and-not-built-yet)

## In one page

| What | Winning it means | When |
|---|---|---|
| **The finish** | Your route's piece is in first. The lab makes the batch from it, your faction gets the route's capped chunk of season Fame, and your side starts the stand holding the clean room | The last 10% (about 4–9 real days at the design point) |
| **The clean room** | Your side holds it when the stand is won: you control the cure | The stand: 15 minutes, plus at most 3 of overtime |
| **The terms** | Your route's people decide how the cure reaches people | The release, straight after the stand |

Both count: a route can finish and lose the room, or take the room without finishing.

**The ring of doors.** Each faction route needs one thing another faction holds, and every door has
a slower other way, so no route is ever locked out:

| Route | Questline | Its piece | Its door (who holds it) | The other way |
|---|---|---|---|---|
| The Haven | The One Who Didn't Turn | Her blood, by her own yes: two bags and a last draw at dawn | Her boy's line in the book (the Red Hand) | She gives without it, one step harder |
| Bastion | Held for Collection | The captain's box from the Day 2 column, signed in at the lab | The slips on the bus driver's clipboard (the Haven) | The CO signs a manifest from the four ledgers |
| The Red Hand | The Cook | A case of cut doses cooked on a tray of the 90% batch, proved on someone who asked | A tray of the batch (Halcyon) | Four stolen doses: a harder base |
| Halcyon | The Lower Store | The reference vials from the closed site, compared in the lab | Item 114, the key to Bastion's chain (Bastion) | The sergeant's second key, or cut the chain |
| Independent | One Each | Three bags from the eight who walked out of the Day 6 trial, each by their own yes, and a tray off the books | Whoever holds each of the eight (every faction) | Donors at a Haven medic, when fewer than three are alive |

Every route also needs live tissue from the infected ([Live tissue](#live-tissue)).

## The gate

Research stops at **90%** while the five routes are registered and the finale is on
(`IsAtFinaleGate`; [DESIGN.md](DESIGN.md#cure-research-and-the-seasons-end)). Reaching it:

- the gate news, heard when someone is in the world: *"The lab has gone as far as samples can take
  it: 90 percent. What's missing isn't in a sample case, and every faction in the Zone thinks it
  knows where it is."*;
- the lab's open channel: *"Ninety. It won't go further on what we have..."*;
- the route's worth is fixed: **Route worth** (0.25) × the leading faction's season Fame, never under
  **Floor** (150), never over **Ceiling** (0: none);
- the people and places are seeded as season canon ([Items and people](#items-and-people));
- every route begins in the finale's own lane (outside **Max active** and the faction slots), each
  with its own opening line (the Haven's secure channel, Bastion's call, the Red Hand's open channel,
  the duty officer's numbers, and the ham and the Doctor for the Independent);
- each route's **desk** opens ([The lock](#the-lock)).

**What players see.** The cure percent everywhere (`{cure_percent}`, the HUD, the lab's line) is
**90 plus the furthest track**, at most 99 until the seal. Each route's journal entry ends with the
lab's readout, by faction, never by player: *"Ninety-four. The Haven's work is at seven... Nobody's
is at four."* A desk's "How do things stand?" gives the readout, the route's worth (*"About 375
season Fame to the faction that finishes."*), what you'd bring, and for a locked player their
route's pact and items. Lock counts are never shown.

**What carries on.** Samples are still taken and paid into the backlog; they don't move research
past 90. Ordinary missions, errands and majors carry on.

## The lock

**The desks** are the commit node (the dialogue confirm node is planned). Each route has one, from
the gate until the stand is won, not on the radio: the route's clerk near its stronghold leader (the
Independent's: the Doctor at the lab gate), with a go-between and a lost-and-found beside it.

| Choice | Does |
|---|---|
| **What would it mean?** | The lock card, tagged **[Point of no return]**: the route's admission, what you bring (your items for it) and what you leave. It arms the next choice for 30 s |
| **Write me down [no return]** | Locks you to the route, within 30 s of hearing it |
| **How do things stand?** | The finale page ([The gate](#the-gate)) |
| **The price door** (faction routes) / **The Coat Run** (Independent) | Below, and [The Coat Run](#take-the-lab-and-the-coat-run) |
| **Pact with ...** (the go-between, four choices) | Propose, agree or end a pact ([Pacts](#pacts-and-deals)) |
| **Give back what you found** (the lost-and-found) | Your items of this route go back to their seats; 10 Favor with its faction if you're not locked to it ("found it") |

**Who may lock** (`CanCommit`): research at the gate or past it and the stand not won; not locked
this season; and `CanSideWith`: yes unless Hunted by the route's faction; the Hunted lock if they're
owed (the parley, [REPUTATION.md](REPUTATION.md#the-parley-and-the-knock)) or have paid the route's
price door. The Independent route is always open.

**Price doors** open the route only. Infamy stays.

| Route | The price door |
|---|---|
| The Haven | **In pencil**: at dusk, unarmed, 4 food. A council head writes you on the List in pencil. It opens the route, not the door |
| Bastion | **Sign on, attached**: hand over one firearm (taken from your weapon slots). Sedlak writes you on the roster |
| The Red Hand | **A line in the book**: whatever Red Hand Favor you have goes on it (up to 60) |
| Halcyon | **The volunteers' book**: unarmed, a draw of your own blood and a notebook number |

**The lock** is per identity for the season, and dying doesn't undo it. All locks end at the cure.
Late joiners lock until the stand is won, but a lock made after the batch is ready doesn't count at
that stand. An **active lock** has 2+ hours played this season (**Active lock hours**, × the wait
scale) and was online in the last 2 real days (**Active lock days**): only active locks count for
pacts, holding the room and deciding.

**After the lock:** other routes' work refuses you (*"You've picked your side. This isn't it."*);
ordinary errands and missions carry on; your route's sworn work is told to you as it comes up; and
the route's questline is in your journal whether you took part or not.

## How a route runs

The same four acts on every route:

| Act | Kind | Track | What |
|---|---|---|---|
| I | Open | +2 | The question: what's missing, where it is, who holds the door |
| II | Sworn | +1, then +2 | The door or its other way, then the piece |
| III | Open | +2 | Live tissue ([Live tissue](#live-tissue)) |
| IV | Sworn, last | +3 | Bring it in; the last step at the lab. Needs a tray in the lab |

- **Open work** is decided by anyone not locked to another route who can side with the route's
  faction (anyone at all for the Independent). **Sworn work** only by the route's locked players.
  Everyone else is refused at the choice.
- **Pay** for each step: the decider gets at most 10 Fame and 20 Favor with the route's faction
  (the Independent: 20 Haven Favor and no Fame), once per route per player. On sworn acts, unlocked
  players who took part get 10 Favor and no Fame.
- **Points** come when a step is done or something is handed in, never for carrying, and are never
  taken back. A track never passes 9 before Act IV's last step, which takes it to 10. Steps resolve
  one at a time on the server, so two routes never finish together.
- **Acts II and III run side by side**: the tissue hunt is offered as soon as Act I is done; Act IV
  waits for both.
- **Never a dead end.** A route never fizzles: a stage that fails or times out is offered again. It
  freezes at another route's finish and ends at the cure.
- **Scenes.** Every stage is a scene: people at places, put down only within 300 m of a player, each
  with up to four choices (talk, persuade, sneak, bribe, a hold, a hand-in). Persuade uses the fixed
  score (`TryPersuade`): your access rank with the faction, +1 for a listed background, against 1–4;
  the majors' flags move it one step. Bribes are Favor. Sneaks are the questline sneak (unseen, no
  shots). A hold keeps a place for minutes while the noise brings the infected.
- **Time windows** are game-clock waits (no time skip): the Haven draws at dusk or after dark and
  walks up the hill at dawn (05:00–09:00); the Red Hand's still runs on the lab's night shift
  (20:30–05:00) and the proof by day; the Independent asks at dusk and fills the batch sheet at a
  change of shift.
- **Claims.** Route stages never claim the lab. The controller claims it for the batch hold and the
  stand. Routes claim their own subjects instead (`person:<key>`: the woman, the captain, the cook), held until the route ends.
- **No crossing-lines mark.** Finale stages never write the questline engine's crossing-lines mark:
  the lock replaces it, and ends at the cure.

## The five routes

### The Haven: The One Who Didn't Turn

She was on the bus on Day 2, bitten in the church doorway on Day 4, kept back from the Day 6 hill by
her council head, and crossed off the List with the pencil the houses use for the dead. Since then
she's "the lodger", moved from house to house at dusk (the rota moves her every 3 real days). Her son
runs with a Red Hand crew. Nobody, the writing included, ever says why she didn't turn.

| Stage | Ways through |
|---|---|
| **I. The Crossed-Out Name** (open) | The chair (Known with the Haven); her name off the List (with `list.read`); watch a house at dusk for the second bowl (unseen); feed her hungry house (3 food); the lab's Day 6 ledger (Known with Halcyon, or 20 Halcyon Favor) |
| **II. And my boy** (sworn) | Her yes: persuade 3 on Haven rank (Nurse, Doctor or Elder +1; one easier with her name read at supper, one harder if she was sent to the far house). Then the door: pay his line (30 Red Hand Favor, Trusted; 15 under Paid at the Door), walk him out at night (unseen), or it's already clear (the book sent off loud, or a pact with the Red Hand). The other way: she gives without it, one step harder |
| **II. Three Times** (sworn, at dusk or after dark) | Draw a bag at the edge of her town (a cool bag, 15 minutes out of the cold, +10 with a Nurse or Doctor carrying; draws 12 real hours apart; five at most, never a sixth), store two at the lab. "Two is enough" (Trusted with the Haven) spares her the dawn |
| **IV. Up the Hill** (sworn, last, at dawn) | Walk up the hill with her and bring her in at the gate; or draw at first light and run it up in 12 minutes. Spared: the lab's 2 real days of work on two bags |

Terms: **her house, and her boy**; **the far house first** (only under workers first); **by the bite
book**; **the crossed-out first** (with `list.read`). Nobody chooses: a Merciful or Idealist chair
reads the bite book, else her house first.

### Bastion: Held for Collection

On Day 2 the battalion's medical officer took a tube from everyone in Sedlak's column. "Held for
collection", he wrote, and took the freezers to the cold store in the hills (`held.store`), where he
still is. He opens to "collection", only the CO can say it, and a tube goes only with its slip.

| Stage | Ways through |
|---|---|
| **I. Request Instructions** (open) | Sedlak at the roster (Trusted); bribe the quartermaster (20 Bastion Favor); the bus driver (Known with the Haven); the diesel line in the Red Hand's book (15 Red Hand Favor; sets `held.rh_knows`) |
| **II. Names for the tubes** (sworn) | The door: the driver's slips (persuade 2 on Haven rank, Police or Elder +1), the clipboard at night (unseen), or the census (Counted, or a pact with the Haven). The other way: the CO signs a manifest from the four ledgers (Trusted; sets `held.named`) |
| **II. Collection** (sworn) | At the cold store: collection on the CO's word (persuade 3 on Bastion rank, Soldier or Police +1; one easier under Bastion, Command, one harder under Expended); a signal form (40 Bastion Favor); the box while he sleeps (unseen); or cut the door (+15 Bastion Infamy, one tube spoils). The box keeps 45 minutes out of the cold. Bringing Twice home (persuade 3 on Red Hand rank) adds his tube and sets `held.processed` |
| **IV. Signed For** (sworn, last) | Sign the box in at the lab. Fewer than three good tubes: the receipt goes out short and waits 2 real days a missing tube. The receipt is read on Bastion's call |

Terms: **the roster** (not if the box came by a form, sneaking or force); **the column**; **by need**
(Trusted); **the census** (only under Bastion, Command).

### The Red Hand: The Cook

The cook makes the dusk meal at the Red Hand's stronghold and, after supper, the cut dose on a farm
still (`redhand.still`). On a tray of the lab's batch, she says, it would hold every time. Every cut
dose a player takes is a line in her book (`cook.lines`, `cook.paid`).

| Stage | Ways through |
|---|---|
| **I. The Kitchen Book** (open) | Ask her at supper (Known); win her round (persuade 2, Ex-Con, Lab Tech, Doctor or Nurse +1); follow her after supper (unseen; `cook.still_known`); her sister (3 food). Hearing her sets `cook.heard` |
| **II. A Tray** (sworn) | The door, a tray: the director (Trusted with Halcyon), a page of her tally (a pact with Halcyon, or `proof.redhand`), a field lead (40 Red Hand Favor), or the night store (unseen; Halcyon morale −5). One tray out per route; the lab has three and rebuilds one a real day. The tray keeps 15 minutes out of the cold. The other way: four stolen doses (30 Red Hand Favor), a harder base |
| **II. A Night on the Still** (sworn, 20:30–05:00) | Keep the still running 8 minutes while she cuts the pan, or run it cold (unseen, a clean run). The case keeps without a cold clock |
| **IV. The Lab's Word** (sworn, last, by day) | The proof on someone bitten who asked: 60%, +15 for a clean run or the formula sold, −15 on the harder base; each failure is a step higher and the third pan always holds. Then the case in at the lab. Her own term: the boss marks her line paid (persuade 3; `cook.line_paid`) |

Terms: **the boss's price** (Trusted); **pay when you can** (someone heard her); **the first pan to
the houses** (Known with the Haven); **the book first**.

### Halcyon: The Lower Store

The reference stock, the first isolate, is in a dewar in the closed site's lower store
(`origin.site.lower`), behind the chain the cordon detail put on at 04:10 on Day 1. Two keys were cut.
"Proof" stays the major's: the route never re-decides the paper's first page.

| Stage | Ways through |
|---|---|
| **I. Reference: None** (open) | The duty officer (persuade 2, Lab Tech or Doctor +1); the director's stand-in for the vial map (persuade 3, 2 with `weeks.record`); the sergeant; the records room (unseen). Under The Whole of It the director simply says |
| **II. Item 114** (sworn) | The door: the adjutant (persuade 2 on Bastion rank, Soldier or Police +1), the quartermaster (40 Bastion Favor), the key board (unseen), or a pact with Bastion / Under Seal. The other way: the sergeant's second key (Known with the Haven), or cut the chain (a 3-minute hold, +10 Bastion Infamy) |
| **II. Something Cold** (sworn) | Borrow the vet's tank (Known with the Haven, or 25 Haven Favor: a real day out of the cold; else the courier's case, 30 minutes), then hold at the dewar (4 minutes, under 3 with the map) |
| **IV. Samples First** (sworn, last) | Hand the vials in; the comparison takes 28 real hours (56 without Wolff's method), then a 4-minute hold while the centrifuge runs |

Terms: **the authors first**; **the volunteers first**; **by the numbers**; **published**.

### Independent: One Each

On Day 6 ten bitten Haven people walked up the hill for the first trial dose, and eight walked out.
Whatever it taught their blood, they've carried it longest. A retired schoolteacher with a long-wire
aerial (the ham, `ham.house`) has copied every number the lab read into exercise books.

| Stage | Ways through |
|---|---|
| **I. Eight Walked Out** (open) | The ham's exercise books; the Day 6 ledger (Known with Halcyon, or 20 Halcyon Favor); the Doctor at the lab gate (persuade 1, Doctor or Nurse +1) |
| **II. Asking** (sworn, at dusk) | Each of the eight, by their own yes and their keeper's leave: a Haven house's council head (persuade 2 on Haven rank, Nurse +1), Sedlak (Known with Bastion), a crew boss (30 Red Hand Favor) or a field lead. Each gives one bag. The other way, when fewer than three are alive: donors at a Haven medic |
| **II. Three bags** (sworn) | Store three bags at the lab, "for everyone" |
| **IV. For Everyone** (sworn, last, at a change of shift) | A tray off the books (a field lead, persuade 2 or 1 with `authors.open`; or 30 Halcyon Favor), then the batch sheet, "Released to: -": the director (persuade 3, easier under The Whole of It or with an Idealist or Merciful director), 40 Halcyon Favor, or the records room at night (unseen) |

Terms: **open** (one each); **one side, with a condition** (all of them, exiles too / no names taken /
one tin, not two / a home batch at every house); **sold**; **kept back**. Nobody chooses: open.

## Live tissue

Act III of every route is open work built on the tribes' hunts. As soon as a route's Act I is done,
the controller offers it a tissue hunt (the route that has waited longest first; at most one hunt of
each kind at a time across all routes):

| Specimen | Counts |
|---|---|
| A leader taken alive (Capture the Index's rules) | +2 at once |
| Else one of a pack's guard or young, or a special (Live Capture's rules) | +2 after 12 real hours of the lab's work |
| Nothing to hunt with on this server (no prefabs) | +2 after a real day: the lab grows it from its own samples |

Whoever hands the case in credits their own route if they're locked to one that still needs tissue,
else the route the hunt was offered for, and is paid as an open step. A route whose tissue is in gets
no more hunts. Each route's words: Who It Was, Contact Captured, Fresh Stock, The Index, Nobody's
Index (never "alpha", "tribe" or "crawler"). The packs' side (Fury, succession) is
[INFECTED.md](INFECTED.md)'s.

## What routes do to each other

### Fighting routes

Two routes are **fighting** for 2 real days (**Fight memory days**) after a hostile act: killing one
of its locked players, picking up or handing in its item, breaking a pact. During the stand every two
sides are fighting. Fighting closes the parley with that route's faction, and sets the stand's
hostility bits.

### Pacts and deals

A pact is two routes standing together. A locked player proposes at their route's go-between (one
real day to agree); it's made when each route has at least one vote and a majority of its active
locks voted (under All Stations, one vote each at once for the Haven and Bastion). One pact per route;
Bastion and the Red Hand can't while the book went to Bastion or the orders say Expended. Under Seal
starts a Bastion–Halcyon pact made. Ending one before the stand (a majority of either route) costs
nothing. **Breaking one** (taking the room from a partner, or Take the Lab against it): +15 Infamy with
the partner's faction for every active lock of the breaking route, and the pact ends. A pact opens a
door it names (the routes' "by the pact" choices), and at the stand the two routes are one side. A
holder's partner at 7 or more is a **deal**: its condition goes into the terms and its rules apply at
half next season. Deals never move Fame.

### Items on the road

Route items drop where their carrier dies and are picked up by whoever walks over them: a rival keeps
it, gives it back at the route's desk ("found it"), or lets it spoil. A dropped item nobody comes near
goes back to its seat after 2 hours of play or a real day. **Last Mile:** when a route reaches 8,
everyone hears *"One of them is close."*, and every 10 minutes the other routes' locked players get a
rough area for any of its items on the move.

### Kills between route players

- **Outside the stand,** killing a player locked to a faction route costs **+3 Infamy** with that
  faction (the Haven, for an Independent-locked victim), at most **12 an hour**, tagged war.
- **At the stand,** within 150 m of the lab, fire between locked players of different sides costs
  nothing, and the lab's safe-zone rule doesn't apply to locked players. Killing an unlocked player
  there: +10 Halcyon Infamy.
- **Your own side:** +10 Infamy with your route's faction; at the stand you no longer count for the
  room or its Fame.
- Nothing new is paid for these kills; the built bounties are unchanged.

## The finish, the batch and the seal

When a route's last step lands its track reaches 10 and it **finishes**: the route's Fame chunk is
paid to its faction (the Independent's is held for the release), the route becomes the **holder**,
the other routes freeze (their stage work called off with no pay and no penalty, their items gone)
and the race doesn't reopen this season. The finish takes a tray from the lab (the Red Hand's case
brings its own); with none in, the last step waits for one.

**The batch hold:** 12 real hours **and** 2 hours of play (**Batch real hours**, **Batch play hours**,
× the wait scale). The lab is claimed for it. Players can still lock (they count at a later stand),
and the Coat Run can be tried.

**The seal.** When the batch is ready, every channel says *"The batch seals in an hour."*, again at 30
and 10 minutes (**Seal call minutes**; **Seal hour** makes it wait for that server hour first). At the
seal one `AddFinaleResearch` call takes research to 100, credited to the finisher's faction, and the
stand starts (its clock waits for the first player at the lab, as built). The finisher starts as
holder; after a passed Coat Run, the Independent; after the slow finish, nobody.

## The stand: a fight for the clean room

The stand's waves, timing and abandon rule are as built ([DESIGN.md](DESIGN.md#cure-research-and-the-seasons-end)).
On top:

| Rule | |
|---|---|
| **The clean room** | 10 m (**Clean room radius**) round a land point 15–25 m from the lab, the same all season |
| **Who counts** | Active-locked players who locked before the batch was ready, online, alive and inside. Unlocked players, AI and anyone who shot their own side never count |
| **Sides** | Each route, or two routes in a pact |
| **Taking it** | **Force:** the only side inside counts up. **Tally** (**Contest**): the side with the most counted players inside counts up. A side reaching its take time becomes the holder; a count not counting up falls back 1 s a second |
| **The take time** | 90 s (**Take seconds**) minus 5 s (**Take per point**) a point on the side's frozen track (a pact uses the higher), never under 45 s (**Take floor**). The finisher retakes in 45 s |
| **Holding it** | The holder needn't be inside; it only has to stop anyone else's count reaching its take time |
| **The end** | At 15:00 the holder controls the cure. While a challenger's count is above zero, **overtime** runs, 3 minutes at most (**Overtime seconds**). The holder wins ties |
| **What everyone sees** | Within 150 m of the lab, a short notice when a count moves 15 s or the room changes hands: *"CLEAN ROOM: Bastion. The Haven taking: 0:35 of 0:60."* Routes, never players |

**Groups.** At most two friendly faction groups (**Groups at the lab**) stand 40–120 m out: the
finisher's faction first, then factions whose route has active locks, then those a major sends
([QUESTLINES.md](QUESTLINES.md#the-stands-inputs): the Haven at the gate after The Whole of It,
Bastion by its stance, the Red Hand on the lab's road by its last page); never Bastion under
Expended. A group on the lab's road stands 150–250 m out. Halcyon never takes sides: its guard is the lab's own. Within 1.5 km of the lab a
player locked to a side fighting a faction's route carries that faction's hostility bit, so its
group fights them (never Halcyon's bit; not in Tally mode). The groups are removed when the stand ends.

**No disguise near the lab** during the stand (built: [REPUTATION.md](REPUTATION.md#borrowed-kit)).

### Take the Lab and the Coat Run

- **Take the Lab** (the Red Hand route, once a season; **Take the Lab**). Offered at the stand to the
  Red Hand route if it has a lock, unless the Red Hand's last page closed it (`book.last` 1–4). It
  leaves the random pool while the finale runs. The lorry backs up to the **loading bay** (25 m from
  the lab, away from the clean room). A Red Hand-locked player Trusted with the Red Hand starts the
  hold; every Red Hand-locked player who counts at the stand holds it: 120 s (**Bay seconds**; 90 with
  `authors.batch_sold`, 150 with `finale.plan_sold`), paused while a counted player of the holder's
  side is within 15 m. Done: the Red Hand route holds the room at once (`how` theft). Against a pact
  partner it's a betrayal.
- **The Coat Run** (the Independent, once a season; **Coat Run**). During another route's batch hold,
  at a change of shift (05:30 or 17:30), an Independent-locked player passing in Halcyon's kit (the
  built look) or with the Lab Tech background takes the desk's Coat Run. A holder-route player within
  15 m of the clean room calls them out. Passed: the batch is re-labelled inside the lab ("Released
  to: -") and the Independent starts the stand as holder; the batch's route hears it at the seal.
  Caught: +3 Halcyon Infamy, and the batch's route is told at once. Spent either way.

### How it was won (`finale.how`)

| How | When |
|---|---|
| **theft** | Take the Lab got the room |
| **betrayal** | The room was taken from a pact partner |
| **deal** | The holder had a pact partner at 7 or more |
| **force** | The room changed hands at the stand |
| **held** | Nobody took it |
| **handed over** | Nobody held it ([Fallbacks](#fallbacks)) |

## The release: terms and deciders

The moment the stand is won the cure goes out (as built: infection −70%, the line stands down). The
controller's route goes to its **release**: the duty officer brings the form to the clean room, and
for 3 minutes the route's locked players there choose its terms ([The five routes](#the-five-routes)).
Nobody chooses: the stronghold leader's traits pick. The **deciders** are up to four of those who
chose, recorded by name. With a pact partner, the partner's faction is the condition. The terms
apply the route's world effects (one call each, through the finale's lane of the season budget:
morale ±10, strength ±5 a faction) and pay the deciders: +15 Fame and 30 Favor with the route's
faction (the Independent: open +10 with each faction, one side +15 with it, sold or kept back +5
Infamy with each).

## Fame

**The route's chunk.** One payment into a faction's **season Fame** (never a player's standing), at
the finish, once a season, never taken back or paid again. Its size is fixed at the gate. It counts
in full until the faction reaches **Soft line** (1.10) × the best other faction's season Fame; above
that, half. The Independent's chunk goes to nobody unless it controls and chooses one side: then that
faction gets half. Log: *"Finale: Bastion's route paid 375 season Fame (375 full, 0 half)."*

**The stand's Fame** replaces the built Halcyon +50 and Haven +30: +30 with the route's faction for
the controller's players at the lab for half the stand or in the room 60 s, and +20 Halcyon for
everyone at the lab for half the stand who isn't on a rival side ("the lab never fell"). It's paid
after the winner and the record, so it decides nothing. The hall of fame's "Held the final stand"
lists them.

**Caps.** Over all finale content, 50 Fame per faction per player; 60 Favor and 40 Infamy per faction
per player. The finale is the one place past the limits every questline keeps
([QUESTLINES.md](QUESTLINES.md#limits-every-questline-keeps)).

| Control decides | Season Fame decides |
|---|---|
| Who has the cure, its terms, the rules next season in full | The hall of fame's line (*"The cure: the Haven, by force. The Zone's Fame: Bastion, 1640."*), the faction when nobody holds the room, and the Fame leader's rules at half next season |

## Fallbacks

| Case | What happens |
|---|---|
| **The slow finish** | After a real week at the gate the built fallback (**Finale fallback**, 1% a real day) feeds the **lab-alone track** instead of research. At 10 the lab makes the batch on its own work: the same hold and seal, nobody starts as holder, no route Fame, `cure.slow` set |
| **Nobody holds the room** | The cure is **handed over** to the faction with the most season Fame |
| **A lost stand** | As built: the batch is ruined, research back to 90, the lab never falls. The finisher stays finished; locks and pacts stand. The next batch comes from the finisher's remake step (its route's wait, about a session or a day) or the lab's backlog, whichever is first; then the seal, and the finisher starts as holder again. Stands lost are recorded by day |
| **A route's subject is lost** | She walks off after a third betrayal; the eight who died stay dead. *Simplified:* the branch stages (What She Left, The Book Without Her...) aren't built ([designs/FINALE.md](../designs/FINALE.md)) |
| **No tissue to hunt** | The lab grows it after a real day |
| **Small servers** | Every act has a solo way; in Tally mode a lone holder keeps a room nobody contests |
| **The finale off** | Research runs to 100 and the cure goes to the most season Fame, as before |

## Endings and next season

The cure's notice names the controller and its terms (*"The Haven holds the cure. It goes hand to
hand, and the first box went to her house."*). Each route ends with a line: controlled, finished and
lost the room (*"Our cook's pan is in their batch. Write that down."*), or never finished (*"Reference
stock: wanted."*). Losing routes keep everything they were paid. The contest ends at the cure: the
stand's hostility comes off, and parley reopens.

**Season flags** for the epilogue: `finale.route` (the controller, route + 1: 1 the Haven, 2 Bastion,
3 the Red Hand, 4 Halcyon, 5 Independent; 0 nobody), `finale.finisher` (0 the lab alone), `finale.how`,
`finale.terms`, `finale.condition`, `finale.partner`, `route.<n>.state` (0 not worked, 1 worked, 3
finished), and the subjects' (`held.named`, `held.processed`, `cook.line_paid`, `cook.heard`...).
`last_season.json` keeps the route, finisher, how, terms, partner, deciders and the half rule set.

**Next season** ([DESIGN.md](DESIGN.md#next-season-rules-from-the-last-winner)): the controller's
rules in full; the Independent by its terms (open: half the Haven's and half Halcyon's; one side: that
faction's at half; sold: only the Red Hand's black market; kept back: nothing); then at most one half
rule set on top: a deal partner's, else the season's Fame leader's when it isn't the controller
(**Fame leader rule**).

## Items and people

**Items** (`RZ_FinaleItems`, saved) are data carried by one identity: her bags, the captain's box, a
tray, the cook's case, the reference vials. Each keeps its route, its holder or where it lies, its
cold clock (counting only outside the cold: within 100 m of the lab, 60 m of a Halcyon holding or
Bastion's stronghold) and its chain of carriers. An item at 0 spoils, and its route is told. Items for
another route never count for yours.

**People** (`RZ_FinalePeople`, saved), seeded at the gate as season canon: the woman (`unturned.name`,
her house on the rota) and her son; the captain at `held.store` (1.5–5 km from Bastion's stronghold,
best 2–4 km from the lab, away from towns; `held.count` 280–340); Twice, Private Marek Fiala, with the
crew nearest the store; the cook at the Red Hand's stronghold, the still (`redhand.still`, outside a
town 1–4 km away, never within 1.5 km of the Long Table's farm) and her sister at the nearest Haven
house; the lower store (`origin.site.lower`, within 150 m of the closed site), the sergeant and the
vet; the eight (`day6.cohort.<n>`: most at Haven houses, one at a Bastion seat, one at a Red Hand
seat, one at a Halcyon seat, one or two dead); and the ham (`ham.house`, an empty village never within
2 km of the farm). Each records alive, holder, the route their yes went to (one at a time), draws and
betrayals. Nobody is killed off-screen, and consent is a check in every draw and dose.

## Simplified, and not built yet

*Simplified:* the commit node is the desks (dialogue's confirm node is planned); `CanSideWith` is
the finale's own (unless Hunted, owed, or the price door); route assets, pitches at 80% and 85%, route
marks, lock cards, Hand to Hand and the word overlays aren't built, so the lock card's "you bring" is
your items; the deal needs no gate meeting; the clean room is a land point (a named child entity on
the lab prefab is a VERIFY); unconscious players count while alive (VERIFY); Contest Auto plays as
Force (reading the player-damage setting is a VERIFY); the stand's line is a notice, not a HUD line;
the journal's finale page is the desk's "How do things stand?" and the readout under each route;
Halcyon's price door takes no blood draw; the Red Hand's line takes what Favor you have; each route's
stages are one scene each with the ways listed above, not every way in the design.

**Planned** ([designs/FINALE.md](../designs/FINALE.md)): the subjects' branches (What She Left, Twice
for the Haven, Processing Again's donors and the Scavenger, The Book Without Her and The Bottles, second
best for Halcyon); the moves (drain, spoil the case, guard the door, custody, sell the plan, hand it
over, Last Mile's contracts) and notes for sale; the routes' rival choices and the cold sources beyond
the cool bag, case and tank; the fuel clock and Bastion's stage road posts, truck and squad; the
helicopter and the annex as carries; the sit-in at the lab gate; route stages pausing on 3 days with
no active lock; the finale's epilogue carriers.
