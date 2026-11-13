# Duties

**Designed, not built.** The author's ask of 2026-11-13: work from the first day, low-rung jobs
that build trust, taking over a place's duties (patrols, hunts with their people, lab work,
sanitising arrivals, the morgue, the kitchen), repeatable and varied, true to the world's state,
and more sensitive the more you're trusted. Each phase below moves what it builds into
[docs/ERRANDS.md](../docs/ERRANDS.md) and [docs/MISSIONS.md](../docs/MISSIONS.md) and is cut from
this file.

## Why day one was empty

Not a lock. Three rules starve a newcomer:

| Rule | Where | Effect |
|---|---|---|
| Contracts are one shared pool | `RZ_MissionManager.Tick`: `2 + players / 3` contracts alive on the whole server, one made at random every 75 s | A solo player has two contracts in the world, across four factions. The giver they ask usually has none |
| A contract must be one you qualify for | `JobsAt`, `IsEligible` | Each faction has four contracts a Stranger can take and a dozen they can't |
| Errands exist only while something is true | `RZ_Errands.Deal`, the faults at 0.25 a shift | A place where nothing went wrong today has nothing to ask |

## The rules

1. **Ask and you get.** Anyone who gives work has something the moment they're asked. A leader or
   quartermaster with no open contract for you makes one of their own then (one you qualify for,
   that the world can run now). A named person with nothing true to ask gives a **duty**.
2. **The only no is a stated one**: a prerequisite quest, standing not yet earned, your hands full,
   a fight at the fence, Hunted. The locked row says which.
3. **Duties are the place's own work**, taken over for a shift: 5 to 15 minutes, done with your
   hands, never a timer you wait on. They repeat, on a short gap, and no two runs are the same.
4. **A trust ladder**, on the tiers that exist. Each rung's work is more dangerous, pays more and
   matters more to the place. Duties are how a newcomer climbs.
5. **True to the world.** Each run is drawn from what's so now: the outbreak's stage, the town's
   infection and loot, the faction's need, who died today, the weather, the hour, a pack's ground,
   the cure's progress. The ask says why this one, now.
6. **Nothing for free.** Duties pay through the errands' small-pay ledger: Favor inside 40 an hour
   with diminishing returns, Fame never above 39 and 10 a faction a season day. A duty can make
   you Known. Trusted comes from contracts, which rule 1 now puts in reach.

## The ladder

| Rung | Who | The work | Pays |
|---|---|---|---|
| 1 | Anyone not Watched or Hunted | Chores, watched: the kitchen, water and wood, the latrine, washing down, carrying | 3–4 Favor |
| 2 | They know your face: three duties done for that faction this season, or Known | A watch, the line at the gate, the morgue, a patrol leg with their people | 5–6 Favor, 1 Fame |
| 3 | Known | Armed work with their people: a full patrol, a hunt, the bench, the stores | 7–8 Favor, 1–2 Fame |
| 4 | Trusted | The sensitive ones: the list of the dead, who gets a dose, the lock-up, a night post alone, a cull | 9–10 Favor, 2 Fame, and a record |

The Watched get the amends duties that exist (Make It Right, the Latrine, Clean Slate).

## The duties

● = built in the first build (phase 2). The rest wait here.

### Every faction, in its own words

| Duty | Rung | The Haven (a house) | Bastion (a post) | The Red Hand (a camp) | Halcyon (a station, the lab) |
|---|---|---|---|---|---|
| ● **Kitchen** | 1 | Kitchen Duty: the cook's pot | Mess Duty: the field kitchen | The Pot and the Still | Canteen: forty trays |
| ● **The line** | 2 | The Line at the Door | The Wash-Down | The Look-Over | Intake |
| ● **The dead** | 2 | Laying Out | Burial Detail | The Pit's Leavings | The Cold Room |
| ● **A patrol** | 2 (a leg), 3 (armed, further) | The Walk Round | Patrol Leg | The Round | The Sample Walk |
| ● **A hunt** | 3 | For the Pot | — | The Meat Run | — |
| ● **The bench** | 3 | — | — | — | Bench Work |
| Wood and water | 1 | split kindling, carry cans | the water bowser | the still's wood | the boiler |
| Washing down | 1 | the yard, the bedding | the vehicles, the wire | the lorry | the suits |
| A watch | 2 | the roof at dusk | the wire, a night hour | the toll, a quiet hour | the gate log |
| Stock-taking | 3 | the larder against the book | the armoury count | the cut, counted twice | the cold store |
| The names | 4 | The Book of Names: who is told, and how | The casualty return | The debt book | The authors' list |
| Who gets a dose | 4 | the house's one dose | the sergeant's list | who the crew pays for | The Trial List |
| The lock-up | 4 | the one at the gate overnight | The Stockade Watch | the debtor in the shed | isolation |
| A cull | 4 | the old man's dogs | a pack's strays at the wire | — | the animal house |

### How each plays

- **Kitchen.** At the place's kitchen: peel, stir and serve at the table (a hold), or put what
  you carry into the pot: raw meat, tins, a fuel can for the stove. The cook says what the pot
  lacks and why: the truck didn't come, there are two more mouths since Tuesday, it's a burying
  day. A fed place stays fed a little longer (+1 food, capped a day); meat counts double.
- **The line** (the table, below). Arrivals wait at the gate: a family from an infested town, a
  crew lad with a wrapped hand, a courier. Look them over, a few looks for the whole queue, and
  send each **in**, to **wash** (a fever, a liar with nothing worse), or to **hold** for the medic.
  Let a bite in and the place has a bad night. Hold a clean one and they remember you.
- **The dead** (the table). Bodies on the slab from where the dead came today. Check the arms,
  the neck, the papers against the tag. **Bury** the clean and named, **burn** the bitten,
  **hold** the one whose name doesn't match: someone must be told. Bury a bitten one and it gets
  up at night.
- **A patrol.** Walk a real leg with two or three of their people: out from the gate, round
  three points, back. What's found on the way comes from the world: a pack's marks, a wreck, a
  door forced since yesterday, a body, smoke. You're paid for coming back with them. A leg at
  rung 2 stays within 300 m; an armed patrol at rung 3 goes further and later.
- **A hunt.** Out with the place's hunter to ground where there's meat: deer where wildlife is on
  (shoot one, carry the legs back), snares and a field's edge where it isn't (work with your
  hands). The meat goes to the kitchen.
- **The bench** (the table). Halcyon's sample trays: the label against the log, the seal, the
  colour. **File**, **re-run** or **destroy**. A turned sample filed is a number wrong in the
  cure's count for a day.

### The table

One small screen for the line, the dead and the bench, in the house style (the car screen's loom,
the power panel): pad first, judged on the server, almost nothing to read.

- Three to five **subjects** in a row. Each has three **signs**, face down, and one **tell**
  that's free (where they came from, what was found with them).
- A few **looks** for the whole table, fewer than there are signs. A look turns one sign over.
- Give each subject one of three **verdicts**. A legend of three lines says which sign means
  which verdict. The cautious verdict is always allowed and pays half.
- The server holds the truth. How many are bad follows the world: the infection of the town they
  came from, the outbreak's stage, a pack near, a bite declared today.
- **Done badly:** the dangerous miss (a bite buried or let in, a turned sample filed) costs the
  place something small and real, and you 2 Infamy when it comes out. **Done well:** full pay, a
  point of Fame, the names in the book, and the giver remembers.

### What varies

Every duty rolls its run from the world: how many subjects and how many are bad, which town the
arrivals or the dead come from, the patrol's points and what stands at them, what the pot lacks.
Complications are one line and one change: it's raining and the queue is short-tempered (fewer
looks), a truck is due (the cook wants it done before), a pack was heard last night (the leg goes
the other way round), the cure is close (Halcyon's trays matter more, and pay a little more).

## Order of work

1. **Ask and you get** (phase 1): contracts on demand at their giver, with an ask gap; the
   errand dealer's duty fallback; the lock reasons in words.
2. **The first duties** (phase 2): the framework (rungs, the duty gap, runs drawn from the world),
   the table and its three skins, the kitchen, the patrol and the hunt with a real squad.
3. **The rest of rungs 1 and 2** (wood and water, washing down, a watch), then stock-taking.
4. **Rung 4**, each with its record and its consequence: the names, who gets a dose, the lock-up,
   a cull. These touch the houses' people and the cure's doses, so each needs its own look at
   what it may write.

## What it must not do

- No new reputation system, no pay outside the ledger's caps, no loot or doses handed out.
- No bodies or props away from players; a squad only while its player is near (the escort block's
  rules and cap).
- Nothing a pad can't do, nothing that needs reading at speed.
- A duty never writes a leader's memory or vote, like every errand.
