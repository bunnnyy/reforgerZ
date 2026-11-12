# The cure finale

The last tenth of the cure is a fight over who controls it. At 90% the lab stops; five routes race
to finish the batch; players lock to one route for the season; and the final stand decides who holds
the clean room when the cure goes out. Strongholds and the lab never fall: the clean room changes
hands, the lab doesn't.

Code: `Cure/RZ_CureFinale.c` (the controller, saved), `Missions/Questlines/Finale/` (the five routes,
the desks, the tissue hunts, the items, the route assets), `World/RZ_FinalePeople.c` (the people),
`dialogue/finale/routes.rzd` (the pitches and the commit topics). Settings:
**Director > Finale** ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#5-settings-objects)). The pieces the
first pass left out are `Cure/RZ_FinaleRest.c` ([below](#the-rest-of-the-finale)).

## Contents

- [In one page](#in-one-page)
- [The gate](#the-gate)
- [Before the gate: pitches and route assets](#before-the-gate-pitches-and-route-assets)
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
- [The rest of the finale](#the-rest-of-the-finale)
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
**90 plus the furthest track**, at most 99 until the seal. After a lost stand it shows the lab's own
research from 90 until the remake or the backlog makes the batch again. Each route's journal entry ends with the
lab's readout, by faction, never by player: *"Ninety-four. The Haven's work is at seven... Nobody's
is at four."* A desk's "How do things stand?" gives the readout, the route's worth (*"About 375
season Fame to the faction that finishes."*), what you'd bring, and for a locked player their
route's pact and items. Lock counts are never shown.

**What carries on.** Samples are still taken and paid into the backlog; they don't move research
past 90. Ordinary missions, errands and majors carry on.

## Before the gate: pitches and route assets

**Pitches** (with **Dialogue > Trees** on, `dialogue/finale/routes.rzd`). From 75% every leader's
"Ask about the cure." answers *"Ask me again when it's nearly done."* From 80% the four stronghold
leaders (the chair, the CO, the boss, the director) and the Independent's voice on the road (the
Doctor wanderer, else the Courier) give the route's pitch: its best argument, then its admission. At
85% each pitch goes on air once, naming nobody: the four on their factions' open channels, the
Independent's as an unsigned voice on the open channel (log: *"Finale: the routes' pitches went on
air at 85 percent."*). Nothing locks before 90%.

**Route assets** (`RZ_FinaleAssets`, saved with the finale). A story job's outcome can lean toward a
route: at most one asset per outcome, at most 3 per player per route a season, kept by identity and
told to the player (*"That counts for something with Bastion, if you ever stand with them:
standing."*). The finale page and the lock card show them as "What you'd bring". An asset does
nothing until its owner locks to that route; then it's a card played once, in that route's scenes:

| Asset | In that route |
|---|---|
| **Ally** | One rank or persuade check one step easier (someone who vouches) |
| **Proof** | One persuade one step easier |
| **Standing** | One price a third lower, or one leader's check one step easier |

Assets for other routes go dormant at the lock and lapse at the cure. No asset closes a route. Where
they come from (`RZ_StoryKit.Record` hands every story record to `RZ_FinaleAssets.FromRecord`; one-choice
jobs and a few others tag theirs by hand through `RZ_RouteAssets.Add`, [DESIGN.md](DESIGN.md#story-blocks)):

| Outcome (record) | Asset |
|---|---|
| The black box burnt in the stove (`crash.box` stove) | Standing, Bastion |
| The manifest read on the open channel (`crash.box` aired) | Proof, Independent |
| The box to the fence (`crash.box` redhand) / to Bastion (`crash.box` bastion) | Proof, the Red Hand / Bastion |
| A trapped family to a house (`trapped.family` house, hungry) / to Bastion | Ally, the Haven / Bastion |
| A deserter the Haven hid (`struck.fate` haven) / back on the roster / let go | Ally, the Haven / Standing, Bastion / Ally, Independent |
| A scientist who walked out (`colleague.fate` left) | Ally, Independent |
| A distress call answered (`distress.saved`) | Ally, Independent |
| A racket's receipt / a house covered / the tally kept secret (`racket.*`) | Standing, the Red Hand / Standing, the Haven / Proof, the Haven |

## The lock

**The commit node** (with **Dialogue > Trees** on). Each faction route's "Ask about the cure." is an
`always` topic at its stronghold leader and at its Loyal seat leaders, so a leader who refuses you, or
one far away, still offers it; the Independent's is at the Doctor on the road, else the Courier. Its
**What would it mean?** opens a confirm node tagged **[Point of no return]**: the leader's line, then
the finale's three plain lines (the route's admission, what you bring: your items and assets for it,
and what you leave). Its one commit choice ("Write me down.", "Sign the roster.", "Put me in the
book.", "Write me in the notebook.", "Count me as nobody's.") is dimmed with `CanCommit`'s reason
when you can't lock, and the engine checks `CanCommit` again when you pick it
([DIALOGUE.md](DIALOGUE.md#the-finales-commit)).

**The desks**, from the gate until the stand is won, not on the radio: the route's clerk near its
stronghold leader (the Independent's: the Doctor at the lab gate), with a go-between and a
lost-and-found beside it. With Trees off they are the commit node; with Trees on the faction desks
take no names (the leaders do) and keep the rest, and the Independent's desk (the unsigned sheet at
the lab gate) takes names only while no Doctor or Courier is on the road.

| Choice | Does |
|---|---|
| **What would it mean?** (Trees off) | The lock card, tagged **[Point of no return]**: the route's admission, what you bring and what you leave. It arms the next choice for 30 s |
| **Write me down [no return]** (Trees off) | Locks you to the route, within 30 s of hearing it |
| **How do things stand?** | The finale page ([The gate](#the-gate)) |
| **The price door** (faction routes) / **The Coat Run** (Independent) | Below, and [The Coat Run](#take-the-lab-and-the-coat-run) |
| **Pact with ...** (the go-between, four choices) | Propose, agree or end a pact ([Pacts](#pacts-and-deals)) |
| **Give back what you found** (the lost-and-found) | Your items of this route go back to their seats; 10 Favor with its faction if you're not locked to it ("found it") |
| **Ransom it back** / **Spoil another route's find** (the lost-and-found) | [The moves](#the-moves) |
| **Buy the plan** (someone who sells plans, at the Red Hand's and the Independent's desks) | [The moves](#the-moves) |

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

**After the lock:** other routes' work refuses you (*"You've picked your side. This isn't it."*),
except the rivals' moves on it; ordinary errands and missions carry on; your route's sworn work is
told to you as it comes up; and the route's questline is in your journal whether you took part or not.

**A route with no active lock for 3 real days** (× the wait scale, counted from the gate): its sworn
stages pause and its subject's claim (her, the captain, the cook) is let go, claimed again when a lock
is active (log: *"Finale: Halcyon paused (no active lock for 3 real days)."*).

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
  with up to four choices. **The ways:** talk; persuade (the fixed score, `TryPersuade`: your access
  rank with the faction, +1 for a listed background, against 1–4; the majors' flags move it one
  step); threaten (Fear, `RZ_Checks.Threaten`: +3 Infamy or more whatever happens, once a season per
  person by anyone); trade (items handed over: food, fuel); bribe (Favor); sneak (the questline sneak:
  unseen, no shots); trick (passing in a faction's kit: the look, `PassAs`); force (a loud way with
  Infamy, or a hold that keeps a place for minutes while the noise brings the infected); betray (a
  bag or a vial kept back for nobody: Infamy, and the subject remembers); a hand-in. A route asset
  of yours eases one check once ([Route assets](#before-the-gate-pitches-and-route-assets)); a
  spoiled case makes the act's persuades one step harder ([The moves](#the-moves)).
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
| **I. The Crossed-Out Name** (open) | The chair (Known with the Haven); her name off the List (with `list.read`); a council head near her house (Known; read by the head: a Loyal one sends you to the chair unless Merciful, a Paranoid one gives a decoy below Trusted, an Independent one wants 3 food first); the Preacher (persuade 2, Elder +1); watch a house at dusk for the second bowl (unseen); feed her hungry house (3 food); tail the walker (unseen, with `walk.walks_stay`); knock as a house runner (a trick in Haven kit); the lab's Day 6 ledger (Known with Halcyon, or 20 Halcyon Favor); Bastion's Day 2 processing list (Trusted with Bastion, or 15 Bastion Favor); the strays crew (20 Red Hand Favor, or threaten them) |
| **II. And my boy** (sworn) | Her yes: persuade 3 on Haven rank (Nurse, Doctor or Elder +1; one easier with her name read at supper, one harder if she was sent to the far house). Then the door: pay his line (30 Red Hand Favor, Trusted; 15 under Paid at the Door), walk him out at night (unseen), threaten his crew boss (Fear 3, +5 Red Hand Infamy), mark his line paid at the fence's desk (a trick in Red Hand kit), or it's already clear (the book sent off loud, or a pact with the Red Hand). The other way: she gives without it, one step harder |
| **II. Three Times** (sworn, at dusk or after dark) | Draw a bag at the edge of her town (a cool bag, 15 minutes out of the cold, +10 with a Nurse or Doctor carrying; draws 12 real hours apart; five at most, never a sixth), store two at the lab. R1, her Day 2 tube, is one bag: traded at Bastion's medical store (40 Bastion Favor) or taken (unseen), once a season. **Betray:** keep a bag back for nobody (a bag for the Independent, +15 Haven Infamy, a betrayal she counts). "Two is enough" (Trusted with the Haven) spares her the dawn |
| **IV. Up the Hill** (sworn, last, at dawn) | Walk up the hill with her (or the Preacher rings her up, the courier road unseen, the roadblocks paid with 30 Red Hand Favor, Halcyon's gate with 20 Halcyon Favor) and bring her in at the gate; or draw at first light and run it up in 12 minutes. Spared: the lab's 2 real days of work on two bags |
| **Her branch** (she's dead or walked off) | *What She Left:* her notebook taken to the lab stands in for the door. *Twice:* his yes (persuade 2 on Haven or on Red Hand rank; with bread, 1 food, one easier) and up to six bags of his. Act IV is the lab's 2 real days on what she left |

Terms: **her house, and her boy**; **the far house first** (only under workers first); **by the bite
book**; **the crossed-out first** (with `list.read`). Nobody chooses: a Merciful or Idealist chair
reads the bite book, else her house first.

### Bastion: Held for Collection

On Day 2 the battalion's medical officer took a tube from everyone in Sedlak's column. "Held for
collection", he wrote, and took the freezers to the cold store in the hills (`held.store`), where he
still is. He opens to "collection", only the CO can say it, and a tube goes only with its slip.

| Stage | Ways through |
|---|---|
| **I. Request Instructions** (open) | Sedlak at the roster (Trusted); bribe the quartermaster (20 Bastion Favor); read the fuel log in a Bastion coat (a trick); the bus driver (Known with the Haven); the diesel line in the Red Hand's book (15 Red Hand Favor, or threaten the fence; sets `held.rh_knows`) |
| **II. Names for the tubes** (sworn) | The door: the driver's slips (persuade 2 on Haven rank, Police or Elder +1), the clipboard at night (unseen), threaten the driver, or the census (Counted, or a pact with the Haven). The other way: the CO signs a manifest from the four ledgers (Trusted; sets `held.named`), or the ration clerk's copies (2 food) |
| **II. Collection** (sworn) | At the cold store: collection on the CO's word (persuade 3 on Bastion rank, Soldier or Police +1; one easier under Bastion, Command, one harder under Expended); refuel the generator and ask (1 fuel, one step easier, Mechanic +1); a signal form (40 Bastion Favor); read collection off a signals pad (a trick); the box while he sleeps (unseen); or cut the door (+15 Bastion Infamy, one tube spoils). The box keeps 45 minutes out of the cold. Bringing Twice home (persuade 3 on Red Hand rank, or buy him out of his crew for 40 Red Hand Favor) adds his tube and sets `held.processed` |
| **Processing Again** (the branch: the captain is gone or the freezers were spoiled) | Tubes from the living: "Give blood" at a post (one each), Twice's tube (as above), then the medic boxes what there is. Fewer than three tubes is the long way at the lab |
| **IV. Signed For** (sworn, last) | Sign the box in at the lab. Fewer than three good tubes: the receipt goes out short and waits 2 real days a missing tube. The receipt is read on Bastion's call |

Terms: **the roster** (not if the box came by a form, sneaking or force); **the column**; **by need**
(Trusted); **the census** (only under Bastion, Command).

### The Red Hand: The Cook

The cook makes the dusk meal at the Red Hand's stronghold and, after supper, the cut dose on a farm
still (`redhand.still`). On a tray of the lab's batch, she says, it would hold every time. Every cut
dose a player takes is a line in her book (`cook.lines`, `cook.paid`).

| Stage | Ways through |
|---|---|
| **I. The Kitchen Book** (open) | Ask her at supper (Known); win her round (persuade 2, Ex-Con, Lab Tech, Doctor or Nurse +1); eat at her table in crew colours (a trick); follow her after supper (unseen; `cook.still_known`); her sister (3 food); Who Came Back: a paid line at a Bastion post (15 Bastion Favor). Hearing her sets `cook.heard` |
| **II. A Tray** (sworn) | The door, a tray: the director (Trusted with Halcyon), a page of her tally (a pact with Halcyon, or `proof.redhand`), Bastion's key (`authors.bastion_key`), a field lead (40 Red Hand Favor), Two Tins a Copy (2 food), the back door in a coat (a trick in Halcyon kit), robbing a Halcyon courier (+10 Halcyon Infamy), or the night store (unseen; Halcyon morale −5 for a stolen tray). One tray out per route; the lab has three and rebuilds one a real day. The tray keeps 15 minutes out of the cold. The other way: four stolen doses (30 Red Hand Favor), a harder base |
| **II. A Night on the Still** (sworn, 20:30–05:00) | Keep the still running 8 minutes while she cuts the pan, or a clean run: run it cold (unseen), feed it two jerrycans (2 fuel), or talk her through the cut (persuade 2, Lab Tech, Doctor, Nurse or Mechanic +1). The rival crew boss at midnight: cut him in (20 Red Hand Favor; `cook.crew_cut`) or threaten him. The case keeps without a cold clock |
| **IV. The Lab's Word** (sworn, last, by day) | The proof on someone bitten who asked, or on yourself (+10 Red Hand Fame): 60%, +15 for a clean run or the formula sold, −15 on the harder base; each failure is a step higher and the third pan always holds. Then the case in at the lab. Her own term: the boss marks her line paid (persuade 3; `cook.line_paid`) |
| **The Book Without Her** (the branch: the cook is gone, or the pan was spoiled) | Act I copies her tally off the kitchen wall; Act II runs the still by the tally at half speed (16 minutes) or cold by the tally (unseen) |

Terms: **the boss's price** (Trusted); **pay when you can** (someone heard her); **the first pan to
the houses** (Known with the Haven); **the book first**.

### Halcyon: The Lower Store

The reference stock, the first isolate, is in a dewar in the closed site's lower store
(`origin.site.lower`), behind the chain the cordon detail put on at 04:10 on Day 1. Two keys were cut.
"Proof" stays the major's: the route never re-decides the paper's first page.

| Stage | Ways through |
|---|---|
| **I. Reference: None** (open) | The duty officer (persuade 2, Lab Tech or Doctor +1); the director's stand-in for the vial map (persuade 3, 2 with `weeks.record`, or signed out in a Halcyon coat: a trick); the sergeant (asked, or threatened); the records room (unseen). Under The Whole of It the director simply says |
| **II. Item 114** (sworn) | The door: the adjutant (persuade 2 on Bastion rank, Soldier or Police +1), the quartermaster (40 Bastion Favor), the key board (unseen), a wax copy off the outpost commander's bootlace (unseen), Bastion's wrong key swapped on (a trick in Bastion kit), or a pact with Bastion / Under Seal. The other way: the sergeant's second key (Known with the Haven), a crew's lockpick (25 Red Hand Favor), or cut the chain (a 3-minute hold, +10 Bastion Infamy) |
| **II. Something Cold** (sworn) | The vet's tank (Known with the Haven, 25 Haven Favor, Trusted with Halcyon to requisition it, or taken at night unseen: a real day out of the cold; else the courier's case, 30 minutes), then the dewar: hold 4 minutes (under 3 with the map), crack its neck (a 1-minute hold, loud), or split the works' tank (3 minutes, loud). **Betray:** pocket a vial for nobody (a vial for the Independent, +10 Halcyon Infamy, once) |
| **IV. Samples First** (sworn, last) | Hand the vials in; the comparison takes 28 real hours (56 without Wolff's method), then a 4-minute hold while the centrifuge runs |
| **Second best** (the branch: the dewar was spoiled) | Wolff's one marked tube from the isolation room; the comparison takes 78 real hours |

Terms: **the authors first**; **the volunteers first**; **by the numbers**; **published**.

### Independent: One Each

On Day 6 ten bitten Haven people walked up the hill for the first trial dose, and eight walked out.
Whatever it taught their blood, they've carried it longest. A retired schoolteacher with a long-wire
aerial (the ham, `ham.house`) has copied every number the lab read into exercise books.

| Stage | Ways through |
|---|---|
| **I. Eight Walked Out** (open) | The ham's exercise books; the Day 6 ledger (Known with Halcyon, or 20 Halcyon Favor); the Doctor at the lab gate (persuade 1, Doctor or Nurse +1) |
| **II. Asking** (sworn, at dusk) | Each of the eight, by their own yes and their keeper's leave: a Haven house's council head (persuade 2 on Haven rank, Nurse +1), Sedlak (Known with Bastion), a crew boss (30 Red Hand Favor) or a field lead. Each gives one bag. When fewer than three are alive: Twice (his debt, 30 Red Hand Favor), the Scavenger (2 food, once), or donors at a Haven medic (the other way) |
| **II. Three bags** (sworn) | Store three bags at the lab, "for everyone". One unit from another route's subject counts as a bag: her bag, R1, a tube, a box, the case, a vial (handed over: the two routes are fighting) |
| **IV. For Everyone** (sworn, last, at a change of shift) | A tray off the books (a field lead, persuade 2 or 1 with `authors.open`; 30 Halcyon Favor; bought at the still for 30 Red Hand Favor; or stolen from the night store, unseen), then the batch sheet, "Released to: -": the director (persuade 3, easier under The Whole of It or with an Idealist or Merciful director), 40 Halcyon Favor, the records room at night (unseen), or a sit-in at the lab gate (unarmed, a 10-minute hold through the change of shift) |

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

### The moves

What a route's rivals can do to it. Moves on a route's scenes are on **the way round the back**, a spot
beside the subject or the source; only players locked to another route can take them (the label is
shared, the refusal comes when you pick). Every move makes the two routes fighting.

| Move | Where | What it does | Cost and limit |
|---|---|---|---|
| **Spoil the case** | Beside her, the captain or the cook | A true thing told about the route: that act's persuades one step harder | Once per subject per rival route |
| **Custody** | Beside her, the captain or the cook | The subject is walked to the mover's own people (a seat of their faction; the lab gate for the Independent). They give the holder nothing; it counts as a betrayal | +20 Infamy with the subject's faction. The route's people can talk them out of it (persuade 3 on the holder's rank, Elder or Police +1) |
| **Drain** | Beside the freezers, the pan, the dewar | The source spoils for the season (`finale.drained.<route + 1>`) and the route moves to its branch (Processing Again, The Book Without Her, second best) | Once; +5 Infamy with the route's faction, +15 if one of its locked players is within 100 m |
| **Ambush a carry** | Picking it up (built), then a desk's lost-and-found | Give it back ("found it"), keep it, **ransom** it at its route's desk (15 Favor an item with that faction), or **spoil** it at any desk (her bag: +10 Haven Infamy, and she gives again) | The finale's caps |
| **Sell the plan** | Someone who sells plans, at the Red Hand's desk (needs a Rogue Red Hand seat) and the Independent's | Take the Lab's timing (`finale.plan_sold`: the bay takes 150 s), or the Coat Run's night (`finale.coat_sold`: the Coat Run is called out) | Once per route a season; 30 Red Hand Favor or 20 Haven Favor |

**Killing a subject on-screen.** A scene's body for one of the finale's people (her, Twice, the
captain, the cook) that dies is `RZ_FinalePeople.Killed`: +20 Infamy with their faction for the
nearest player within 150 m, the stage is offered again, and the route moves to its branch.

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
| **What everyone sees** | Within 150 m of the lab, the stand's line on the HUD under the noise meter (`StandPanel`, sent every 2 s): *"CLEAN ROOM: BASTION. THE HAVEN TAKING: 0:35 OF 1:00."* Routes, never players. A player whose HUD can't take it gets it as a short notice when a count moves 15 s; everyone gets a notice when the room changes hands |

**Groups.** At most two friendly faction groups (**Groups at the lab**) stand 40–120 m out: the
finisher's faction first, then factions whose route has active locks, then those a major sends
([QUESTLINES.md](QUESTLINES.md#the-stands-inputs): the Haven at the gate after The Whole of It,
Bastion by its stance or HC3's terms, the Red Hand on the lab's road by its last page); never
Bastion under Expended. A group on the lab's road stands 150–250 m out. Halcyon never takes sides: its guard is the lab's own. Within 1.5 km of the lab a
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

**Finished, lost the room.** When the finisher's route doesn't control the cure, its faction's
morale drops 3, and everyone still actively locked to the route gets +10 Fame with its faction (log:
*"finished, lost the room"*).

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
| **A route's subject is lost** | She walks off after a third betrayal, or dies on-screen; the eight who died stay dead. The route moves to its branch: What She Left and Twice (the Haven), Processing Again (Bastion), The Book Without Her (the Red Hand), second best (Halcyon), Twice and the Scavenger (the Independent) ([The five routes](#the-five-routes)) |
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
3 the Red Hand, 4 Halcyon, 5 Independent; 0 nobody), `finale.finisher` (0 the lab alone; the stand
board's *"made from the Haven's work"* when it isn't the controller), `finale.how`, `finale.terms`,
`finale.partner` (the deal or force partner, route + 1; the stand board's two marks), `finale.came`
(how many routes counted toward the room at a stand, the holder's included: the stand board's
*"CAME FOR IT: {n}"*), the moves' (`finale.drained.<n>`, `finale.plan_sold`, `finale.coat_sold`,
`finale.r1`), and the subjects' (`held.named`, `held.processed`, `cook.line_paid`, `cook.heard`,
`cook.crew_cut`...).
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

## The rest of the finale

`Cure/RZ_FinaleRest.c`, owned by the controller and saved with it (`rest`).

**Before the gate.** From 25 to 50% research, about every 90 played minutes, one **seed** goes out
unmarked on a faction's open channel, never saying what it means: the bowl at the back door for a
cat the house hasn't got, the medic's *"We had one who didn't take"*, *"One did not present."*,
Bastion's nightly *"Diesel 20 L: stores."*, smoke past the still's town, *"Reference stock still
wanted."*, the vet's invoice. Before the gate the subjects aren't in the world, so nothing can force
them. After the gate the **lab's readout** goes out on Halcyon's open channel at each change of shift
(06:00, 14:00, 22:00) with a rumour on the Haven's of which route is close, a third of the time
wrong.

**The lock's rest.**
- **Hand to Hand** (errands' promises, [ERRANDS.md](ERRANDS.md)): a local you promised, in the same
  house as one of the eight, makes that person's yes one step easier (their keeper's rank one lower,
  the crew boss 20 Favor, the council head's persuade 1); each promise the Independent's locked
  players made takes a minute off the lab-gate sit-in (10 minutes, 4 at least). Promises never ease a
  stronghold leader.
- **The look's −1**: after the lock, every disguise look with a faction whose route you didn't choose
  (all four for the Independent) is one lower, tagged *[Sworn to another side -1]*, until the cure
  ([REPUTATION.md](REPUTATION.md#borrowed-kit) owns the look).
- **Route marks**: two verge places near the seat of every route with an active lock (an equal share,
  `finale.marks_public` 0, so they never show how many are locked); a route's locked players passing
  within 40 m are told once a real day (*"A strip of sheet tied to a fence post..."*). At the week's
  first dusk the losing routes' come down; the winner's stay.
- **The majors' pages**: each page in the open (`majors.pages_open`) makes the next persuade on a route
  scene one step easier, once.

**Notes and the night man.** Taking part in another route's scene gives you a note about it. Every
route's desk has the night man: *Sell what you know of a route* (15 Favor with the desk's faction,
once a route; half the time a lookout sees it: +10 Infamy with that route's faction, and the two
routes are fighting; a note older than two real days is found out: nothing, +5 Infamy), *Hand over
what you carry* (to a rival route's locked player within 10 m, logged in the item's chain; +15 Infamy
with your own route's faction), *Buy what a rival carries* (Last Mile's Buy It: 30 Favor with the
carrier's route faction, from a carrier within 30 m), *Sign a found item in for its route* (Sign It
In: 5 Fame; at the Independent's desk any route's item, Nobody's). The gate sheet takes **the deal at
the gate**: during a batch hold, one locked player from the holder's route and one from its partner's
put their names down within 200 m of the lab (`finale.gate_deal`); the automatic deal at 7 or more
stays as the fallback.

**The moves' rest.**
- **Drain at a container**: a rival route's locked player beside a route item lying on the ground (within
  6 m, not on it: the pickup is 3 m) for 20 s spoils it, once a container; seen by another route's
  player within 40 m: +5 Infamy with its faction, +15 if the watcher is that route's own.
- **Guard the door**: once a season a route's locked player moves its door (*Guard the door*, on the
  door's scene): after that a rival's talk there needs a pact, and a sneak or a trick fails half the time.
- **Keep It Overnight**: an item carried in the cold at dawn gets an hour of its cold clock back, once.
- **Kills**: a subject killed on-screen is blamed on the killer read from the kill, else the nearest
  player within 150 m.

**Each route's remaining ways** (extra people and choices on its scenes):

| Route | Ways |
|---|---|
| The Haven | Her four terms one by one at her table (never up that hill; nobody first for guns; her name read at supper, Haven morale −5; her boy), the last of them her yes, unarmed (*"Twenty metres off. The gun."*); *Move her tonight* (the Haven's route, once a real day); the hungry house next door sells tonight's spot (2 food, 1 under the Far House; the buyer's route and the Haven fight); the annex's draw with the vet's centrifuge (25 Red Hand Favor), the quiet draw, *Ask her for one more*; *Drive her up in the cab*; *Turn her over on the last mile* (+20 Haven Infamy, her betrayal) |
| Bastion | The four ledgers one by one (three agree: through the door); *Name nobody at all* (persuade 2, the other way); under All Stations the box by need (Known, persuade 1); Act IV's carries: the stage truck, the LZ (`pilot.stays`), on foot quietly, or the box handed in at a checkpoint (up to 10 Bastion Infamy off through `Forgive()`); once the terms are written under Bastion, the 20:30 call reads the release (*"... Minus nineteen. Receipt held. Over."*) |
| The Red Hand | Who Came Back in full: a Haven cook's line and the lab's (10 Favor each; both: Act I done); a made-up answer (+10 Red Hand Infamy); *Tell the boss who's asking* (Known); the Mechanic's coil at the still (persuade 2, a Mechanic one easier: the night on the still done); Halcyon copying the cook's notes (`cook.lab_copy`) |
| Halcyon | Under Expended a Rogue commander sells item 114 (30 Favor), under Under Seal 114 at Known; the canisters' riddle (read the bracelet's date, open the canister with the time; a canister at random loses a vial); the Haven trading silence about the Day 3 page (`ref.haven_silence`); Act IV's ways down (by night, a decoy, the Haven's road, the family at dusk `ref.haven_holds`, the paper's transport case under The Whole of It) |
| Independent | The carry on air (the Doctor reads *"Released to: nobody"* on every open channel) or quietly; the duty officer's vouch (persuade 2) |

**Live tissue's rest.** A pack gives tissue only within reach of the lab (5 km by road; the straight
line × 1.3 stands in) and never twice in 6 season days; a great one's leader is +3, not +2; the
unsided deliverer's tissue goes to the route that still needs it they stand best with. **Fury's road**
is the tribes' (a Furious pack's trails run the capture road toward the lab and come with the stand's
last wave: [INFECTED.md](INFECTED.md)).

**The stand's rest.** The clean room is a child entity named `RZ_CleanRoom` on the lab prefab and the
loading bay `RZ_LoadingBay` (VERIFY: named children come back by `GetName()`), else land points;
unconscious players don't count in the room (VERIFY: `IsUnconscious()` on the server); **Contest Auto**
reads player damage from play: once a player has killed another this season (`finale.pvp_seen`) it plays
as Force, until then as Tally. Under Bastion, Command (and for the Red Hand's carriers when Bastion has
the book, `book.last` 5) **Bastion's checkpoint** stands on the lab's main road 200 m out: a rival
route's carrier within 30 m is searched (passing as Bastion: waved through; else a 90 s wait while
the cold clock runs; leaving early: +10 Bastion Infamy). Questlines hear **the room changing holder**
(`RZ_Questline.OnStandHolder`). The journal's finale page is the desk's *How do things stand?*; the
lock card is a screen when **Dialogue > Trees** is on (the commit node), a notice when it's off.

**The majors' remaining inputs** are read where each check is made: the page ease (above); the Haven's
hungry house under the Far House; Bastion's box by need under All Stations; 114 under Expended and Under
Seal; the LZ under `pilot.stays`; the transport case under The Whole of It; Bastion's checkpoint for the
cook under `book.last` 5; her name stays the Haven's under `list.buried` (the epilogue's channel line
leaves it out). Bastion's last call reads the captain's R numbers when the cure is Bastion's
([QUESTLINES.md](QUESTLINES.md#await-relief)).

**For the epilogue** ([EPILOGUE.md](EPILOGUE.md)): the routes' closing lines on the codas, the losing
routes' marks down at the first dusk, the outlets on `finale.terms`.

## Simplified, and not built yet

*Simplified:* the commit node needs **Dialogue > Trees** (off by default): with Trees off the desks
take the names; the Independent commits at the Doctor or the Courier on the road, or on the open
channel from any radio that can call ([DIALOGUE.md](DIALOGUE.md#the-finales-commit)); `CanSideWith` is the
one rule (unless Hunted, owed, or the price door); assets come from the
story records listed above and the jobs that tag them by hand, and an Ally doesn't open doors or carry
items; the drain at a source is immediate (the container's is 20 s); the route's other ways are choices
on its scenes, with no escort carries, no announced carry every 10 minutes, no Bottles, no volunteer by
kind and no field lead replaced; the lone tissue hunt doesn't keep the 6-day rule and the specials
count after 12 hours like the guards; Halcyon's price door takes no blood draw; the Red Hand's line
takes what Favor you have; Bastion's fuel clock is the refuel way only; the decide scene has no
5-second objection hold (the release scene's first pick, 3 minutes).

**Risks to watch in play-tests** are the roadmap's ([ROADMAP.md](ROADMAP.md#play-test-priorities)).
