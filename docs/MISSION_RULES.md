# Mission rules

The rules every task follows: missions, contracts, errands, duties, the openings, questline stages
and events. Most come from play-testing. Each rule is one line here, with **where it lives** (the doc
that explains it in full) and **how it's kept** (the check or the code that holds it). Use the
checklist at the end before calling a task done.

Status: **held** means code or a check enforces it; **partly** means it's held in most places, with
the gaps named; **open** means it's agreed but not yet enforced everywhere.

## The place

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 1 | **A place is what its words say.** "The chemist's" is a real pharmacy, "the farm" a farm, "the hall" a hall. If there's no such building, the words change ("the old aid post", "my old medical cabinet") or the task isn't offered there. | [PLACES.md](PLACES.md#a-place-is-what-its-words-say) | `RZ_PlaceCheck.WordsTrue`; `run.sh tasks` fails a place whose words are false (`Places words:` lines) | held |
| 2 | **What the words name stands there.** A box at "take the box", crates at "carry the crates", a patient at "treat the boy". If it can't be put out, it falls back to a plainer thing, never to nothing. | [ERRANDS.md](ERRANDS.md#work-sites) | the work sites' looks and their fallbacks | partly: missing crates at The Boiler and the Nurse's missing boy are being fixed |
| 3 | **Real doors only.** Anything "at a door" happens on a real building's doorstep. The mod never spawns a door. | [ERRANDS.md](ERRANDS.md#work-sites) | the locks' door finder | partly: a free-standing door seen in play is being removed |
| 4 | **"In" means inside.** A search, a stack or a patient "in" a building is inside it, on its ground floor. | [PLACES.md](PLACES.md#a-place-is-what-its-words-say) | `RZ_WorkSites.FloorOf` | held |
| 5 | **A line stands at a real way in,** with a path to it: the camp's gate, else the road into town, else the giver's door. | [ERRANDS.md](ERRANDS.md#duties) | `AT_ENTRY` beats, logged | held |
| 6 | **Nothing stands in a wall, a building or the road.** Props use their measured sizes, and a prop that doesn't fit is dropped, not squeezed. | [SITES.md](SITES.md#where-a-place-stands) | `footprints.json`, the fit checks, the site audit | held |
| 7 | **Within walking reach.** A giver offers jobs within 2.5 km and errands within 1.5 km. A fixed far place is handed out by a nearer giver of the same faction, and failing that, it says it's a long walk. | [PLACES.md](PLACES.md#offer-reach) | **Offer reach**, **Errand reach**; `Reach jobs` lines in `run.sh tasks` | held |
| 8 | **Mainland only.** No task sends you to an island, into water or onto the line. | [PLACES.md](PLACES.md) | the place needs; `run.sh tasks` | held |

## The people

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 9 | **The person named is there,** in the world, and the marker sits on their body, not on a stored spot. | [UI.md](UI.md#the-quest-tracker) | `RZ_Errands.GiverAt`; runtime check `RZT_PersonObj` | partly: questline stages still point at the stage's place |
| 10 | **Name, body and role agree.** A man isn't "a patient's mother"; a "Dr" is a doctor. | [DESIGN.md](DESIGN.md#peoples-looks) | role picked first, then name | partly: openings' contacts are being fixed |
| 11 | **People aren't identical.** A line, a ward or a crew wear different clothes. | [DESIGN.md](DESIGN.md#peoples-looks) | **Duty person looks**, the looks lists | held |
| 12 | **People stand where they live,** inside their own fence or camp. | [SITES.md](SITES.md#the-people) | camp life's spots | open: one Halcyon person at Morton stands outside the fence |

## Doing it

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 13 | **One spot, one prompt.** Several things to do at one place are one prompt, named for the spot, that opens a menu of choices. | [UI.md](UI.md#the-object-menu) | `RZ_WorkSites.Gather` | held |
| 14 | **Carrying is carrying.** "Carry the crates in" means picking real crates up and putting them down where they go, not holding a button. | [ERRANDS.md](ERRANDS.md#work-sites) | the lift-and-put-down of the sorting duties | open: being built for every carry task |
| 15 | **Games are played in the world,** with your hands on things that stand there, never as a menu. A menu may be one prop of the game (a clipboard, a ledger). | [UI.md](UI.md#default-first-then-menus) | the interaction audit in UI.md | held |
| 16 | **A hold looks like work.** Holding the key plays an animation and keeps you still, like digging in vanilla. | [ERRANDS.md](ERRANDS.md#work-sites) | `RZ_WorkHold` | held |
| 17 | **The rules are where you need them,** in the tracker and the spot's menu, so you never walk back to a note to reread it. | [ERRANDS.md](ERRANDS.md#duties) | the sorting duties' tracker lines | held |
| 18 | **Default first.** Vanilla's own inventory, looting, healing, doors, cars and radios before anything of ours. | [UI.md](UI.md#default-first-then-menus) | the interaction audit | held |

## Talk and hand-overs

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 19 | **Handing over is a reply you pick.** Nothing completes when you walk up, or when you open the talk screen. | [ERRANDS.md](ERRANDS.md#handed-errands-the-openings-chain) | the [Errand] reply, locked with what's missing | partly: an opening that completed on walking back is being fixed |
| 20 | **The next step is told in words,** by the giver, the next person or the one walking with you. | [DESIGN.md](DESIGN.md#openings-how-every-life-starts) | the openings' chain | partly: same fix as 19 |
| 21 | **Ask again and they repeat it.** A giver tells you your current task when asked. | [DIALOGUE.md](DIALOGUE.md#jobs-on-the-first-replies) | the talk replies | held |
| 22 | **No pitch for work that isn't there.** A giver with nothing says so. | [DIALOGUE.md](DIALOGUE.md#jobs-on-the-first-replies) | the talk replies | held |

## Objectives and the tracker

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 23 | **One objective per action and place.** "Take the box, carry it to the far door, hand it over, sign" are four lines, each at its own place. | [WRITING.md](WRITING.md#objective-lines) | `RZ_DutyGoals`; the objective audit | partly: about 15 errands, 5 deliveries and 6 questline stages still pair two places in one line |
| 24 | **Taking a job tracks it.** The job you agree to in a talk is on the tracker at once. | [UI.md](UI.md#the-quest-tracker) | `RZ_QuestTracker.Push` | held |
| 25 | **Errands and jobs are separate.** A job or the opening's chain never blocks an errand. | [ERRANDS.md](ERRANDS.md) | `CountHeld`, `HasOpenOffer`, `ChoresAt` | held |
| 26 | **A journey, never a return trip.** An opening's steps go on from where you are; nothing sends you back just to report. | [ERRANDS.md](ERRANDS.md#handed-errands-the-openings-chain) | the openings' chain | held |

## Everywhere

| # | Rule | Where it lives | How it's kept | Status |
|---|---|---|---|---|
| 27 | **Gamepad and TV.** Every prompt, menu and step works with a pad, without hover or typing, and reads on a TV. | [UI.md](UI.md#controllers) | the controllers checklist | held |
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
