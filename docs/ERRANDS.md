# Errands

The small, personal jobs one named person asks of one player, each true because something in the
world is true right now, and over the moment it stops being true. Contracts are the factions' work
on the radio ([MISSIONS.md](MISSIONS.md)); questlines are the long stories
([QUESTLINES.md](QUESTLINES.md)); errands are everything in between. What's still to build
(strangers at their spots, locals, twists, board and radio asks, lies and witnesses, the margin,
lock and epilogue cards, about 44 more cards) is designed in [designs/ERRANDS.md](../designs/ERRANDS.md).

Code: `Missions/Errands/` (`RZ_Errands.c` the dealer and the small-pay ledger, `RZ_Errand.c` the
parts, `RZ_ErrandRoutines.c` the routines, `RZ_ErrandRegistry.c` the list and the cards' base,
`Cards/<Family>/` one class per card), `World/RZ_NamedPeople.c` (the givers), `World/RZ_Faults.c`
(the faults table). Settings: **Missions > Errands** ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)).
Server only, except the journal row.

## Contents

- [The idea](#the-idea)
- [Givers](#givers)
- [Being asked](#being-asked)
- [The dealer](#the-dealer)
- [Keeping them true](#keeping-them-true)
- [Ways and steps](#ways-and-steps)
- [Pay: nothing for free](#pay-nothing-for-free)
- [The faults table](#the-faults-table)
- [The routines](#the-routines)
- [The wanderers' asks](#the-wanderers-asks)
- [The cards](#the-cards)
- [Fallout](#fallout)
- [Telemetry and the admin view](#telemetry-and-the-admin-view)
- [Saving and performance](#saving-and-performance)
- [Simplified, and not built yet](#simplified-and-not-built-yet)

## The idea

| | Contract | Errand |
|---|---|---|
| Offered by | the faction, by radio or quartermaster | one named person, in person (never a leader) |
| Offered to | everyone who qualifies | one player, and their helpers |
| Size | 10–40 min | a few minutes, one step, within 1.5 km |
| Exists because | weights, needs, reasons | one live fact |
| Pays | Fame, Favor, crates | a little Favor; Fame up to a ceiling; an effect in kind |

Two layers:

- **Routines** repeat, on cooldowns: per player per place a real day, per player anywhere 3 real
  hours, per place server-wide 20 minutes. 24 at the four holding kinds, and the wanderers' asks.
- **Cards** are hand-written, **once per player per season** (keyed to the identity, not the
  character), at most **2 a real day**. 16 so far.

Errands are never stages and never on the map. Every one has a way without a gun. They spawn no
AI, no crates and no items. Nobody in the Zone calls them errands: the Haven asks you to "do us a
turn", Bastion has "a detail", the Red Hand "a little something", Halcyon "a small request".

## Givers

A giver is a **named person**: a name, a role and a place. **Leaders never give errands**, and no
errand reads or writes a leader's memory, regard, loyalty, attitude or vote, so no errand can close
a stronghold leader (or a finale route) to anyone.

| Where | Who | Shown |
|---|---|---|
| A faction's outpost or stronghold | territory's three named people ([Territory](DESIGN.md#territory)): the cook, the sentry, the toll man... saved with the holding | the first two, on territory's named occupants, while a player is within 300 m |
| A Haven safehouse | four of its own: a cook, a lookout, a storekeeper and a clerk, made from the season's seed, saved with the errands | the first two, dressed onto the house's first two occupants (they stop being plain occupants) |
| The roads | the wanderers ([Wanderers](DESIGN.md#wanderers)), for their own asks | as built |

A routine names the roles it prefers (the Pot the cook, the Set the radio operator); anyone shown
at the place can speak for them. What a giver remembers of you is dialogue's regard on their own
key (`named:<holding>:<i>`, `house:<safehouse>:<i>`; [DIALOGUE.md](DIALOGUE.md#memory)): a done
errand adds "thanks". A wanderer remembers you as helped (their built memory).

## Being asked

- **"Anything need doing?"** is the second reply on a named person (beside **Ask how things are**),
  in the action menu or on the [talk screen](DIALOGUE.md#the-talk-screen). A wanderer's is **Need a
  hand?**, in their free third slot (not on the Kid, the Mechanic, whose slot is Ask about cars, or
  a bitten wanderer, whose slot is the antidote).
- **Waved over.** A safehouse's or holding's shown named person may call a passing player within
  30 m: *"Mara waves you over."* That's an unasked contact: it asks the notice lanes'
  [interruption budget](DESIGN.md#notice-lanes) (`TryContact`, errands last), and every rule of the
  dealer must hold.
- **The offer.** The giver says what they need, in their own voice, and their replies become the
  errand's ways (three at most) and **Not now**, each with what it needs as a tag:
  *"Hand over two tins [2 food]"*, *"Wire in a car battery [Mechanic] [1 battery]"*,
  *"Soften it on the way [Trusted - Haven]"*, *"Add a threat of your own [Feared | +3 Infamy]"*.
  Tags show costs and requirements, never outcomes. The replies are that player's for 90 seconds,
  or until they pick or walk 20 m off; anyone else at the same person hears *"Give us a minute."*
- **Not now** is always free: nothing is remembered, and the offer stands 10 minutes.
- **The reason in their voice.** Asked when nothing's true, or when it isn't safe, the giver says
  why: *"Not while that lot's at the fence."*, *"You've your hands full already."*, *"Nothing on the
  board for you. Check back after the shift change."*
- **Every line is personal**: the giver's lines are spoken (subtitles, or the talk screen under the
  giver's name), the step and the pay are notices to that player only.

## The dealer

`RZ_Errands` runs on the game mode's tick beside the mission manager, outside the random picks and
the contract cap. Steps run on the 5 s tick; checks, faults and call-outs every 30 s.

**Asked** errands skip the gaps, but not safety: no shot within 60 m in the last minute, no infected
within 30 m, not in a vehicle, not Hunted by the giver's faction, fewer than 2 held.

**Unasked** (a call-out) also needs: alive 3 minutes in this life, not in a bite's Final stage, no
open offer, 8 minutes since the last unasked offer, not within 300 m of a running mission's spot,
600 m of the set piece holding the evening ([the calendar](DESIGN.md#the-set-piece-calendar)), 1 km
of the lab during the final stand, or 2 km of the Long Table's farm; and room in the budget. Three
players at most each 30 s, round-robin.

**Candidates** are the routines and cards anchored to the giver's place (a safehouse, a Bastion
post, a Red Hand camp, a Halcyon station or the lab, any holding, a wanderer and their role),
checked cheapest first: the card done this season or two cards today; the routine's cooldowns; a
running clock for timed ones; for whom (standing, background); **Why** (the live facts, which fill
the places and the text); **Still**; the subject free; a way that fits here. Each scores
weight × need × fit × novelty × story: ×1.2 if it answers the giver faction's need, ×1.2 if the
giver has the role, ×1.2 if they remember you well (regard 2+), ×0.8 if uneasy, ×0.3 if its family
was one of your last three, ×1.5 for a card. The pick is weighted among the top three. At most 12
errands live on the server.

## Keeping them true

- **Why and Still.** An errand is dealt only while its facts hold, and re-checked every 30 s and at
  its step. When they stop holding it's **overtaken**: it ends at the next tick, nobody is paid or
  blamed, and the giver says so (*"Never mind. The truck came in after all."*).
- **Soft holds** (`RZ_WorldFacts.SoftHold`, separate from claims): an errand holds its subject (a
  house, a holding, a wreck) so a second errand can't use it. A soft hold never makes a town busy
  for anyone. A **mission's claim** on the subject overtakes any errand on it at the next tick (a
  stage is happening there); a **questline's or an arc's** long hold overtakes only a card whose
  premise lists the subject, and routines go on.
- **Lapse.** A held errand lapses 45 real minutes after it's taken, or 20 minutes after its player
  went offline; the giver *"stopped waiting"*. No penalty.
- **Death.** Held errands lapse quietly. Nothing is lost but the errand.
- **Before the hour.** A way that must be done before a game hour (Before Dark, by half past eight)
  says so in game time and real minutes, and ends if the hour passes.

## Ways and steps

| Way | What it is |
|---|---|
| Give | Hand items over, on the spot or at the step's place: food, a fuel can, a battery, something medical, an antidote (by the prefab's name; mission items never count) |
| Do | Go to a place and stay there long enough: search a farm, stand the gate, dig, count, fetch water |
| Sneak | The same, only while hidden (built [stealth](DESIGN.md#stealth)): seen, and the clock stops |
| Talk | A reply that resolves on the spot, sometimes with a Trust check ([RZ_Checks](DESIGN.md#checks-and-ways)), fixed, no roll |
| Pay | Favor with the giver's faction, paid when picked |
| Press | A threat, through `RZ_Checks.Threaten` (Fear against 2; +3 Infamy whatever happens) |
| Report, leave | Hand the matter to someone else, or let it go the giver's way |

**Places**, all within 1.5 km of the offer: the giver's place, its gate (15–25 m), 200–400 m out,
300–500 m out, the nearest empty village within 1.2 km, the nearest other town, or the card's own
place (a wreck, an impact, another house, the warning point at the line). A way whose place doesn't
exist here isn't offered.

**Done** is coming back: within 15 m of the giver's place (a wanderer: where they stand now), or
asking them again. **Helpers** are the asker's vanilla group within 50 m for 2 minutes or when the
step was done, three at most.

## Pay: nothing for free

Through the one small-pay ledger, per identity for the season (`RZ_ErrandLedger`):

- **Favor** 3–10 an errand (× **Pay scale**), with diminishing returns per player per real hour
  (×1, ×1, ×0.75, ×0.5, ×0.25) inside **40 an hour**. A quiet way pays ×1.3 if no shot was heard
  near it.
- **Fame** 0–2 an errand, as small pay (`RZ_RepFlag.SMALL`: left out of season Fame, the hall of
  fame's Fame lines and the best deed): never above **39** with that faction (errands can make you
  Known, never Trusted), and at most **10** errand Fame per faction per season day. No rival shares.
- **Infamy decides the kind.** At 15+ Infamy with the paying faction, an errand pays **amends**
  instead of Fame: 2 Infamy off through `Forgive()`, at most **8 a life per faction** from errands,
  never below 15 (inside reputation's 30; [REPUTATION.md](REPUTATION.md#working-off-infamy)). The
  amends family (Make It Right, the Latrine, Clean Slate) pays only that while you're Watched.
  Hunted or worse: nothing.
- **Helpers** get half the Favor and at most 1 Fame. In-kind effects go to the asker only.
- **In kind is an effect, not loot**: +2 food at a house, a fault fixed, morale +1 through the
  contracts and errands lane of the [season budget](DESIGN.md#the-season-budget). Errands never hand
  out doses, crates, kit or keys.
- **"Most people helped"**: each errand done counts on the hall of fame's line of that name, until
  the cure. It pays nothing.

## The faults table

Some routines answer a small real fault at a place (`RZ_Faults`), rolled at most once a shift
(05:30 and 17:30) per place, only while a player is within 2 km, each kind at **Fault chance**
(0.25), and cleared at the next shift or when an errand fixes it. **A fault never drains stock.**

| Fault | Where | What it shows |
|---|---|---|
| Relief missing | Bastion posts | one guard fewer at the gate; a line on Bastion's secure channel |
| Set low | Bastion posts | a line on Bastion's secure channel (*"you're all hiss"*) |
| Fuel low | Halcyon stations; safehouses | Halcyon's secure channel; the house's news (*"The generator's on its last cupful"*) |
| Sensor dark | Halcyon stations | a line on Halcyon's secure channel |
| Water low | safehouses | the house's news |

## The routines

✅ all built. Pay is before the diminishing returns; "+2 food" is at the house.

| Routine | Where, giver | True when | Ways and pay |
|---|---|---|---|
| **The Pot** | Haven safehouse, the cook | food under one upkeep, and no Haven truck on its way to it | Hand over two tins [2 food]: 4 Favor, 1 Fame, +2 food. Search the gardens at the nearest empty village (30 s): 5 Favor, +2 food. Writes `errand.fed` |
| **Lights** | safehouse, the storekeeper | the house's fuel fault, dusk within 20 real min (or dusk) | A fuel can, or [Mechanic] a car battery: 5 Favor, the fault fixed. Writes `errand.lit` |
| **Water for {house}** | safehouse, the lookout | the house's water fault | Fetch water from the next town (20 s): 5 Favor (×1.3 quiet), +2 food, fixed |
| **Filter Parts** | safehouse, the storekeeper | the water fault, 8+ people, Known | The pump house part at the next town (25 s; [Mechanic or Lab Tech] 12 s): 8 Favor, fixed |
| **The Gate at Dusk** | safehouse, the lookout | dusk near, one guard or none, a town at 60+ infection within 1.5 km, not Night of the Dead | Stand at the gate 10 real min: 5 Favor |
| **The Burying** | safehouse, the clerk | someone died at the house lately | Dig at the edge of town (30 s): 3 Favor; a world record |
| **The Round** | safehouse, the clerk | [Nurse or Doctor], no medic, 4+ people | A minute at each bed (2 min): 4 Favor |
| **The Newcomer** | safehouse, the clerk | the house took someone in this season day | A coat from the next town (20 s), or [Nurse or Doctor] check him over (30 s): 4 Favor |
| **The Children's Hour** | safehouse | 5+ people, 08:00–17:00 | Watch the yard 5 real min: 4 Favor |
| **Make It Right** (amends) | safehouse, the lookout | the Haven Watches you | Keep the gate (3 min) or carry the night water (15 s at the next town): amends |
| **Shift Change** | Bastion post | the relief fault | Wake the relief 200–400 m out (10 s): 4 Favor, fixed. Report him: 1 Fame. Stand the post (3 min): 6 Favor, fixed |
| **The Set** | Bastion post, the radio operator | the set fault, 17:30–20:30 | A battery: 5 Favor, fixed |
| **The Wire** | Bastion post | a Bastion truck lost within 1.5 km in a season day, or a town at 50+ within 1.5 km; not Watched | Man the wire 5 min: 6 Favor |
| **The Count** | Bastion post | a Bastion truck lost near, or the post short; Known | Count three stacks (30 s), then write what's there (6 Favor) or what they want (10 Favor; a world record) |
| **The Bag** | Bastion post | night, Bastion morale under 40 | Put his letter in the mail bag: no pay; Bastion morale +1 |
| **Fuel for the Post** | Bastion post | the post short after a lost truck | A fuel can: 6 Favor |
| **The Latrine** (amends) | Bastion post | Watched or Stranger with Bastion | Dig it (30 s): Watched, amends; Stranger, 3 Favor |
| **Fuel for the Lorry** | Red Hand camp, the toll man | the Red Hand needs supplies | A fuel can: 6 Favor. Or say there's no fuel left |
| **Medicine** | Red Hand camp (not the stronghold), the cook | always | Two tins for the still: 6 Favor |
| **Short Change** | Red Hand camp, the lookout | Red Hand morale under 55; Known | Watch the collectors (a minute) and name him, or [Police or Ex-Con] pick him out (20 s): 8 Favor. Or watch and warn him: nothing |
| **The Pit Count** | Red Hand camp | a Fight Pit running | Count the bets into the book (2 min): 5 Favor |
| **The Freezers** | Halcyon station or the lab | the fuel fault, evening or night | A fuel can or a battery: 6 Favor, 1 Fame, fixed |
| **Night Readings** | Halcyon station or the lab | the sensor fault | A battery at the sensor 300–500 m out, or reset it unseen (15 s hidden): 5 Favor, fixed |
| **Labels** | Halcyon station or the lab | night | Relabel the trays (2 min; [Lab Tech] 1 min): 4 Favor |

Four missions left the random pool for these and are gone from the code: Fuel for the Lights
(Lights), Water Purifier (Filter Parts), Water Run (Water for {house}) and Medicine Before Dark
(Before Dark).

## The wanderers' asks

| Ask | Wanderer | True when | Ways and pay |
|---|---|---|---|
| **Before Dark** ✅ | the Doctor | 15:00–20:00, a town at 30+ infection within 1.5 km | Take something medical to the bitten survivor there before 20:30, or [Nurse or Doctor] dress the bite (45 s): 6 Haven Favor |
| **Wanted Poster** ✅ | any wanderer | you're Kill on Sight with Bastion or the Red Hand | Pay him (8 Favor), lean on him (a threat), or let him sell where you are (a hunter squad comes now, if one hunts you) |
| **Barter Chain** ✅ | three wanderers | as built ([MISSIONS.md](MISSIONS.md#unarmed-missions)) | Started from one of them when nothing else is asked; offered to that player alone, paid 2 Fame and 8 Favor; it left the random pool |
| **Ask About Cars** ✅ | the Mechanic | as built | His own slot, Ask about cars, and the Car Keys job |
| **Whoever Sits There Now** ✅ | the Courier | as built | His own slot, Ask for work: the letters |

## The cards

✅ all built. Once per player per season (taken), at most 2 offered a real day.

| Card | Family, giver | True when | Ways |
|---|---|---|---|
| **Sleeve Down** | house, a Haven named person | a house of 5+, a bite on the roads within 1.5 km in 3 played hours | Something for the fever [1 medical]; [Nurse or Doctor] ask to see their arm; tell the clerk. The truth deck draws flu or a bite (6 in 10) for that house, kept 3 season days; the same replies show either way. A bite kept quiet: the house loses someone in the night. Seen: declared, 2 Haven Fame. Told: walked out at dusk (a bite), or shamed (flu: their regard −1) |
| **A Word to {house}** | house, the clerk | this house short, another lived-in house within 1.5 km fed | Carry the word to the other house: as it was said [Known], softened [Trusted - Haven], or with your own threat [Feared]: 5 Favor; which you chose is a world record |
| **I'd Rather Be Wrong** | house | a house with a medic, the lab, before the cure; you're a Nurse, Doctor or Lab Tech | Go through the numbers [Doctor or Nurse], read them the labels [Lab Tech]: 3 Favor. Or agree |
| **A Word for Me** | house | Champion with the Haven; the house's head alive | Speak to the head (10 s beside them): a world record, no vote moved. Or say you're no one to ask |
| **The Truck Didn't Come** | aftermath, the storekeeper | this house's truck lost within 1.5 km in 2 played hours (never to whoever robbed it, or their group) | Salvage the wreck (30 s): 4 Favor, +2 food. [Known: Red Hand] ask a toll lad who: 3 Favor, a rumour naming no one |
| **The Smoke Was Ours** | aftermath, a Bastion named person | shells within 1.5 km in 2 played hours (never to whoever called them) | Count at the impact (10 s): 5 Favor. Two rations to the nearest house: 1 Bastion Fame, 1 Haven Fame, +2 food there. Tell the Haven whose shells: a world record |
| **Is It True?** | aftermath, a Haven named person | a rumour going round names a town within 1.5 km | Go and look (15 s): 4 Favor. Tell them it's true: the rumour runs 6 hours longer, no pay |
| **The Letter Nobody Can Take** | lost, the Courier | a house that lost people lately, another lived-in house within 1.5 km | Carry it there: 4 Haven Favor. Or give it back |
| **The Trader's Detour** | lost, a Trader | his destination's town at 60+ infection, or a mission there | Scout his road ahead (20 s): 4 Haven Favor; he remembers you helped |
| **The Deserter's Way Round** | line, the Deserter | he likes you, a Bastion road post within 1.5 km | Meet him past the post (10 s): 4 Haven Favor; helped. Turn him in at the post: 2 Bastion Fame, a rumour, a world record |
| **Nothing Goes Out** | line, a Bastion named person | the perimeter sealed, a sealed checkpoint within 1.65 km, Bastion morale under 50 | Leave his letter under a stone at the warning point, 150 m short (5 s), read or not: 3 Favor; the world record `errand.letter_at_line` with its epilogue line (carrier: a prop) |
| **His Effects** | dead, any faction's named person (Bastion: The Locker; others: The Wreck's Name) | that faction's truck lost within 1.5 km in a season day (never to whoever robbed it) | Bring his things back (15 s at the wreck): 5 Favor, 1 Fame; Bastion's boxes "for when the gates open" as an epilogue line. [Known: Red Hand] sell them to the fence: 5 Red Hand Favor, "kit lost" |
| **The Name on the Tag** | dead, the clerk | a death within 1.5 km in a season day (the background and how long they lasted, never the name), or the fallout of a Distress Call or Survivors Trapped whose people died | Leave the satchel on a cairn where they fell (10 s): 4 Favor. Or pin the line to the board. A world record with an epilogue line (carrier: the board) |
| **Not for the Crew** | crews, a Red Hand named person | a bite within 1 km in the last played hour, a Halcyon station within 1.5 km, before the cure | Walk him to the station: 4 Halcyon Favor, a world record. Your own antidote: 6 Red Hand Favor. Tell the book-keeper: 2 Red Hand Fame |
| **Wildcard Money** | crews, a Red Hand named person | Wildcard or Tolerated with the Red Hand; another Red Hand holding within 1.5 km | Carry the cut there: 6 Favor; skim a little: 8 Favor; give it to a hungry house within 1.5 km: +2 food there, 2 Haven Fame, +8 Red Hand Infamy |
| **Clean Slate** (amends) | any faction's named person | that faction Watches you | The worst shift going (a minute): amends |
| **Buy It Back** (fallout) | aftermath, a Bastion named person; Known with Bastion | Crash Site's fallout: the box not handed in, a post within 3 km of the wreck | A fence's runner sits on the box at the wreck: five tins there, or 25 Red Hand Favor (only offered if you have it), then hand it in: 6 Favor (at most the fallout's cap); the world record `crash.box` = bought back |

## Fallout

`RZ_Errands.Fallout`, called by a failed story job ([DESIGN.md](DESIGN.md#story-blocks)). One errand of
the job's kind waits at its subject for 3 hours: for the first **20 minutes** only the job's
participants can be dealt it, then anyone. A giver within 3 km of the subject deals it first (×3 its
score), and it pays at most **30%** of the failed job's base, whatever the way says. Done, it's gone;
a second fallout of the same kind at the same place replaces the first. Not saved.

| Failed job | Fallout errand |
|---|---|
| Protection Racket (time up) | The Pot, at the house |
| Crash Site (the box not handed in) | Buy It Back |
| Distress Call (the people died), Survivors Trapped (all dead) | The Name on the Tag, their names to the nearest house |

## Telemetry and the admin view

- **Log lines**: every deal (`Errands: dealt <key> to <player> by <giver> (asked|called out)`), take,
  done, overtake and fault; every fallout (`Errands: fallout <card> at <grid> from <title> (...)`); and every 30 minutes (**Telemetry**) a status line per family: dealt,
  declined, lapsed, overtaken, done, Favor, Fame and amends paid, and the ways taken.
- **The admin view needs no typing**: on the talk screen a server admin sees one more reply on any
  named person, wanderer or occupant, **[Admin] Errands near here**: the live count, the nearest
  player's last deal and why each of the others wasn't dealt (*"lights: not true. pot: cooling
  down."*), and their errands. It's also written to the log.

## Saving and performance

- **Saved** (state.json, wiped with the season): each identity's ledger (cards done, cooldowns, the
  hour's Favor, cards today, errand Fame today, amends this life, the last three families, errands
  done), the safehouses' named people, and each house's count of errands done there. Held errands,
  offers, faults and soft holds are runtime only: a restart lets them lapse.
- **Cheap**: no map scans. A deal checks one giver's anchored definitions; steps are distance
  checks on the 5 s tick; faults, guards and call-outs run every 30 s.

## Simplified, and not built yet

*Simplified:* a step's place is a point, with no scene person or prop at it yet (the scene-people
spawner comes with strangers); the lines have two or three variants each, not always three; a card
dealt counts toward the day's two even if declined. Not built yet, and designed in
[designs/ERRANDS.md](../designs/ERRANDS.md): strangers at their spots and the footfall grid, locals,
twists beyond Sleeve Down's truth, board and radio asks, found hooks, the other fallout cards (His
Notebook), errands' own lies (the store and the witness rule are built, [DESIGN.md](DESIGN.md#lies-and-witnesses)), promises, the rest of the cards (backgrounds, bad standing, traces, the margin,
lock and epilogue cards), callbacks and threads from errands, and the `mission_errand` icon (the
journal uses the unarmed icon).
