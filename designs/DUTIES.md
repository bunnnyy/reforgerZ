# Duties

**Designed, not built.** What's built is in the docs and not repeated here: work from the first
day ([docs/MISSIONS.md](../docs/MISSIONS.md#who-gets-offered-what)), the duty rules, the trust
ladder, the duties of rungs 1 to 3, the sorting duties played at their place and the duties'
people ([docs/ERRANDS.md](../docs/ERRANDS.md#duties)). This file holds what waits. Each commit
moves what it builds into those docs and cuts it from here.

## The author's rules for every duty

From 2026-11-13, in the author's words:

1. *"Make the games preferably in person. Rather than a menu. This goes for anything that might be
   a game. Menu can be created as part of game."* (Built as a rule:
   [docs/UI.md](../docs/UI.md#default-first-then-menus).)
2. *"Patrols, and others etc. something interesting should happen or see something or whatever.
   Doesn't just have to be shooting."* Every patrol, hunt, watch and walk-out has something to see,
   find or decide, drawn from the world's real state, most of it not a fight, with a choice where
   one fits (report it, keep it quiet, help, take), remembered by the people it concerns. A find
   reported well is how trust is earned; a find pocketed is how it's lost, if seen. The other
   duties get small turns too (the kitchen runs short, the well rope snaps).
3. *"Always have duties offer alongside. But yes, the more pressing takes over as priority."*
4. *"Start building trusted too."*
5. *"Adjust about 60% of these to be like the observation ward in a way, where you have to solve
   and figure things out and you really feel like you're leading this errand/job."* For at least
   twelve of the twenty below (and the three sorting duties), the heart is working something out
   from what you find in person (signs, marks, counts that don't match, what people say against
   what you see), a conclusion that can be right or wrong, and consequences either way. And you
   lead: the people with you do what you decide. The other 40% stay simple, physical and quick, so
   a newcomer isn't examined on day one.

Keep everything inside the caps and guardrails. A longer or extended run pays a little more. Every
duty counts toward the trust ladder at its rung. Where a routine, card or mission already is the
duty, extend that instead of adding a twin.

## Rung 4, what waits

Built: the lock-up (the Haven, Bastion, the Red Hand), the Book of Names, the casualty return, the
trial list and a cull (the Haven, Bastion). Waiting:

| Duty | Waits for |
|---|---|
| The Red Hand's debt book, Halcyon's authors' list (the names) | Written the way the casualty return is: evidence at a place, a word that follows someone |
| Who gets a dose: the house's one dose, the sergeant's list, who the crew pays for | Doses that exist outside the lab. It must never hand a dose to the player, and what it writes needs the finale's rules read first |
| Halcyon's isolation (the lock-up), the animal house (a cull) | The observation ward and its people (below) |
| Bastion's lock-up dressed with the blindfold and JTF Bronze Props' `Detainee` scene | The props, slotted on the PC |

## The twenty approved duties (round 3)

Approved by the author with the changes below. Lowest rungs first, each complete, in person, with
things that happen, one commit per faction. **W** = work it out and lead; **S** = simple, physical
and quick.

### The Haven

Built: the school room, the garden, the door round, the roll call and the swap table
([docs/ERRANDS.md](../docs/ERRANDS.md#the-havens-duties)). Waiting on the swap table:

- **Its conversations as `.rzd` trees.** The author asked for real conversations
  (`dialogue/**/*.rzd`, many short ones with their own people, wants and tells). As built, each
  trader is a beat: what they say, three things to handle and ask, three calls. The talk engine has
  no speaker for a duty's customer; giving it one (a speaker kind, its effects back into the scene)
  is its own piece of work.
- **The things on the table as real items.** The loot tables' and Suspicious Item Props' things
  (cash, the gold bar, the necklace, the cards, a dead man's ID) are pick-up items, so they can't
  stand as a work site's look without being lootable. They need prop copies with nothing to use.

### Bastion

Built: gate duty, stag at the wire (the watch), the loudspeaker tape, kit inspection and the field
telephone ([docs/ERRANDS.md](../docs/ERRANDS.md#bastions-duties)). Waiting: the forged passports and
soldier's IDs of Suspicious Item Props as things in the hand at the gate and in a kit (they need our
own child prefabs with era-right names first); the follow-on where a missing rifle is bought back at
a Red Hand toll (today it's reported, and the routine Fell Off the Inventory is the buying back).

### The Red Hand

Built: stripping a wreck, the still, the count, the toll and spotter
([docs/ERRANDS.md](../docs/ERRANDS.md#the-red-hands-duties)). Waiting: the toll weighted by feuds,
truces and who holds the road (today: the stage, the hour, the weather, the town's sickness); the
marked goods as real things on the tarp and in the toll's box (they're pick-up items: they need
prop copies); spotter's board changing what the crew does (today it's recorded, rumoured and kept
as a saved counter that nothing reads yet); a night watch by sound.

### Halcyon

| Duty | Kind | What it is |
|---|---|---|
| The sample round | W | Swabs from marked spots round a town, in the suit. Find the source from the readings |
| The decon line | W | See below |
| The observation ward | W | See below |
| The incinerator | W | See below |
| The cold chain run | W | See below |

- **The decon line.** *"More varieties on what can happen, this is super good."* **15 kinds of
  arrival and 25 faults**, combined per run. Arrivals (who, from where, in what state, how they
  take being checked): a sampling team from a hot town, a body-bag detail, a Bastion visitor who
  won't give up his kit, a recruit doing it wrong, a child brought for testing, the director, and
  more. Faults (what's wrong, how it shows when you look in person, the right call, what a wrong
  call costs): a torn suit, a cracked visor, a splash on a boot, a leaking sample case, a filter
  past its hours, a bite under the suit, something smuggled under it (a pet, a letter, vials), and
  more. Your stations are worked by hand (spray, brush, rinse, the lamp, the seal tug, the log) and
  can run short (disinfectant, water); you can pass, re-run, send to the room, report or cover. A
  bad pass has a cost later; too strict has one too.
- **The observation ward.** *"More things you can observe, more intricate diagnosis, etc. more
  options of what to do."* Signs taken in person at each bed (temperature, pulse, eyes to a torch,
  the skin and the bite's edge, breath, speech, appetite, tremor, temper, sleep, the chart's
  history at the bed's foot), a diagnosis worked out from them (a plain fever, each stage of the
  infection, the trial dose's side-effects, someone faking to get out or to get the dose, fear),
  and several things to do (fluids, cooling, restraints, a sedative, isolation, call the doctor,
  tell the subject the truth, let family in, write the chart true or the way the researcher wants).
  It feeds the research where it's done well, and a missed turn is a turned subject in the ward.
- **The incinerator.** *"More varieties again I like it."* Sealed bags to the burn pit. What's
  really in a bag varies against its label (clinical waste, suits, a whole body nobody logged,
  files someone wants gone: read, burn, keep or pass on; live samples sent to burn by mistake; a bag
  that moves; things stripped from the dead; ammunition that mustn't burn; wet bags that won't
  light without fuel), and around it (the log doesn't match the bag count, someone asks for one
  more off the books, the smoke draws questions). The log against the bags.
- **The cold chain run.** *"More varieties of items, and things that can happen."* Cargo between
  station and lab. It varies (trial vials, blood samples, a culture that must stay level, doses,
  sealed results, a heavy cold box) with its own rule each (time, no running, upright, dark,
  unopened), and things happen (the power out at the far end, a checkpoint wants it opened, a toll
  wants the box, someone begs for one dose, the cold pack fails and you need a cellar, a well or a
  stream, a fall, nobody there to receive it, the count one short at the hand-over). Your plan when
  it goes wrong.

### The numbers, and the items to work in

- *"Those are perfect. And for decon line, I meant that's how many variations. But you can make
  more variations for sure to make it interesting. 5 and 5 was good for like how many per run of
  that actual errand. And the toll however many interactions feel good for the 5-15 minute rule."*
  So: the decon line's pool is at least 15 arrivals and 25 faults, and one run draws about 5
  arrivals with about 5 faults among them. The toll's pool is about 50 interactions, and one shift
  has as many as fit a 5 to 15 minute duty.
- **Suspicious Item Props** is a dependency (merge `claude/sleepy-volta-nwm433` before building with
  it). Plain pick-up items under `PrefabsEditable/Auto/Props/Civilian/Suspicious Items/`:
  `{696CC1F6BC5BE3F7}E_StackofCash.et`, `{7B605E8E4B952837}E_GoldBrick.et`,
  `{0CF6E65BB0AAFDE6}E_PearlNecklace.et`, `{D0476EFD1BB2CCB3}E_CreditCards.et` (these four are
  already rare loot); `{1F1E24829D04D94D}E_USPassport.et`, `{9FF272641E7F2C53}E_UKPassport.et`,
  `{405F6A7443646EF8}E_RUPassport.et` (forged passports); `{479A3C8B3C95D643}E_USMilitaryID.et`,
  `{A66D9268D93A8861}E_RUMilitaryID.et` (soldier's IDs); `{9C85B00E7EBE4AA5}E_DisassembledGlock.et`
  (a pistol in parts); `{ABE8D94854706E1A}E_Wiring.et` (a spool of copper wire);
  `{EDA4F5D74A1C12F1}E_OxycodonePills.et` (a large pill bottle); `{55476D9B2E3F4C63}E_CocaineBrick.et`,
  `{AAB632A193BF6B69}E_WeedBrick.et` (drug bricks); `{B6A80DB9A683D7E1}E_Blindfold.et` (a bandana
  blindfold worn as a hat). Never `E_Smartphone.et`.
- Known problems: the passports, both IDs, the wiring and the drug bricks name an inventory icon
  the mod doesn't ship; the RU passport and ID say "Russian Federation", and the pills name a 1996
  brand, so their names are wrong for 1989. To use the papers, IDs or pills, make our own child
  prefabs under `Prefabs/ReforgerZ/` that inherit theirs and set an era-right name and description
  (as `RZ_Water_Dirty.et` does), written as text with a `.meta`, and list them for the PC to open.
- Where they fit: forged papers and IDs among the real ones at gate duty and in kit inspection's
  contraband; the marked goods (gold, necklace, cards, cash) in the count and at the toll; a dead
  man's ID and the valuables at the swap table and the roll call; the pill bottle in the ward and
  the incinerator's bags; the blindfold for Bastion's lock-up on the Trusted rung. JTF Bronze Props
  (a dependency) has `Detainee` and `InterrogationRoom` scene groups: `tools/camp_kit/world_kit.txt`
  lists every usable scene and prop by where it suits, for dressing any duty's place.
- Two errand or card ideas from the props work, to build if they fit cleanly: a soldier's ID
  brought back from a patrol that didn't return; a forger selling false hope at the line.

## Order of work

1. **The twenty**, one commit per faction, lowest rungs first.
2. **Rung 4's versions that wait** (above).

Rules 1 to 3 are built, with their engines (the sorting game, the scenes of beats, the squads):
[docs/ERRANDS.md](../docs/ERRANDS.md#things-that-happen). The duties below are written on them.

## What it must not do

- No new reputation system, no pay outside the ledger's caps, no loot or doses handed out beyond
  what a duty names (the still's vodka, a snare's meat).
- No bodies or props away from players; a squad only while its player is near (the escort block's
  rules and cap).
- Nothing a pad can't do, nothing that needs reading at speed.
- A duty never writes a leader's memory or vote, like every errand.
