# The cure finale: what's left

**Status: designed, not built.** Approved by the author on 2026-11-11. Phase 6 is built in part:
the controller, the lock, the five routes' acts, live tissue, pacts, the stand as a fight for the
clean room, Take the Lab, the Coat Run, terms, the route Fame and the fallbacks are in
[docs/FINALE.md](../docs/FINALE.md). This file holds only what's still to build, with the numbers
and lines the author approved. The author's decisions it follows (2–9 and 15) are applied in the
built part; the plan is [PLAN.md](PLAN.md#the-build-order).

## Contents

- [Before the gate](#before-the-gate)
- [The lock's other pieces](#the-locks-other-pieces)
- [Each route's remaining ways](#each-routes-remaining-ways)
- [The subjects' branches](#the-subjects-branches)
- [Live tissue's remaining rules](#live-tissues-remaining-rules)
- [The moves and notes](#the-moves-and-notes)
- [The stand's remaining pieces](#the-stands-remaining-pieces)
- [The majors' remaining inputs](#the-majors-remaining-inputs)
- [What the finale hands the epilogue](#what-the-finale-hands-the-epilogue)
- [Engine needs still open](#engine-needs-still-open)
- [Risks](#risks)

## Before the gate

**Route assets** (the missions design's "leans"): at most one per mission outcome, at most 3 per
player per route a season, shown on the finale page as "What you'd bring". An asset does nothing until
its owner locks to that route; then it's a card played once:

| Asset | In that route |
|---|---|
| Ally | Someone the route can call on once: opens a door, vouches at one check (one step easier), or carries a route item up to 600 m (an escort block that ends at a place) |
| Proof | One talk check one step easier, or one search or deduction skipped |
| Standing | One leader's price a third lower, or one leader's check one step easier |

Assets for other routes go dormant at the lock and lapse at the cure. No asset can close a route.

**Hand to Hand** (errands, up to 6 promises a player): a promised local in the same house as one of the
eight makes that person's yes one step easier; each promised local at the Independent's lab-gate
sit-in takes a minute off it. Promises never ease a stronghold leader.

**Pitches.** From 75%, route topics answer *"Ask me again when it's nearly done."* From 80%, "Ask
about the cure" works at the chair, the CO, the boss and the director (the Independent: the Doctor
wanderer, else the Courier, else an unsigned sheet at the lab gate). At 85% each pitch goes on air
once, naming nobody. Each gives its best argument, then its admission.

**Seeds** from 25–50% research, unmarked, about one line in fifteen of its kind: the house that puts
a bowl out for a cat it hasn't got; the medic's *"We had one who didn't take"*; the readout's *"One
did not present."*; the store's line on the nightly call with the temperature creeping up; *"Diesel 20
L: stores (HFC)"* in post ledgers; Day 2 slips in safehouse drawers (*"NR-2 / 147. {name}, Bus 1.
Retain this slip and present it at the barrier."*); smoke at night past the still's town; *"Reference
stock still wanted"*; the vet's invoice. Before the gate every subject's place refuses force (*"It's
Bastion's chain. Somebody signed for it."*).

**The lab's readout** at each change of shift on the open channel (the built readout is in the
journal and at the desks), and rumours of who is close, some of them wrong.

## The lock's other pieces

- **The commit node in dialogue:** an `always` topic at the faction's stronghold leader or any Loyal
  seat leader, a node tagged **[Point of no return]** with the three lines (the desks are its
  fallback). The Independent commits on the open channel at any powered house radio.
- **CanSideWith** as reputation's own (the finale has its own today).
- **After the lock:** looks −1 with every faction whose route you didn't choose (all four for the
  Independent); an option that leans toward another route refuses you; your word's overlay
  ("attached", "the other crew", "an uncontrolled variable", "nobody's"); face-to-face route lines,
  cool on the first visit and back to normal over two real days; route marks on the verge, an equal
  share per route with an active lock (`finale.marks_public` 0); settling up closed with fighting
  routes (parley is built).
- **Lock cards** in errands' deck and the Hand to Hand lines.
- **A route with no active lock for 3 real days:** its stage contracts pause and its claims (her
  house, the still, the store) are let go, claimed again when a lock is active.
- **Notes about other routes** stay yours; selling one is crossing that faction (+10 Infamy if a
  lookout sees, and its leader remembers).

## Each route's remaining ways

**The Haven.** Act I: council heads by trait (Merciful tell anyone Known, Loyal send you to the chair,
Paranoid give a decoy below Trusted, Independent want a job first); the Preacher; the walker with
`walk.walks_stay`; Bastion's Day 2 processing list (Trusted, or 15 Bastion Favor); tailing the walker;
20 Red Hand Favor to the strays crew; tonight's spot (the house or an outbuilding, never the same two
nights, cold at the next dusk; speak to her by day or dusk, weapons left 20 m off); a hungry house
selling the spot (25% every 10 minutes while short); a Haven-locked player moving her once a real day.
Act II: her four terms one by one (not up that hill, guns and the bite book, her name read at supper
for Haven morale −5, her boy); threaten his crew boss; the fence's desk trick; the draw needs somewhere
(the annex or a hall with a generator, a pharmacy fridge and the vet's centrifuge for 25 Red Hand
Favor) and someone (a house medic, the Doctor, a field lead, a Nurse, Doctor or Lab Tech player); the
4-minute hold or the quiet draw; R1, her Day 2 tube, as one bag (traded, 40 Bastion Favor, or taken);
"One more"; betraying a bag (+15 Haven Infamy). Act IV: the Preacher's bells; sneaking the courier
road; a truck's cab through the roadblocks; bribes (30 Red Hand, 20 Halcyon); betraying her on the last
mile.

**Bastion.** The fuel clock (the last diesel run at 05:30, 40–60 hours of play, a tube spoils every 8
dry hours; refuelling +10 hours and the captain's goodwill); the 20:30 call reading the release; the
bus driver as a face at the door (terms locked to *the column*); threaten and trick ways; sneaking
with item 113 or a pick; the four ledgers one by one, Patient Zero's Ledger's deduction, and choosing
who's told her name; "No names at all" by stance; the Red Hand's Cold Meat raid, the drain, the Haven's
sit-in, Halcyon's courier, guarding the clipboard. Act IV's carries: the stage truck and road posts
(up to 2), the helicopter LZ, on foot quietly, the Haven's road, the crews' roads, handing the box in at
a checkpoint through `Forgive()`; the box at a Red Hand drop for a real day and 8 hours. Processing
Again's sources: the living reactive (her draw, Twice by the crew's term or 40 Red Hand Favor, the
Scavenger unarmed with food twice), donors (a "Give blood" action, D a post) and the long way's Power
the Lab rule. The presence floor (`SetPresenceFloor(Bastion, 0.25)`) while it has an active lock.

**The Red Hand.** Who Came Back (three paid lines at other factions; a false answer +10 Red Hand
Infamy); Bastion stopping her on the road; telling the boss someone's asking; copying the tally; the
tray by Two Tins a Copy, by `authors.bastion_key`, by tricking the back door in a coat, by robbing a
courier; the still's 2 jerrycans and 6 boxes of antiviral; the Mechanic's coil, the reader's
thermometer, talking her through it; the rival crew boss at midnight (and `cook.crew_cut`'s swap at
the proof); Bastion's raid, Halcyon watching (`cook.lab_copy`), the Haven fetching water, sabotage;
the volunteer by kind (a crewman with his boss's say, a Haven member, a Bastion soldier, a lab
volunteer, Twice, yourself +10 Red Hand Fame); the case's carry announced every 10 minutes and its
ways (escort, front door, lorry, sneaking, buying it off the crew boss); her leverage over the boss.

**Halcyon.** The canisters' riddle (the bracelet and the date; a wrong canister costs a vial); a vial
a minute (40 s for a Lab Tech) up to the container's size; the works' flask (4 vials, 3 minutes of
loud valves) and splitting the tank; the vet's tank by sneaking or requisition; Bastion's wrong key
(item 113); the outpost commander's bootlace and its wax copy; the Red Hand's ex-con picking the chain;
the Haven trading silence about the Day 3 page; explosives cracking the dewar's neck; pocketing a vial.
Act IV: the Red Hand's line when it leaves the site, rough circles for rivals, the ways down (drive,
fly, walk by night, the Haven's road, Secure the Shipment, the crews' roads, decoys), the family at
dusk (`ref.haven_holds`), the rival contracts on it; the comparison's play-hours part, Power the Lab
taking a quarter off and samples an hour each.

**Independent.** One unit from another route's subject (a draw from her, R3's tube, one dose from the
cook, one vial); the carry on air or quiet (the Doctor's broadcast and the duty officer's vouch); the
sit-in at the lab gate through the 17:30 change of shift (10 minutes unarmed, a minute off per
promised local); a tray bought at the still (a deal); stealing a tray; the field lead running the
sheet and being replaced; Halcyon closing the ledger; Bastion pulling its attached one back.

## The subjects' branches

Never a dead end, never killed off-screen:
- **The Haven:** *What She Left* (her notebook, 1 point every 2 real days) and *Twice* (his yes on
  Haven or Red Hand rank, bread one step easier, harder in Bastion's uniform; up to six bags). Remake:
  her next draw or her tube.
- **Bastion:** *Processing Again* as above; *bring him home* in full (the Deserter, the crew's book, the
  roll's *"Here"*).
- **The Red Hand:** taken by another route (a Bust Out, an escort, talking her out); *The Book Without
  Her* (the tally, reading the cut, a paid line's blood, a player running the still at half speed);
  *The Bottles* (five paid lines, two nights, the proof from 45%); the still moved to the lorry.
- **Halcyon:** *Second best* (Wolff in the isolation room, refusable by a Haven player; one marked tube;
  the long way with three specials' samples and 3 real days plus 6 hours of play).
- **Independent:** the eight's fallback by Twice, the Scavenger or donors.
- **Killing a subject on-screen** (a scene's body): +20 Infamy with their faction and the route moves
  to its branch (`RZ_FinalePeople.Killed` is there; the scenes don't report a death yet).

## Live tissue's remaining rules

- **Best first within reach** (5 km of the lab by road; straight line × 1.3 as a fallback): a leader at
  its lair by day past the scouts or on its trail at night; a great one +3; two guards (+2 after 12 real
  hours); two specials from Live Capture (+2 after a real day); two special samples and a real day.
- **Scarce:** the same tribe never gives tissue twice within 6 season days.
- **The unsided deliverer chooses** the route at the hand-in.
- **Fury's road:** for 2 counted nights the Furious tribe's trails and its day party walk the capture
  road toward the lab, and it comes with the stand's last wave.
- **Each route's own way:** the Haven's house naming who it was and lending a second wake reset;
  Bastion's squad with a cage and the contact board; the Red Hand's moat sold by its crew (the moat
  opens); Halcyon's tagged index and Track Them Home; the Independent's borrowed, bought or built cage.
  The wake clock's resets at a field station (twice under No Vehicles), none by helicopter.

## The moves and notes

| Move | What it does | Cost and limit |
|---|---|---|
| **Ambush a carry** | Give it back, keep it, spoil it or ransom it (picking up and giving back are built) | Spoiling one of her bags: +10 Haven Infamy, she gives again |
| **Drain** | 20 s at a container on the ground or at a source: its contents spoil; a source is gone for the season | Once per container or source; seen +5 / +15 Infamy |
| **Spoil the case** | Tell a subject a true thing about a rival route: that rival's persuade one step harder for the act | Once per subject per route |
| **Guard the door** | The door's route moves it once (the clipboard, 114, the trays, her boy's line) | Sneak and trick one step harder, talk needs a pact |
| **Custody** | A group walks a subject to an outpost, camp or field station | +20 Infamy with the subject's faction; they give the holder nothing |
| **Sell the plan** | A route's Rogue seat leader sells Take the Lab's timing (`finale.plan_sold`: the bay 150 s) or the Coat Run's night | Once per route a season |
| **Hand it over** | An item put down and picked up by a rival within 10 s, logged | +15 Infamy with the giver's route faction |
| **Last Mile's contracts** | Take It, Drain It, Search It, Keep It Overnight, Buy It, Sign It In, Nobody's | As the contracts |

**Notes** as carried note items, sold to any faction's leader for 15 Favor; false notes found out by
events. **The deal at the gate:** one locked player from each route agrees at the lab gate during the
batch hold (the built deal is automatic for a partner at 7 or more).

## The stand's remaining pieces

- The clean room as a named child entity on the lab prefab, and the loading bay as a second.
- Reading unconsciousness, and the server's player-damage setting for **Contest** Auto.
- The stand's HUD line under the noise meter (UI.md, LAYOUTS.md), the journal's finale page and the
  lock card as a screen.
- Bastion's checkpoint on the lab's main road under Bastion, Command (a rival's carry through it is
  searched: a look, or a 90 s wait while its clock runs).
- The decide scene's 5-second hold with one objection a player and Stand with (missions' hold).
- The stand board's *"CAME FOR IT: {n}"*.
- Halcyon's safe-zone rule for fire *at Halcyon's people* by locked players during the stand.

## The majors' remaining inputs

Each page in the open (`majors.pages_open`) makes one finale check one step easier, once. Not yet
read: `list.buried` (her name stays the Haven's), `haven.rule` 2's house known and more readily sold,
All Stations' *by need* at Known and the captain one step easier, Expended's Rogue commander selling
114, `book.last` 5's Bastion stopping the cook on the road, The Whole of It lending the transport
case, Under Seal's 114 at Known, `pilot.stays` and the helicopter, HC1's Wolff and notebook for the
comparison and second best. **Changes to the majors' text:** HC3's ending reading the finale (Under
Bastion's guard, Open doors, The Red Hand's batch, The lab alone); the codas' clauses (the captain
and the four R numbers at Bastion's walk, her name at the Haven's gate).

## What the finale hands the epilogue

The flags are written ([docs/FINALE.md](../docs/FINALE.md#endings-and-next-season)). Still to come, with
the epilogue ([EPILOGUE.md](EPILOGUE.md)): the records of the stand's holders with their seconds,
batches taken and stands lost on the record snapshot; `RouteAtLock` per identity; the routes' lines
on their carriers (the Haven's evening channel line *"If you can hear this: her name is
{unturned_name}..."*, Bastion's receipt call and *"Minus nineteen"*, the cook's *"Soup. It keeps."*,
the readout's *"Reference stock: wanted."*, the chalk *"WE TRIED."*), the branch lines, the losing
routes' marks coming down at the first dusk, the dose outlets reading `finale.terms`, grudges and
hunter squads standing down once at the cure (*"It's over. Talk."*), and tone's variants for every
ending.

## Engine needs still open

| Needs | From | Until then |
|---|---|---|
| The commit node, `always` topics, pact topics | Dialogue | The desks (built) |
| `CanSideWith` | Reputation | The finale's own (built) |
| `Asset()` | Missions | No assets |
| The decide-scene hold | Missions | The release scene's first pick, 3 minutes (built) |
| `RZ_QuestVote` for pacts | Majors | The controller's own tally (built) |
| The look's −1 at the lock | Disguise | Looks unchanged |
| `Forgive()` for Hand it in | Reputation | Not built |
| `SetPresenceFloor` | Majors | No floor |
| `Person(key)` deaths from scene bodies | Questline engine | Subjects die only by betrayal |

## Risks

| Risk | What to watch |
|---|---|
| **PvP inside the stand** | Tally keeps it working with player damage off, but the take time, the 10 m room, overtime and the body budget need play-tests at 5 and 20 players |
| **Small servers and time zones** | One active lock per route or none; Seal hour exists and the fallback guarantees a cure, but a 3-player server may mostly see uncontested or slow finishes |
| **The ring of doors as a stall** | A door-holding route can stall a rival; the other ways' costs need tuning in play-tests |
| **Asymmetry** | The Independent has no group at the stand and Halcyon's lab gives its route none: watch lock counts by route |
| **The Fame chunk may feel hollow** | About 375 at the design point is a model: calibrate **Route worth** from the status log and `last_season.json` |
| **Pacing** | An uncontested route takes about 4–5 real days; contested routes on a one-tribe map can run long. The fallback is about 17 days |
| **Save size** | Locks, tracks, items, people, pacts and the stand's counts: measure `state.json` after a season |
