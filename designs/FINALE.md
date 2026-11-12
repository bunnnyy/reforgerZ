# The cure finale: what's left

**Status: designed, not built.** Approved by the author on 2026-11-11. Phase 6 is built in part:
the controller, the lock and its dialogue commit node, pitches and route assets, the five routes with
their ways and branches, live tissue, pacts and the moves between routes, the stand as a fight for the
clean room with its HUD line, Take the Lab, the Coat Run, terms, the route Fame and the fallbacks are in
[docs/FINALE.md](../docs/FINALE.md). This file holds only what's still to build, with the numbers and
lines the author approved. The plan is [PLAN.md](PLAN.md#the-build-order).

## Contents

- [Waiting on other systems](#waiting-on-other-systems)
- [Before the gate](#before-the-gate)
- [The lock's other pieces](#the-locks-other-pieces)
- [Each route's remaining ways](#each-routes-remaining-ways)
- [Live tissue's remaining rules](#live-tissues-remaining-rules)
- [The remaining moves and notes](#the-remaining-moves-and-notes)
- [The stand's remaining pieces](#the-stands-remaining-pieces)
- [The majors' remaining inputs](#the-majors-remaining-inputs)
- [What the finale hands the epilogue](#what-the-finale-hands-the-epilogue)
- [Risks](#risks)

## Waiting on other systems

| Needs | From | Until then |
|---|---|---|
| Promises (Hand to Hand, lock cards) | Errands | Not built |
| The word overlays (the look's −1 at the lock is built: [docs/REPUTATION.md](../docs/REPUTATION.md#borrowed-kit)) | Disguise | No overlays |
| `CanSideWith` as reputation's own | Reputation | The finale's own (built) |
| `Forgive()` for Hand it in at a checkpoint | Reputation | Not built |
| `SetPresenceFloor` | Majors | No floor |
| `RZ_QuestVote` for pacts | Majors | The controller's own tally (built) |
| The decide-scene hold | Missions | The release scene's first pick, 3 minutes (built) |
| Radio speakers (the Independent commits on the open channel) | Dialogue | The Doctor or the Courier on the road (built) |
| Tagging more outcomes with route assets, and refusing a locked player an option whose asset is another route's | Missions ([docs/DESIGN.md](../docs/DESIGN.md#story-blocks)) | The story records listed in docs/FINALE.md give assets (built) |

**Hand to Hand** (errands, up to 6 promises a player): a promised local in the same house as one of the
eight makes that person's yes one step easier; each promised local at the Independent's lab-gate
sit-in takes a minute off it. Promises never ease a stronghold leader.

## Before the gate

**Seeds** from 25–50% research, unmarked, about one line in fifteen of its kind: the house that puts
a bowl out for a cat it hasn't got; the medic's *"We had one who didn't take"*; the readout's *"One
did not present."*; the store's line on the nightly call with the temperature creeping up; *"Diesel 20
L: stores (HFC)"* in post ledgers; Day 2 slips in safehouse drawers (*"NR-2 / 147. {name}, Bus 1.
Retain this slip and present it at the barrier."*); smoke at night past the still's town; *"Reference
stock still wanted"*; the vet's invoice. Before the gate every subject's place refuses force (*"It's
Bastion's chain. Somebody signed for it."*).

**The lab's readout** at each change of shift on the open channel (the built readout is in the
journal and at the desks), and rumours of who is close, some of them wrong.

**An Ally asset's other uses:** opening a door, or carrying a route item up to 600 m (an escort block
that ends at a place).

## The lock's other pieces

- **After the lock:** an option that leans toward another route refuses you; face-to-face route
  lines, cool on the first visit and back to normal over two real days; route marks on the verge, an
  equal share per route with an active lock (`finale.marks_public` 0); settling up closed with
  fighting routes (parley is built).
- **Notes about other routes** stay yours; selling one is crossing that faction (+10 Infamy if a
  lookout sees, and its leader remembers).

## Each route's remaining ways

**The Haven.** Her four terms taken one by one (not up that hill, guns and the bite book, her name read
at supper for Haven morale −5, her boy); tonight's spot (the house or an outbuilding, never the same
two nights, cold at the next dusk; speak to her by day or dusk, weapons left 20 m off); a hungry house
selling the spot (25% every 10 minutes while short); a Haven-locked player moving her once a real day;
the draw needs somewhere (the annex or a hall with a generator, a pharmacy fridge and the vet's
centrifuge for 25 Red Hand Favor) and someone (a house medic, the Doctor, a field lead, a Nurse, Doctor
or Lab Tech player); the 4-minute hold or the quiet draw; "One more"; a truck's cab through the
roadblocks; betraying her on the last mile.

**Bastion.** The fuel clock in full (the last diesel run at 05:30, 40–60 hours of play, a tube spoils
every 8 dry hours; refuelling +10 hours and the captain's goodwill); the 20:30 call reading the
release; the bus driver as a face at the door (terms locked to *the column*); the four ledgers one by
one, Patient Zero's Ledger's deduction, and choosing who's told her name; "No names at all" by stance;
the Red Hand's Cold Meat raid, the Haven's sit-in, Halcyon's courier, guarding the clipboard. Act IV's
carries: the stage truck and road posts (up to 2), the helicopter LZ, on foot quietly, the Haven's road,
the crews' roads, handing the box in at a checkpoint through `Forgive()`; the box at a Red Hand drop
for a real day and 8 hours. Processing Again's other sources: her draw, the Scavenger unarmed with food
twice, and the long way's Power the Lab rule; *bring him home* in full (the Deserter, the crew's book,
the roll's *"Here"*).

**The Red Hand.** Who Came Back in full (three paid lines at other factions; a false answer +10 Red Hand
Infamy); Bastion stopping her on the road; telling the boss someone's asking; the Mechanic's coil;
Bastion's raid, Halcyon watching (`cook.lab_copy`), the Haven fetching water, sabotage; the volunteer by
kind (a crewman with his boss's say, a Haven member, a Bastion soldier, a lab volunteer, Twice); the
case's carry announced every 10 minutes and its ways (escort, front door, lorry, sneaking, buying it off
the crew boss); her leverage over the boss. Her other branches: taken by another route (a Bust Out, an
escort); *The Bottles* (five paid lines, two nights, the proof from 45%).

**Halcyon.** The canisters' riddle (the bracelet and the date; a wrong canister costs a vial); a vial a
minute (40 s for a Lab Tech) up to the container's size; the Haven trading silence about the Day 3
page. Act IV: the Red Hand's line when it leaves the site, rough circles for rivals, the ways down
(drive, fly, walk by night, the Haven's road, Secure the Shipment, the crews' roads, decoys), the family
at dusk (`ref.haven_holds`), the rival contracts on it; the comparison's play-hours part, Power the Lab
taking a quarter off and samples an hour each; second best refusable by a Haven player, and its three
specials' samples.

**Independent.** The carry on air or quiet (the Doctor's broadcast and the duty officer's vouch); the
field lead running the sheet and being replaced; Halcyon closing the ledger; Bastion pulling its
attached one back.

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

## The remaining moves and notes

| Move | What it does | Cost and limit |
|---|---|---|
| **Drain at a container** | 20 s at a route item on the ground (the built drain is immediate, at a source) | Once per container; seen +5 / +15 Infamy |
| **Guard the door** | The door's route moves it once (the clipboard, 114, the trays, her boy's line) | Sneak and trick one step harder, talk needs a pact |
| **Hand it over** | An item put down and picked up by a rival within 10 s, logged | +15 Infamy with the giver's route faction |
| **Last Mile's contracts** | Take It, Drain It, Search It, Keep It Overnight, Buy It, Sign It In, Nobody's | As the contracts |

**Notes** as carried note items, sold to any faction's leader for 15 Favor; false notes found out by
events. **The deal at the gate:** one locked player from each route agrees at the lab gate during the
batch hold (the built deal is automatic for a partner at 7 or more). A subject killed on-screen is
blamed on the killer read from the kill (today the nearest player).

## The stand's remaining pieces

- The clean room as a named child entity on the lab prefab, and the loading bay as a second.
- Reading unconsciousness, and the server's player-damage setting for **Contest** Auto.
- The journal's finale page and the lock card as a screen.
- Bastion's checkpoint on the lab's main road under Bastion, Command (a rival's carry through it is
  searched: a look, or a 90 s wait while its clock runs).
- The decide scene's 5-second hold with one objection a player and Stand with (missions' hold).
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

The flags are written ([docs/FINALE.md](../docs/FINALE.md#endings-and-next-season), `finale.came`
included). Still to come, with the epilogue ([EPILOGUE.md](EPILOGUE.md)): the records of the stand's
holders with their seconds, batches taken and stands lost on the record snapshot; `RouteAtLock` per
identity; the routes' lines on their carriers (the Haven's evening channel line *"If you can hear this:
her name is {unturned_name}..."*, Bastion's receipt call and *"Minus nineteen"*, the cook's *"Soup. It
keeps."*, the readout's *"Reference stock: wanted."*, the chalk *"WE TRIED."*), the branch lines, the
losing routes' marks coming down at the first dusk, the dose outlets reading `finale.terms`, grudges and
hunter squads standing down once at the cure (*"It's over. Talk."*), and tone's variants for every
ending.

## Risks

| Risk | What to watch |
|---|---|
| **PvP inside the stand** | Tally keeps it working with player damage off, but the take time, the 10 m room, overtime and the body budget need play-tests at 5 and 20 players |
| **Small servers and time zones** | One active lock per route or none; Seal hour exists and the fallback guarantees a cure, but a 3-player server may mostly see uncontested or slow finishes; routes with no active lock pause after 3 days |
| **The ring of doors as a stall** | A door-holding route can stall a rival; the other ways' costs need tuning in play-tests |
| **The moves as griefing** | Custody and the drain move a route to its branch; watch how often they're used and whether the branches feel like a punishment |
| **Asymmetry** | The Independent has no group at the stand and Halcyon's lab gives its route none: watch lock counts by route |
| **The Fame chunk may feel hollow** | About 375 at the design point is a model: calibrate **Route worth** from the status log and `last_season.json` |
| **Pacing** | An uncontested route takes about 4–5 real days; contested routes on a one-tribe map can run long. The fallback is about 17 days |
| **Save size** | Locks, tracks, items, people, assets, pacts and the stand's counts: measure `state.json` after a season |
