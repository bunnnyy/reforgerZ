# Mission rules

The rules every task follows: missions, contracts, errands, duties, the openings, questline stages
and events. Most come from play-testing. Each rule is one line here, with **where it lives** (the doc
that explains it in full) and **how it's kept** (the check or the code that holds it). Use the
checklist at the end before calling a task done.

Status: **held** means code or a check enforces it; **partly** means it's held in most places, with
the gaps named; **open** means it's agreed but not yet enforced everywhere.

Two checks keep them. **`tools/mission_rules.py`** reads the source (seconds; run it before every
commit that adds or changes a task: [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#mission-rules-check)):
rules 1, 3, 9, 10, 14, 19, 23, 24, 26 and 27. **The runtime test** checks what only a running server
shows (`RZT rules` and `RZT person objectives` lines): rules 2, 9 and 19. What neither can see is
for the play-test. [The audit](#the-audit) below is the last full pass.

## The place

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 1 | **A place is what its words say.** "The chemist's" is a real pharmacy, "the farm" a farm, "the hall" a hall. If there's no such building, the words change ("the old aid post", "my old medical cabinet") or the task isn't offered there. | [PLACES.md](PLACES.md#a-place-is-what-its-words-say) | `RZ_PlaceCheck.WordsTrue`; `run.sh tasks` fails a place whose words are false (`Places words:` lines); `mission_rules.py` (a building named in a file that never asks for one) | held |
| 2 | **What the words name stands there.** A box at "take the box", crates at "carry the crates", a patient at "treat the boy". If it can't be put out, it falls back to a plainer thing, never to nothing. | [ERRANDS.md](ERRANDS.md#work-sites) | the work sites' looks and their fallbacks; runtime `RZT rules FAIL thing` | held: a work site always shows the thing its words name, else a box, logged; the Nurse's son and the Doctor's bitten friend lie there (runtime `RZT openings`, `RZT rules thing`) |
| 3 | **Real doors only.** Anything "at a door" happens on a real building's doorstep. The mod never spawns a door. | [ERRANDS.md](ERRANDS.md#work-sites) | the locks' door finder; `mission_rules.py` (a prefab built on a base-game door) | held: no door is spawned; the boarded doorway goes up only in a real doorway (a trap's barricade within 3 m of one, a work look within 1.2 m) |
| 4 | **"In" means inside.** A search, a stack or a patient "in" a building is inside it, on its ground floor. | [PLACES.md](PLACES.md#a-place-is-what-its-words-say) | `RZ_WorkSites.FloorOf` | held |
| 5 | **A line stands at a real way in,** with a path to it: the camp's gate, else the road into town, else the giver's door. | [ERRANDS.md](ERRANDS.md#duties) | `AT_ENTRY` beats, logged | held |
| 6 | **Nothing stands in a wall, a building or the road.** Props use their measured sizes, and a prop that doesn't fit is dropped, not squeezed. | [SITES.md](SITES.md#where-a-place-stands) | `footprints.json`, the fit checks, the site audit | held |
| 7 | **Within walking reach.** A giver offers jobs within 2.5 km and errands within 1.5 km. A fixed far place is handed out by a nearer giver of the same faction, and failing that, it says it's a long walk. | [PLACES.md](PLACES.md#offer-reach) | **Offer reach**, **Errand reach**; `Reach jobs` lines in `run.sh tasks` | held |
| 8 | **Mainland only.** No task sends you to an island, into water or onto the line. The one exception is the perimeter's own jobs (The Blind Spot, The Crossing, Shot at the Line, the Last Pilot's wreck), whose place is the line by design: still dry and on the mainland (`mission.line`). | [PLACES.md](PLACES.md) | the place needs; the live check; `run.sh tasks` | held |

## The people

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 9 | **The person named is there,** in the world, and the marker sits on their body, not on a stored spot. | [UI.md](UI.md#the-quest-tracker) | `RZ_Errands.GiverAt`; runtime check `RZT_PersonObj`; `mission_rules.py` (a marker on a stored spot) | held: a questline stage's person stands still at the stage's place, and an escort's place follows them |
| 10 | **Every person in the world is a man** (the stack has only male bodies); women may be mentioned, never met. A name, the words for them (he, father, brother, Mr) and the role agree: a "Dr" is a doctor. | [DESIGN.md](DESIGN.md#peoples-looks), [WRITING.md](WRITING.md) | `RZ_NoteText.ManName` (and `FirstName`, the same list) for every name; role picked first, then name; an older save's women's names mapped to men's as they load (`RZ_NoteText.ManFromOld`); `mission_rules.py` (a woman's name or word in a shown line; the allowlist names each woman only mentioned) | held |
| 11 | **People aren't identical.** A line, a ward or a crew wear different clothes. | [DESIGN.md](DESIGN.md#peoples-looks) | **Duty person looks**, the looks lists | held |
| 12 | **People stand where they live,** inside their own fence or camp. | [SITES.md](SITES.md#the-people) | camp life's spots | open: one Halcyon person at Morton stands outside the fence |

## Doing it

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 13 | **One spot, one prompt.** Several things to do at one place are one prompt, named for the spot, that opens a menu of choices. | [UI.md](UI.md#the-object-menu) | `RZ_WorkSites.Gather` | held |
| 14 | **Carrying is carrying.** "Carry the crates in" means picking real crates up and putting them down where they go, not holding a button. | [ERRANDS.md](ERRANDS.md#work-sites) | `RZ_Carry` (real things picked up and put down); `mission_rules.py` (a hold whose words say carry, stack or load, unless its way is carried for real) | held |
| 15 | **Games are played in the world,** with your hands on things that stand there, never as a menu. A menu may be one prop of the game (a clipboard, a ledger). | [UI.md](UI.md#default-first-then-menus) | the interaction audit in UI.md | held |
| 16 | **A hold looks like work.** Holding the key plays an animation and keeps you still, like digging in vanilla. | [ERRANDS.md](ERRANDS.md#work-sites) | `RZ_WorkHold` | held |
| 17 | **The rules are where you need them,** in the tracker and the spot's menu, so you never walk back to a note to reread it. | [ERRANDS.md](ERRANDS.md#duties) | the sorting duties' tracker lines | held |
| 18 | **Default first.** Vanilla's own inventory, looting, healing, doors, cars and radios before anything of ours. | [UI.md](UI.md#default-first-then-menus) | the interaction audit | held |

## Talk and hand-overs

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 19 | **Handing over is a reply you pick.** Nothing completes when you walk up, or when you open the talk screen. | [ERRANDS.md](ERRANDS.md#handed-errands-the-openings-chain) | the [Errand] reply, locked with what's missing; `mission_rules.py` (a distance check to a person that completes); runtime `RZT rules FAIL walk-up` | held: every errand, handed ones too, hands in by its reply while the talk screen is on; a handed step ends where it's done and a hand-over counts only by its reply (runtime `RZT openings`); walking back hands in only with the talk screen off |
| 20 | **The next step is told in words,** by the giver, the next person or the one walking with you. | [DESIGN.md](DESIGN.md#openings-how-every-life-starts) | the openings' chain | held |
| 21 | **Ask again and they repeat it.** A giver tells you your current task when asked. | [DIALOGUE.md](DIALOGUE.md#jobs-on-the-first-replies) | the talk replies | held |
| 22 | **No pitch for work that isn't there.** A giver with nothing says so. | [DIALOGUE.md](DIALOGUE.md#jobs-on-the-first-replies) | the talk replies | held |

## Objectives and the tracker

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 23 | **One objective per action and place.** "Take the box, carry it to the far door, hand it over, sign" are four lines, each at its own place. | [WRITING.md](WRITING.md#objective-lines) | `RZ_DutyGoals`; `ObjThen` (missions), `RZ_ErrandWay.First` (errands); `mission_rules.py` | held |
| 24 | **Taking a job tracks it.** The job you agree to in a talk is on the tracker at once. | [UI.md](UI.md#the-quest-tracker) | `RZ_QuestTracker.Push`; `mission_rules.py` (a kind of job the tracker never lists) | held |
| 25 | **Errands and jobs are separate.** A job or the opening's chain never blocks an errand. | [ERRANDS.md](ERRANDS.md) | `CountHeld`, `HasOpenOffer`, `ChoresAt` | held |
| 26 | **A journey, never a return trip.** An opening's steps go on from where you are; nothing sends you back just to report. | [ERRANDS.md](ERRANDS.md#handed-errands-the-openings-chain) | the openings' chain; `mission_rules.py` (an opening's step that only sends you back) | held |

## Everywhere

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 27 | **Gamepad and TV.** Every prompt, menu and step works with a pad, without hover or typing, and reads on a TV. | [UI.md](UI.md#controllers) | the controllers checklist; `mission_rules.py` (an input action without a pad binding) | held |
| 28 | **Words follow the voice.** Objective lines, briefs and replies follow the writing rules. | [WRITING.md](WRITING.md) | `voice_lint.py` | held |

## Checklist for a new or changed task

- [ ] Its words are true of the place it's given (`run.sh tasks` shows no `Places words:` line).
- [ ] Everything its words name stands there: the thing, the person, the door (a real one).
- [ ] Every prop and person stands clear of walls, inside where it says "in", at the way in for a line.
- [ ] It's offered within walking reach, or says it's a long walk.
- [ ] One prompt per spot; carrying is carrying; a game is played in the world.
- [ ] Hand-overs are replies; the next step is said.
- [ ] One objective line per action and place; the marker is on the person when it's a person.
- [ ] It's tracked when taken, and doesn't block or get blocked by an errand.
- [ ] It works with a pad and reads on a TV; `voice_lint.py` passes.
- [ ] `python3 tools/mission_rules.py --strict` passes (a hit that's right goes in `tools/mission_rules_allow.txt` with its reason).

## The audit

The last full pass over every task, 2026-11-14 (`mission_rules.py` and a read of each family). Hits
before the fixes, then after: 60 hits before (rule 1: 5, 3: 2, 10: 10, 14: 6, 19: 2, 23: 35);
after, none open: 15 right as they are (in the allowlist with why) and 16 pending the work named. Those
16 were closed the same day once that work was in: none pending now.

| Family | Rules broken | What happened |
|---|---|---|
| Deliveries and contracts (Weapons Cache, Requisition, Research Data, Specimen from the Nest, Missing Scout, Rescue the Pilot, Secure the Shipment, Live Capture, The Informant, Bust Out, Bring Them Home, Private Collection) | 23 | Fixed: two lines each (`ObjThen`): find or free or trap it, then bring it; the second becomes current when it's carried, walking or caged |
| Puzzles (Patient Zero's ledger, the trail) | 23 | Right as they are: a puzzle's thinking has no place; the trail is a step chain whose steps are the lines |
| Errand cards that "sell it at the fence" (Notebook 31 twice, The Name on the Tape, His Effects, The Patrol That Didn't Come Back) | 23, 1 | Fixed: the sale was never a step (the Red Hand's Favor comes with the hand-in), so the words now say it's taken *for* the fence, at one place |
| Errands at two places (the ring, twice; Petr's key) | 23 | Fixed: a first place of their own (`RZ_ErrandWay.First`): search the grass for the ring, then the mother's door; the key from under the step, then Petr in the queue |
| The Boy (two ways) | 23, 2 | Fixed: the step is done with the boy where he is; the words now say you talk him round, not walk him somewhere nothing checks |
| The snare line (duty) | 23 | Fixed: "then bring what's caught to {giver}", so the way back is its own line |
| Questline stages (Authors: the Red Hand's offer; The Long Table: the dead) | 23 | Fixed: two lines each (`ObjThen`): hear the offer, then the director; stand where they fell, then the host |
| The Price of the Door | 1 | Fixed: "a barn outside {barn}" is a real farm building at that town's edge, and he sleeps inside it; a town without one isn't picked |
| The finale's Red Hand route, act 4 | 23 | Fixed: two lines (`ObjThen` on the route's scene): the proof dose to the volunteer (the marker on him), then the cook's case to the lab gate once the dose has held (the run's `proved` flag) |
| People's names (holding people, wanderers, the priest in a mystery, every generated name) | 10 | Fixed: a man's name for everyone (`RZ_NoteText.FirstName` is `ManName` now) |
| The Haven chair's clerk (The Cargo) | 10 | Fixed: a man at the table |
| Written women who stand in the world: the openings' mothers, the swap table's people, the sister at the door (Halcyon), the finale's lodger, cook, cook's sister and vet, The List's wife, the Red Hand storekeeper and daughter, the Long Table's mother, Dr Wolff, the field lead, the director, the toll's passers, the strangers, the chair, nurses and medics | 10 | Fixed: all men (the author: "make it all male"); the lodger is "the man who didn't turn", the Long Table's mother is its father (Adam Moravec), The List's wife is the missing man's brother; 130 lines name women only mentioned (a dead wife, a voice), listed in the allowlist |
| The Boiler's crates, Margin Proof's chairs | 14, 2 | Fixed: carried for real (`RZ_Carry`); Margin Proof's words now say you carry three out, as you do |
| The openings' scratched door, the boarded doorway (a trap's barricade, the BOARDS work look) | 3 | Fixed: the scratched door is gone (the clue is on a real door); the boards go up only in a real doorway: a trap's barricade within 3 m of one, snapped into it, a work look within 1.2 m |
| Handed errands (an opening's step) | 19 | Fixed: `RZ_Errands` no longer leaves handed errands out of the reply rule; every handed step ends where it's done (`Hand` makes sure, with a warning), and a hand-over (The Cut) counts only by its reply, even with no opening behind it. With the talk screen off, walking back stays the hand-in |
| The perimeter's own jobs (The Blind Spot, The Crossing, Shot at the Line, the Last Pilot's wreck) | 8 | Right as designed: their place is the line; checked against `mission.line` (dry, mainland, the line allowed), so the live check no longer fails them for it |
| Crash Site, Last Helicopter (an island in the full runtime test) | 8 | Fixed: the wreck and the LZ are tried again a few towns over until they're on the mainland and off the line |
| Halcyon's station at Morton: a named person's body outside the fence | 12 | Open: a place fault for the PC to report where they should stand (the board) |
| Everything else (events, leaders, arcs, the Long Table, checkpoints, supply, the epilogue's cards, the duties' scenes) | none found | The checks found nothing; rules 2, 13 and 16 there are for the play-test |
